/*----------------------------------------------------------------
Author:			Orlando Cazalet-Hyams
Description :	CPU-side constant buffer structs.
----------------------------------------------------------------*/

#pragma once
#include <d3d11.h>
#include "SimpleMath.h"

__declspec(align(16))
struct CBField_RarelyChanges
{
	float	halfGrassWidth;
	float	maxTessDensity;
	float	minTessDensity;
	float	nearTess;
	float	farTess;
};


__declspec(align(16))
struct CBBasicShader_ChangesPerFrame
{
	DirectX::XMFLOAT4X4 worldViewProj;
};

__declspec(align(16))
struct CBField_ChangesPerFrame
{
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 viewProj;
	DirectX::XMFLOAT4X4 worldViewProj;
	float				time;
};

__declspec(align(16))
struct CBField_Light
{
	DirectX::XMFLOAT4	camera;
	DirectX::XMFLOAT4	light;
	DirectX::XMFLOAT4	ambient;
	DirectX::XMFLOAT4	diffuse; 
	DirectX::XMFLOAT4	specular;
	float				shiny;
};


__declspec(align(16))
struct CBWindForceChangesPerFrame
{
	unsigned int numCuboids;
	unsigned int numSpheres;
	unsigned int numInstances;
	float time;
	float deltaTime;
};

__declspec(align(16))
struct CBDefaultObject_ChangesPerObject
{
	DirectX::XMMATRIX	m_worldViewProj;
	DirectX::XMFLOAT3	m_worldPos;
	float				m_specularPow;
	int					m_lit;
};

__declspec(align(16))
struct CBDefaultObject_ChangesPerFrame
{
	DirectX::XMFLOAT3	m_cameraPos;
	float				pad0;
	DirectX::XMFLOAT3	m_lightPos;
	float				m_intensity;
};
