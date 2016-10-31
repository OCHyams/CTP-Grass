#pragma once
__declspec(align(16))
struct CBGrassGeometry
{
	float tessDensity;
	float halfGrassWidth;
	float time;
	float wind_x;
	float wind_y;
	float wind_z;
	float padding[2];
	//DirectX::XMFLOAT3 wind;
	//float padding[2];
};

__declspec(align(16))
struct CBSingleMatrix
{
	DirectX::XMMATRIX m_mat;
};