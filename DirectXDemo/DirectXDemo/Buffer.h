#pragma once
#include <d3d11_2.h>


class Buffer
{
public:
	Buffer() = default;
	~Buffer() { cleanup(); }

	//Pass NULL to subresource, uav or srv if you don't need them
	bool init(	ID3D11Device* _device, 
				const D3D11_BUFFER_DESC*, 
				const D3D11_SUBRESOURCE_DATA*, 
				const D3D11_UNORDERED_ACCESS_VIEW_DESC*, 
				const D3D11_SHADER_RESOURCE_VIEW_DESC*);

	void cleanup();

	ID3D11Buffer*					getBuffer() const	{ return m_buffer; }
	ID3D11UnorderedAccessView*		getUAV() const		{ return m_uav; }
	ID3D11ShaderResourceView*		getSRV() const		{ return m_srv; }

private:
	ID3D11Buffer*				m_buffer = nullptr;
	ID3D11UnorderedAccessView*	m_uav = nullptr;
	ID3D11ShaderResourceView*	m_srv = nullptr;
};