#pragma once
#include <DirectXMath.h>
struct BasicVertex
{
	DirectX::XMFLOAT3 pos;
	
};

struct BezierVertex
{
	DirectX::XMFLOAT3 pos;
	float tVal;
};