#include "SimpleGrass.h"
#include <d3dcompiler.h>
#include "dxerr.h"
#include <DirectXMath.h>
#include "DXHelper.h"
#include "Input.h"
#include "Time.h"
#include "ServiceLocator.h"

SimpleGrass::SimpleGrass() :	m_vs(nullptr),
								m_hs(nullptr),
								m_ds(nullptr),
								m_ps(nullptr),
								m_CB_geometry(nullptr),
								m_CB_world(nullptr),
								m_inputLayout(nullptr),
								m_vertexBuffer(nullptr)
{}

SimpleGrass::~SimpleGrass()
{
	unload();
}

bool SimpleGrass::load(ID3D11Device* _device)
{
	m_curDensity = 10.f;
	m_maxDensity = 36.f;
	m_minDensity = 3.f;
	HRESULT result;

	int shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
#endif

	//VS SHADER
	ID3DBlob* vsBuffer = nullptr;
	result = D3DCompileFromFile(L"SimpleGrass.fx", NULL, NULL, "VS_Main", "vs_5_0", shaderFlags, 0, &vsBuffer, NULL);
	if (FAILED(result))
	{
		MessageBox(0, "Error loading vertex shader.", "Shader compilation", MB_OK);
		return false;
	}

	_device->CreateVertexShader(vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), 0, &m_vs);
	if (FAILED(result))
	{
		if (vsBuffer) vsBuffer->Release();
		DXTRACE_MSG(L"Couldn't create the vertex shader.");
		return false;
	}

	//PS SHADER
	ID3DBlob* psBuffer = nullptr;
	result = D3DCompileFromFile(L"SimpleGrass.fx", NULL, NULL, "PS_Main", "ps_5_0", shaderFlags, 0, &psBuffer, NULL);
	if (FAILED(result))
	{
		MessageBox(0, "Error loading pixel shader.", "Shader compilation", MB_OK);
		return false;
	}

	result = _device->CreatePixelShader(psBuffer->GetBufferPointer(), psBuffer->GetBufferSize(), 0, &m_ps);
	psBuffer->Release();

	if (FAILED(result))
	{
		DXTRACE_MSG(L"Couldn't create the pixel shader.");
		return false;
	}

	//GS SHADER
	ID3DBlob* gsBuffer = nullptr;
	result = D3DCompileFromFile(L"SimpleGrass.fx", NULL, NULL, "GS_Main", "gs_5_0", shaderFlags, 0, &gsBuffer, NULL);
	if (FAILED(result))
	{
		MessageBox(0, "Error loading geometry shader.", "Shader compilation", MB_OK);
		return false;
	}

	result = _device->CreateGeometryShader(gsBuffer->GetBufferPointer(), gsBuffer->GetBufferSize(), 0, &m_gs);
	gsBuffer->Release();

	if (FAILED(result))
	{
		DXTRACE_MSG(L"Couldn't create the geometry shader.");
		return false;
	}

	//HS SHADER
	ID3DBlob* hsBuffer = nullptr;
	result = D3DCompileFromFile(L"SimpleGrass.fx", NULL, NULL, "HS_Main", "hs_5_0", shaderFlags, 0, &hsBuffer, NULL);
	if (FAILED(result))
	{
		MessageBox(0, "Error loading hull shader.", "Shader compilation", MB_OK);
		return false;
	}

	result = _device->CreateHullShader(hsBuffer->GetBufferPointer(), hsBuffer->GetBufferSize(), 0, &m_hs);
	hsBuffer->Release();

	if (FAILED(result))
	{
		DXTRACE_MSG(L"Couldn't create the hull shader.");
		return false;
	}

	//DS SHADER
	ID3DBlob* dsBuffer = nullptr;
	result = D3DCompileFromFile(L"SimpleGrass.fx", NULL, NULL, "DS_Main", "ds_5_0", shaderFlags, 0, &dsBuffer, NULL);
	if (FAILED(result))
	{
		MessageBox(0, "Error loading domain shader.", "Shader compilation", MB_OK);
		return false;
	}

	result = _device->CreateDomainShader(dsBuffer->GetBufferPointer(), dsBuffer->GetBufferSize(), 0, &m_ds);
	dsBuffer->Release();

	if (FAILED(result))
	{
		DXTRACE_MSG(L"Couldn't create the domain shader.");
		return false;
	}

	//INPUT LAYOUT
	D3D11_INPUT_ELEMENT_DESC vsLayout[] =
	{
		{ "POSITION",0, DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "T_VAL",0, DXGI_FORMAT_R32_FLOAT ,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 }
	};

	unsigned int totalLayoutElements = ARRAYSIZE(vsLayout);

	result = _device->CreateInputLayout(vsLayout, totalLayoutElements, vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), &m_inputLayout);
	vsBuffer->Release();

	if (FAILED(result))
	{
		DXTRACE_MSG(L"Couldn't create the input layout.");
		return false;
	}

	//VERTEX DATA
	BezierVertex verts[] =
	{
		{DirectX::XMFLOAT3(0.f, 0.f, 0.f), 0.f},
		{DirectX::XMFLOAT3(0.f, 0.2f, 0.f), 0.33f},
		{DirectX::XMFLOAT3(0.f, 0.4f, 0.f), 0.66f},
		{DirectX::XMFLOAT3(0.0f, 0.6f, -0.1f), 1.f}
	};

	D3D11_BUFFER_DESC vDesc;
	ZeroMemory(&vDesc, sizeof(vDesc));
	vDesc.Usage = D3D11_USAGE_DEFAULT;
	vDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vDesc.ByteWidth = sizeof(BezierVertex) * 4;

	D3D11_SUBRESOURCE_DATA resourceData;
	ZeroMemory(&resourceData, sizeof(resourceData));
	resourceData.pSysMem = verts;

	result = _device->CreateBuffer(&vDesc, &resourceData, &m_vertexBuffer);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Couldn't create the vertex buffer.");
		return false;
	}

	//RASTERIZER
	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(rasterDesc));
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_NONE; //probably want no culling for grass b/c it's flat!
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_WIREFRAME;//D3D11_FILL_WIREFRAME  D3D11_FILL_SOLID
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	result = _device->CreateRasterizerState(&rasterDesc, &m_rasterizer);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Couldn't create the rasterizer state.");
		return false;
	}

	//CBUFFER
	//DS
	if (!DXHelper::createBasicConstBuffer(&m_CB_geometry,
		_device, sizeof(CBGrassGeometry),
		L"Couldn't create the hull shader const buffer."))
	{
		return false;
	}
	
	//WORLD
	if (!DXHelper::createBasicConstBuffer(&m_CB_world,
		_device, sizeof(CBSingleMatrix),
		L"Couldn't create the world const buffer."))
	{
		return false;
	}


	//default grass width
	m_halfGrassWidth = 0.04f;

	return true;
}

void SimpleGrass::unload()
{
	if (m_vertexBuffer) m_vertexBuffer->Release();
	if (m_inputLayout) m_inputLayout->Release();
	if (m_CB_world) m_CB_world->Release();
	if (m_CB_geometry) m_CB_geometry->Release();
	if (m_ps) m_ps->Release();
	if (m_ds) m_ds->Release();
	if (m_hs) m_hs->Release();
	if (m_vs) m_vs->Release();

	m_vertexBuffer = nullptr;
	m_inputLayout = nullptr;
	m_ps = nullptr;
	m_vs = nullptr;
}

void SimpleGrass::update()
{
	GameObject::update();
	Input* input = OCH::ServiceLocator<Input>::get();
	
	if (input->getKeyDown(DIK_LEFT))
	{
		m_curDensity = --m_curDensity >= m_minDensity ? m_curDensity : m_minDensity;
	}
	else if (input->getKeyDown(DIK_RIGHT))
	{
		m_curDensity = ++m_curDensity <= m_maxDensity ? m_curDensity : m_maxDensity;
	}
}

void SimpleGrass::draw(const DrawData& _data)
{
	unsigned int stride = sizeof(BezierVertex);
	unsigned int offset = 0;

	_data.m_dc->IASetInputLayout(m_inputLayout);
	_data.m_dc->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	_data.m_dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);//D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST D3D11_PRIMITIVE_TOPOLOGY_POINTLIST D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST
	_data.m_dc->VSSetShader(m_vs, 0, 0);
	_data.m_dc->PSSetShader(m_ps, 0, 0);
	_data.m_dc->GSSetShader(m_gs, 0, 0);
	_data.m_dc->HSSetShader(m_hs, 0, 0);
	_data.m_dc->DSSetShader(m_ds, 0, 0);
	_data.m_dc->RSSetState(m_rasterizer);

	using namespace DirectX;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	_data.m_dc->Map(m_CB_world, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	XMMATRIX* dataPtr = (XMMATRIX*)mappedResource.pData;
	*dataPtr = XMMatrixTranspose(XMLoadFloat4x4(&m_worldViewProj));
	_data.m_dc->Unmap(m_CB_world, 0);

	Time* t = OCH::ServiceLocator<Time>::get();
	CBGrassGeometry buffer = { m_curDensity, m_halfGrassWidth, t->time, m_wind.x, m_wind.y, m_wind.z, 0 , 0 };
	_data.m_dc->Map(m_CB_geometry, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &buffer, sizeof(CBGrassGeometry));
	_data.m_dc->Unmap(m_CB_geometry, 0);

	_data.m_dc->HSSetConstantBuffers(0, 1, &m_CB_geometry);
	_data.m_dc->GSSetConstantBuffers(0, 1, &m_CB_geometry);
	_data.m_dc->DSSetConstantBuffers(0, 1, &m_CB_geometry);
	_data.m_dc->VSSetConstantBuffers(0, 1, &m_CB_geometry);
	_data.m_dc->VSSetConstantBuffers(1, 1, &m_CB_world);
	_data.m_dc->Draw(4, 0);
}

