#pragma once
#include "Object.h"

class WindCuboid : public Object
{
public:
	DirectX::XMFLOAT3	m_extents;
	DirectX::XMFLOAT3	m_initalVelocity;
};