//#include "Field_14_03.h"
//#include <d3dcompiler.h>
//#include "dxerr.h"
//#include <DirectXMath.h>
//#include "DXHelper.h"
//#include "Input.h"
//#include "Time.h"
//#include "ServiceLocator.h"
//#include "DDSTextureLoader.h"
//#include <random>
//#include "Camera.h"
//#include <functional>
//#include <vector>
//#include "objLoader.h"
//#include <stdlib.h>
//#include "Shorthand.h"
//#include "FieldUtilities.h"
//
//field::Shaders			Field_14_03::s_shaders = field::Shaders();
//ID3D11RasterizerState*	Field_14_03::s_rasterizer = nullptr;
//ID3D11InputLayout*		Field_14_03::s_inputLayout = nullptr;
//ID3D11Buffer*			Field_14_03::s_vertexBuffer = nullptr;
//ID3D11ShaderResourceView* Field_14_03::s_texture = nullptr;
//ID3D11SamplerState*		Field_14_03::s_samplerState = nullptr;
//ID3D11ComputeShader*	Field_14_03::s_csWind = nullptr;
//
//Field_14_03::Field_14_03()
//{
//}
//
//Field_14_03::~Field_14_03()
//{
//}
//
//bool Field_14_03::loadShared(ID3D11Device* _device)
//{
//	HRESULT result;
//	ID3DBlob* buffer = nullptr;
//	unsigned int shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
//#if defined(DEBUG) || defined(_DEBUG)
//	shaderFlags |= D3DCOMPILE_DEBUG;
//#endif
//
//	//VS SHADER
//	result = D3DCompileFromFile(L"Field.fx", NULL, NULL, "VS_Main", "vs_5_0", shaderFlags, 0, &buffer, NULL);
//	if (FAILED(result))
//	{
//		RELEASE(buffer);
//		MessageBox(0, "Error loading vertex shader.", "Shader compilation", MB_OK);
//		return false;
//	}
//	_device->CreateVertexShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), 0, &s_shaders.m_vs);
//	if (FAILED(result))
//	{
//		RELEASE(buffer);
//		DXTRACE_MSG(L"Couldn't create the vertex shader.");
//		return false;
//	}
//
//	//INPUT LAYOUT
//	D3D11_INPUT_ELEMENT_DESC vsLayout[] =
//	{
//		//PER_VERTEX
//		{ "SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,D3D11_INPUT_PER_VERTEX_DATA, 0 },
//		{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//		{ "FLEX", 0, DXGI_FORMAT_R32_FLOAT ,0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//		//PER_INSTANCE
//		{ "INSTANCE_ROTATION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
//		{ "INSTANCE_LOCATION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
//		{ "INSTANCE_WIND", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 28, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
//	};
//	unsigned unsigned int totalLayoutElements = ARRAYSIZE(vsLayout);
//
//	result = _device->CreateInputLayout(vsLayout, totalLayoutElements, buffer->GetBufferPointer(), buffer->GetBufferSize(), &s_inputLayout);
//	RELEASE(buffer);
//	if (FAILED(result))
//	{
//		DXTRACE_MSG(L"Couldn't create the input layout.");
//		return false;
//	}
//
//	//PS SHADER
//	result = D3DCompileFromFile(L"Field.fx", NULL, NULL, "PS_Main", "ps_5_0", shaderFlags, 0, &buffer, NULL);
//	if (FAILED(result))
//	{
//		RELEASE(buffer);
//		MessageBox(0, "Error loading pixel shader.", "Shader compilation", MB_OK);
//		return false;
//	}
//
//	result = _device->CreatePixelShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), 0, &s_shaders.m_ps);
//	RELEASE(buffer);
//	if (FAILED(result))
//	{
//		DXTRACE_MSG(L"Couldn't create the pixel shader.");
//		return false;
//	}
//
//	//GS SHADER
//	result = D3DCompileFromFile(L"Field.fx", NULL, NULL, "GS_Main", "gs_5_0", shaderFlags, 0, &buffer, NULL);
//	if (FAILED(result))
//	{
//		RELEASE(buffer);
//		MessageBox(0, "Error loading geometry shader.", "Shader compilation", MB_OK);
//		return false;
//	}
//
//	result = _device->CreateGeometryShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), 0, &s_shaders.m_gs);
//	RELEASE(buffer);
//	if (FAILED(result))
//	{
//		DXTRACE_MSG(L"Couldn't create the geometry shader.");
//		return false;
//	}
//
//	//HS SHADER
//	result = D3DCompileFromFile(L"Field.fx", NULL, NULL, "HS_Main", "hs_5_0", shaderFlags, 0, &buffer, NULL);
//	if (FAILED(result))
//	{
//		RELEASE(buffer);
//		MessageBox(0, "Error loading hull shader.", "Shader compilation", MB_OK);
//		return false;
//	}
//
//	result = _device->CreateHullShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), 0, &s_shaders.m_hs);
//	RELEASE(buffer);
//	if (FAILED(result))
//	{
//		DXTRACE_MSG(L"Couldn't create the hull shader.");
//		return false;
//	}
//
//	//DS SHADER
//	result = D3DCompileFromFile(L"Field.fx", NULL, NULL, "DS_Main", "ds_5_0", shaderFlags, 0, &buffer, NULL);
//	if (FAILED(result))
//	{
//		RELEASE(buffer);
//		MessageBox(0, "Error loading domain shader.", "Shader compilation", MB_OK);
//		return false;
//	}
//
//	result = _device->CreateDomainShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), 0, &s_shaders.m_ds);
//	RELEASE(buffer);
//	if (FAILED(result))
//	{
//		DXTRACE_MSG(L"Couldn't create the domain shader.");
//		return false;
//	}
//
//	//CS
//	ID3DBlob* errors=nullptr;
//	result = D3DCompileFromFile(L"GrassWindFrustumCS.hlsl", NULL, NULL, "main", "cs_5_0", shaderFlags, 0, &buffer, &errors);
//	if (FAILED(result))
//	{
//		RELEASE(buffer);
//		if (errors)
//		{
//			OutputDebugStringA((char*)errors->GetBufferPointer());
//			errors->Release();
//		}
//		MessageBox(0, "Error loading compute shader.", "Shader compilation", MB_OK);
//		return false;
//	}
//	_device->CreateComputeShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), 0, &s_csWind);
//	if (FAILED(result))
//	{
//		RELEASE(buffer);
//		MessageBox(0, "Couldn't create the compute shader.", "Creating shader", MB_OK);
//		return false;
//	}
//
//	RELEASE(buffer);
//
//	//VERTEX DATA
//	field::Vertex verts[] =
//	{
//		//pos, binormal, normal, flex																							//flex
//		{ DirectX::XMFLOAT3(0.f, 0.f, 0.f),   DirectX::XMFLOAT3(1.f, 0.f, 0.f), DirectX::XMFLOAT3(0.f, 0.f, -1.f), 0.f },		//0
//		{ DirectX::XMFLOAT3(0.f, 0.2f, 0.f),  DirectX::XMFLOAT3(1.f, 0.f, 0.f), DirectX::XMFLOAT3(0.f, 0.f, -1.f), 0.1f },	//0.111
//		{ DirectX::XMFLOAT3(0.f, 0.4f, 0.f),  DirectX::XMFLOAT3(1.f, 0.f, 0.f), DirectX::XMFLOAT3(0.f, 0.f, -1.f), 0.3f },	//0.445
//		{ DirectX::XMFLOAT3(0.0f, 0.6f, 0.08f),  DirectX::XMFLOAT3(1.f, 0.f, 0.f), DirectX::XMFLOAT3(0.f, 0.f, -1.f), 1.f }		//1
//	};
//
//	D3D11_BUFFER_DESC vDesc;
//	ZeroMemory(&vDesc, sizeof(vDesc));
//	vDesc.Usage = D3D11_USAGE_DEFAULT;
//	vDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//	vDesc.ByteWidth = sizeof(field::Vertex) * 4;
//
//	D3D11_SUBRESOURCE_DATA resourceData;
//	ZeroMemory(&resourceData, sizeof(resourceData));
//	resourceData.pSysMem = verts;
//
//	result = _device->CreateBuffer(&vDesc, &resourceData, &s_vertexBuffer);
//	if (FAILED(result))
//	{
//		DXTRACE_MSG(L"Couldn't create the vertex buffer.");
//		return false;
//	}
//
//	//RASTERIZER
//	D3D11_RASTERIZER_DESC rasterDesc;
//	ZeroMemory(&rasterDesc, sizeof(rasterDesc));
//	rasterDesc.AntialiasedLineEnable = false;
//	rasterDesc.CullMode = D3D11_CULL_NONE;
//	rasterDesc.DepthBias = 0;
//	rasterDesc.DepthBiasClamp = 0.0f;
//	rasterDesc.DepthClipEnable = true;
//	rasterDesc.FillMode = D3D11_FILL_SOLID;//D3D11_FILL_WIREFRAME  D3D11_FILL_SOLID
//	rasterDesc.FrontCounterClockwise = false;
//	rasterDesc.MultisampleEnable = false;
//	rasterDesc.ScissorEnable = false;
//	rasterDesc.SlopeScaledDepthBias = 0.0f;
//
//	// Create the rasterizer state from the description we just filled out.
//	result = _device->CreateRasterizerState(&rasterDesc, &s_rasterizer);
//	if (FAILED(result))
//	{
//		DXTRACE_MSG(L"Couldn't create the rasterizer state.");
//		return false;
//	}
//
//	//texture... for now don't bother parameterising it
//	// Load the texture in.
//	using namespace DirectX;
//	result = CreateDDSTextureFromFile(_device, L"../Resources/GRASS.dds", nullptr, &s_texture);
//	if (FAILED(result))
//	{
//		MessageBox(0, "Failed to load ../Resources/GRASS.DDS", "Texture loading", MB_OK);
//		return false;
//	}
//
//	//sampler
//	D3D11_SAMPLER_DESC samplerDesc;
//	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
//	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
//	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
//	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
//	samplerDesc.MipLODBias = 0.0f;
//	samplerDesc.MaxAnisotropy = 1;
//	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
//	samplerDesc.BorderColor[0] = 0;
//	samplerDesc.BorderColor[1] = 0;
//	samplerDesc.BorderColor[2] = 0;
//	samplerDesc.BorderColor[3] = 0;
//	samplerDesc.MinLOD = 0;
//	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
//
//	// Create the texture sampler state.
//	result = _device->CreateSamplerState(&samplerDesc, &s_samplerState);
//	if (FAILED(result))
//	{
//		MessageBox(0, "Failed to create sampler state", "Texture sampler", MB_OK);
//		return false;
//	}
//
//
//
//	return true;
//}
//
//void Field_14_03::unloadShared()
//{
//	s_shaders.release();
//	RELEASE(s_rasterizer);
//	RELEASE(s_inputLayout);
//	RELEASE(s_vertexBuffer);
//	RELEASE(s_texture);
//	RELEASE(s_samplerState);
//	RELEASE(s_csWind);
//}
//
//bool Field_14_03::load(ID3D11Device* _device, ObjModel* _model, float _density)
//{
//	/*Generate the field of grass*/
//	std::vector<field::Instance> instances;
//	field::generateField(_device, _model, _density, instances);
//	m_maxInstanceCount = instances.size();
//
//	/*Build the Instance (double) buffer*/
//	D3D11_BUFFER_DESC bufferDesc;
//	D3D11_SUBRESOURCE_DATA instanceData;
//	instanceData.pSysMem		 = instances.data();
//	instanceData.SysMemPitch	 = 0;
//	instanceData.SysMemSlicePitch= 0;
//
//	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
//	bufferDesc.BindFlags		=	D3D11_BIND_UNORDERED_ACCESS |
//									D3D11_BIND_SHADER_RESOURCE |
//									D3D11_BIND_VERTEX_BUFFER;
//	bufferDesc.ByteWidth		= sizeof(field::Instance) * m_maxInstanceCount;
//	bufferDesc.MiscFlags		= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
//
//	D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc;
//	ZeroMemory(&UAVDesc, sizeof(UAVDesc));
//	UAVDesc.ViewDimension		= D3D11_UAV_DIMENSION_BUFFER;
//	UAVDesc.Buffer.FirstElement = 0;
//	UAVDesc.Format				= DXGI_FORMAT_R32_TYPELESS;
//	UAVDesc.Buffer.NumElements	= m_maxInstanceCount * 12;
//	UAVDesc.Buffer.Flags		= D3D11_BUFFER_UAV_FLAG_RAW;
//
//	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
//	ZeroMemory(&SRVDesc, sizeof(SRVDesc));
//	SRVDesc.ViewDimension		= D3D11_SRV_DIMENSION_BUFFEREX;
//	SRVDesc.Format				= DXGI_FORMAT_R32_TYPELESS;
//	SRVDesc.BufferEx.Flags		= D3D11_BUFFEREX_SRV_FLAG_RAW;
//	SRVDesc.BufferEx.FirstElement = 0;
//	SRVDesc.BufferEx.NumElements= m_maxInstanceCount * 12;
//
//	/*Set up instance buffers*/
//	CHECK_FAIL(m_instanceDoubleBuffer.init(_device, &bufferDesc, &instanceData, &UAVDesc, &SRVDesc));
//
//	/*CBuffers - Immutable*/
//	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
//	bufferDesc.Usage			= D3D11_USAGE_IMMUTABLE;
//	bufferDesc.BindFlags		= D3D11_BIND_CONSTANT_BUFFER;
//	bufferDesc.CPUAccessFlags	= 0;
//	bufferDesc.MiscFlags		= 0;
//	bufferDesc.StructureByteStride = 0;
//	
//	m_CB_CS_Consts.maxInstances	= m_maxInstanceCount;
//
//	CHECK_FAIL(m_CB_CS_Consts.init(_device, &bufferDesc));
//
//	/*CBuffers - Default*/
//	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
//	CHECK_FAIL(m_CB_CS_RW.init(_device, &bufferDesc));
//
//	/*CBuffers - Dynamic*/
//	bufferDesc.Usage			= D3D11_USAGE_DYNAMIC;
//	bufferDesc.CPUAccessFlags	= D3D10_CPU_ACCESS_WRITE;
//	CHECK_FAIL(m_CB_CS_RO.init(_device, &bufferDesc));
//
//	/*Default light settings*/
//	m_CB_light.ambient = { 100.f / 255.f, 100.f / 255.f, 100 / 255.f, 0.0f };
//	m_CB_light.diffuse = { 120.f / 255.f, 120.f / 255.f, 100.f / 255.f, 0.0f };
//	m_CB_light.specular = { 20 / 255.f, 20.f / 255.f, 20.f / 255.f, 0.0f };
//	m_CB_light.shiny = 8.0f;
//	CHECK_FAIL(m_CB_light.init(_device, &bufferDesc));
//	CHECK_FAIL(m_CB_viewproj.init(_device, &bufferDesc));
//	CHECK_FAIL(m_CB_geometry.init(_device, &bufferDesc));
//
//	//To-Do@ Create the shader's shared mem (front & back idx)
//	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
//	bufferDesc.BindFlags =	D3D11_BIND_UNORDERED_ACCESS |
//									D3D11_BIND_SHADER_RESOURCE |
//									D3D11_BIND_VERTEX_BUFFER;
//	bufferDesc.ByteWidth = sizeof(field::Instance) * m_maxInstanceCount;
//	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
//
//	ZeroMemory(&UAVDesc, sizeof(UAVDesc));
//	UAVDesc.ViewDimension		= D3D11_UAV_DIMENSION_BUFFER;
//	UAVDesc.Buffer.FirstElement = 0;
//	UAVDesc.Format				= DXGI_FORMAT_R32_TYPELESS;
//	UAVDesc.Buffer.NumElements	= m_maxInstanceCount * 12;
//	UAVDesc.Buffer.Flags		= D3D11_BUFFER_UAV_FLAG_RAW;
//
//	ZeroMemory(&SRVDesc, sizeof(SRVDesc));
//	SRVDesc.ViewDimension		= D3D11_SRV_DIMENSION_BUFFEREX;
//	SRVDesc.Format				= DXGI_FORMAT_R32_TYPELESS;
//	SRVDesc.BufferEx.Flags		= D3D11_BUFFEREX_SRV_FLAG_RAW;
//	SRVDesc.BufferEx.FirstElement = 0;
//	SRVDesc.BufferEx.NumElements= m_maxInstanceCount * 12;
//
//	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
//	bufferDesc.BindFlags =	D3D11_BIND_UNORDERED_ACCESS |
//									D3D11_BIND_SHADER_RESOURCE |
//									D3D11_BIND_VERTEX_BUFFER;
//	bufferDesc.ByteWidth = sizeof(field::Instance) * m_maxInstanceCount;
//	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
//
//	ZeroMemory(&UAVDesc, sizeof(UAVDesc));
//	UAVDesc.ViewDimension		= D3D11_UAV_DIMENSION_BUFFER;
//	UAVDesc.Buffer.FirstElement = 0;
//	UAVDesc.Format				= DXGI_FORMAT_R32_TYPELESS;
//	UAVDesc.Buffer.NumElements	= m_maxInstanceCount * 12;
//	UAVDesc.Buffer.Flags		= D3D11_BUFFER_UAV_FLAG_RAW;
//
//	ZeroMemory(&SRVDesc, sizeof(SRVDesc));
//	SRVDesc.ViewDimension		= D3D11_SRV_DIMENSION_BUFFEREX;
//	SRVDesc.Format				= DXGI_FORMAT_R32_TYPELESS;
//	SRVDesc.BufferEx.Flags		= D3D11_BUFFEREX_SRV_FLAG_RAW;
//	SRVDesc.BufferEx.FirstElement = 0;
//	SRVDesc.BufferEx.NumElements= m_maxInstanceCount * 12;
//
//	/*CS shared mem buffer*/
//	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
//	bufferDesc.BindFlags = D3D11_USAGE_DEFAULT | D3D11_BIND_UNORDERED_ACCESS;
//	bufferDesc.StructureByteStride = sizeof(unsigned int);
//	bufferDesc.ByteWidth = sizeof(unsigned int) * 2;
//	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
//	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//
//	ZeroMemory(&UAVDesc, sizeof(UAVDesc));
//	UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
//	UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
//	UAVDesc.Buffer.FirstElement = 0;
//	UAVDesc.Buffer.NumElements = 2;
//	UAVDesc.Buffer.Flags = 0;
//	m_CSAddressIdicies.init(_device, &bufferDesc, NULL, &UAVDesc, NULL);
//
//
//	return true;
//}
//
//void Field_14_03::unload()
//{
//	m_CSAddressIdicies.cleanup();
//	m_instanceDoubleBuffer.cleanup();
//	m_CB_CS_RO.cleanup();
//	m_CB_CS_RW.cleanup();
//}
//
//void Field_14_03::draw(const DrawData& _data)
//{
//	updateConstBuffers(_data);
//	
//	ID3D11ShaderResourceView*	CS_SRV[] = { m_instanceDoubleBuffer.front()->getSRV(),
//											m_windManager->getCuboidBuffer().getSRV(),    
//											m_windManager->getSphereBuffer().getSRV() };
//	ID3D11UnorderedAccessView*	CS_UAV[] = { m_instanceDoubleBuffer.back()->getUAV(), m_CSAddressIdicies.getUAV() };
//	ID3D11Buffer* constbuffers[] = { m_CB_CS_RW.getBuffer(), m_CB_CS_RO.getBuffer(), m_CB_CS_Consts.getBuffer() };
//	_data.m_dc->CSSetShaderResources(0, ARRAYSIZE(CS_SRV), CS_SRV);
//	_data.m_dc->CSSetUnorderedAccessViews(0, ARRAYSIZE(CS_UAV), CS_UAV, 0);
//	_data.m_dc->CSSetShader(s_csWind, NULL, 0);
//	_data.m_dc->CSSetConstantBuffers(0, ARRAYSIZE(constbuffers), constbuffers);
//
//	_data.m_dc->Dispatch((unsigned int)ceil((float)m_maxInstanceCount / (float)m_threadsPerGroupX), 1, 1);
//
//	m_instanceDoubleBuffer.swap();
//
//	NULLIFY_STATIC_ARRAY_OF_PTR(CS_SRV);
//	NULLIFY_STATIC_ARRAY_OF_PTR(CS_UAV);
//	NULLIFY_STATIC_ARRAY_OF_PTR(constbuffers);
//	_data.m_dc->CSSetShaderResources(0, ARRAYSIZE(CS_SRV), CS_SRV);
//	_data.m_dc->CSSetUnorderedAccessViews(0, ARRAYSIZE(CS_UAV), CS_UAV, 0);
//	_data.m_dc->CSSetShader(nullptr, NULL, 0);
//	_data.m_dc->CSSetConstantBuffers(0, ARRAYSIZE(constbuffers), constbuffers);
//
//
//	unsigned unsigned int strides[2];
//	unsigned unsigned int offsets[2];
//	ID3D11Buffer* bufferPointers[2];
//	// Set the buffer strides.
//	strides[0] = sizeof(field::Vertex);
//	strides[1] = sizeof(field::Instance);
//	// Set the buffer offsets.
//	offsets[0] = 0;
//	offsets[1] = 0;
//	// Set the array of pointers to the vertex and instance buffers.
//	bufferPointers[0] = s_vertexBuffer;
//	bufferPointers[1] = m_instanceDoubleBuffer.front()->getBuffer(); 
//
//	// Set the vertex buffer to active in the input assembler so it can be rendered.
//	_data.m_dc->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);
//	_data.m_dc->IASetInputLayout(s_inputLayout);
//	_data.m_dc->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, NULL);
//	_data.m_dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
//
//	//update rasterizer
//	_data.m_dc->RSSetState(s_rasterizer);
//
//	//apply shaders
//	s_shaders.apply(_data.m_dc);
//
//	//set constant buffers
//	ID3D11Buffer* VSBuffers[] = { m_CB_geometry.getBuffer(), m_CB_viewproj.getBuffer(), m_CB_light.getBuffer() };
//	ID3D11Buffer* HSBuffers[] = { m_CB_geometry.getBuffer(), nullptr, nullptr };
//	ID3D11Buffer* DSBuffers[] = { m_CB_geometry.getBuffer(), nullptr, nullptr };
//	ID3D11Buffer* GSBuffers[] = { m_CB_geometry.getBuffer(), m_CB_viewproj.getBuffer(), m_CB_light.getBuffer() };
//	ID3D11Buffer* PSBuffers[] = { nullptr, nullptr, m_CB_light.getBuffer() };
//	_data.m_dc->VSSetConstantBuffers(0, ARRAYSIZE(VSBuffers), VSBuffers);
//	_data.m_dc->HSSetConstantBuffers(0, ARRAYSIZE(HSBuffers), HSBuffers);
//	_data.m_dc->DSSetConstantBuffers(0, ARRAYSIZE(DSBuffers), DSBuffers);
//	_data.m_dc->GSSetConstantBuffers(0, ARRAYSIZE(GSBuffers), GSBuffers);
//	_data.m_dc->PSSetConstantBuffers(0, ARRAYSIZE(PSBuffers), PSBuffers);
//
//	//sampler
//	_data.m_dc->PSSetSamplers(0, 1, &s_samplerState);
//	//texture
//	_data.m_dc->PSSetShaderResources(0, 1, &s_texture);
//	/*Draw call*/
//	//@for now just render all of them 
//	m_curInstanceCount = m_maxInstanceCount;
//	_data.m_dc->DrawInstanced(4, m_curInstanceCount, 0, 0);
//
//	/*Cleanup*/
//	bufferPointers[0] = nullptr;
//	bufferPointers[1] = nullptr;
//	strides[0] = 0;
//	strides[1] = 0;
//	_data.m_dc->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);
//	_data.m_dc->IASetInputLayout(nullptr);
//
//	VSBuffers[0] = VSBuffers[1] = VSBuffers[2] = nullptr;
//	HSBuffers[0] = HSBuffers[1] = HSBuffers[2] = nullptr;
//	DSBuffers[0] = DSBuffers[1] = DSBuffers[2] = nullptr;
//	GSBuffers[0] = GSBuffers[1] = GSBuffers[2] = nullptr;
//	PSBuffers[0] = PSBuffers[1] = PSBuffers[2] = nullptr;
//
//	_data.m_dc->VSSetConstantBuffers(0, ARRAYSIZE(VSBuffers), VSBuffers);
//	_data.m_dc->HSSetConstantBuffers(0, ARRAYSIZE(HSBuffers), HSBuffers);
//	_data.m_dc->DSSetConstantBuffers(0, ARRAYSIZE(DSBuffers), DSBuffers);
//	_data.m_dc->GSSetConstantBuffers(0, ARRAYSIZE(GSBuffers), GSBuffers);
//	_data.m_dc->PSSetConstantBuffers(0, ARRAYSIZE(PSBuffers), PSBuffers);
//
//}
//
//void Field_14_03::updateConstBuffers(const DrawData& _data)
//{
//	/*CS Buffers*/
//	m_CB_CS_RO.deltaTime	= static_cast<float>(_data.m_deltaTime);
//	m_CB_CS_RO.time			= static_cast<float>(_data.m_time);
//	m_CB_CS_RO.numCuboids	= m_windManager->getCuboids().size();
//	m_CB_CS_RO.numSpheres	= m_windManager->getSpheres().size();
//	//m_CBcpu_CSRO.frustum	= frustum; To-Do
//	m_CB_CS_RO.mapUpdate(_data.m_dc);
//
//	/*Don't need to touch m_CB_CS_RW because it gets updated in the CS*/
//
//	/*view-projection*/
//	DirectX::XMMATRIX viewproj = XMMatrixTranspose(XMLoadFloat4x4(&_data.m_cam->getViewProj()));
//	DirectX::XMStoreFloat4x4(&m_CB_viewproj.m_wvp, viewproj);
//	m_CB_viewproj.mapUpdate(_data.m_dc);
//
//	/*Geometry stuff (and other random things)*/
//	m_CB_geometry.halfGrassWidth = m_halfGrassWidth;
//	m_CB_geometry.time = 0; //Need this to not change for the new field stuffs, ill remove if it works i guess? static_cast<float>(_data.m_time);
//	m_CB_geometry.farTess = 6.0f;
//	m_CB_geometry.nearTess = 0.4f;
//	m_CB_geometry.minTessDensity = 3.0f;
//	m_CB_geometry.maxTessDensity = 9.0f;
//	m_CB_geometry.mapUpdate(_data.m_dc);
//
//	/*Light stuff*/
//	DirectX::XMFLOAT3 camPos = _data.m_cam->getPos();
//	m_CB_light.camera = DirectX::XMFLOAT4(camPos.x, camPos.y, camPos.z, 1.f);
//	STOREF4(&m_CB_light.light, DirectX::XMVectorMultiply(_data.m_cam->calcViewDir(), VEC3(-1, -1, -1)));
//	m_CB_light.mapUpdate(_data.m_dc);
//
//	/*Not really a constbuffer but it needs to be updated all the same*/
//	unsigned int addrIdx[] = { 0, m_maxInstanceCount - 1 };
//	_data.m_dc->UpdateSubresource(m_CSAddressIdicies.getBuffer(), 0, NULL, addrIdx, 0,0);
//}
