#pragma once
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include "FieldRenderData.h"
#include <vector>
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
		Node*					m_children[8];
		bool					m_leaf;
		Node()
		{
			memset(m_children, (int)nullptr, 8);
			m_leaf			= false;
		}

		Node(	const DirectX::XMFLOAT3& _min, 
				const DirectX::XMFLOAT3& _max,
				Node** _children,
				int _numChildren = 8)
		{
			m_AABB = DirectX::BoundingBox(_min, _max);
			if (_numChildren = 0)
			{
				m_leaf = true;
			}
			else
			{
				//Copy child addresses
				memcpy(m_children, _children, _numChildren);
			}
		}

		~Node()
		{
			m_instances.empty();
		}
	};

	/////////////////////////////////////////////////
	///	Builds an octree and returns a ptr to the root.
	/////////////////////////////////////////////////
	Node* build(const ObjModel&				_model, 
				const DirectX::XMFLOAT3&	_position, 
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