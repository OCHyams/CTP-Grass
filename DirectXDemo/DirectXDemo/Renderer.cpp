#include "Renderer.h"
#include "MeshInfo.h"
#include "RenderInfo.h"
#include "Camera.h"
#include "VertexData.h"
#include "Shorthand.h"
#include "DefaultObjectRenderInfo.h"
#include <d3dcompiler.h>
#include "ConstantBuffers.h"
bool Renderer::load(ID3D11Device* _device)
{
	using namespace DirectX;

	HRESULT result;
	int shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
#endif

	/*Default Object Shader*/
	DefaultObjcetRenderInfo* defaultData = new DefaultObjcetRenderInfo();
	ID3DBlob* vsBuffer = nullptr;
	result = D3DCompileFromFile(L"DefaultObjectVS.hlsl", NULL, NULL, "main", "vs_5_0", shaderFlags, 0, &vsBuffer, NULL);
	if (FAILED(result))
	{
		MessageBox(0, "Error loading vertex shader.", "Default Object Shader", MB_OK);
		return false;
	}
	_device->CreateVertexShader(vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), 0, &defaultData->m_vs);
	if (FAILED(result))
	{
		RELEASE(vsBuffer);
		MessageBox(0, "Couldn't create the vertex shader.", "Default Object Shader", MB_OK);
		return false;
	}

	//INPUT LAYOUT
	D3D11_INPUT_ELEMENT_DESC vsLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12 , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	unsigned int totalLayoutElements = ARRAYSIZE(vsLayout);
	result = _device->CreateInputLayout(vsLayout, totalLayoutElements, vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), &defaultData->m_il);
	vsBuffer->Release();
	if (FAILED(result))
	{
		MessageBox(0, "Couldn't create the input layout.", "Default Object Shader", MB_OK);
		return false;
	}

	//PS SHADER
	ID3DBlob* psBuffer = nullptr;
	result = D3DCompileFromFile(L"DefaultObjectPS.hlsl", NULL, NULL, "main", "ps_5_0", shaderFlags, 0, &psBuffer, NULL);
	if (FAILED(result))
	{
		MessageBox(0, "Error loading pixel shader.", "Default Object Shader", MB_OK);
		return false;
	}
	result = _device->CreatePixelShader(psBuffer->GetBufferPointer(), psBuffer->GetBufferSize(), 0, &defaultData->m_ps);
	psBuffer->Release();
	if (FAILED(result))
	{
		MessageBox(0, "Couldn't create the vertex shader.", "Default Object Shader", MB_OK);
		return false;
	}

	//RASTERIZER STATE
	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(rasterDesc));
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;//@weird model problems
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	result = _device->CreateRasterizerState(&rasterDesc, &defaultData->m_rs);
	if (FAILED(result))
	{
		MessageBox(0, "Couldn't create the rasterizer state.", "Default Object Shader", MB_OK);
		return false;
	}

	//Buffers
	//Changes per object 
	D3D11_BUFFER_DESC	bufferdesc;
	ID3D11Buffer*		buffPtr;
	ZeroMemory(&bufferdesc, sizeof(bufferdesc));
	bufferdesc.Usage = D3D11_USAGE_DEFAULT;
	bufferdesc.ByteWidth = sizeof(CBDefaultObject_ChangesPerObject);
	bufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	result = _device->CreateBuffer(&bufferdesc, NULL, &buffPtr);
	if (FAILED(result))
	{
		MessageBox(0, "Error creating per-object const buffer", "Default Object Shader", MB_OK);
		return false;
	}
	defaultData->m_vsBuffers.push_back(buffPtr);
	defaultData->m_psBuffers.push_back(buffPtr);
	buffPtr->AddRef();

	//Changes per frame
	ZeroMemory(&bufferdesc, sizeof(bufferdesc));
	bufferdesc.Usage = D3D11_USAGE_DEFAULT;
	bufferdesc.ByteWidth = sizeof(CBDefaultObject_ChangesPerFrame);
	bufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	result = _device->CreateBuffer(&bufferdesc, NULL, &buffPtr);
	if (FAILED(result))
	{
		MessageBox(0, "Error creating per-frame const buffer", "Default Object Shader", MB_OK);
		return false;
	}
	defaultData->m_vsBuffers.push_back(buffPtr);
	defaultData->m_psBuffers.push_back(buffPtr);
	buffPtr->AddRef();

	/*Add the effect to the map of stored FX*/
	m_fx.insert(std::pair<FX, RenderInfo*>(FX::DEFAULT, defaultData));


	return true;
}

void Renderer::cleanup()
{
	/*Clean up FX*/
	for (auto fxPair : m_fx)
	{
		//Release the render info (shaders & buffers)
		fxPair.second->release();
		//delete the RenderInfo
		delete fxPair.second;
		fxPair.second = nullptr;
	}
	m_fx.clear();

	/*Clean up meshes*/
	for (auto mesh : m_meshes)
	{
		//Release the mesh info (buffers)
		if (mesh.second) {
			mesh.second->release();
			//delete the mesh
			delete mesh.second;
			mesh.second = nullptr;
		}
	}
	m_meshes.clear();

	/*Clean up model data*/
	for (auto model : m_objModels)
	{
		//Release the mesh info (buffers)
		model.second->release();
		//delete the mesh
		delete model.second;
		model.second = nullptr;
	}
	m_objModels.clear();
}

void Renderer::render(const DrawData& data)
{
	/*Iterate over "effects"*/
	for (auto pass : m_fx)
	{
		/*Apply context settings that are required for this pass*/
		pass.second->apply(data.m_dc);
		pass.second->updatePerFrameBuffers(data);

		/*Iterate over objects*/
		for (auto object : m_meshObjects)
		{
			/*If the object has a render flag matching that for this pass...*/
			if (object->m_renderFlags & (unsigned int)pass.first)
			{
				pass.second->updatePerObjectBuffers(data, *object);
				MeshInfo* mesh = m_meshes[(int)object->m_meshID];
				if (mesh)
				{
					mesh->apply(data.m_dc);
					data.m_dc->DrawIndexed(mesh->m_vCount, 0, 0);

					/*Let the object do any additional rendering...*/
					/*Care must be taken to preserve the device state and/or return it to how it was before the draw call*/
					/*It's up to the object to provide all IA contex settings save for the Input Layout*/
					object->draw(data);
				}
			}
		}
	}
	RenderInfo::remove(data.m_dc);
}

bool Renderer::registerMesh(int _id, const std::string& _fpath, ObjModel::MESH_TOPOLOGY _inputTopology, DirectX::XMFLOAT4X4 _transform, ID3D11Device* _device)
{
	if (getObjModel(_id))
	{
#ifdef DEBUG
		MessageBox(0, "Mesh has been registered already", "Mesh object", MB_OK);
#endif
		return true;
	}

	if (!loadMesh(_fpath, _inputTopology, _id, _device, _transform))
	{
		MessageBox(0, "Error loading or creating model", "Mesh object", MB_OK);
		return false;
	}
	return true;
}

ObjModel* Renderer::getObjModel(int meshIdx)
{
	auto itr = m_objModels.find(meshIdx);
	if (itr != m_objModels.end())
	{
		return itr->second;
	}

	return nullptr;
}

MeshInfo* Renderer::loadMeshHelper(const ObjModel& model, ID3D11Device* _device)
{
	MeshInfo* mesh = new MeshInfo();
	int vertCount = model.getTotalVerts();
	mesh->m_vCount = vertCount;

	float* vertElementPtr = model.getVertices();
	float* normElementPtr = model.getNormals();
	float* texElementPtr = model.getTexCoords();

	DefaultVertex* vBuffer = new DefaultVertex[vertCount];
	for (int i = 0; i < vertCount; i++)
	{
		if (vertElementPtr)
		{
			memcpy(&vBuffer[i].m_pos, vertElementPtr, sizeof(float) * 3);
			vertElementPtr += 3;
		}
		if (normElementPtr)
		{
			memcpy(&vBuffer[i].m_normal, normElementPtr, sizeof(float) * 3);
			normElementPtr += 3;
		}
		if (texElementPtr)
		{
			memcpy(&vBuffer[i].m_texCoord, texElementPtr, sizeof(float) * 2);
			texElementPtr += 2;
		}
	}


	/*Create the vertex buffer*/
	D3D11_BUFFER_DESC vDesc;
	ZeroMemory(&vDesc, sizeof(vDesc));
	vDesc.Usage = D3D11_USAGE_DEFAULT;
	vDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vDesc.ByteWidth = sizeof(DefaultVertex) * vertCount;
	D3D11_SUBRESOURCE_DATA resourceData;
	ZeroMemory(&resourceData, sizeof(resourceData));
	resourceData.pSysMem = vBuffer;
	ID3D11Buffer* vertexBufferPtr;
	HRESULT result = _device->CreateBuffer(&vDesc, &resourceData, &vertexBufferPtr);
	if (FAILED(result))
	{
		MessageBox(0, "Couldn't create the vertex buffer.", "Mesh Object", MB_OK);
		delete mesh;
		delete[] vBuffer;
		return nullptr;
	}
	mesh->m_vbs.push_back(vertexBufferPtr);


	WORD* idxBuffer = new WORD[model.getTotalVerts()];
	for (int i = 0; i < model.getTotalVerts(); ++i)
	{
		idxBuffer[i] = i;
	}


	D3D11_BUFFER_DESC iDesc;
	ZeroMemory(&iDesc, sizeof(iDesc));
	iDesc.Usage				= D3D11_USAGE_DEFAULT;
	iDesc.BindFlags			= D3D11_BIND_INDEX_BUFFER;
	iDesc.ByteWidth			= sizeof(WORD) *  model.getTotalVerts();
	ZeroMemory(&resourceData, sizeof(resourceData));
	resourceData.pSysMem	= idxBuffer;
	result = _device->CreateBuffer(&iDesc, &resourceData, &mesh->m_ib);
	if (FAILED(result))
	{
		MessageBox(0, "Couldn't create the index buffer.", "Mesh Object", MB_OK);
		delete mesh;
		delete[] vBuffer;
		return nullptr;
	}

	mesh->m_strides.push_back(sizeof(DefaultVertex));
	mesh->m_offsets.push_back(0);
	mesh->m_ibOffset = 0;

	delete[] vBuffer;

	return mesh;
}

MeshInfo* Renderer::loadMesh(const std::string& _fpath, ObjModel::MESH_TOPOLOGY inputTopology, int idx, ID3D11Device* _device, const DirectX::XMFLOAT4X4& _transform)
{
	/*Load the model and build the vertex buffer*/
	ObjModel* model = new ObjModel;


	bool result = model->loadOBJ(_fpath.c_str(), _transform, inputTopology);
	if (!result)
	{
		MessageBox(0, "Couldn't load obj.", "Mesh Object", MB_OK);
		delete model;
		return nullptr;
	}

	MeshInfo* mesh = loadMeshHelper(*model, _device);
	if (mesh == nullptr) MessageBox(0, "Couldn't create mesh info", "Mesh Object", MB_OK);
	m_meshes.insert(std::pair<int, MeshInfo*>(idx, mesh));
	m_objModels.insert(std::pair<int, ObjModel*>(idx, model));

	return mesh;
}