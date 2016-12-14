#pragma once
#include "GameObject.h"
#include "basicVertex.h"
#include <d3d11_2.h>
#include <math.h>
#include "ConstantBuffers.h"
#include "FieldRenderData.h"
class ObjModel;
/////////////////////////////////////////////////
/// Field of grass 
/////////////////////////////////////////////////
class Field final
{
public:
	Field();
	~Field();

	/////////////////////////////////////////////////
	/// Compile and load the grass shader
	/////////////////////////////////////////////////
	static bool loadShared(ID3D11Device*);
	/////////////////////////////////////////////////
	/// unload the grass shader
	/////////////////////////////////////////////////
	static void unloadShared();

	bool load(ID3D11Device*, int _instanceCount, DirectX::XMFLOAT2 _size, DirectX::XMFLOAT3	_pos);
	/*Model is only used during set-up*/
	bool load(ID3D11Device*, ObjModel*, float density);

	void unload();

	void update();
	void draw(const DrawData&);

	static void updateCameraPosition(const DirectX::XMFLOAT3& _pos) { s_cameraPos = _pos; }
	void setWind(const DirectX::XMFLOAT3& _wind) { m_wind = _wind; }

	//public for now cus lazy
	static DirectX::XMFLOAT4X4		s_viewproj;
	float							m_length;
	//float							m_sdLength;//standard diviation
	float							m_halfGrassWidth;
	//float							m_sdHlafGrassWidth;//standard diviation
private:
	/////////////////////////////////////////////////
	/// Shared data
	/////////////////////////////////////////////////
	static field::Shaders			s_shaders;
	static ID3D11RasterizerState*	s_rasterizer;
	static ID3D11InputLayout*		s_inputLayout;
	static ID3D11Buffer*			s_vertexBuffer;	//this can be static for now but there should probably be a range of these for different grass positions..?
	static 
	ID3D11ShaderResourceView*		s_texture; 
	static ID3D11SamplerState*		s_samplerState;
	/////////////////////////////////////////////////
	/// Constant buffers
	/////////////////////////////////////////////////
	ID3D11Buffer*			m_CB_geometry;
	ID3D11Buffer*			m_CB_viewproj;
	ID3D11Buffer*			m_CB_light;
	/////////////////////////////////////////////////
	/// Constant buffers cpu side
	/////////////////////////////////////////////////
	CBWorldViewProj			m_CBcpu_viewproj;
	CBField					m_CBcpu_geometry;
	CBFieldLight			m_CBcpu_light;
	/////////////////////////////////////////////////
	/// Instance data
	/////////////////////////////////////////////////
	field::Instance*		m_instances = nullptr;
	ID3D11Buffer*			m_instanceBuffer;
	int						m_instanceCount;
	/////////////////////////////////////////////////
	/// LOD factors
	/////////////////////////////////////////////////
	float					m_maxDensity;
	float					m_minDensity;
	float					m_curDensity;
	/////////////////////////////////////////////////
	/// Wind data stored here for testing!
	/////////////////////////////////////////////////
	DirectX::XMFLOAT3		m_wind;
	static
	DirectX::XMFLOAT3		s_cameraPos;
	/////////////////////////////////////////////////
	///	Field data
	/////////////////////////////////////////////////
	//rectangle dimentions of field
	DirectX::XMFLOAT2		m_size;
	DirectX::XMFLOAT3		m_pos;

	void updateConstBuffers();

	/////////////////////////////////////////////////
	///	Allocates m_instanceCount of Instance, don't forget to clear up mem!
	/////////////////////////////////////////////////
	field::Instance* generateInstanceData();
	void buildInstanceBuffer();
	bool loadBuffers(ID3D11Device*);
};
