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
		Node(	DirectX::XMVECTOR _min,
				DirectX::XMVECTOR _max,
				Node* _parent)
			:	m_parent(_parent)
		{
			DirectX::BoundingBox::CreateFromPoints(m_AABB, _min, _max);
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
				field::Instance*			_instances, 
				int							_numInstances,
				float						_minGrassLength);

	/////////////////////////////////////////////////
	///	Deletes all nodes including root.
	/////////////////////////////////////////////////
	void cleanup(Node* _root);

	/////////////////////////////////////////////////
	///	Removes child nodes who contain no grass.
	/////////////////////////////////////////////////
	void prune(Node* _root);

	/////////////////////////////////////////////////
	///	Coppies a grass instance into a leaf node.
	///	Returns false if the grass couldn't be added.
	/////////////////////////////////////////////////
	bool addGrass(Node* _root, const field::Instance& _instance);

}