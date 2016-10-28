#pragma once
#include "GameObject.h"
#include "basicVertex.h"
#include <d3d11_2.h>
#include "ConstantBuffers.h"

/////////////////////////////////////////////////
/// Single blade of grass defined by a 4 point bezier curve
/////////////////////////////////////////////////
class SimpleGrass : public GameObject
{
public:
	SimpleGrass();
	virtual ~SimpleGrass();


	virtual bool load(ID3D11Device*) override;
	virtual void unload() override;

	virtual void update(float _dt) override;
	virtual void draw(ID3D11DeviceContext* _dc) override;

protected:
	/////////////////////////////////////////////////
	/// Shaders
	/////////////////////////////////////////////////
	ID3D11VertexShader* m_vs;
	ID3D11PixelShader* m_ps;
	ID3D11GeometryShader* m_gs;
	ID3D11HullShader* m_hs;
	ID3D11DomainShader* m_ds;
	ID3D11RasterizerState* m_rasterizer;
	/////////////////////////////////////////////////
	/// Constant buffers
	/////////////////////////////////////////////////
	ID3D11Buffer* m_CB_density;
	ID3D11Buffer* m_CB_world;
	//ID3D11Buffer* m_CB_viewProj;
	/////////////////////////////////////////////////
	/// Shader misc
	/////////////////////////////////////////////////
	ID3D11InputLayout* m_inputLayout;
	ID3D11Buffer* m_vertexBuffer;
	/////////////////////////////////////////////////
	/// LOD factors
	/////////////////////////////////////////////////
	float m_maxDensity;
	float m_minDensity;
	float m_curDensity;
};