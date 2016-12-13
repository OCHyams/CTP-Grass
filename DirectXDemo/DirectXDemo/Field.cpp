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
#include <functional>
//statics
DirectX::XMFLOAT3		Field::s_cameraPos		= DirectX::XMFLOAT3();
DirectX::XMFLOAT4X4		Field::s_viewproj		= DirectX::XMFLOAT4X4();
field::Shaders			Field::s_shaders		= field::Shaders();
ID3D11RasterizerState*	Field::s_rasterizer		= nullptr;
ID3D11InputLayout*		Field::s_inputLayout	= nullptr;
ID3D11Buffer*			Field::s_vertexBuffer	= nullptr;
ID3D11ShaderResourceView* Field::s_texture		= nullptr;
ID3D11SamplerState*		Field::s_samplerState	= nullptr;

//ez release
#define RELEASE(x) if (x) { x->Release(); x = nullptr; }


Field::Field()
{
}

Field::~Field()
{
	unload();//just incase they didn't unload it themselves (cost of a few if statements)
}

bool Field::loadShared(ID3D11Device* _device)
{
	HRESULT result;
	ID3DBlob* buffer = nullptr;
	int shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
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
		buffer->Release();
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
		{ "INSTANCE_WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE_WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE_WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE_WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE_ROTATION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE_LOCATION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 80, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};
	unsigned int totalLayoutElements = ARRAYSIZE(vsLayout);

	result = _device->CreateInputLayout(vsLayout, totalLayoutElements, buffer->GetBufferPointer(), buffer->GetBufferSize(), &s_inputLayout);
	RELEASE(buffer);
	if (FAILED(result))
	{
		RELEASE(buffer);
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
	buffer->Release();
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
		//pos, binormal, normal, flex
		{ DirectX::XMFLOAT3(0.f, 0.f, 0.f),   DirectX::XMFLOAT3(1.f, 0.f, 0.f), DirectX::XMFLOAT3(0.f, 0.f, -1.f), 0.f },
		{ DirectX::XMFLOAT3(0.f, 0.2f, 0.f),  DirectX::XMFLOAT3(1.f, 0.f, 0.f), DirectX::XMFLOAT3(0.f, 0.f, -1.f), 0.111f },
		{ DirectX::XMFLOAT3(0.f, 0.4f, 0.f),  DirectX::XMFLOAT3(1.f, 0.f, 0.f), DirectX::XMFLOAT3(0.f, 0.f, -1.f), 0.445f },
		{ DirectX::XMFLOAT3(0.0f, 0.6f, 0.08f),  DirectX::XMFLOAT3(1.f, 0.f, 0.f), DirectX::XMFLOAT3(0.f, 0.f, -1.f), 1.f }
	};

	D3D11_BUFFER_DESC vDesc;
	ZeroMemory(&vDesc, sizeof(vDesc));
	vDesc.Usage = D3D11_USAGE_DEFAULT;
	vDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vDesc.ByteWidth = sizeof(field::Vertex) * 4;

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
		//do not return false, as for now there is no grass D:
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


bool Field::load(	ID3D11Device*		_device, 
					int					_instanceCount, 
					DirectX::XMFLOAT2	_size, 
					DirectX::XMFLOAT3	_pos)
{
	m_instanceCount = _instanceCount;
	m_size = _size;
	m_pos = _pos;

	using namespace DirectX;
	HRESULT result;

	field::Instance* instances;
	D3D11_BUFFER_DESC instanceBufferDesc;
	D3D11_SUBRESOURCE_DATA instanceData;

	// Create the instance array.
	instances = generateInstanceData();

	// Set up the description of the instance buffer.
	instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	instanceBufferDesc.ByteWidth = sizeof(field::Instance) * m_instanceCount;
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instanceBufferDesc.CPUAccessFlags = 0;
	instanceBufferDesc.MiscFlags = 0;
	instanceBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the instance data.
	instanceData.pSysMem = instances;
	instanceData.SysMemPitch = 0;
	instanceData.SysMemSlicePitch = 0;

	// Create the instance buffer.
	result = _device->CreateBuffer(&instanceBufferDesc, &instanceData, &m_instanceBuffer);
	if (FAILED(result))
	{
		MessageBox(0, "Error creating instance buffer.", "Field", MB_OK);
		return false;
	}
	// Release the instance array now that the instance buffer has been created and loaded.
	delete[] instances;
	instances = nullptr;


	//CBUFFER
	//Geometry buffer
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
	bufferdesc.Usage = D3D11_USAGE_DEFAULT;
	bufferdesc.ByteWidth = sizeof(CBFieldLight);
	bufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferdesc.CPUAccessFlags = 0;
	bufferdesc.MiscFlags = 0;
	bufferdesc.StructureByteStride = 0;
	result = _device->CreateBuffer(&bufferdesc, NULL, &m_CB_light);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Error: Couldn't create the light const buffer.");
		return false;
	}
	return true;
}

void Field::unload()
{
	RELEASE(m_instanceBuffer);
	RELEASE(m_CB_geometry);
	RELEASE(m_CB_viewproj);
}

void Field::update()
{
	updateConstBuffers();
}

void Field::draw(const DrawData& _data)
{
	unsigned int strides[2];
	unsigned int offsets[2];
	ID3D11Buffer* bufferPointers[2];

	// Set the buffer strides.
	strides[0] = sizeof(field::Vertex);
	strides[1] = sizeof(field::Instance);
	// Set the buffer offsets.
	offsets[0] = 0;
	offsets[1] = 0;
	// Set the array of pointers to the vertex and instance buffers.
	bufferPointers[0] = s_vertexBuffer;
	bufferPointers[1] = m_instanceBuffer;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	_data.m_dc->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);
	_data.m_dc->IASetInputLayout(s_inputLayout);
	_data.m_dc->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, NULL);
	_data.m_dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

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

	//sampler
	_data.m_dc->PSSetSamplers(0, 1, &s_samplerState);
	//texture
	_data.m_dc->PSSetShaderResources(0, 1, &s_texture);

	_data.m_dc->DrawInstanced(4, m_instanceCount, 0, 0);
}

void Field::updateConstBuffers()
{
	using namespace DirectX;
	Time* t = OCH::ServiceLocator<Time>::get();

	//Geometry generation buffer
	//for now just use 0 rotation...
	XMFLOAT3 rotation = { 0,0,0 };
	XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotation));
	rot = XMMatrixMultiply(rot, XMLoadFloat4x4(&s_viewproj)); 
	XMVECTOR tan = XMVector4Transform(XMLoadFloat4(&XMFLOAT4(m_halfGrassWidth, 0, 0, 0)), rot);
	XMFLOAT4 tanf4;
	XMStoreFloat4(&tanf4, tan);

	m_CBcpu_geometry.tessDensity = /*@m_curDensity*/9;
	m_CBcpu_geometry.binormal = tanf4;
	m_CBcpu_geometry.halfGrassWidth = m_halfGrassWidth;
	m_CBcpu_geometry.time = (float)t->time;
	m_CBcpu_geometry.wind = m_wind;
	
	//view projection buffer
	m_CBcpu_viewproj.m_wvp = XMMatrixTranspose(XMLoadFloat4x4(&s_viewproj));
	
	m_CBcpu_light.intensity = 0.7f;
	m_CBcpu_light.light = XMFLOAT4(0.f,.5f,0.f,1.f);
	m_CBcpu_light.camera = XMFLOAT4(s_cameraPos.x, s_cameraPos.y, s_cameraPos.z,1.f);
}


#define MAX(x,y)    (x)>(y)?(x):(y)
field::Instance* Field::generateInstanceData()
{
	using namespace DirectX;
	field::Instance* data = new field::Instance[m_instanceCount];

//http://math.stackexchange.com/questions/466198/algorithm-to-get-the-maximum-size-of-n-squares-that-fit-into-a-rectangle-with-a
	double x = m_size.x, y = m_size.y, n = m_instanceCount;
	double px = ceil(sqrt(n*x / y));
	double sx, sy;
	if (floor(px*y / x)*px<n)  //does not fit, y/(x/px)=px*y/x
		sx = y / ceil(px*y / x);
	else
		sx = x / px;
	double py = ceil(sqrt(n*y / x));
	if (floor(py*x / y)*py<n)  //does not fit
		sy = x / ceil(x*py / y);
	else
		sy = y / py;

	double side = MAX(sy, sx);

	int xcount = floor(m_size.x / side);
	int zcount = floor(m_size.y / side);
	float xoffset = 0;
	float zoffset = 0;
	int index = 0;

	//RNG
	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(0, 2 * XM_PI);
	auto randAngle = std::bind(distribution, generator);

	for (int x = 0; x < xcount; ++x)
	{
		for (int z = 0; z < zcount; ++z)
		{
			if (index >= m_instanceCount) break;
			//scale
			//
			//rotation
			XMFLOAT4 rotQuat = XMFLOAT4(0,1,0, randAngle());
			data[index].rotation = rotQuat;
			//translation
			XMFLOAT3 position = XMFLOAT3(m_pos.x + xoffset, m_pos.y, m_pos.z + zoffset);
			XMVECTOR translation = XMLoadFloat3(&position);
			data[index].location = XMFLOAT3(m_pos.x + xoffset, m_pos.y, m_pos.z + zoffset);

			//world
			//XMMATRIX world = XMMatrixTranslationFromVector(translation);//shouldn't work

			XMMATRIX world = XMMatrixRotationY(rotQuat.w); 
			world = XMMatrixMultiply(world, XMMatrixTranslationFromVector(translation));

			XMStoreFloat4x4(&data[index].world, XMMatrixTranspose(world));


			index++;
			zoffset += side;
		}
		xoffset += side;
		zoffset = 0;
	}

	return data;
}

#undef MAX(x,y)
#undef DELETE(x)