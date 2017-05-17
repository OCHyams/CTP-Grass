#include "Buffer.h"
#include "Shorthand.h"

bool Buffer::init(ID3D11Device* _device, 
					const D3D11_BUFFER_DESC* _bufferDesc, 
					const D3D11_SUBRESOURCE_DATA* _initialData, 
					const D3D11_UNORDERED_ACCESS_VIEW_DESC* _uavDesc, 
					const D3D11_SHADER_RESOURCE_VIEW_DESC* _srvDesc)
{
	HRESULT result;
	result = _device->CreateBuffer(_bufferDesc, _initialData, &m_buffer);
	if (FAILED(result))
	{
		MessageBox(0, "Error creating buffer.", "Buffer", MB_OK);
		return false;
	}

	if (_uavDesc)
	{
		if (FAILED(_device->CreateUnorderedAccessView(m_buffer, _uavDesc, &m_uav)))
		{
			MessageBox(0, "Error creating UAV.", "Buffer", MB_OK);
			return false;
		}
	}

	if (_srvDesc)
	{
		if (FAILED(_device->CreateShaderResourceView(m_buffer, _srvDesc, &m_srv)))
		{
			MessageBox(0, "Error creating SRV.", "Buffer", MB_OK);
			return false;
		}
	}

	return true;
}

void Buffer::cleanup()
{
	RELEASE(m_srv);
	RELEASE(m_uav);
	RELEASE(m_buffer);
}
