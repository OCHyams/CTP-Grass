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
	float	m_halfGrassWidth;
	float	m_maxTessDensity;
	float	m_minTessDensity;
	float	m_nearTess;
	float	m_farTess;
};


__declspec(align(16))
struct CBBasicShader_ChangesPerFrame
{
	DirectX::XMFLOAT4X4 m_worldViewProj;
};

__declspec(align(16))
struct CBField_ChangesPerFrame
{
	DirectX::XMFLOAT4X4 m_world;
	DirectX::XMFLOAT4X4 m_viewProj;
	DirectX::XMFLOAT4X4 m_worldViewProj;
	float				m_time;
};

__declspec(align(16))
struct CBField_Light
{
	DirectX::XMFLOAT4	m_camera;
	DirectX::XMFLOAT4	m_light;
	DirectX::XMFLOAT4	m_ambient;
	DirectX::XMFLOAT4	m_diffuse; 
	DirectX::XMFLOAT4	m_specular;
	float				m_shiny;
};


__declspec(align(16))
struct CBWindForceChangesPerFrame
{
	unsigned int m_numCuboids;
	unsigned int m_numSpheres;
	unsigned int m_numInstances;
	float m_time;
	float m_deltaTime;
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
