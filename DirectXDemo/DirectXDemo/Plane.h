#pragma once
#include "GameObject.h"
#include "basicVertex.h"
#include <d3d11_2.h>
#include <math.h>
#include "ConstantBuffers.h"
class Plane : public GameObject
{
public:
	Plane();
	virtual ~Plane();

	virtual bool load(ID3D11Device*) override;
	virtual void unload() override;

	virtual void update() override;
	virtual void draw(const DrawData&) override;

protected:
	/////////////////////////////////////////////////
	/// Shaders
	/////////////////////////////////////////////////
	ID3D11VertexShader*		m_vs;
	ID3D11PixelShader*		m_ps;
	ID3D11RasterizerState*	m_rasterizer;
	/////////////////////////////////////////////////
	/// Constant buffers
	/////////////////////////////////////////////////
	ID3D11Buffer*			m_CB_world;
	CBWorldViewProj			m_CBcpu_world;
	/////////////////////////////////////////////////
	/// Shader misc
	/////////////////////////////////////////////////
	ID3D11InputLayout*		m_inputLayout;
	ID3D11Buffer*			m_vertexBuffer;
	ID3D11Buffer*			m_indexBuffer;
};