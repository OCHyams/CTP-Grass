#pragma once
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include "FieldRenderData.h"
#include <vector>
#include "Shorthand.h"
class ObjModel;
namespace Octree
{
	/////////////////////////////////////////////////
	///	Octree node. Use with Octree::build() & Octree::cleanup(Node*).
	/////////////////////////////////////////////////
	struct Node
	{
		DirectX::BoundingBox	m_AABB;
		std::vector
			<field::Instance>	m_instances;
		std::vector<Node*>		m_children;
		Node*					m_parent;

		Node() = delete;
		Node(	DirectX::XMVECTOR _point0,
				DirectX::XMVECTOR _point1,
				Node* _parent)
			:	m_parent(_parent)
		{
			DirectX::BoundingBox::CreateFromPoints(m_AABB, _point0, _point1);
		}

		~Node()
		{
			m_instances.clear();
		}
	};

	/////////////////////////////////////////////////
	///	Builds an octree and returns a ptr to the root.
	/////////////////////////////////////////////////
	Node* build(const ObjModel&				_model, 
				DirectX::XMVECTOR			_position,
				const DirectX::XMFLOAT3&	_minSize, 
				float						_minGrassLength,
				float						_grassDensity);

	/////////////////////////////////////////////////
	///	Deletes all nodes including root.
	/////////////////////////////////////////////////
	void cleanup(Node* _root);

	/////////////////////////////////////////////////
	///	Removes child nodes who contain no grass.
	/// Call after adding all grass to tree.
	/////////////////////////////////////////////////
	void prune(Node* _root);

	/////////////////////////////////////////////////
	///	Coppies a grass instance into a leaf node.
	///	Returns nullptr if the grass couldn't be added.
	/// Returns the node that it was added to.
	/////////////////////////////////////////////////
	Node* addGrass(Node* _root, const field::Instance& _instance);

	/////////////////////////////////////////////////
	///	Coppies a grass instances that are in view 
	///	into a buffer. @If there is time etc, implement this as a compute shader
	/////////////////////////////////////////////////
	void frustumCull(Node* _root, const DirectX::BoundingFrustum& _frustum, field::Instance* _buffer, int _bufferSize, unsigned int& _numInstances);

	void noCull(Node* _root, field::Instance* _buffer, int _bufferSize, unsigned int& _numInstances);
}