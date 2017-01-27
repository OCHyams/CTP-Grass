#pragma once
#include "GameObject.h"
#include "basicVertex.h"
#include <d3d11_2.h>
#include <math.h>
#include "ConstantBuffers.h"
#include "FieldRenderData.h"
#include "Octree.h"
#include <vector>
#include "OctreeDebugger.h"
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
	bool m_drawOctree = true;

	/////////////////////////////////////////////////
	/// Should apply frustum culling?
	/////////////////////////////////////////////////
	bool m_frustumCull = true;

	/////////////////////////////////////////////////
	/// This shouldn't be public *oops*@
	/////////////////////////////////////////////////
	WindManager* m_windManager;
	

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
	//Don't use this anymore
	bool load(ID3D11Device*, int _instanceCount, DirectX::XMFLOAT2 _size, DirectX::XMFLOAT3	_pos);
	/*Model is only used during set-up*/ //@_minOctreeNodeSize should get calculated from collision data
	bool load(ID3D11Device*, ObjModel*, float density, DirectX::XMFLOAT3 _pos, const DirectX::XMFLOAT3& _minOctreeNodeSize/*, const DirectX::XMMATRIX& _transform*/);

	void unload();

	//void update();
	void draw(const DrawData&);

	int getMaxNumBlades() { return m_maxInstanceCount; }
	int getCurNumBlades() { return m_curInstanceCount; }

	//public for now cus lazy
	//static DirectX::XMFLOAT4X4		s_viewproj;
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
	int						m_maxInstanceCount;
	int						m_curInstanceCount;
	/////////////////////////////////////////////////
	/// LOD factors
	/////////////////////////////////////////////////
	float					m_maxDensity;
	float					m_minDensity;
	float					m_curDensity;
	/////////////////////////////////////////////////
	/// Octree root node
	/////////////////////////////////////////////////
	Octree::Node*			m_octreeRoot = nullptr;
	OctreeDebugger			m_octreeDebugger;
	//static
	DirectX::XMFLOAT3		m_cameraPos;
	/////////////////////////////////////////////////
	/// Wind data for per-instance wind
	/////////////////////////////////////////////////
	ID3D11UnorderedAccessView*	m_instancesUAV;//@
	ID3D11Buffer*				m_instanceUAVBufferOut;
	ID3D11ShaderResourceView*	m_instanceSRV;
	ID3D11Buffer*				m_instanceSRVBufferIn;
	/////////////////////////////////////////////////
	///	Field data
	/////////////////////////////////////////////////
	//rectangle dimentions of field
	DirectX::XMFLOAT2		m_size;
	DirectX::XMFLOAT3		m_pos;

	void updateConstBuffers(const DrawData&);

	/////////////////////////////////////////////////
	///	Allocates m_instanceCount of Instance, don't forget to clear up mem!
	/////////////////////////////////////////////////
	field::Instance* generateInstanceData();//@not in use atm...
	void buildInstanceBuffer();
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
	void addPatch(/*std::vector<field::Instance>& _field,*//* const Triangle& _tri*/float* verts, int _numBlade, Octree::Node* _nodeCache);
};


