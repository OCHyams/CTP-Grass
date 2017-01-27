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
	float tan_w;
};

__declspec(align(16))
struct CBField
{
	float	halfGrassWidth;
	float	time;
	float	minTessDensity;
	float	maxTessDensity;
	float	nearTess;
	float	farTess;
};
__declspec(align(16))
struct CBWorldViewProj
{
	DirectX::XMFLOAT4X4 m_wvp;
};

__declspec(align(16))
struct CBFieldLight
{
	DirectX::XMFLOAT4	camera;//@change to float3s
	DirectX::XMFLOAT4	light;	//@
	float				intensity;
};

__declspec(align(16))
struct CBLight
{
	float pos_x;
	float pos_y;
	float pos_z;
	float intensity;
	float cam_x;
	float cam_y;
	float cam_z;
};

__declspec(align(16))
struct CBWindForceChangesPerFrame
{
	unsigned int numCuboids;
	unsigned int numSpheres;
	unsigned int numInstances;
};