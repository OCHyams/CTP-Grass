/*----------------------------------------------------------------
Author:			Orlando Cazalet-Hyams
Description :	Helper class that draws an octree for debugging.
----------------------------------------------------------------*/

#pragma once
#include "GPUOctree.h"
#include <d3dcompiler.h>
#include "dxerr.h"
#include <DirectXMath.h>
#include "FieldRenderData.h"
#include "Shaders.h"
/////////////////////////////////////////////////
///	Class to help debugg an octree.
/////////////////////////////////////////////////
class GPUOctreeDebugger
{
public:
	static bool loadShared(ID3D11Device* _device);
	static void unloadShared();

	void draw(ID3D11DeviceContext* _dc, const DirectX::XMFLOAT4X4& _transform, const DirectX::XMFLOAT4X4& _viewProj, const GPUOctree& _tree);
private:
	static DXHelper::Shaders		s_shaders;
	static ID3D11RasterizerState*	s_rasterizer;
	static ID3D11InputLayout*		s_inputLayout;
	static ID3D11Buffer*			s_indexBuffer;
	static ID3D11Buffer*			s_vertexBuffer;
	static ID3D11Buffer*			s_m_CB_wvp;
};