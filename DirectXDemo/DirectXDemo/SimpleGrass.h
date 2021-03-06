#pragma once
#include "GameObject.h"
#include "basicVertex.h"
#include <d3d11_2.h>
#include <math.h>
#include "ConstantBuffers.h"

/////////////////////////////////////////////////
/// Single blade of grass defined by a 4 point bezier curve
/////////////////////////////////////////////////
class SimpleGrass : public GameObject
{
public:
	SimpleGrass();
	virtual ~SimpleGrass();

	static bool loadShared(ID3D11Device*);
	static void unloadShared();

	static void beginDraw(const DrawData& _data);


	virtual bool load(ID3D11Device*) override;
	virtual void unload() override;

	virtual void update() override;
	virtual void draw(const DrawData&) override;

	void setGrassWidth(float _width) { m_halfGrassWidth = _width / 2; }
	void setWind(const DirectX::XMFLOAT3& _wind) { m_wind = _wind; }

	static void updateCameraPosition(const DirectX::XMFLOAT3& _pos) { s_cameraPos = _pos; }
protected:
	/////////////////////////////////////////////////
	/// Shaders
	/////////////////////////////////////////////////
	static ID3D11VertexShader*		m_vs;
	static ID3D11PixelShader*		m_ps;
	static ID3D11GeometryShader*	m_gs;
	static ID3D11HullShader*		m_hs;
	static ID3D11DomainShader*		m_ds;
	static ID3D11RasterizerState*	m_rasterizer;
	/////////////////////////////////////////////////
	/// Constant buffers
	/////////////////////////////////////////////////
	ID3D11Buffer*			m_CB_geometry;
	ID3D11Buffer*			m_CB_worldviewproj;
	ID3D11Buffer*			m_CB_light;
	/////////////////////////////////////////////////
	/// Constant buffers cpu side
	/////////////////////////////////////////////////
	CBField_ChangesPerFrame			m_CBcpu_worldviewproj;
	CBGrassGeometry			m_CBcpu_geometry;
	CBLight					m_CBcpu_light;
	/////////////////////////////////////////////////
	/// Shader misc
	/////////////////////////////////////////////////
	static ID3D11InputLayout* m_inputLayout;
	ID3D11Buffer*			m_vertexBuffer;
	/////////////////////////////////////////////////
	/// LOD factors
	/////////////////////////////////////////////////
	float					m_maxDensity;
	float					m_minDensity;
	float					m_curDensity;
	float					m_halfGrassWidth;
	/////////////////////////////////////////////////
	/// Wind data stored here for testing!
	/////////////////////////////////////////////////
	DirectX::XMFLOAT3		m_wind;
	static 
	DirectX::XMFLOAT3		s_cameraPos;

	void updateConstBuffers();
};