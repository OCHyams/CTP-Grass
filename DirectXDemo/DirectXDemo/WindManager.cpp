#include "WindManager.h"
#include "Shorthand.h"
#include <algorithm>

void WindManager::updateResources(ID3D11DeviceContext* _dc)
{
	/*Update buffers*/
	//Cuboids
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	HRESULT result = _dc->Map(m_cuboidBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, m_cuboids.data(), m_cuboids.size() * sizeof(WindCuboid));
	_dc->Unmap(m_cuboidBuffer, 0);
	assert(!FAILED(result));

	//Spheres
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	result = _dc->Map(m_sphereBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, m_spheres.data(), m_spheres.size() * sizeof(WindSphere));
	_dc->Unmap(m_sphereBuffer, 0);
	assert(!FAILED(result));
}

bool WindManager::load(ID3D11Device* _device, int _maxRects, int _maxSpheres)
{
	/*Create buffers*/
	//Cuboids
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.ByteWidth = _maxRects * sizeof(WindCuboid);
	bufferDesc.CPUAccessFlags = D3D11_USAGE_DYNAMIC;//@i put that there...
	bufferDesc.StructureByteStride = sizeof(WindCuboid);
	D3D11_SUBRESOURCE_DATA subresoure;
	ZeroMemory(&subresoure, sizeof(subresoure));
	subresoure.pSysMem = m_cuboids.data();
	if (FAILED(_device->CreateBuffer(&bufferDesc, &subresoure, &m_cuboidBuffer))) return false;
	//Spheres
	bufferDesc.ByteWidth = _maxSpheres * sizeof(WindSphere);
	bufferDesc.StructureByteStride = sizeof(WindSphere);
	subresoure.pSysMem = m_spheres.data();
	if (FAILED(_device->CreateBuffer(&bufferDesc, &subresoure, &m_sphereBuffer))) return false;

	/*Create shader resource views*/
	//Cuboids
	ZeroMemory(&bufferDesc, sizeof(bufferDesc)); 
	m_cuboidBuffer->GetDesc(&bufferDesc);
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.BufferEx.FirstElement = 0;
	srvDesc.BufferEx.NumElements = bufferDesc.ByteWidth / bufferDesc.StructureByteStride;
	if (FAILED(_device->CreateShaderResourceView(m_cuboidBuffer, &srvDesc, &m_cuboidSRV))) return false;
	//Spheres
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	m_sphereBuffer->GetDesc(&bufferDesc);
	srvDesc.BufferEx.NumElements = bufferDesc.ByteWidth / bufferDesc.StructureByteStride;
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
