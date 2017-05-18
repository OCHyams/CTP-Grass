#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
namespace field
{
	using namespace DirectX;

	__declspec(align(16))
	struct Vertex
	{
		XMFLOAT3	pos;
		XMFLOAT3	bitangent;
		//XMFLOAT3	normal;
		float		flexibility;
	};

	__declspec(align(16))
	struct Instance
	{
		XMFLOAT4		rotation;
		XMFLOAT3		location;
		XMFLOAT3		wind;
		int				octreeIdx =-1;
	};
}