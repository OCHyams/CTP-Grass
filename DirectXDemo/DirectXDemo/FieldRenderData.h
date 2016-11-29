#pragma once
#include <DirectXMath.h>
namespace field
{
	using namespace DirectX;

	__declspec(align(16))
	struct Vertex
	{
		XMFLOAT3	pos;
		float		flexibility;
	};

	__declspec(align(16))
	struct Instance
	{
		XMFLOAT4X4	world;
	};


	struct Shaders
	{
		ID3D11VertexShader*		m_vs = nullptr;
		ID3D11HullShader*		m_hs = nullptr;
		ID3D11DomainShader*		m_ds = nullptr;
		ID3D11GeometryShader*	m_gs = nullptr;
		ID3D11PixelShader*		m_ps = nullptr;


		inline void apply(ID3D11DeviceContext* _dc)
		{
			_dc->VSSetShader(m_vs, 0, 0);
			_dc->HSSetShader(m_hs, 0, 0);
			_dc->DSSetShader(m_ds, 0, 0);
			_dc->GSSetShader(m_gs, 0, 0);
			_dc->PSSetShader(m_ps, 0, 0);
		}

		inline void release()
		{
			if (m_vs) { m_vs->Release(); m_vs = nullptr; }
			if (m_hs) { m_hs->Release(); m_hs = nullptr; }
			if (m_ds) { m_ds->Release(); m_ds = nullptr; }
			if (m_gs) { m_gs->Release(); m_gs = nullptr; }
			if (m_ps) { m_ps->Release(); m_ps = nullptr; }
		}
	};
}