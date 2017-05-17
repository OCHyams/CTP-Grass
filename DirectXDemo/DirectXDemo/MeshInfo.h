#pragma once
#include <d3d11_2.h>
#include <vector>
#include "DrawData.h"
#include "Shorthand.h"
class MeshInfo
{
public:
	std::vector<ID3D11Buffer*>		m_vbs;
	ID3D11Buffer*					m_ib		= nullptr;
	int								m_vCount	= 0;
	std::vector<unsigned int>		m_strides;
	std::vector<unsigned int>		m_offsets;
	DXGI_FORMAT						m_ibFormat = DXGI_FORMAT_R16_UINT;
	UINT							m_ibOffset = 0;
	D3D11_PRIMITIVE_TOPOLOGY		m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	void apply(ID3D11DeviceContext* _dc)
	{
		/*Input assembly*/
		if (m_vbs.size()) _dc->IASetVertexBuffers(0, m_vbs.size(), m_vbs.data(), m_strides.data(), m_offsets.data());
		_dc->IASetIndexBuffer(m_ib, m_ibFormat, m_ibOffset);
		_dc->IASetPrimitiveTopology(m_topology);
	}

	static void remove(ID3D11DeviceContext* _dc)
	{
		UINT stride = 0, offset = 0;
		/*Input assembly*/
		_dc->IASetVertexBuffers(0, 1, nullptr, &stride, &offset);
		_dc->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
		_dc->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED);
	}

	void release()
	{
		/*Input assembly*/
		RELEASE(m_ib);
		for (auto buffer : m_vbs) RELEASE(buffer);
		m_vbs.clear();
		m_strides.clear();
		m_offsets.clear();
	}
};