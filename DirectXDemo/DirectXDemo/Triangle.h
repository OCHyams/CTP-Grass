#pragma once
#include <d3d11.h>
#include <SimpleMath.h>
#include "Shorthand.h"

struct Triangle
{
	/* Calculate surface area from 3 * float3 positions */
	static float surfaceArea(float* verts)
	{
		using namespace DirectX;
		/*Calc surface area*/
		XMVECTOR a = VEC3(*(verts), *(verts + 1), *(verts + 2));
		XMVECTOR b = VEC3(*(verts + 3), *(verts + 4), *(verts + 5));
		XMVECTOR c = VEC3(*(verts + 6), *(verts + 7), *(verts + 8));
		XMVECTOR sa = 0.5 * XMVector3Length(XMVector3Cross((a - c), (b - c)));
		return XMVectorGetX(sa);
	}
};