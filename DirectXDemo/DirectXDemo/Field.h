#pragma once
#include "DrawData.h"
#include "basicVertex.h"
#include <d3d11_2.h>
#include <math.h>
#include "ConstantBuffers.h"
#include "FieldRenderData.h"
#include <vector>
#include "GPUOctree.h"
#include "DoubleBuffer.h"
#include "GPUOctreeDebugger.h"
#include "ConstantBuffer.h"
class ObjModel;
class WindManager;

/////////////////////////////////////////////////
/// Field of grass 
/////////////////////////////////////////////////
class Field final
{
public:
	/////////////////////////////////////////////////
	/// Should draw the Octree?
	/////////////////////////////////////////////////
	bool m_drawGPUOctree = false;

	/////////////////////////////////////////////////
	/// Should stop frustum culling?
	/////////////////////////////////////////////////
	bool m_noCulling = false;

	/////////////////////////////////////////////////
	/// This shouldn't be public *oops*@
	/////////////////////////////////////////////////
	WindManager* m_windManager;
	
	Field();
	~Field();

	/////////////////////////////////////////////////
	/// Compile and load the grass shaders @Should not compile in build version
	/////////////////////////////////////////////////
	static bool loadShared(ID3D11Device*);
	/////////////////////////////////////////////////
	/// Unload the grass shaders
	/////////////////////////////////////////////////
	static void unloadShared();
	/////////////////////////////////////////////////
	/// Initialise the field
	/////////////////////////////////////////////////
	bool load(ID3D11Device*, ObjModel*, float density, DirectX::XMFLOAT3 _pos, const DirectX::XMFLOAT3& _minOctreeNodeSize);

	void updateLODAndWidth(CBField_RarelyChanges& _newBuffer);
	const CBField_RarelyChanges& getCurrentLODAndWidth() const { return m_CBField_RarelyChanges; };

	void unload();

	//void update();
	void draw(const DrawData&);

	unsigned int getMaxNumBlades() { return m_maxInstanceCount; }
	unsigned int getCurNumBlades() { return m_curInstanceCount; }

	float							m_length;
	float							m_halfGrassWidth;

private:
	/////////////////////////////////////////////////
	/// New octree
	/////////////////////////////////////////////////
	GPUOctreeDebugger	m_gpuOctreeDebugger;
	GPUOctree			m_gpuOctree;	
	Buffer				m_pseudoAppend;
	Buffer				m_indirectArgs;

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
	bool								m_CBField_RarelyChanges_dirty;
	CBuffer<CBField_RarelyChanges>		m_CBField_RarelyChanges;
	CBuffer<CBField_ChangesPerFrame>	m_CBField_ChangesPerFrame;
	CBuffer<CBField_Light>				m_CBField_Light;
	/////////////////////////////////////////////////
	/// Instance data
	/////////////////////////////////////////////////
	DoubleBuffer			m_instanceDoubleBuffer;
	unsigned int			m_maxInstanceCount;
	unsigned int			m_curInstanceCount;
	/////////////////////////////////////////////////
	/// 
	/////////////////////////////////////////////////
	DirectX::XMFLOAT3		m_cameraPos;
	/////////////////////////////////////////////////
	///	Field data
	/////////////////////////////////////////////////
	DirectX::XMFLOAT3		m_pos;//@should get wvp to parent the nodes & grass so objcs can move!

	void updateConstBuffers(const DrawData&);

	bool loadBuffers(ID3D11Device*);

	/*Stuff for proc gen grass*/
	struct Triangle
	{
		//Calculate surface area from 3 * float3 positions
		static float surfaceArea(float* verts)
		{
			using namespace DirectX;
			/*Calc surface area*/
			XMVECTOR a = VEC3(*(verts), *(verts + 1), *(verts + 2));
			XMVECTOR b = VEC3(*(verts + 3), *(verts + 4), *(verts + 5));
			XMVECTOR c = VEC3(*(verts + 6), *(verts + 7), *(verts + 8));
			XMVECTOR sa = 0.5 * XMVector3Length(XMVector3Cross((a - c), (b - c)));
			return sa.m128_f32[0];
		}

		DirectX::XMFLOAT3	m_verts[3];
		DirectX::XMFLOAT3	m_norms[3];
		float				m_surfaceArea;

		Triangle(const DirectX::XMFLOAT3* _verts, const DirectX::XMFLOAT3* _norms)
		{
			using namespace DirectX;
			/*Get verts & norms*/
			memcpy(m_verts, _verts, sizeof(DirectX::XMFLOAT3) * 3);
			if (_norms) memcpy(m_norms, _norms, sizeof(DirectX::XMFLOAT3) * 3);
			else m_norms[0] = m_norms[1] = m_norms[2] = { 0,1,0 };

			/*Calc surface area*/
			XMVECTOR a = XMLoadFloat3(&m_verts[0]);
			XMVECTOR b = XMLoadFloat3(&m_verts[1]);
			XMVECTOR c = XMLoadFloat3(&m_verts[2]);
			XMVECTOR sa = 0.5 * XMVector3Length(XMVector3Cross((a - c), (b - c)));
			m_surfaceArea = sa.m128_f32[0];
		}

	};

	void addPatch(std::vector<field::Instance>& _field, float* verts, unsigned int _numBlade);
};


