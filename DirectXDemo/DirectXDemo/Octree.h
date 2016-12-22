#pragma once
#include <DirectXMath.h>
#include "FieldRenderData.h"

class ObjModel;
namespace Octree
{
	class Node
	{
		DirectX::XMFLOAT3		m_pos;
		DirectX::XMFLOAT2		m_size;
		int						m_numInstances;
		field::Instance*		m_instances;
		Node*					m_children[8];
	};

	Node* build(const ObjModel& _model, const DirectX::XMFLOAT2& _minSize, field::Instance* _instances, int _numInstances);

	void cleanup(Node* _head);

}