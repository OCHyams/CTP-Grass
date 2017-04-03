#include "Field.h"
#include <d3dcompiler.h>
#include "dxerr.h"
#include <DirectXMath.h>
#include "DXHelper.h"
#include "Input.h"
#include "Time.h"
#include "ServiceLocator.h"
#include "DDSTextureLoader.h"
#include <random>
#include "Camera.h"
#include <functional>
#include <vector>
#include "objLoader.h"
#include <stdlib.h>
#include "Shorthand.h"
#include "WindManager.h"
#include "Buffer.h"
#include "AntTweakBar.h"
//statics
//DirectX::XMFLOAT3		Field::s_cameraPos		= DirectX::XMFLOAT3();
//DirectX::XMFLOAT4X4		Field::s_viewproj		= DirectX::XMFLOAT4X4();
field::Shaders			Field::s_shaders		= field::Shaders();
ID3D11RasterizerState*	Field::s_rasterizer		= nullptr;
ID3D11InputLayout*		Field::s_inputLayout	= nullptr;
ID3D11Buffer*			Field::s_vertexBuffer	= nullptr;
ID3D11ShaderResourceView* Field::s_texture		= nullptr;
ID3D11SamplerState*		Field::s_samplerState	= nullptr;

Field::Field()
{
}

Field::~Field()
{
	//unload();
}

bool Field::loadShared(ID3D11Device* _device)
{
	HRESULT result;
	ID3DBlob* buffer = nullptr;
	unsigned int shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
#endif

	//VS SHADER
	result = D3DCompileFromFile(L"Field.fx", NULL, NULL, "VS_Main", "vs_5_0", shaderFlags, 0, &buffer, NULL);
	if (FAILED(result))
	{
		RELEASE(buffer);
		MessageBox(0, "Error loading vertex shader.", "Shader compilation", MB_OK);
		return false;
	}
	_device->CreateVertexShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), 0, &s_shaders.m_vs);
	if (FAILED(result))
	{
		RELEASE(buffer);
		DXTRACE_MSG(L"Couldn't create the vertex shader.");
		return false;
	}

	//INPUT LAYOUT
	D3D11_INPUT_ELEMENT_DESC vsLayout[] =
	{
		//PER_VERTEX
		{ "SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "FLEX", 0, DXGI_FORMAT_R32_FLOAT ,0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//PER_INSTANCE
	/*	{ "INSTANCE_WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE_WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE_WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE_WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },*/
		{ "INSTANCE_ROTATION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, /*64*/0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE_LOCATION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, /*80*/16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE_WIND", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 28, D3D11_INPUT_PER_INSTANCE_DATA, 1}
	};
	unsigned unsigned int totalLayoutElements = ARRAYSIZE(vsLayout);

	result = _device->CreateInputLayout(vsLayout, totalLayoutElements, buffer->GetBufferPointer(), buffer->GetBufferSize(), &s_inputLayout);
	RELEASE(buffer);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Couldn't create the input layout.");
		return false;
	}

	//PS SHADER
	result = D3DCompileFromFile(L"Field.fx", NULL, NULL, "PS_Main", "ps_5_0", shaderFlags, 0, &buffer, NULL);
	if (FAILED(result))
	{
		RELEASE(buffer);
		MessageBox(0, "Error loading pixel shader.", "Shader compilation", MB_OK);
		return false;
	}

	result = _device->CreatePixelShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), 0, &s_shaders.m_ps);
	RELEASE(buffer);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Couldn't create the pixel shader.");
		return false;
	}

	//GS SHADER
	result = D3DCompileFromFile(L"Field.fx", NULL, NULL, "GS_Main", "gs_5_0", shaderFlags, 0, &buffer, NULL);
	if (FAILED(result))
	{
		RELEASE(buffer);
		MessageBox(0, "Error loading geometry shader.", "Shader compilation", MB_OK);
		return false;
	}

	result = _device->CreateGeometryShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), 0, &s_shaders.m_gs);
	RELEASE(buffer);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Couldn't create the geometry shader.");
		return false;
	}

	//HS SHADER
	result = D3DCompileFromFile(L"Field.fx", NULL, NULL, "HS_Main", "hs_5_0", shaderFlags, 0, &buffer, NULL);
	if (FAILED(result))
	{
		RELEASE(buffer);
		MessageBox(0, "Error loading hull shader.", "Shader compilation", MB_OK);
		return false;
	}

	result = _device->CreateHullShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), 0, &s_shaders.m_hs);
	RELEASE(buffer);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Couldn't create the hull shader.");
		return false;
	}

	//DS SHADER
	result = D3DCompileFromFile(L"Field.fx", NULL, NULL, "DS_Main", "ds_5_0", shaderFlags, 0, &buffer, NULL);
	if (FAILED(result))
	{
		RELEASE(buffer);
		MessageBox(0, "Error loading domain shader.", "Shader compilation", MB_OK);
		return false;
	}

	result = _device->CreateDomainShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), 0, &s_shaders.m_ds);
	RELEASE(buffer);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Couldn't create the domain shader.");
		return false;
	}
	
	//VERTEX DATA
	field::Vertex verts[] =
	{
		//pos, binormal, normal, flex																							//flex
		{ DirectX::XMFLOAT3(0.f, 0.f, 0.f),   DirectX::XMFLOAT3(1.f, 0.f, 0.f), DirectX::XMFLOAT3(0.f, 0.f, -1.f), 0.f },		//0
		{ DirectX::XMFLOAT3(0.f, 0.2f, 0.f),  DirectX::XMFLOAT3(1.f, 0.f, 0.f), DirectX::XMFLOAT3(0.f, 0.f, -1.f), 0.1f },	//0.111
		{ DirectX::XMFLOAT3(0.f, 0.4f, 0.f),  DirectX::XMFLOAT3(1.f, 0.f, 0.f), DirectX::XMFLOAT3(0.f, 0.f, -1.f), 0.3f },	//0.445
		{ DirectX::XMFLOAT3(0.0f, 0.6f, 0.08f),  DirectX::XMFLOAT3(1.f, 0.f, 0.f), DirectX::XMFLOAT3(0.f, 0.f, -1.f), 1.f }		//1
	};

	D3D11_BUFFER_DESC vDesc;
	ZeroMemory(&vDesc, sizeof(vDesc));
	vDesc.Usage			= D3D11_USAGE_DEFAULT;
	vDesc.BindFlags		= D3D11_BIND_VERTEX_BUFFER;
	vDesc.ByteWidth		= sizeof(field::Vertex) * 4;

	D3D11_SUBRESOURCE_DATA resourceData;
	ZeroMemory(&resourceData, sizeof(resourceData));
	resourceData.pSysMem = verts;

	result = _device->CreateBuffer(&vDesc, &resourceData, &s_vertexBuffer);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Couldn't create the vertex buffer.");
		return false;
	}

	//RASTERIZER
	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(rasterDesc));
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_NONE; 
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;//D3D11_FILL_WIREFRAME  D3D11_FILL_SOLID
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	result = _device->CreateRasterizerState(&rasterDesc, &s_rasterizer);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Couldn't create the rasterizer state.");
		return false;
	}

	//texture... for now don't bother parameterising it
	// Load the texture in.
	using namespace DirectX;
	result = CreateDDSTextureFromFile(_device, L"../Resources/GRASS.dds", nullptr, &s_texture);
	if (FAILED(result))
	{
		MessageBox(0, "Failed to load ../Resources/GRASS.DDS", "Texture loading", MB_OK);
		return false;
	}

	//sampler
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	result = _device->CreateSamplerState(&samplerDesc, &s_samplerState);
	if (FAILED(result))
	{
		MessageBox(0, "Failed to create sampler state", "Texture sampler", MB_OK);
		return false;
	}

	return true;
}

void Field::unloadShared()
{
	s_shaders.release();
	RELEASE(s_rasterizer);
	RELEASE(s_inputLayout);
	RELEASE(s_vertexBuffer);
	RELEASE(s_texture);
	RELEASE(s_samplerState);
}


//bool Field::load(	ID3D11Device*		_device, 
//					int					_instanceCount, 
//					DirectX::XMFLOAT2	_size, 
//					DirectX::XMFLOAT3	_pos)
//{
//	m_maxInstanceCount = _instanceCount;
//	m_size = _size;
//	m_pos = _pos;
//
//	using namespace DirectX;
//	HRESULT result;
//
//	field::Instance* instances;
//	D3D11_BUFFER_DESC instanceBufferDesc;
//	D3D11_SUBRESOURCE_DATA instanceData;
//
//	// Create the instance array.
//	instances = generateInstanceData();
//
//	// Set up the description of the instance buffer.
//	instanceBufferDesc.Usage				= D3D11_USAGE_DEFAULT;
//	instanceBufferDesc.ByteWidth			= sizeof(field::Instance) * m_maxInstanceCount;
//	instanceBufferDesc.BindFlags			= D3D11_BIND_VERTEX_BUFFER;
//	instanceBufferDesc.CPUAccessFlags		= 0;
//	instanceBufferDesc.MiscFlags			= 0;
//	instanceBufferDesc.StructureByteStride	= 0;
//	// Give the subresource structure a pointer to the instance data.
//	instanceData.pSysMem = instances; 
//	instanceData.SysMemPitch = 0;
//	instanceData.SysMemSlicePitch = 0;
//
//	// Create the instance buffer.
//	result = _device->CreateBuffer(&instanceBufferDesc, &instanceData, &m_instanceBuffer);
//	if (FAILED(result))
//	{
//		MessageBox(0, "Error creating instance buffer.", "Field", MB_OK);
//		return false;
//	}
//
//	// Release the instance array now that the instance buffer has been created and loaded.
//	delete[] instances;
//	instances = nullptr;
//
//
//	return loadBuffers(_device);
//}


bool Field::load(ID3D11Device* _device, ObjModel* _model, float _density, DirectX::XMFLOAT3 _pos, const DirectX::XMFLOAT3& _minOctreeNodeSize)
{
	m_pos = _pos;

	using namespace DirectX;
	HRESULT result;

	/*Procedurally generate grass positions*/
	float truncationAccumulator = 0;
	float* vertElementPtr = _model->getVertices();
	float* normElementPtr = _model->getNormals();
	unsigned int numVerts = _model->getTotalVerts();
	m_maxInstanceCount = 0;

	XMFLOAT3 triVerts[3];
	XMFLOAT3 triNorms[3];
	XMVECTOR posVec = LF3(&m_pos);
	
	std::vector<field::Instance> field;

	/*For every triangle, currently testing faster implementation*/
	for (unsigned int i = 0; i < numVerts; i += 3)
	{
		/*Get vert positions*/
		memcpy(triVerts, vertElementPtr, sizeof(float) * 9);
		vertElementPtr += 9;
		/*Transform verts*///@For now just translate
		for (unsigned int i = 0; i < 3; ++i)
		{
			//XMVECTOR transformedVert = XMVector3Transform(LF3(&triVerts[i]), _transform);
			//XMStoreFloat3(&triVerts[i], transformedVert);
			XMStoreFloat3(&triVerts[i], posVec + LF3(&triVerts[i]));
		}

		/*Get vert normals*/
		if (normElementPtr)
		{
			memcpy(triNorms, normElementPtr, sizeof(float) * 9);
			normElementPtr += 9;
		}

		/*Store vert positions & calc surface area*/
		float sa = Triangle::surfaceArea((float*)triVerts);
		/*Calc number of blades*/
		unsigned int numBlades = (int)std::truncf(sa * _density);
		/*Deal with trunc rounding accumulation*/
		truncationAccumulator += std::fmodf(_density *  sa, 1.0f);
		if (truncationAccumulator >= 1.0f)
		{
			numBlades += (int)truncationAccumulator;
			truncationAccumulator -= (int)truncationAccumulator;
		}

		/*Add the patch to the field and octree*/
		addPatch(field,(float*)triVerts, numBlades);
		m_maxInstanceCount += numBlades;
	}


	/*Default light settings*/
	m_CBcpu_light.ambient = { 100.f/255.f, 100.f/255.f, 100/255.f, 0.0f };
	m_CBcpu_light.diffuse = { 120.f/255.f, 120.f/255.f, 100.f/255.f, 0.0f };
	m_CBcpu_light.specular = { 20/255.f, 20.f/255.f, 20.f/255.f, 0.0f };
	m_CBcpu_light.shiny = 8.0f;

	/*Set up GUI*/
	TwBar* GUI = TwNewBar("Field");
	TwDefine(" Settings position='10 10' ");
	TwDefine(" Settings size='300 200' ");
	TwDefine(" Settings movable= true ");
	TwDefine(" Settings resizable= true ");
	TwAddVarRW(GUI, "Ambient", TwType::TW_TYPE_COLOR4F, &m_CBcpu_light.ambient, "");
	TwAddVarRW(GUI, "Diffuse", TwType::TW_TYPE_COLOR4F, &m_CBcpu_light.diffuse, "");
	TwAddVarRW(GUI, "Specular", TwType::TW_TYPE_COLOR4F, &m_CBcpu_light.specular, "");
	TwAddVarRW(GUI, "Shiny", TwType::TW_TYPE_FLOAT, &m_CBcpu_light.shiny, "");


	//gpu octree@
	m_gpuOctree.build(*_model, VEC3(0, 0, 0), _minOctreeNodeSize, 1.0f, field, _device);

	///*build instance buffer...*/
	D3D11_BUFFER_DESC bufferDesc;
	D3D11_SUBRESOURCE_DATA initialData;
	D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;

	/*Create instance buffers*/
	ZeroMemory(&initialData, sizeof(initialData));
	initialData.pSysMem = field.data();

	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS |
							D3D11_BIND_SHADER_RESOURCE |
							D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(field::Instance) * m_maxInstanceCount;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

	//UAV
	ZeroMemory(&UAVDesc, sizeof(UAVDesc));
	UAVDesc.ViewDimension		= D3D11_UAV_DIMENSION_BUFFER;
	UAVDesc.Buffer.FirstElement = 0;
	UAVDesc.Format				= DXGI_FORMAT_R32_TYPELESS;
	UAVDesc.Buffer.NumElements	= m_maxInstanceCount * 12;//(48 (size of aligned instnace) / 4 (size of R32) = 12))
	UAVDesc.Buffer.Flags		= D3D11_BUFFER_UAV_FLAG_RAW;

	//SRV
	ZeroMemory(&SRVDesc, sizeof(SRVDesc));
	SRVDesc.ViewDimension			= D3D11_SRV_DIMENSION_BUFFEREX;
	SRVDesc.Format					= DXGI_FORMAT_R32_TYPELESS;
	SRVDesc.BufferEx.Flags			= D3D11_BUFFEREX_SRV_FLAG_RAW;
	SRVDesc.BufferEx.FirstElement	= 0;
	SRVDesc.BufferEx.NumElements	= m_maxInstanceCount * 12; //(48 (size of aligned instnace) / 4 (size of R32) = 12))

	m_instanceDoubleBuffer.init(_device, &bufferDesc, &initialData, &UAVDesc, &SRVDesc);

	//append buffer for new octree sys
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.BindFlags			= D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags		= 0;
	bufferDesc.Usage				= D3D11_USAGE_DEFAULT;
	bufferDesc.MiscFlags			= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
	bufferDesc.ByteWidth			= m_maxInstanceCount * sizeof(field::Instance);

	ZeroMemory(&UAVDesc, sizeof(UAVDesc));
	UAVDesc.ViewDimension			= D3D11_UAV_DIMENSION_BUFFER;
	UAVDesc.Buffer.FirstElement		= 0;
	UAVDesc.Format					= DXGI_FORMAT_R32_TYPELESS;
	UAVDesc.Buffer.NumElements		= m_maxInstanceCount * 12;//instances * (48 (size of aligned instnace) / 4 (size of R32) = 12))
	UAVDesc.Buffer.Flags			= D3D11_BUFFER_UAV_FLAG_RAW;
	m_pseudoAppend.init(_device, &bufferDesc, NULL, &UAVDesc, NULL);

	//indirect args
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.BindFlags			= D3D11_BIND_UNORDERED_ACCESS;
	bufferDesc.CPUAccessFlags		= 0;
	bufferDesc.Usage				= D3D11_USAGE_DEFAULT;
	bufferDesc.MiscFlags			= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
	bufferDesc.ByteWidth			= 4 * sizeof(unsigned int);
	bufferDesc.StructureByteStride	= sizeof(unsigned int);

	ZeroMemory(&UAVDesc, sizeof(UAVDesc));
	UAVDesc.ViewDimension			= D3D11_UAV_DIMENSION_BUFFER;
	UAVDesc.Buffer.FirstElement		= 0;
	UAVDesc.Format					= DXGI_FORMAT_R32_UINT;
	UAVDesc.Buffer.NumElements		= 4;
	UAVDesc.Buffer.Flags			= 0;
	m_indirectArgs.init(_device, &bufferDesc, NULL, &UAVDesc, NULL);

	return loadBuffers(_device);
}

void Field::unload()
{
	m_gpuOctree.cleanup();
	RELEASE(m_CB_geometry);
	RELEASE(m_CB_viewproj);
	RELEASE(m_CB_light);
	m_instanceDoubleBuffer.cleanup();
	m_pseudoAppend.cleanup();
	m_indirectArgs.cleanup();
}


void Field::draw(const DrawData& _data)
{
	using namespace DirectX;	
	updateConstBuffers(_data);

	/*Frustum culling@ new octree*/
	XMMATRIX		transform = XMMatrixMultiply(XMMatrixRotationRollPitchYawFromVector(LF3(&_data.m_cam->getRot())), XMMatrixTranslationFromVector(LF3(&_data.m_cam->getPos())));
	BoundingFrustum frustum(_data.m_cam->calcLargeProjMatrix());
	frustum.Transform(frustum, transform);
	m_gpuOctree.frustumCull(frustum, m_noCulling);
	m_gpuOctree.updateResources(_data);
	
	/*Draw Octree*/
	if (m_drawGPUOctree) m_gpuOctreeDebugger.draw(_data.m_dc, _data.m_cam->getViewProj(), m_gpuOctree);

	//@@@refresh indirect args buffer
	unsigned int indirectArgsReset[] = { 4, 0, 0, 0/*, m_maxInstanceCount -1 */};
	_data.m_dc->UpdateSubresource(m_indirectArgs.getBuffer(), NULL, NULL, indirectArgsReset, sizeof(unsigned int)*ARRAYSIZE(indirectArgsReset), 0);

	/*Update wind resources*/
	m_windManager->updateResources(_data.m_dc, m_maxInstanceCount);
	//Dispatch the wind update //@with new octree
	m_windManager->applyWindForces(m_instanceDoubleBuffer.back()->getUAV() , m_pseudoAppend.getUAV(), m_indirectArgs.getUAV(), m_instanceDoubleBuffer.front()->getSRV(), m_gpuOctree.getTreeBuffer().getSRV(),_data.m_dc, m_maxInstanceCount);
	//The resource is now already on the GPU
	m_instanceDoubleBuffer.swap();
	

	/*Draw field*/
	unsigned unsigned int strides[2];
	unsigned unsigned int offsets[2];
	ID3D11Buffer* bufferPointers[2];

	// Set the buffer strides.
	strides[0] = sizeof(field::Vertex);
	strides[1] = sizeof(field::Instance);
	// Set the buffer offsets.
	offsets[0] = 0;
	offsets[1] = 0;
	// Set the array of pointers to the vertex and instance buffers.
	bufferPointers[0] = s_vertexBuffer;
	bufferPointers[1] = /*m_instanceDoubleBuffer.front()->getBuffer(); */ m_pseudoAppend.getBuffer(); //@trying out ignoring the pseudo append buffer

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	_data.m_dc->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);
	_data.m_dc->IASetInputLayout(s_inputLayout);
	_data.m_dc->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, NULL);
	_data.m_dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	//update rasterizer
	_data.m_dc->RSSetState(s_rasterizer);

	//apply shaders
	s_shaders.apply(_data.m_dc);

	//update subresources
	_data.m_dc->UpdateSubresource(m_CB_viewproj, 0, 0, &m_CBcpu_viewproj, 0, 0);
	_data.m_dc->UpdateSubresource(m_CB_geometry, 0, 0, &m_CBcpu_geometry, 0, 0);
	_data.m_dc->UpdateSubresource(m_CB_light, 0, 0, &m_CBcpu_light, 0, 0);

	//set constant buffers
	_data.m_dc->HSSetConstantBuffers(0, 1, &m_CB_geometry);
	_data.m_dc->GSSetConstantBuffers(0, 1, &m_CB_geometry);
	_data.m_dc->DSSetConstantBuffers(0, 1, &m_CB_geometry);
	_data.m_dc->VSSetConstantBuffers(0, 1, &m_CB_geometry);
	_data.m_dc->VSSetConstantBuffers(1, 1, &m_CB_viewproj);
	_data.m_dc->GSSetConstantBuffers(1, 1, &m_CB_viewproj);
	_data.m_dc->GSSetConstantBuffers(2, 1, &m_CB_light);
	_data.m_dc->PSSetConstantBuffers(2, 1, &m_CB_light);
	_data.m_dc->VSSetConstantBuffers(2, 1, &m_CB_light);

	//sampler
	_data.m_dc->PSSetSamplers(0, 1, &s_samplerState);
	//texture
	_data.m_dc->PSSetShaderResources(0, 1, &s_texture);
	/*Draw call*/
	//_data.m_dc->DrawInstanced(4, m_curInstanceCount, 0, 0);
	_data.m_dc->DrawInstancedIndirect(m_indirectArgs.getBuffer(), 0);

	/*Cleanup*/
	bufferPointers[0] = nullptr;
	bufferPointers[1] = nullptr;
	strides[0] = 0;
	strides[1] = 0;
	_data.m_dc->IASetVertexBuffers(0, ARRAYSIZE(bufferPointers), bufferPointers, strides, offsets);
	_data.m_dc->IASetInputLayout(nullptr);
}

void Field::updateConstBuffers(const DrawData& _data)
{
	using namespace DirectX;
	Time* t = OCH::ServiceLocator<Time>::get();

	XMFLOAT3 camPos = _data.m_cam->getPos();


	m_CBcpu_geometry.halfGrassWidth = m_halfGrassWidth;
	m_CBcpu_geometry.time = (float)t->time;
	m_CBcpu_geometry.farTess = 6.0f;
	m_CBcpu_geometry.nearTess = 0.4f;
	m_CBcpu_geometry.minTessDensity = 3.0f;
	m_CBcpu_geometry.maxTessDensity = 9.0f;

	//view projection buffer
	XMMATRIX viewproj = XMMatrixTranspose(XMLoadFloat4x4(&_data.m_cam->getViewProj()));
	XMStoreFloat4x4(&m_CBcpu_viewproj.m_wvp, viewproj);
	
	/*light*/
	m_CBcpu_light.camera = XMFLOAT4(camPos.x, camPos.y, camPos.z, 1.f);
	STOREF4(&m_CBcpu_light.light, XMVectorMultiply(_data.m_cam->calcViewDir(), VEC3(-1, -1, -1)));
}

bool Field::loadBuffers(ID3D11Device* _device)
{
	//CBUFFER
	//Geometry buffer
	HRESULT result;
	D3D11_BUFFER_DESC bufferdesc;
	ZeroMemory(&bufferdesc, sizeof(bufferdesc));
	bufferdesc.Usage = D3D11_USAGE_DEFAULT;
	bufferdesc.ByteWidth = sizeof(CBField);
	bufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferdesc.CPUAccessFlags = 0;
	bufferdesc.MiscFlags = 0;
	bufferdesc.StructureByteStride = 0;
	result = _device->CreateBuffer(&bufferdesc, NULL, &m_CB_geometry);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Couldn't create the geometry const buffer.");
		return false;
	}

	//World view proj
	ZeroMemory(&bufferdesc, sizeof(bufferdesc));
	bufferdesc.Usage = D3D11_USAGE_DEFAULT;
	bufferdesc.ByteWidth = sizeof(CBWorldViewProj);
	bufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferdesc.CPUAccessFlags = 0;
	bufferdesc.MiscFlags = 0;
	bufferdesc.StructureByteStride = 0;
	result = _device->CreateBuffer(&bufferdesc, NULL, &m_CB_viewproj);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Error: Couldn't create the world const buffer.");
		return false;
	}

	//Light
	ZeroMemory(&bufferdesc, sizeof(bufferdesc));
	bufferdesc.Usage				= D3D11_USAGE_DEFAULT;
	bufferdesc.ByteWidth			= sizeof(CBFieldLight);
	bufferdesc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
	bufferdesc.CPUAccessFlags		= 0;
	bufferdesc.MiscFlags			= 0;
	bufferdesc.StructureByteStride	= 0;
	result = _device->CreateBuffer(&bufferdesc, NULL, &m_CB_light);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Error: Couldn't create the light const buffer.");
		return false;
	}
	return true;
}

void Field::addPatch(std::vector<field::Instance>& _field, float* verts, unsigned int _numBlades)
{
	using namespace DirectX;
	
	//RNG
	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(0, 1);
	std::uniform_real_distribution<float> angleDistribution(0, 2 * XM_PI);
	auto rand = std::bind(distribution, generator);
	auto randAngle = std::bind(angleDistribution, generator);

	XMVECTOR a = VEC3(*(verts), *(verts + 1), *(verts + 2));
	XMVECTOR b = VEC3(*(verts + 3), *(verts + 4), *(verts + 5));
	XMVECTOR c = VEC3(*(verts + 6), *(verts + 7), *(verts + 8));

	//Use barycentric coordinates [Orthmann] to place grass
	for (unsigned int i = 0; i < _numBlades; ++i)
	{
		field::Instance instance;

		/*Barycentric coords*/ //See paper and orthman for barycentric sampling
		float u = rand();
		float v = rand();

		//http://cgg.mff.cuni.cz/~jaroslav/papers/2013-meshsampling/2013-meshsampling-paper.pdf
		//https://classes.soe.ucsc.edu/cmps160/Fall10/resources/barycentricInterpolation.pdf
		//http://math.stackexchange.com/questions/18686/uniform-random-point-in-triangle
		//http://chrischoy.github.io/research/barycentric-coordinate-for-mesh-sampling/ 
		XMVECTOR translation =	(1 - sqrt(u)) * a +
								sqrt(u) * (1 - v) * b +
								sqrt(u) * v * c;

		/*Rotation*/
		float angle = randAngle();
		XMVECTOR euler = XMLoadFloat3(&XMFLOAT3(0, angle, 0));
		XMVECTOR quat = XMQuaternionRotationRollPitchYawFromVector(euler);
		XMStoreFloat4(&instance.rotation, quat);

		/*Translation*/
		XMStoreFloat3(&instance.location, translation);
		instance.wind = { 0 ,0, 0 };

		/*Add to octree*/
		_field.push_back(instance);
	}
}

#undef MAX(x,y)