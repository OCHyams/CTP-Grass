#pragma once
class ArcCamera; 
struct ID3D11DeviceContext;

struct DrawData
{
	const ArcCamera* m_cam;
	ID3D11DeviceContext*  m_dc;
	const double m_time;
	const double m_deltaTime;
};