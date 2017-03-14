#include "WindManager_14_03.h"
#include <algorithm>

void WindManager_14_03::updateResources(ID3D11DeviceContext* _dc)
{
	/*Update buffers*/
	//Cuboids
	if (m_cuboids.size())
	{
		ID3D11Buffer* buffer = m_cuboidBuffer.getBuffer();
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		HRESULT result = _dc->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, m_cuboids.data(), m_cuboids.size() * sizeof(WindCuboid));
		_dc->Unmap(buffer, 0);
		assert(!FAILED(result));
	}

	//Spheres
	if (m_spheres.size())
	{
		ID3D11Buffer* buffer = m_sphereBuffer.getBuffer();
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		HRESULT result = _dc->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, m_spheres.data(), m_spheres.size() * sizeof(WindCuboid));
		_dc->Unmap(buffer, 0);
		assert(!FAILED(result));
	}
}

bool WindManager_14_03::load(ID3D11Device* _device, unsigned int _maxCuboids, unsigned int _maxSpheres)
{
	m_maxCuboids = _maxCuboids;
	m_maxSpheres = _maxSpheres;

	if ((m_maxCuboids + m_maxSpheres) <= 0)
	{
		return true;
	}

	D3D11_BUFFER_DESC bufferDesc;
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

	/*Init cuboid buffer*/
	CHECK_FAIL(m_cuboidBuffer.init(_device, &bufferDesc, NULL, NULL, &srvDesc));

	/*Init sphere buffer*/
	bufferDesc.ByteWidth = _maxSpheres * sizeof(WindSphere);
	bufferDesc.StructureByteStride = sizeof(WindSphere);
	srvDesc.BufferEx.NumElements = _maxSpheres;
	CHECK_FAIL(m_sphereBuffer.init(_device, &bufferDesc, NULL, NULL, &srvDesc));
	return true;
}

void WindManager_14_03::unload()
{
	m_cuboidBuffer.cleanup();
	m_sphereBuffer.cleanup();
	m_cuboids.clear();
	m_spheres.clear();
}

WindCuboid* WindManager_14_03::createWindCuboid()
{
	if (m_cuboids.size() < m_maxCuboids)
	{
		m_cuboids.push_back(WindCuboid());
		return &m_cuboids.back();
	}
	return nullptr;
}

WindSphere* WindManager_14_03::createWindSphere()
{
	if (m_spheres.size() < m_maxCuboids)
	{
		m_spheres.push_back(WindSphere());
		return &m_spheres.back();
	}
	return nullptr;
}

void WindManager_14_03::remove(WindCuboid* _cuboid)
{
	ERASE_REMOVE_IF(m_cuboids, [_cuboid](const WindCuboid& _val) { return &_val == _cuboid; });
}

void WindManager_14_03::remove(WindSphere* _sphere)
{
	ERASE_REMOVE_IF(m_spheres, [_sphere](const WindSphere& _val) { return &_val == _sphere; });
}

void WindManager_14_03::removeAll()
{
	m_spheres.clear();
	m_spheres.shrink_to_fit();

	m_cuboids.clear();
	m_cuboids.shrink_to_fit();
}
