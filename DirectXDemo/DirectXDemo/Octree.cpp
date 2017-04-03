#include "Octree.h"
#include "objLoader.h"
#include <memory>
#include <vector>
#include <algorithm>
#include <stack>
#include "Shorthand.h"

Octree::Node* Octree::build(const ObjModel& _model, DirectX::XMVECTOR _position, const DirectX::XMFLOAT3& _minSize, float _minGrassLength, float _density)
{
	using namespace DirectX;

	/*Calculate the size of the bounding box of the model*/
	float* vtxElePtr	= _model.getVertices();
	//Set the first vertex as the largest and smallest to begin with
	XMFLOAT3 current	= { *vtxElePtr, *(vtxElePtr + 1), *(vtxElePtr + 2) };
	XMVECTOR smallest	= LF3(&current); 
	XMVECTOR largest	= LF3(&current);


	for (int i = 0; i < _model.getTotalVerts(); ++i)
	{
		//Get next three floats, store in XMFLOAT3 current
		memcpy(&current, vtxElePtr, sizeof(float)*3);
		vtxElePtr += 3;
		//Component-wise compare and store
		largest = XMVectorMax(LF3(&current), largest);
		smallest = XMVectorMin(LF3(&current), smallest);
	}

	//Increase height of bounding box to encompass all the grass @Possibly just adjust the extents so the AABB is a little larger than the model all round? Seems ok for now :)
	largest += VEC3(0.1f, _minGrassLength, 0.1f);
	smallest -= VEC3(0.1f, 0.1f, 0.1f);

	/*Root starts as a leaf with no children. Translate AABB points.*/
	Node* root = new Node(smallest + _position, largest + _position, nullptr);
	
	/*Keep a record of leaf nodes to quickly iterate over them and reserve mem for grass*/
	std::vector<Node*> leaves;
	leaves.push_back(root);

	/*Build child nodes*/
	std::stack<Node*> stack;
	stack.push(root);

	XMVECTOR minSize = LF3(&_minSize); 
	//XMVECTOR halfMinSize = minSize / 2;
	XMVECTOR childSize;
	while (!stack.empty())
	{
		//Pop top node
		Node* current = stack.top();
		stack.pop();

		childSize = LF3(&current->m_AABB.Extents); //Because extents == half width/height/depth
		XMFLOAT3 numChildren = {1,1,1};
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
			//Remove this guy from the leaf stacks
			std::remove(leaves.begin(), leaves.end(), current);

			//HOURS OF DEBUGGING TO ARRIVE AT THIS
			for (int i = 0; i < numChildren.x; ++i)
			{
				float x0 = current->m_AABB.Center.x - ((current->m_AABB.Extents.x) * (1- i));
				float x1 = current->m_AABB.Center.x + ((current->m_AABB.Extents.x * (2 - numChildren.x))) + (current->m_AABB.Extents.x * i);
				for (int j = 0; j < numChildren.y; ++j)
				{
					float y0 = current->m_AABB.Center.y - ((current->m_AABB.Extents.y) * (1 - j));
					float y1 = current->m_AABB.Center.y + ((current->m_AABB.Extents.y * (2 - numChildren.y))) + (current->m_AABB.Extents.y * j);
					for (int k = 0; k < numChildren.z; ++k)
					{
						float z0 = current->m_AABB.Center.z - ((current->m_AABB.Extents.z) * (1 - k));
						float z1 = current->m_AABB.Center.z + ((current->m_AABB.Extents.z * (2 - numChildren.z))) + (current->m_AABB.Extents.z * k);
				
						Node* child = new Node(VEC3(x0, y0, z0), VEC3(x1, y1, z1), current);
						current->m_children.push_back(child);
						stack.push(child);
						leaves.push_back(child);
					}
				}
			}
		}

		///*Test if the node can contain children*//*OLD IMPLEMENTATION, INFERIOR*/
		//BoundingBox testAABB;
		//XMVECTOR childSize = LF3(&current->m_AABB.Extents) * 0.5f;
		////Move to origin
		//testAABB.Center = { 0, 0, 0 };
		////Resize
		//XMStoreFloat3(&testAABB.Extents, childSize);
		////If node is large enough to contain all 8 children
		//if (testAABB.Contains(minSize))
		//{
		//	//Center point of current node
		//	XMVECTOR center = LF3(&current->m_AABB.Center);
		//	//Octree segments
		//	XMVECTOR oct[8] =
		//	{
		//		{ 1 ,  1 ,  1 },
		//		{ -1 ,  1 ,  1 },
		//		{ -1 ,  1 , -1 },
		//		{ 1 ,  1 , -1 },
		//		{ 1 , -1 ,  1 },
		//		{ -1 , -1 ,  1 },
		//		{ -1 , -1 , -1 },
		//		{ 1 ,  -1 , -1 }
		//	};

		//	/*Create new children and push onto stack*/
		//	for (int i = 0; i < 8; ++i)
		//	{
		//		XMVECTOR outer = center + oct[i] * LF3(&current->m_AABB.Extents);
		//		Node* child = new Node(center, outer, current);
		//		current->m_children.push_back(child);
		//		stack.push(child);
		//	}	
		//}
	}

	/*Reserving roughly the right ammount of memory will help speed up generation*/
	for(auto leaf: leaves)
	{
		/*Just take surface area of base * density because usually only one plane is covered in any node*/
		/*mul by 2 because extents are 1/2*/
		leaf->m_instances.reserve(_density * 2.0 * leaf->m_AABB.Extents.x * leaf->m_AABB.Extents.z);
	}

	return root;
}

void Octree::cleanup(Node* _root)
{
	/*Early out if _root ptr is null*/
	if (_root == nullptr) return;

	std::stack<Node*> tree;
	tree.push(_root);

	/*Depth first traversal of nodes*/
	while (!tree.empty())
	{
		//Pop top node
		Node* current = tree.top();
		tree.pop();

		//Store child nodes
		for (Node* child : current->m_children)
		{
			tree.push(child);
		}

		//Delete the current node
		delete current;
	}
}

void Octree::prune(Node* _root)
{
	/*Early out if _root ptr is null*/
	if (_root == nullptr) return;

	std::stack<Node*> tree;
	bool complete = false;

	/*Iterate until comepletly pruned*/
	while (!complete)
	{
		tree.push(_root);
		complete = true;

		/*Depth first traversal of nodes*/
		while (!tree.empty())
		{
			//Pop top node
			Node* current = tree.top();
			tree.pop();

			//If node is a leaf
			if (current->m_children.empty())
			{
				//If there are no instances of grass
				//&& it has a parent
				if (current->m_instances.empty() && current->m_parent)
				{
					//Iterate at least once more
					complete = false;

					Node* parent = current->m_parent;
					//Remove node from parent, delete it
					parent->m_children.erase(std::remove_if(
											parent->m_children.begin(),
											parent->m_children.end(),
											[current](Node* node)
											{
												if (node == current)
												{
													delete current;
													return true;
												}
												return false;
											}));
				}
			}
			else //If node is not leaf
			{
				//Push children
				for (Node* child : current->m_children)
				{
					tree.push(child);
				}
			}
		}
	}
	return;
}

Octree::Node* Octree::addGrass(Node* _root, const field::Instance& _instance)
{
	/*Early out if _root ptr is null*/
	if (_root == nullptr) return nullptr;

	std::stack<Node*> tree;
	tree.push(_root);

	/*Depth first traversal of nodes*/
	while (tree.size())
	{
		//Pop top node
		Node* current = tree.top();
		tree.pop();

		//If this node contains the grass
		if (current->m_AABB.Contains(LF3(&_instance.location)))
		{
			//If this is a leaf node
			if (current->m_children.empty())
			{
				//Add the grass instance
				current->m_instances.push_back(_instance);
				return current;
			}
			//Else if not leaf node
			//Push child nodes
			for (Node* child : current->m_children)
			{
				tree.push(child);
			}
		}
	}

	return nullptr;
}

void Octree::frustumCull(Node* _root, const DirectX::BoundingFrustum& _frustum, field::Instance* _buffer, int _bufferSize, unsigned int& _numInstances)
{
	/*Early out if _root ptr is null*/
	if (_root == nullptr) return;

	std::stack<Node*> tree;
	tree.push(_root);

	_numInstances = 0;

	/*Depth first traversal of nodes*/
	while (tree.size())
	{
		//Pop top node
		Node* current = tree.top();
		tree.pop();

		//If this node is within the frustum //@Changed from contains to intersects
		if (_frustum.Intersects(current->m_AABB))
		{
			//If this is a leaf node
			if (current->m_children.empty())
			{
				//If the buffer is full, exit function//@not the most elegant solution
				//int distanceToSize = _bufferSize - _numInstances - current->m_instances.size();
				//if (distanceToSize < 0) return;
				
				//Add the grass instances to the buffer... @too slow!
				memcpy(_buffer + _numInstances, current->m_instances.data(), current->m_instances.size() * sizeof(field::Instance));
				_numInstances += current->m_instances.size();
			}
			//Else if not leaf node
			//Push child nodes
			for (Node* child : current->m_children)
			{
				tree.push(child);
			}
		}
	}
}

void Octree::noCull(Node * _root, field::Instance * _buffer, int _bufferSize, unsigned int& _numInstances)
{/*Early out if _root ptr is null*/
	if (_root == nullptr) return;

	std::stack<Node*> tree;
	tree.push(_root);

	_numInstances = 0;

	/*Depth first traversal of nodes*/
	while (tree.size())
	{
		//Pop top node
		Node* current = tree.top();
		tree.pop();


		//If this is a leaf node
		if (current->m_children.empty())
		{
			//If the buffer is full, exit function//@not the most elegant solution
			//int distanceToSize = _bufferSize - _numInstances - current->m_instances.size();
			//if (distanceToSize < 0) return;

			//Add the grass instances to the buffer
			memcpy(_buffer + _numInstances, current->m_instances.data(), current->m_instances.size() * sizeof(field::Instance));
			_numInstances += current->m_instances.size();
		}
		//Else if not leaf node
		//Push child nodes
		for (Node* child : current->m_children)
		{
			tree.push(child);
		}
	}
	
}
