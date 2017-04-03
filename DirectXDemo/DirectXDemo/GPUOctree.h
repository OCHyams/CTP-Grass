#pragma once
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include "FieldRenderData.h"
#include <vector>
#include "Shorthand.h"
#include "Buffer.h"
#include "DrawData.h"
class ObjModel;
class GPUOctree
{
public:
	struct Node
	{
		DirectX::BoundingBox	m_AABB;
		int m_childIdx[8];
		int m_idx		= -1;
		int m_parentIdx = -1;

		Node() = delete;
		Node(DirectX::XMVECTOR _point0,
			DirectX::XMVECTOR _point1,
			int _parentIdx,
			int _idx)
			: m_parentIdx(_parentIdx), m_idx(_idx)
		{
			DirectX::BoundingBox::CreateFromPoints(m_AABB, _point0, _point1);
			memset(m_childIdx, -1, 8 * sizeof(int));
		}

		bool operator==(const Node& _rhs) 
		{
			return m_idx == _rhs.m_idx;
		}
		bool hasChildren() const
		{
			for (int i = 0; i < 8; ++i) if (hasChild(i)) return true;
			return false;
		}
		bool hasChild(int _childIdx) const
		{
			return idxNotNull(m_childIdx[_childIdx]);
		}
		static bool idxNotNull(int _idx) { return _idx >= 0; }
	};

	__declspec(align(16))
	struct GPUNode
	{
		GPUNode() = delete;
		GPUNode(const Node& _node)
		{
			memcpy(m_childIdx, _node.m_childIdx, 8 * sizeof(int));
			m_idx = _node.m_idx;
			m_parentIdx = _node.m_parentIdx;
		}
		int m_childIdx[8];
		int m_idx		=-1;
		int m_parentIdx =-1;
		int m_visible	= 0; //gpu doesn't like bools

	};

	/////////////////////////////////////////////////
	///	Sets the Octree node idx for the grass instances.
	/////////////////////////////////////////////////
	void build(	const ObjModel&				_model,
				DirectX::XMVECTOR			_position,
				const DirectX::XMFLOAT3&	_minSize,
				float						_minGrassLength,
				std::vector<field::Instance>& _instances,
				ID3D11Device* _device);

	/////////////////////////////////////////////////
	///	Removes all nodes.
	/////////////////////////////////////////////////
	void cleanup();

	/////////////////////////////////////////////////
	///	Marks GPUNodes as visible if in frusutm.
	/////////////////////////////////////////////////
	void frustumCull(const DirectX::BoundingFrustum& _frustum);

	/////////////////////////////////////////////////
	///	Update GPU resources after a frustum cull.
	/////////////////////////////////////////////////
	void updateResources(const DrawData&);

	Node getNode(int _idx) const { return m_nodes[_idx]; }
	GPUNode getGPUNode(int _idx) const { return m_gpuNodes[_idx]; }
	const Buffer& getTreeBuffer() const { return m_gpuBuffer; }
	int getNumNodes() const { return m_nodes.size(); }

private:
	/////////////////////////////////////////////////
	///	Removes child nodes who contain no grass.
	/// Call after build, before creating gpu resources.
	/////////////////////////////////////////////////
	void prune(std::vector<int>& _emptyIdx);
	/////////////////////////////////////////////////
	///	Initialise GPU resources.
	/////////////////////////////////////////////////
	void initResources(ID3D11Device* _device);


	
	std::vector<Node>		m_nodes;
	std::vector<GPUNode>	m_gpuNodes;
	Buffer					m_gpuBuffer;
};