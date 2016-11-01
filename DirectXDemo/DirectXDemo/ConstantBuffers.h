#pragma once
__declspec(align(16))
struct CBGrassGeometry
{
	float tessDensity;
	float halfGrassWidth;
	float time;
	//wind
	float wind_x;
	float wind_y;
	float wind_z;
	//base position
	float pos_x;
	float pos_y;
	float pos_z;
	//tangent
	float tan_x;
	float tan_y;
	float tan_z;
};

__declspec(align(16))
struct CBWorldViewProj
{
	DirectX::XMMATRIX m_wvp;
};