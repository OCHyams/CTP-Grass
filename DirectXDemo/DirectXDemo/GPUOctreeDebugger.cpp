#include "GPUOctreeDebugger.h"
#include "VertexData.h"
#include <stack>
#include "ConstantBuffers.h"
#include "Shorthand.h"

DXHelper::Shaders		GPUOctreeDebugger::s_shaders = DXHelper::Shaders();
ID3D11RasterizerState*	GPUOctreeDebugger::s_rasterizer = nullptr;
ID3D11InputLayout*		GPUOctreeDebugger::s_inputLayout = nullptr;
ID3D11Buffer*			GPUOctreeDebugger::s_vertexBuffer = nullptr;
ID3D11Buffer*			GPUOctreeDebugger::s_indexBuffer = nullptr;
ID3D11Buffer*			GPUOctreeDebugger::s_m_CB_wvp = nullptr;

bool GPUOctreeDebugger::loadShared(ID3D11Device* _device)
{
	/*Early out if we've already loaded stuff*/
	if (s_m_CB_wvp) return true;

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
	_device->CreateVertexShader(vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), 0, &s_shaders.m_vs);
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
	result = _device->CreatePixelShader(psBuffer->GetBufferPointer(), psBuffer->GetBufferSize(), 0, &s_shaders.m_ps);
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
	result = _device->CreateInputLayout(vsLayout, totalLayoutElements, vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), &s_inputLayout);
	vsBuffer->Release();

	if (FAILED(result))
	{
		DXTRACE_MSG(L"Couldn't create the input layout.");
		return false;
	}

	//VERTEX DATA
	BasicVertex verts[] =
	{
		//Base
		XMFLOAT4(-0.5f, -0.5f, 0.5f, 1.f),
		XMFLOAT4(-0.5f, -0.5f, -0.5f, 1.f),
		XMFLOAT4(0.5f, -0.5f, -0.5f, 1.f),
		XMFLOAT4(0.5f, -0.5f, 0.5f, 1.f),
		//TOP
		XMFLOAT4(-0.5f, 0.5f, 0.5f, 1.f),
		XMFLOAT4(0.5f, 0.5f, 0.5f, 1.f),
		XMFLOAT4(0.5f, 0.5f, -0.5f, 1.f),
		XMFLOAT4(-0.5f, 0.5f, -0.5f, 1.f),

	};

	D3D11_BUFFER_DESC vDesc;
	ZeroMemory(&vDesc, sizeof(vDesc));
	vDesc.Usage = D3D11_USAGE_DEFAULT;
	vDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vDesc.ByteWidth = sizeof(BasicVertex) * ARRAYSIZE(verts);

	D3D11_SUBRESOURCE_DATA resourceData;
	ZeroMemory(&resourceData, sizeof(resourceData));
	resourceData.pSysMem = verts;

	result = _device->CreateBuffer(&vDesc, &resourceData, &s_vertexBuffer);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Couldn't create the vertex buffer.");
		return false;
	}

	//Index buffer
	WORD indicies[] =
	{
		0,1,2,
		2,3,0,
		4,5,6,
		6,7,4,
		0,3,5,
		5,4,0,
		3,2,6,
		6,5,3,
		2,1,7,
		7,6,2,
		1,0,4,
		4,7,1
	};

	D3D11_BUFFER_DESC iDesc;
	ZeroMemory(&iDesc, sizeof(iDesc));
	iDesc.Usage = D3D11_USAGE_DEFAULT;
	iDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	iDesc.ByteWidth = sizeof(WORD) * ARRAYSIZE(indicies);
	ZeroMemory(&resourceData, sizeof(resourceData));
	resourceData.pSysMem = indicies;
	result = _device->CreateBuffer(&iDesc, &resourceData, &s_indexBuffer);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Couldn't create the index buffer.");
		return false;
	}

	//RASTERIZER
	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(rasterDesc));
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_NONE; //D3D11_CULL_NONE D3D11_CULL_FRONT
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_WIREFRAME;//D3D11_FILL_WIREFRAME  D3D11_FILL_SOLID
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out
	result = _device->CreateRasterizerState(&rasterDesc, &s_rasterizer);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Couldn't create the rasterizer state.");
		return false;
	}

	D3D11_BUFFER_DESC worldDesc;
	ZeroMemory(&worldDesc, sizeof(worldDesc));
	worldDesc.Usage = D3D11_USAGE_DEFAULT;
	worldDesc.ByteWidth = sizeof(CBField_ChangesPerFrame);
	worldDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	worldDesc.CPUAccessFlags = 0;
	worldDesc.MiscFlags = 0;
	worldDesc.StructureByteStride = 0;
	result = _device->CreateBuffer(&worldDesc, NULL, &s_m_CB_wvp);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Error: Couldn't create the w-v-p const buffer.");
		return false;
	}

	return true;
}

void GPUOctreeDebugger::unloadShared()
{
	s_shaders.release();
	RELEASE(s_rasterizer);
	RELEASE(s_inputLayout);
	RELEASE(s_indexBuffer);
	RELEASE(s_vertexBuffer);
	RELEASE(s_m_CB_wvp);
}

void GPUOctreeDebugger::draw(ID3D11DeviceContext* _dc, const DirectX::XMFLOAT4X4& _transform, const DirectX::XMFLOAT4X4& _viewProj, const GPUOctree& _tree)
{
	unsigned int stride = sizeof(BasicVertex);
	unsigned int offset = 0;

	/*Input assembler*/
	_dc->IASetInputLayout(s_inputLayout);
	_dc->IASetVertexBuffers(0, 1, &s_vertexBuffer, &stride, &offset);
	_dc->IASetIndexBuffer(s_indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	_dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	/*Rasterizer state*/
	_dc->RSSetState(s_rasterizer);

	/*Shaders*/
	s_shaders.apply(_dc);

	/*Buffers*/
	_dc->VSSetConstantBuffers(0, 1, &s_m_CB_wvp);
	_dc->PSSetConstantBuffers(0, 1, &s_m_CB_wvp);
	
	std::stack<int> tree;
	tree.push(0);

	/*Depth first traversal of nodes*/
	while (!tree.empty())
	{
		//Pop top node
		const int currentIdx = tree.top();
		tree.pop();

		GPUOctree::Node current = _tree.getNode(currentIdx);
	
		//If not leaf node
		if (current.hasChildren())
		{		//Push child nodes
			for (int childIdx : current.m_childIdx)
			{
				if (GPUOctree::Node::idxNotNull(childIdx)) tree.push(childIdx);
			}
		}
		else
		{
			//Draw it
			using namespace DirectX;
			CBBasicShader_ChangesPerFrame cbuff;
			XMMATRIX wvp = XMMatrixScalingFromVector(LF3(&current.m_AABB.Extents) * 2);
			wvp = XMMatrixMultiply(wvp, XMMatrixTranslationFromVector(LF3(&current.m_AABB.Center)));
			wvp = XMMatrixMultiply(wvp, LF44(&_transform));
			wvp = XMMatrixMultiply(wvp, LF44(&_viewProj));
			XMStoreFloat4x4(&cbuff.m_worldViewProj, TRANSPOSE(wvp));
			_dc->UpdateSubresource(s_m_CB_wvp, 0, 0, &cbuff, 0, 0);

			_dc->DrawIndexed(36, 0, 0);
		}
	}
}
