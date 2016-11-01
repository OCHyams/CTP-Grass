#pragma once
class Camera; 
class ID3D11DeviceContext;

struct DrawData
{
	const Camera* m_cam;
	ID3D11DeviceContext*  m_dc;

};