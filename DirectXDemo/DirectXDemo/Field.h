/*----------------------------------------------------------------
Author:			Orlando Cazalet-Hyams
Description :	A Field handles generating & rendering grass over	
				a single mesh, octree is local, wind-manager is
				set up to be shared (but doesn't have to be).
----------------------------------------------------------------*/

#pragma once
#include "DrawData.h"
#include "VertexData.h"
#include <d3d11.h>
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
	bool load(ID3D11Device*, ObjModel*, float density, DirectX::XMFLOAT3 _pos, const DirectX::XMFLOAT3& _minOctreeNodeSize, WindManager* _windManager);

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
	WindManager*					m_windManager;
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

	void addPatch(std::vector<field::Instance>& _field, float* verts, unsigned int _numBlade);
};


