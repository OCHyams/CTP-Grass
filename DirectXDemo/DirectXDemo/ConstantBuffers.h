#pragma once
__declspec(align(16))
struct CBSimpleGrass_DS
{
	float m_tessDensity[4];
};

__declspec(align(16))
struct CBSimpleGrass_GS
{
	float m_halfBaseWidth;
	float m_padding[3];
};

__declspec(align(16))
struct CBSingleMatrix
{
	DirectX::XMMATRIX m_mat;
};