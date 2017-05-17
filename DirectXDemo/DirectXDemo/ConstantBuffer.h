/*----------------------------------------------------------------
Author:			Orlando Cazalet-Hyams
Description :	Constant buffer class, uses decorator pattern
				so that the memebers can be accessed as TData.
				To supply inital data, set member values before
				calling init().
----------------------------------------------------------------*/

#pragma once
#include <d3d11.h>
#include "Shorthand.h"

template <typename TData>
class CBuffer final : public TData
{
public:
	CBuffer() = default;
	~CBuffer() { cleanup(); }

	void mapUpdate( ID3D11DeviceContext* _dc, ::D3D11_MAP _mapType = D3D11_MAP_WRITE_DISCARD);
	void subresourceUpdate( ID3D11DeviceContext* _dc );
	//Size is automatically set to sizeof(TData)
	bool init( ID3D11Device* _device, const D3D11_BUFFER_DESC* _bufferDesc );
	void cleanup();

	ID3D11Buffer* getBuffer() { return m_buffer; }
private:
	ID3D11Buffer* m_buffer;
};

template<typename TData>
inline void CBuffer<TData>::mapUpdate(ID3D11DeviceContext* _dc, ::D3D11_MAP _mapType)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	_dc->Map(m_buffer, 0, _mapType, 0, &mappedResource);

	memcpy(mappedResource.pData, (TData*)this, sizeof(TData));

	_dc->Unmap(m_buffer, 0);
}

template<typename TData>
inline void CBuffer<TData>::subresourceUpdate(ID3D11DeviceContext* _dc)
{
	_dc->UpdateSubresource(m_buffer, 0, 0, (TData*)this, 0, 0);
}

template<typename TData>
inline bool CBuffer<TData>::init(ID3D11Device* _device, const D3D11_BUFFER_DESC* _bufferDesc)
{
	D3D11_BUFFER_DESC newBufferDesc = *_bufferDesc;
	newBufferDesc.ByteWidth = SIZEOF_ROUND_TO_BOUNDARY(TData, 16);
	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = (TData*)this;

	HRESULT result = _device->CreateBuffer(&newBufferDesc, &data, &m_buffer);
	if (FAILED(result))
	{
		MessageBox(0, "Error creating buffer.", "CBuffer", MB_OK);
		return false;
	}
	return true;
}

template<typename TData>
inline void CBuffer<TData>::cleanup()
{
	RELEASE(m_buffer);
}
