#pragma once
#include "Object.h"

class WindRect : public Object
{
public:
	DirectX::XMFLOAT3	m_extents;
	DirectX::XMFLOAT3	m_initalVelocity;
	float				m_fallOffCoefficent;

protected:
private:
};