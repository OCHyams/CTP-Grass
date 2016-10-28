#pragma once
__declspec(align(16))
struct CBGrassGeometry
{
	float tessDensity;
	float halfGrassWidth;
	float padding[2];
};

__declspec(align(16))
struct CBSingleMatrix
{
	DirectX::XMMATRIX m_mat;
};