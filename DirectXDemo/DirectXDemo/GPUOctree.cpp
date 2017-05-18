#include "GPUOctree.h"
#include "objLoader.h"
#include "Shorthand.h"
#include <stack>
#include <map>
#include <algorithm>
void GPUOctree::build(const ObjModel & _model, DirectX::XMVECTOR _position, const DirectX::XMFLOAT3 & _minSize, float _minGrassLength, std::vector<field::Instance>& _instances, ID3D11Device* _device)
{
	using namespace DirectX;

	/*Calculate the size of the bounding box of the model*/
	float* vtxElePtr = _model.getVertices();
	//Set the first vertex as the largest and smallest to begin with
	XMFLOAT3 current = { *vtxElePtr, *(vtxElePtr + 1), *(vtxElePtr + 2) };
	XMVECTOR smallest = LF3(&current);
	XMVECTOR largest = LF3(&current);


	for (int i = 0; i < _model.getTotalVerts(); ++i)
	{
		//Get next three floats, store in XMFLOAT3 current
		memcpy(&current, vtxElePtr, sizeof(float) * 3);
		vtxElePtr += 3;
		//Component-wise compare and store
		largest = XMVectorMax(LF3(&current), largest);
		smallest = XMVectorMin(LF3(&current), smallest);
	}

	//Increase height of bounding box to encompass all the grass
	largest += VEC3(0.1f, _minGrassLength, 0.1f);
	smallest -= VEC3(0.1f, 0.1f, 0.1f);

	/*Root starts as a leaf with no children. Translate AABB points.*/
	Node root = Node(smallest + _position, largest + _position, -1, 0);

	std::vector<Node> leaves;
	leaves.push_back(root);

	/*Build child nodes*/
	std::stack<Node> stack;
	stack.push(root);

	XMVECTOR minSize = LF3(&_minSize);
	XMVECTOR childSize;
	while (!stack.empty())
	{
		//Pop top node
		const Node current = stack.top();
		stack.pop();

		//push onto nodes vector
		m_nodes.push_back(current);

		//get index
		const int idx = m_nodes.size() -1;
		//set index
		m_nodes[idx].m_idx = idx;
	
		
		childSize = LF3(&current.m_AABB.Extents); //Because extents == half width/height/depth

		XMFLOAT3 numChildren = { 1,1,1 };
		XMVECTOR childSubtractMin = childSize - minSize;
		//If node can contain children X
		if (childSubtractMin.m128_f32[0] >= 0)
		{
			numChildren.x = 2;
		}
		//If node can contain children Y
		if (childSubtractMin.m128_f32[1] >= 0)
		{
			numChildren.y = 2;
		}
		//If node can contain children Z
		if (childSubtractMin.m128_f32[2] >= 0)
		{
			numChildren.z = 2;
		}

		//If at least one of the directions can spawn a new node....
		XMVECTOR avgChildren = XMVector3Dot(VEC3(1, 1, 1), LF3(&numChildren));
		if (avgChildren.m128_f32[0] > 3)
		{
			//remove this guy from leaves stack
			std::remove(leaves.begin(), leaves.end(), current);

			int childIdx = 0;
			for (int i = 0; i < numChildren.x; ++i)
			{
				float x0 = current.m_AABB.Center.x - ((current.m_AABB.Extents.x) * (1 - i));
				float x1 = current.m_AABB.Center.x + ((current.m_AABB.Extents.x * (2 - numChildren.x))) + (current.m_AABB.Extents.x * i);
				for (int j = 0; j < numChildren.y; ++j)
				{
					float y0 = current.m_AABB.Center.y - ((current.m_AABB.Extents.y) * (1 - j));
					float y1 = current.m_AABB.Center.y + ((current.m_AABB.Extents.y * (2 - numChildren.y))) + (current.m_AABB.Extents.y * j);
					for (int k = 0; k < numChildren.z; ++k)
					{
						float z0 = current.m_AABB.Center.z - ((current.m_AABB.Extents.z) * (1 - k));
						float z1 = current.m_AABB.Center.z + ((current.m_AABB.Extents.z * (2 - numChildren.z))) + (current.m_AABB.Extents.z * k);

						//next index
						int nodeIdx = m_nodes.size();
						//create child
						Node child = Node(VEC3(x0, y0, z0), VEC3(x1, y1, z1), current.m_idx, nodeIdx);
						//set parent ref to child
						m_nodes[current.m_idx].m_childIdx[childIdx] = nodeIdx;
						//push child onto nodes vector
						m_nodes.push_back(child);
						//push onto itteration stack
						stack.push(child);
						//save as a leaf
						leaves.push_back(child);
						++childIdx;
					}
				}
			}
		}
	}

	if (m_nodes.empty()) return;

	//add grass
	std::vector<int> containsGrass;
	addGrass(_instances, containsGrass);

	//prune tree
	prune(_instances, containsGrass);

	//create gpu nodes
	for (auto& node : m_nodes)
	{
		m_gpuNodes.push_back(node);
	}

	//init gpu resources
	initResources(_device);
}

void GPUOctree::cleanup()
{
	m_gpuNodes.clear();
	m_nodes.clear();
	m_gpuBuffer.cleanup();
}

void GPUOctree::frustumCull(const DirectX::BoundingFrustum& _frustum, bool _noCull)
{
	/*Early out if _root ptr is null*/
	if (m_nodes.empty()) return;

	std::stack<int> tree;
	tree.push(0);


	for (auto& node : m_gpuNodes) node.m_visible = 0;

	/*Depth first traversal of nodes*/
	while (tree.size())
	{
		//Pop top node
		const int currentIdx = tree.top();
		tree.pop();

		//If this node is within the frustum 
		if (_noCull || _frustum.Intersects(m_nodes[currentIdx].m_AABB))
		{
			m_gpuNodes[currentIdx].m_visible = 1;
			//If not leaf node
			//Push child nodes
			if (m_nodes[currentIdx].hasChildren())
			{
				for (int childIdx : m_nodes[currentIdx].m_childIdx)
				{
					if (Node::idxNotNull(childIdx)) tree.push(childIdx);
				}
			}
		}
	}
}

void GPUOctree::updateResources(const DrawData& _data)
{
	//update visibility
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	HRESULT result = _data.m_dc->Map(m_gpuBuffer.getBuffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, m_gpuNodes.data(), m_gpuNodes.size() * sizeof(GPUNode));
	_data.m_dc->Unmap(m_gpuBuffer.getBuffer(), 0);
	assert(!FAILED(result));
}

static void fixIdx(int& _cur, std::map<int, int>& _map)
{
	if (_cur < 0) return;
	_cur = _map[_cur];
}

void GPUOctree::prune(std::vector<field::Instance>& _field, std::vector<int>& _containsGrassIdx)
{
	/*Early out if _root ptr is null*/
	if (m_nodes.empty()) return;

	std::stack<int> tree;
	bool complete = false;

	/*Iterate until comepletly pruned*/
	while (!complete)
	{
		tree.push(0);
		complete = true;

		/*Depth first traversal of nodes*/
		while (!tree.empty())
		{
			//Pop top node
			int currentIdx = tree.top();
			tree.pop();

			//If node is a leaf
			if (!m_nodes[currentIdx].hasChildren())
			{
				//If there are no instances of grass
				//&& it has a parent
				if (Node::idxNotNull(m_nodes[currentIdx].m_parentIdx) 
					&& std::find_if(_containsGrassIdx.begin(), _containsGrassIdx.end(), [=](const int& idx) { return idx == currentIdx; }) == _containsGrassIdx.end())
				{
					//Iterate at least once more
					complete = false;
					int parentIdx = m_nodes[currentIdx].m_parentIdx;
					//Remove node from parent, delete it
					for (int i = 0; i < 8; ++i)
					{
						if (m_nodes[parentIdx].m_childIdx[i] == currentIdx)
						{
							m_nodes[parentIdx].m_childIdx[i] = -1;
							break;
						}
					}
				}
			}
			else //If node is not leaf
			{
				//Push children
				for (int childIdx : m_nodes[currentIdx].m_childIdx)
				{
					if (Node::idxNotNull(childIdx)) tree.push(childIdx);
				}
			}
		}
	}

	//Now just iterate over the remaining tree, add to new structure
	std::vector<Node> prunedOctree;
	//store the changes so the grass and internal idx can be updated
	std::map<int, int> indexMap;
	//Tree will alywas be empty, since it's a condition for leaving the previous nested loop
	tree.push(0);

	/*Depth first traversal of nodes*/
	while (!tree.empty())
	{
		//Pop top node
		int currentIdx = tree.top();
		tree.pop();

		//Add to fresh container
		int prunedIdx = prunedOctree.size();
		prunedOctree.push_back(m_nodes[currentIdx]);

		//map the index change
		std::pair<int,int> pair(currentIdx, prunedIdx);
		indexMap.insert(pair);


		//If node is not a leaf
		if (m_nodes[currentIdx].hasChildren())
		{
			//Push children
			const int numChildren = ARRAYSIZE(prunedOctree[prunedIdx].m_childIdx);
			for (int i=0; i< numChildren; ++i)
			{
				int originalChildIdx = prunedOctree[prunedIdx].m_childIdx[i];
				//if there is a child
				if (Node::idxNotNull(originalChildIdx))
				{
					//add to tree traversal stack
					tree.push(originalChildIdx);
				}
			}
		}
	}

	m_nodes.clear();
	//m_nodes = prunedOctree;
	for (int i =0; i < prunedOctree.size(); ++i)
	{
		for (int j = 0; j < ARRAYSIZE(prunedOctree[i].m_childIdx); ++j) fixIdx(prunedOctree[i].m_childIdx[j], indexMap);
		fixIdx(prunedOctree[i].m_idx, indexMap);
		fixIdx(prunedOctree[i].m_parentIdx, indexMap);
		m_nodes.push_back(prunedOctree[i]);
	}
	m_nodes.shrink_to_fit();

	//finally, fix up the idx value for the grass instances
	for (auto& instance : _field)
	{
		fixIdx(instance.octreeIdx, indexMap);
	}

	return;
}

void GPUOctree::initResources(ID3D11Device* _device)
{
	D3D11_SUBRESOURCE_DATA subresourceData;
	ZeroMemory(&subresourceData, sizeof(subresourceData));
	subresourceData.pSysMem			= m_gpuNodes.data();

	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.BindFlags			= D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	bufferDesc.Usage				= D3D11_USAGE_DYNAMIC;
	bufferDesc.MiscFlags			= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.ByteWidth			= m_gpuNodes.size() * sizeof(GPUNode);
	bufferDesc.StructureByteStride	= sizeof(GPUNode);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format					= DXGI_FORMAT_UNKNOWN;
	srvDesc.BufferEx.NumElements	= m_gpuNodes.size();
	srvDesc.BufferEx.Flags			= 0;
	srvDesc.ViewDimension			= D3D11_SRV_DIMENSION_BUFFEREX;
	
	m_gpuBuffer.init(_device,&bufferDesc, &subresourceData, nullptr, &srvDesc);
}

void GPUOctree::addGrass(std::vector<field::Instance>& _field, std::vector<int>& _containsGrass)
{
	_containsGrass.clear();
	std::stack<int> tree;

	tree.push(0);
	for (auto& instance : _field)
	{
		tree.push(0);
		/*Depth first traversal of nodes*/
		while (tree.size())
		{
			//Pop top node
			int currentIdx = tree.top();
			tree.pop();

			//If this node contains the grass
			if (m_nodes[currentIdx].m_AABB.Contains(LF3(&instance.location)))
			{
				//If this is a leaf node
				if (!m_nodes[currentIdx].hasChildren())
				{
					instance.octreeIdx = currentIdx;
					_containsGrass.push_back(currentIdx);
					break;
				}
				//Else if not leaf node
				//Push child nodes
				for (int childIdx : m_nodes[currentIdx].m_childIdx)
				{
					if (Node::idxNotNull(childIdx)) tree.push(childIdx);
				}
			}
		}
	}
}
