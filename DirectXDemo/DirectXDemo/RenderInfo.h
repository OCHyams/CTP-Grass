#pragma once
#include <d3d11.h>
#include <vector>
#include "DrawData.h"
#include "Shorthand.h"
class MeshObject;
class RenderInfo
{
public:
	/*Input assembly*/
	ID3D11InputLayout*			m_il = nullptr;
	/*Rastierizer state*/
	ID3D11RasterizerState*		m_rs = nullptr;
	/*Shaders*/
	ID3D11VertexShader*			m_vs = nullptr;
	ID3D11HullShader*			m_hs = nullptr;
	ID3D11DomainShader*			m_ds = nullptr;
	ID3D11GeometryShader*		m_gs = nullptr;
	ID3D11PixelShader*			m_ps = nullptr;
	/*Buffers*/
	std::vector<ID3D11Buffer*> m_vsBuffers;
	std::vector<ID3D11Buffer*> m_hsBuffers;
	std::vector<ID3D11Buffer*> m_dsBuffers;
	std::vector<ID3D11Buffer*> m_gsBuffers;
	std::vector<ID3D11Buffer*> m_psBuffers;

	virtual void updatePerObjectBuffers(const DrawData&, const MeshObject&) {}
	virtual void updatePerFrameBuffers(const DrawData&) {}

	void apply(ID3D11DeviceContext* _dc)
	{	
		/*Input assembly*/
		_dc->IASetInputLayout(m_il);
		/*Shaders*/
		_dc->VSSetShader(m_vs, 0, 0);
		_dc->HSSetShader(m_hs, 0, 0);
		_dc->DSSetShader(m_ds, 0, 0);
		_dc->GSSetShader(m_gs, 0, 0);
		_dc->PSSetShader(m_ps, 0, 0);
		/*Rastierizer state*/
		_dc->RSSetState(m_rs);
		/*Buffers*/
		if (m_vsBuffers.size()) _dc->VSSetConstantBuffers(0, m_vsBuffers.size(), m_vsBuffers.data());
		if (m_hsBuffers.size()) _dc->HSSetConstantBuffers(0, m_hsBuffers.size(), m_hsBuffers.data());
		if (m_dsBuffers.size()) _dc->DSSetConstantBuffers(0, m_dsBuffers.size(), m_dsBuffers.data());
		if (m_gsBuffers.size()) _dc->GSSetConstantBuffers(0, m_gsBuffers.size(), m_gsBuffers.data());
		if (m_psBuffers.size()) _dc->PSSetConstantBuffers(0, m_psBuffers.size(), m_psBuffers.data());

	}

	static void remove(ID3D11DeviceContext* _dc)
	{
		/*Input assembly*/
		_dc->IASetInputLayout(nullptr);
		/*Shaders*/
		_dc->VSSetShader(nullptr, 0, 0);
		_dc->HSSetShader(nullptr, 0, 0);
		_dc->DSSetShader(nullptr, 0, 0);
		_dc->GSSetShader(nullptr, 0, 0);
		_dc->PSSetShader(nullptr, 0, 0);
		/*Rastierizer state*/
		_dc->RSSetState(nullptr);
		/*Buffers*/
		ID3D11Buffer* dummy[] = { nullptr };
		_dc->VSSetConstantBuffers(0, 1, dummy);
		_dc->HSSetConstantBuffers(0, 1, dummy);
		_dc->DSSetConstantBuffers(0, 1, dummy);
		_dc->GSSetConstantBuffers(0, 1, dummy);
		_dc->PSSetConstantBuffers(0, 1, dummy);
	}

	void release()
	{
		/*Input assembly*/
		RELEASE(m_il);
		/*Shaders*/
		RELEASE(m_vs);
		RELEASE(m_hs);
		RELEASE(m_ds);
		RELEASE(m_gs);
		RELEASE(m_ps);
		/*Rastierizer state*/
		RELEASE(m_rs);
		/*Buffers*/
		for (auto buffer : m_vsBuffers) RELEASE(buffer);
		m_vsBuffers.clear();
		for (auto buffer : m_hsBuffers) RELEASE(buffer);
		m_hsBuffers.clear();
		for (auto buffer : m_dsBuffers) RELEASE(buffer);
		m_dsBuffers.clear();
		for (auto buffer : m_gsBuffers) RELEASE(buffer);
		m_gsBuffers.clear();
		for (auto buffer : m_psBuffers) RELEASE(buffer);
		m_psBuffers.clear();
	}
};