#pragma once
#include "GameObject.h"
#include "basicVertex.h"
#include <d3d11_2.h>
#include <math.h>
#include "ConstantBuffers.h"
#include "FieldRenderData.h"
#include <vector>
#include "ConstantBuffer.h"
#include "DoubleBuffer.h"
#include "WindManager_14_03.h"
class ObjModel;

class Field_14_03
{
public:
	const int m_threadsPerGroupX = 256;
	WindManager_14_03*				m_windManager;
	float							m_halfGrassWidth;
	//float m_length; //To-Do@.

	Field_14_03();
	~Field_14_03();

	/////////////////////////////////////////////////
	/// Compile and load the grass shader
	/////////////////////////////////////////////////
	static bool loadShared(ID3D11Device*);
	/////////////////////////////////////////////////
	/// unload the grass shader
	/////////////////////////////////////////////////
	static void unloadShared();

	bool load(ID3D11Device*, ObjModel*, float density);

	void unload();

	void draw(const DrawData&);

	unsigned int getMaxNumBlades() { return m_maxInstanceCount; }
	//Won't work at the moment!
	unsigned int getCurNumBlades() { return m_curInstanceCount; }

private:
	/////////////////////////////////////////////////
	/// Shared data
	/////////////////////////////////////////////////
	static field::Shaders			s_shaders;
	static ID3D11RasterizerState*	s_rasterizer;
	static ID3D11InputLayout*		s_inputLayout;
	static ID3D11Buffer*			s_vertexBuffer;
	static
		ID3D11ShaderResourceView*		s_texture;
	static ID3D11SamplerState*		s_samplerState;
	static ID3D11ComputeShader*		s_csWind;
	/////////////////////////////////////////////////
	/// Constant buffers 
	/////////////////////////////////////////////////
	CBuffer<CBWorldViewProj>		m_CB_viewproj;
	CBuffer<CBField>				m_CB_geometry;
	CBuffer<CBFieldLight>			m_CB_light;	
	CBuffer<CBGrassWindFrustum_ChangesPerFrameRO>	m_CB_CS_RO;
	CBuffer<CBGrassWindFrustum_ChangesPerFrameRW>	m_CB_CS_RW;
	CBuffer<CBGrassWindFrustum_NeverChanges>		m_CB_CS_Consts;
	/////////////////////////////////////////////////
	/// Instance data
	/////////////////////////////////////////////////
	int						m_maxInstanceCount;
	int						m_curInstanceCount;
	/////////////////////////////////////////////////
	/// LOD factors
	/////////////////////////////////////////////////
	float					m_maxDensity;
	float					m_minDensity;
	float					m_curDensity;
	/////////////////////////////////////////////////
	/// Wind data for per-instance wind
	/////////////////////////////////////////////////
	DoubleBuffer			m_instanceDoubleBuffer;
	Buffer					m_CSAddressIdicies;
	void updateConstBuffers(const DrawData&);
};