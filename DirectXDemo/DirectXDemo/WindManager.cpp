#include "WindManager.h"
#include "Shorthand.h"
#include <algorithm>
#include <d3dcompiler.h>
ID3D11ComputeShader* WindManager::s_cs = nullptr;

void WindManager::updateResources(ID3D11DeviceContext* _dc)
{
	/*Update buffers*/
	//Cuboids
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	HRESULT result = _dc->Map(m_cuboidBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, m_cuboids.data(), m_cuboids.size() * sizeof(WindCuboid));
	mappedResource.RowPitch = m_cuboids.size() * sizeof(WindCuboid);
	_dc->Unmap(m_cuboidBuffer, 0);
	assert(!FAILED(result));

	//Spheres
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	result = _dc->Map(m_sphereBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, m_spheres.data(), m_spheres.size() * sizeof(WindSphere));
	mappedResource.RowPitch = m_spheres.size() * sizeof(WindSphere);
	_dc->Unmap(m_sphereBuffer, 0);
	assert(!FAILED(result));

}

bool WindManager::loadShared(ID3D11Device* _device)
{
	HRESULT result;
	ID3DBlob* buffer = nullptr;
	int shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
#endif

	//VS SHADER
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

bool WindManager::load(ID3D11Device* _device, int _maxRects, int _maxSpheres)
{
	/*Create buffers*/
	//Cuboids
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.BindFlags			= /*D3D11_BIND_UNORDERED_ACCESS |*/ D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.ByteWidth			= _maxRects * sizeof(WindCuboid);
	bufferDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;//@i put that there...
	bufferDesc.Usage				= D3D11_USAGE_DYNAMIC;
	bufferDesc.MiscFlags			= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride	= sizeof(WindCuboid);
	if (FAILED(_device->CreateBuffer(&bufferDesc, NULL, &m_cuboidBuffer))) return false;
	//Spheres
	bufferDesc.ByteWidth			= _maxSpheres * sizeof(WindSphere);
	bufferDesc.StructureByteStride	= sizeof(WindSphere);
	if (FAILED(_device->CreateBuffer(&bufferDesc, NULL, &m_sphereBuffer))) return false;

	/*Create shader resource views*/
	//Cuboids
	ZeroMemory(&bufferDesc, sizeof(bufferDesc)); 
	m_cuboidBuffer->GetDesc(&bufferDesc);
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.ViewDimension			= D3D11_SRV_DIMENSION_BUFFEREX;
	srvDesc.Format					= DXGI_FORMAT_UNKNOWN;
	srvDesc.BufferEx.FirstElement	= 0;
	srvDesc.BufferEx.NumElements	= _maxRects;
	if (FAILED(_device->CreateShaderResourceView(m_cuboidBuffer, &srvDesc, &m_cuboidSRV))) return false;
	//Spheres
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	m_sphereBuffer->GetDesc(&bufferDesc);
	srvDesc.ViewDimension			= D3D11_SRV_DIMENSION_BUFFEREX;
	srvDesc.Format					= DXGI_FORMAT_UNKNOWN;
	srvDesc.BufferEx.FirstElement	= 0;
	srvDesc.BufferEx.NumElements	= _maxSpheres;
	if (FAILED(_device->CreateShaderResourceView(m_sphereBuffer, &srvDesc, &m_sphereSRV))) return false;

	return true;
}

void WindManager::unload()
{
	RELEASE(m_cuboidBuffer);
	RELEASE(m_sphereBuffer);
	RELEASE(m_cuboidSRV);
	RELEASE(m_sphereSRV);
}

WindCuboid* WindManager::CreateWindCuboid()
{
	if (m_cuboids.size() < m_maxCuboids)
	{
		m_cuboids.push_back(WindCuboid());
		return &m_cuboids.back();
	}
	return nullptr;
}

WindSphere* WindManager::CreateWindSphere()
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

void WindManager::applyWindForces(ID3D11UnorderedAccessView* _outGrass, ID3D11ShaderResourceView* _inGrass, ID3D11DeviceContext* _dc, int _numInstances)
{
	ID3D11ShaderResourceView* views[3] =
	{
		m_cuboidSRV,
		m_sphereSRV,
		_inGrass
	};

	/*Dispatch*/
	_dc->CSSetShader(s_cs, NULL, 0);
	_dc->CSSetShaderResources(0, 3, views);
	_dc->CSSetUnorderedAccessViews(0, 1, &_outGrass, nullptr);
	_dc->Dispatch(ceil(_numInstances/ m_threadsPerGroupX), 1, 1);

	views[0] = nullptr;
	views[1] = nullptr;

	/*Cleanup*/
	_dc->CSSetShader(nullptr, NULL, 0);
	_dc->CSSetShaderResources(0, 3, views);
	ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
	_dc->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
}