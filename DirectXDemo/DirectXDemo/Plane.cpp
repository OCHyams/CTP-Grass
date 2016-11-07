#include "Plane.h"
#include <d3dcompiler.h>
#include "dxerr.h"
#include <DirectXMath.h>
#include "DXHelper.h"
Plane::Plane()
{
}

Plane::~Plane()
{
}

bool Plane::load(ID3D11Device* _device)
{
	using namespace DirectX;

	HRESULT result;
	int shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
#endif

	//VS SHADER
	ID3DBlob* vsBuffer = nullptr;
	result = D3DCompileFromFile(L"BasicShader.fx", NULL, NULL, "VS_Main", "vs_5_0", shaderFlags, 0, &vsBuffer, NULL);
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
	result = D3DCompileFromFile(L"BasicShader.fx", NULL, NULL, "PS_Main", "ps_5_0", shaderFlags, 0, &psBuffer, NULL);
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

	//INPUT LAYOUT
	D3D11_INPUT_ELEMENT_DESC vsLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
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
	BasicVertex verts[] =
	{
		XMFLOAT4(-0.5f, 0.f, -0.5f, 1.f),
		XMFLOAT4(0.5f, 0.f, -0.5f, 1.f),
		XMFLOAT4(0.5f, 0.f, 0.5f, 1.f),
		XMFLOAT4(-0.5f, 0.f, -0.5f, 1.f)
	};

	D3D11_BUFFER_DESC vDesc;
	ZeroMemory(&vDesc, sizeof(vDesc));
	vDesc.Usage = D3D11_USAGE_DEFAULT;
	vDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vDesc.ByteWidth = sizeof(BasicVertex) * ARRAYSIZE(verts);

	D3D11_SUBRESOURCE_DATA resourceData;
	ZeroMemory(&resourceData, sizeof(resourceData));
	resourceData.pSysMem = verts;

	result = _device->CreateBuffer(&vDesc, &resourceData, &m_vertexBuffer);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Couldn't create the vertex buffer.");
		return false;
	}

	//Index buffer
	WORD indicies[] =
	{
		3,1,0,2,1,3
	};
	D3D11_BUFFER_DESC iDesc;
	ZeroMemory(&iDesc, sizeof(iDesc));
	iDesc.Usage = D3D11_USAGE_DEFAULT;
	iDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	iDesc.ByteWidth = sizeof(WORD) * ARRAYSIZE(indicies);
	ZeroMemory(&resourceData, sizeof(resourceData));
	resourceData.pSysMem = indicies;
	result = _device->CreateBuffer(&iDesc, &resourceData, &m_indexBuffer);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Couldn't create the index buffer.");
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

	// Create the rasterizer state from the description we just filled out
	result = _device->CreateRasterizerState(&rasterDesc, &m_rasterizer);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Couldn't create the rasterizer state.");
		return false;
	}

	//WORLD
	if (!DXHelper::createBasicConstBuffer(&m_CB_world,
		_device, sizeof(CBWorldViewProj),
		L"Couldn't create the world const buffer."))
	{
		return false;
	}

	return true;
}

void Plane::unload()
{
	if (m_indexBuffer) m_indexBuffer->Release();


	if (m_vertexBuffer) m_vertexBuffer->Release();
	if (m_inputLayout) m_inputLayout->Release();
	if (m_ps) m_ps->Release();
	if (m_vs) m_vs->Release();
	if (m_CB_world) m_CB_world->Release();
	m_vertexBuffer = nullptr;
	m_inputLayout = nullptr;
	m_ps = nullptr;
	m_vs = nullptr;
}

void Plane::update()
{
	GameObject::update();
}

void Plane::draw(const DrawData& _data)
{
	unsigned int stride = sizeof(BasicVertex);
	unsigned int offset = 0;

	_data.m_dc->IASetInputLayout(m_inputLayout);
	_data.m_dc->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	_data.m_dc->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	_data.m_dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_data.m_dc->VSSetShader(m_vs, 0, 0);
	_data.m_dc->HSSetShader(nullptr, 0, 0);
	_data.m_dc->DSSetShader(nullptr, 0, 0);
	_data.m_dc->GSSetShader(nullptr, 0, 0);
	_data.m_dc->PSSetShader(m_ps, 0, 0);
	_data.m_dc->RSSetState(m_rasterizer);

	using namespace DirectX;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	_data.m_dc->Map(m_CB_world, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	CBWorldViewProj* dataPtr = (CBWorldViewProj*)mappedResource.pData;
	XMMATRIX wvp = XMLoadFloat4x4(&m_worldViewProj);
	dataPtr->m_wvp = XMMatrixTranspose(wvp);
	_data.m_dc->Unmap(m_CB_world, 0);

	_data.m_dc->VSSetConstantBuffers(0, 1, &m_CB_world);
	_data.m_dc->PSSetConstantBuffers(0, 1, &m_CB_world);

	_data.m_dc->DrawIndexed(6, 0, 0);
}
