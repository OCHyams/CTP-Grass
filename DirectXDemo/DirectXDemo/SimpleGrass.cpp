#include "SimpleGrass.h"
#include <d3dcompiler.h>
#include "dxerr.h"
#include <DirectXMath.h>
#include "DXHelper.h"
#include "Input.h"
#include "Time.h"
#include "ServiceLocator.h"

DirectX::XMFLOAT3 SimpleGrass::s_cameraPos = DirectX::XMFLOAT3();
ID3D11VertexShader*		SimpleGrass::m_vs = nullptr;
ID3D11PixelShader*		SimpleGrass::m_ps = nullptr;
ID3D11GeometryShader*	SimpleGrass::m_gs = nullptr;
ID3D11HullShader*		SimpleGrass::m_hs = nullptr;
ID3D11DomainShader*		SimpleGrass::m_ds = nullptr;
ID3D11RasterizerState*	SimpleGrass::m_rasterizer = nullptr;
ID3D11InputLayout*		SimpleGrass::m_inputLayout = nullptr;

SimpleGrass::SimpleGrass() :	
								m_CB_geometry(nullptr),
								m_CB_worldviewproj(nullptr),
								m_vertexBuffer(nullptr),
								m_CB_light(nullptr)
{}

SimpleGrass::~SimpleGrass()
{
	unload();
}

bool SimpleGrass::loadShared(ID3D11Device* _device)
{
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
	//INPUT LAYOUT
	D3D11_INPUT_ELEMENT_DESC vsLayout[] =
	{
		{ "SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "T_VAL",0, DXGI_FORMAT_R32_FLOAT ,0,12,D3D11_INPUT_PER_VERTEX_DATA,0 }
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

	//RASTERIZER
	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(rasterDesc));
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_NONE; //probably want no culling for grass b/c it's flat!
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;//D3D11_FILL_WIREFRAME  D3D11_FILL_SOLID
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
	return true;
}

void SimpleGrass::unloadShared()
{
	if (m_ps) m_ps->Release();
	if (m_ds) m_ds->Release();
	if (m_hs) m_hs->Release();
	if (m_vs) m_vs->Release();
	if (m_inputLayout) m_inputLayout->Release();
	if (m_rasterizer) m_rasterizer->Release();
}

void SimpleGrass::beginDraw(const DrawData& _data)
{

	_data.m_dc->IASetInputLayout(m_inputLayout);
	_data.m_dc->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, NULL);

	_data.m_dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);//D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST D3D11_PRIMITIVE_TOPOLOGY_POINTLIST D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST

																						   //set shaders
	_data.m_dc->VSSetShader(m_vs, 0, 0);
	_data.m_dc->PSSetShader(m_ps, 0, 0);
	_data.m_dc->GSSetShader(m_gs, 0, 0);
	_data.m_dc->HSSetShader(m_hs, 0, 0);
	_data.m_dc->DSSetShader(m_ds, 0, 0);
	_data.m_dc->RSSetState(m_rasterizer);
}

bool SimpleGrass::load(ID3D11Device* _device)
{
	m_curDensity = 10.f;
	m_maxDensity = 12.f;
	m_minDensity = 3.f;
	HRESULT result;

	//VERTEX DATA
	BezierVertex verts[] =
	{
		{ DirectX::XMFLOAT3(0.f, 0.f, 0.f), 0.f},
		{ DirectX::XMFLOAT3(0.f, 0.2f, 0.f), 0.33f},
		{ DirectX::XMFLOAT3(0.f, 0.4f, 0.f), 0.66f},
		{ DirectX::XMFLOAT3(0.0f, 0.6f, 0.f), 1.f}
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

	//CBUFFER
	//Geometry buffer
	D3D11_BUFFER_DESC bufferdesc;
	ZeroMemory(&bufferdesc, sizeof(bufferdesc));
	bufferdesc.Usage = D3D11_USAGE_DEFAULT;
	bufferdesc.ByteWidth = sizeof(CBGrassGeometry);
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
	bufferdesc.ByteWidth = sizeof(CBField_ChangesPerFrame);
	bufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferdesc.CPUAccessFlags = 0;
	bufferdesc.MiscFlags = 0;
	bufferdesc.StructureByteStride = 0;
	result = _device->CreateBuffer(&bufferdesc, NULL, &m_CB_worldviewproj);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Error: Couldn't create the world const buffer.");
		return false;
	}

	//Light
	ZeroMemory(&bufferdesc, sizeof(bufferdesc));
	bufferdesc.Usage = D3D11_USAGE_DEFAULT;
	bufferdesc.ByteWidth = sizeof(CBLight);
	bufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferdesc.CPUAccessFlags = 0;
	bufferdesc.MiscFlags = 0;
	bufferdesc.StructureByteStride = 0;
	result = _device->CreateBuffer(&bufferdesc, NULL, &m_CB_light);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Error: Couldn't create the world const buffer.");
		return false;
	}


	//default grass width
	m_halfGrassWidth = 0.012f;

	return true;
}

void SimpleGrass::unload()
{
	if (m_CB_light) m_CB_light->Release();
	if (m_vertexBuffer) m_vertexBuffer->Release();
	if (m_CB_worldviewproj) m_CB_worldviewproj->Release();
	if (m_CB_geometry) m_CB_geometry->Release();

	m_CB_light = nullptr;
	m_vertexBuffer = nullptr;
	m_inputLayout = nullptr;
}

void SimpleGrass::update()
{
	using namespace DirectX;
	GameObject::update();

	/*change LOD with camera distance*/ 
	XMVECTOR difference = XMVectorSubtract(XMLoadFloat3(&s_cameraPos), XMLoadFloat3(&m_pos));
	XMVECTOR vdist = XMVector3Length(difference);
	float distance = vdist.m128_f32[0];
	float __near = 0.5;
	float __far = 0.6f;

	if (distance < __near) m_curDensity = m_maxDensity;
	else if (distance > __far) m_curDensity = m_minDensity;
	else /*if between __near and __far*/
	{
		__far -= __near;
		distance -= __near;
		float norm = distance / __far;

		float maxDens = m_maxDensity - m_minDensity;
		float lerped = maxDens * (1-norm);
		m_curDensity = lerped + m_minDensity;
	}

	updateConstBuffers();
}

void SimpleGrass::draw(const DrawData& _data)
{
	//set intermidiate assembly settings
	unsigned int stride = sizeof(BezierVertex);
	unsigned int offset = 0;
	_data.m_dc->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	//update subresources
	_data.m_dc->UpdateSubresource(m_CB_worldviewproj, 0, 0, &m_CBcpu_worldviewproj, 0, 0);
	_data.m_dc->UpdateSubresource(m_CB_geometry, 0, 0, &m_CBcpu_geometry, 0, 0);
	_data.m_dc->UpdateSubresource(m_CB_light, 0, 0, &m_CBcpu_light, 0, 0);

	//set constant buffers
	_data.m_dc->HSSetConstantBuffers(0, 1, &m_CB_geometry);
	_data.m_dc->GSSetConstantBuffers(0, 1, &m_CB_geometry);
	_data.m_dc->DSSetConstantBuffers(0, 1, &m_CB_geometry);
	_data.m_dc->VSSetConstantBuffers(0, 1, &m_CB_geometry);
	_data.m_dc->VSSetConstantBuffers(1, 1, &m_CB_worldviewproj);
	_data.m_dc->GSSetConstantBuffers(1, 1, &m_CB_worldviewproj);
	_data.m_dc->VSSetConstantBuffers(2, 1, &m_CB_light);
	_data.m_dc->PSSetConstantBuffers(2, 1, &m_CB_light);

	//draw verticies
	_data.m_dc->Draw(4, 0);
}

void SimpleGrass::updateConstBuffers()
{
	using namespace DirectX;
	Time* t = OCH::ServiceLocator<Time>::get();

	//Geometry generation buffer
	XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_rot));
	//rot = XMMatrixMultiply(rot, XMLoadFloat4x4(&m_worldViewProj)); //<-- this shouldn't correct i don't think...
	rot = XMMatrixMultiply(rot, XMLoadFloat4x4(&s_viewProj)); //<-- this seeems more correct... (regardless, grass still doesn't work properly :C)
	XMVECTOR tan = XMVector4Transform(XMLoadFloat4(&XMFLOAT4(m_halfGrassWidth, 0, 0, 0)), rot);
	XMFLOAT4 tanf4;
	XMStoreFloat4(&tanf4, tan);
	m_CBcpu_geometry = { m_curDensity, m_halfGrassWidth, (float)t->time, m_wind.x, m_wind.y, m_wind.z, m_pos.x, m_pos.y, m_pos.z, tanf4.x, tanf4.y, tanf4.z, tanf4.w };

	//world view projection buffer
	XMStoreFloat4x4(&m_CBcpu_worldviewproj.m_wvp, XMLoadFloat4x4(&m_worldViewProj));

	//light
	//do this for now!
	m_CBcpu_light = {0,0,0,0,0,0,0};

}

