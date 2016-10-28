#include "Triangle.h"
#include <d3dcompiler.h>
#include "dxerr.h"
#include <DirectXMath.h>
ShaderDemoObject::ShaderDemoObject() :	m_vs(nullptr),
										m_ps(nullptr),
										m_inputLayout(nullptr),
										m_vertexBuffer(nullptr)
{}

ShaderDemoObject::~ShaderDemoObject()
{
	unload();
}

bool ShaderDemoObject::load(ID3D11Device* _device)
{
	HRESULT result;

	int shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
#endif

	//VS SHADER
	ID3DBlob* vsBuffer = nullptr;
	result = D3DCompileFromFile(L"SolidGreenColour.fx",NULL ,NULL,"VS_Main", "vs_4_0", shaderFlags,0, &vsBuffer, NULL);
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

	//INPUT LAYOUT
	D3D11_INPUT_ELEMENT_DESC vsLayout[] =
	{
		{"POSITION",0, DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0}
	};
	unsigned int totalLayoutElements = ARRAYSIZE(vsLayout);

	result = _device->CreateInputLayout(vsLayout, totalLayoutElements, vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), &m_inputLayout);
	vsBuffer->Release();

	if (FAILED(result))
	{
		DXTRACE_MSG(L"Couldn't create the input layout.");
		return false;
	}

	//PS SHADER
	ID3DBlob* psBuffer = nullptr;
	result = D3DCompileFromFile(L"SolidGreenColour.fx", NULL, NULL, "PS_Main", "ps_4_0", shaderFlags, 0, &psBuffer, NULL);
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
	result = D3DCompileFromFile(L"SimpleGrass.fx", NULL, NULL, "GS_Main", "gs_4_0", shaderFlags, 0, &gsBuffer, NULL);
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

	//VERTEX DATA
	BasicVertex verts[] =
	{
		DirectX::XMFLOAT3(0.25f, 0.25f, 0.5f),
		DirectX::XMFLOAT3(0.25f, -0.25f, 0.5f),
		DirectX::XMFLOAT3(-0.25f, -0.25f, 0.5f)
	};

	D3D11_BUFFER_DESC vDesc;
	ZeroMemory(&vDesc, sizeof(vDesc));
	vDesc.Usage = D3D11_USAGE_DEFAULT;
	vDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vDesc.ByteWidth = sizeof(BasicVertex)*3;

	D3D11_SUBRESOURCE_DATA resourceData;
	ZeroMemory(&resourceData, sizeof(resourceData));
	resourceData.pSysMem = verts;

	result = _device->CreateBuffer(&vDesc, &resourceData, &m_vertexBuffer);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Couldn't create the vertex buffer.");
		return false;
	}

	return true;
}

void ShaderDemoObject::unload()
{
	if (m_vertexBuffer) m_vertexBuffer->Release();
	if (m_inputLayout) m_inputLayout->Release();
	if (m_ps) m_ps->Release();
	if (m_vs) m_vs->Release();

	m_vertexBuffer = nullptr;
	m_inputLayout = nullptr;
	m_ps = nullptr;
	m_vs = nullptr;
}

void ShaderDemoObject::update(float _dt)
{
}

void ShaderDemoObject::draw(ID3D11DeviceContext* _dc)
{
	unsigned int stride = sizeof(BasicVertex);
	unsigned int offset = 0;

	_dc->IASetInputLayout(m_inputLayout);
	_dc->IASetVertexBuffers(0,1,&m_vertexBuffer, &stride, &offset);
	_dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_dc->VSSetShader(m_vs, 0, 0);
	_dc->PSSetShader(m_ps, 0, 0);
	_dc->GSSetShader(m_gs, 0, 0);
	_dc->Draw(3, 0);
}
