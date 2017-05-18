#include "WindManager.h"
#include "Shorthand.h"
#include <algorithm>
#include <d3dcompiler.h>
#include "ConstantBuffers.h"

ID3D11ComputeShader* WindManager::s_cs = nullptr;

void WindManager::updateResources(ID3D11DeviceContext* _dc, unsigned int _numInstances, float _time, float _deltaTime)
{
	
	/*Update buffers*/
	//Cuboids
	if (m_cuboids.size())
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		HRESULT result = _dc->Map(m_cuboidBuffer.getBuffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, m_cuboids.data(), m_cuboids.size() * sizeof(WindCuboid));
		_dc->Unmap(m_cuboidBuffer.getBuffer(), 0);
		assert(!FAILED(result));
	}

	//Spheres
	if (m_spheres.size())
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		HRESULT result = _dc->Map(m_sphereBuffer.getBuffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, m_spheres.data(), m_spheres.size() * sizeof(WindSphere));
		_dc->Unmap(m_sphereBuffer.getBuffer(), 0);
		assert(!FAILED(result));
	}
	
	//CBuffer
	//CBWindForceChangesPerFrame cb = { m_cuboids.size(), m_spheres.size(), _numInstances, _time, _deltaTime };//Should be in it's own buffer for sure
	m_CB_changesPerFrame.m_numCuboids = m_cuboids.size();
	m_CB_changesPerFrame.m_numSpheres = m_spheres.size();
	m_CB_changesPerFrame.m_numInstances = _numInstances;
	m_CB_changesPerFrame.m_time = _time;
	m_CB_changesPerFrame.m_deltaTime = _deltaTime;
	m_CB_changesPerFrame.subresourceUpdate(_dc);
}

bool WindManager::loadShared(ID3D11Device* _device)
{
	HRESULT result;
	ID3DBlob* buffer = nullptr;
	int shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
#endif

	result = D3DCompileFromFile(L"WindForce.hlsl", NULL, NULL, "main", "cs_5_0", shaderFlags, 0, &buffer, NULL);
	if (FAILED(result))
	{
		RELEASE(buffer);
		MessageBox(0, "Error loading compute shader.", "Shader compilation", MB_OK);
		return false;
	}
	_device->CreateComputeShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), 0, &s_cs);
	if (FAILED(result))
	{
		RELEASE(buffer);
		MessageBox(0, "Couldn't create the compute shader.", "Creating shader", MB_OK);
		return false;
	}

	RELEASE(buffer);
}

void WindManager::unloadShared()
{
	RELEASE(s_cs);
}

bool WindManager::load(ID3D11Device* _device, int _maxCuboids, int _maxSpheres)
{

	/*Set members*/
	m_maxSpheres = _maxSpheres;
	m_maxCuboids = _maxCuboids;

	/*Create constant buffer*/
	HRESULT result;
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage				= D3D11_USAGE_DEFAULT;
	bufferDesc.CPUAccessFlags		= 0;
	bufferDesc.MiscFlags			= 0;
	bufferDesc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.StructureByteStride	= 0;
	RETURN_IF_FAILED(m_CB_changesPerFrame.init(_device, &bufferDesc));

	if ((_maxCuboids + _maxSpheres) <= 0)
	{
		return true;
	}

	/*Init cuboid buffer*/
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.BindFlags			= D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.ByteWidth			= _maxCuboids * sizeof(WindCuboid);
	bufferDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	bufferDesc.Usage				= D3D11_USAGE_DYNAMIC;
	bufferDesc.MiscFlags			= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride	= sizeof(WindCuboid);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.ViewDimension			= D3D11_SRV_DIMENSION_BUFFEREX;
	srvDesc.Format					= DXGI_FORMAT_UNKNOWN;
	srvDesc.BufferEx.FirstElement	= 0;
	srvDesc.BufferEx.NumElements	= _maxCuboids;

	RETURN_IF_FAILED(m_cuboidBuffer.init(_device, &bufferDesc, NULL, NULL, &srvDesc));

	/*Init sphere buffer*/
	bufferDesc.ByteWidth			= _maxSpheres * sizeof(WindSphere);
	bufferDesc.StructureByteStride	= sizeof(WindSphere);
	srvDesc.BufferEx.NumElements	= _maxSpheres;

	RETURN_IF_FAILED(m_sphereBuffer.init(_device, &bufferDesc, NULL, NULL, &srvDesc));
	return true;
}

void WindManager::unload()
{
	m_sphereBuffer.cleanup();
	m_cuboidBuffer.cleanup();
}

WindCuboid* WindManager::createWindCuboid()
{
	if (m_cuboids.size() < m_maxCuboids)
	{
		m_cuboids.push_back(WindCuboid());
		return &m_cuboids.back();
	}
	return nullptr;
}

WindSphere* WindManager::createWindSphere()
{
	if (m_spheres.size() < m_maxCuboids)
	{
		m_spheres.push_back(WindSphere());
		return &m_spheres.back();
	}
	return nullptr;
}

void WindManager::remove(WindCuboid* _rect)
{
	ERASE_REMOVE_IF(m_cuboids, [_rect](const WindCuboid& _val) { return &_val == _rect;});
}

void WindManager::remove(WindSphere* _sphere)
{
	ERASE_REMOVE_IF(m_spheres, [_sphere](const WindSphere& _val) { return &_val == _sphere; });
}

void WindManager::removeAll()
{
	m_spheres.clear();
	m_spheres.shrink_to_fit();

	m_cuboids.clear();
	m_cuboids.shrink_to_fit();
}

void WindManager::applyWindForces(ID3D11UnorderedAccessView* _outGrass, ID3D11UnorderedAccessView* _frustumCulled, ID3D11UnorderedAccessView* _indirectArgs,
									ID3D11ShaderResourceView* _inGrass, ID3D11ShaderResourceView* _inOctree, ID3D11DeviceContext* _dc, int _numInstances)
{
	ID3D11ShaderResourceView* views[4] =
	{	
		_inGrass,
		m_cuboidBuffer.getSRV(),
		m_sphereBuffer.getSRV(),
		_inOctree
	};
	ID3D11UnorderedAccessView* uav[3] =
	{
		_outGrass,
		_frustumCulled,
		_indirectArgs
	};

	ID3D11Buffer* cbuffers[] =
	{
		m_CB_changesPerFrame.getBuffer()
	};

	/*Dispatch*/
	_dc->CSSetShader(s_cs, NULL, 0);
	_dc->CSSetShaderResources(0, ARRAYSIZE(views), views);
	_dc->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav), uav, nullptr);
	_dc->CSSetConstantBuffers(0, ARRAYSIZE(cbuffers), cbuffers);
	const unsigned int threadGroupCountX = (unsigned int)ceil((float)_numInstances / (float)THREADS_PER_GROUP_X);
	const unsigned int threadGroupCountY = 1;
	const unsigned int threadGroupCountZ = 1;
	_dc->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);

	/*Cleanup*/
	NULLIFY_STATIC_ARRAY_OF_PTR(views);
	NULLIFY_STATIC_ARRAY_OF_PTR(uav);
	NULLIFY_STATIC_ARRAY_OF_PTR(cbuffers);
	_dc->CSSetShader(nullptr, NULL, 0);
	_dc->CSSetShaderResources(0, ARRAYSIZE(views), views);
	_dc->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav), uav, nullptr);
	_dc->CSSetConstantBuffers(0, ARRAYSIZE(cbuffers), cbuffers);
}
