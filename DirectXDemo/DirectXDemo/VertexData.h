#pragma once
#include <d3d11.h>
#include "SimpleMath.h"

struct DefaultVertex
{
	DirectX::XMFLOAT3 m_pos;
	DirectX::XMFLOAT3 m_normal;
	DirectX::XMFLOAT2 m_texCoord;
};


struct BasicVertex
{
	DirectX::XMFLOAT4 pos;
};
