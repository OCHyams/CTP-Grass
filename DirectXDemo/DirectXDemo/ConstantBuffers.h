#pragma once
#include <d3d11_2.h>
#include "SimpleMath.h"

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
	DirectX::XMFLOAT4	light;
	DirectX::XMFLOAT4	ambient;
	DirectX::XMFLOAT4	diffuse; 
	DirectX::XMFLOAT4	specular;
	float				shiny;
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

__declspec(align(16))
struct CBDefaultObject_ChangesPerObject
{
	DirectX::XMMATRIX	m_worldViewProj;
	DirectX::XMFLOAT3	m_worldPos;
	float				m_specularPow;
};

__declspec(align(16))
struct CBDefaultObject_ChangesPerFrame
{
	DirectX::XMFLOAT3	m_cameraPos;
	float				pad0;
	DirectX::XMFLOAT3	m_lightPos;
	float				m_intensity;
};


__declspec(align(16))
struct CBGrassWindFrustum_ChangesPerFrameRO
{
	unsigned int numCuboids;
	unsigned int numSpheres;
	float time;
	float deltaTime;
}; 

struct CBGrassWindFrustum_NeverChanges
{
	unsigned int maxInstances;
};

__declspec(align(16))
struct CBGrassWindFrustum_ChangesPerFrameRW
{
	unsigned int numInstances;
};
