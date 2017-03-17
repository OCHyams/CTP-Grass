#include "DoubleBuffer.h"
#include "Shorthand.h"

void DoubleBuffer::swap()
{
	Buffer* temp = m_front;
	m_front = m_back;
	m_back = temp;
}

const Buffer* DoubleBuffer::front()
{
	return m_front;
}

const Buffer* DoubleBuffer::back()
{
	return m_back;
}

bool DoubleBuffer::init(ID3D11Device* _device, const D3D11_BUFFER_DESC* _bufferDesc, const D3D11_SUBRESOURCE_DATA* _initialData, const D3D11_UNORDERED_ACCESS_VIEW_DESC* _UAVDesc, const D3D11_SHADER_RESOURCE_VIEW_DESC* _SRVDesc)
{
	if (m_first.init(_device, _bufferDesc, _initialData, _UAVDesc, _SRVDesc)
		&& m_second.init(_device, _bufferDesc, _initialData, _UAVDesc, _SRVDesc))
	{
		m_front = &m_first;
		m_back = &m_second;
		return true;
	}
	return false;
}

void DoubleBuffer::cleanup()
{
	m_first.cleanup();
	m_second.cleanup();
	m_front = nullptr;
	m_back = nullptr;
}

