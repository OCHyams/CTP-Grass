#pragma once
class ArcCamera; 
class ID3D11DeviceContext;

struct DrawData
{
	const ArcCamera* m_cam;
	ID3D11DeviceContext*  m_dc;

};