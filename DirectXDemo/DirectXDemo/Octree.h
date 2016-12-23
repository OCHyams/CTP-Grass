#pragma once
#include <DirectXMath.h>
#include "FieldRenderData.h"

class ObjModel;
namespace Octree
{
	/////////////////////////////////////////////////
	///	Octree node. Use with Octree::build() & Octree::cleanup(Node*).
	/////////////////////////////////////////////////
	struct Node
	{
		DirectX::XMFLOAT3		m_pos;
		DirectX::XMFLOAT3		m_halfSize;
		int						m_numInstances;
		field::Instance*		m_instances;
		Node*					m_children[8];

		Node()
		{
			memset(m_children, (int)nullptr, 8);
			m_instances		= nullptr;
			m_numInstances	= 0;
			m_halfSize		= { 0, 0, 0 };
			m_pos			= { 0, 0, 0 };
		}

		Node(	const DirectX::XMFLOAT3& _pos, 
				const DirectX::XMFLOAT3& _halfSize,
				int _numInstances,
				field::Instance* _instances,
				Node** _children,
				int _numChildren = 8) :
				/*Initialisation*/
				m_pos(_pos), m_halfSize(_halfSize), 
				m_numInstances(_numInstances), m_instances(_instances)
		{
			/*Copy child addresses*/
			memcpy(m_children, _children, _numChildren);
		}
	};

	/////////////////////////////////////////////////
	///	Builds an octree and returns a ptr to the root.
	/////////////////////////////////////////////////
	Node* build(const ObjModel& _model, const DirectX::XMFLOAT3& _position, const DirectX::XMFLOAT3& _minSize, field::Instance* _instances, int _numInstances);

	/////////////////////////////////////////////////
	///	Deletes all nodes including root.
	/////////////////////////////////////////////////
	void cleanup(Node* _root);

}