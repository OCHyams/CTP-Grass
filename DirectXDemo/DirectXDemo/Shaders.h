#pragma once
#include <vector>
#include <d3d11_2.h>
#include <typeinfo.h>
#include <string>
#include <d3dcompiler.h>


namespace DXHelper
{

	bool readBytesFromFile(const char* _fpath, char** _out_bytes, int* _out_length);

	template <typename T>
	T* createShaderFromBytes(ID3D11Device* _device, const char* _bytes, int _length)
	{
		HRESULT hr = E_FAIL;
		T* shader = nullptr;
		const std::type_info& templateType = typeid(T);
		const std::type_info& vs = typeid(ID3D11VertexShader);
		const std::type_info& hs = typeid(ID3D11HullShader);
		const std::type_info& ds = typeid(ID3D11DomainShader);
		const std::type_info& gs = typeid(ID3D11GeometryShader);
		const std::type_info& ps = typeid(ID3D11PixelShader);

		if (templateType == vs)
		{
			hr = _device->CreateVertexShader(_bytes, _length, nullptr, (ID3D11VertexShader**)&shader);
		}
		else if (templateType == hs)
		{
			hr = _device->CreateHullShader(_bytes, _length, nullptr, (ID3D11HullShader**)&shader);
		}
		else if (templateType == ds)
		{
			hr = _device->CreateDomainShader(_bytes, _length, nullptr, (ID3D11DomainShader**)&shader);
		}
		else if (templateType == gs)
		{
			hr = _device->CreateGeometryShader(_bytes, _length, nullptr, (ID3D11GeometryShader**)&shader);
		}
		else if (templateType == ps)
		{
			hr = _device->CreatePixelShader(_bytes, _length, nullptr, (ID3D11PixelShader**)&shader);
		}

		if (FAILED(hr))
		{
			MessageBox(0, "Couldn't create shader from bytes.", "createFromBytes", MB_OK);
			return nullptr;
		}

		return shader;
	}

	struct CompileFromFileArgs
	{
		LPCWSTR fileName;
		const D3D_SHADER_MACRO* defines = NULL;
		ID3DInclude* inlcude = NULL;
		LPCSTR entryPoint;
		LPCSTR target;
		UINT flags1
#if defined(DEBUG) || defined(_DEBUG)
			= D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG;
#else
			= D3DCOMPILE_ENABLE_STRICTNESS;
#endif
		UINT flags2 = 0;
		ID3DBlob** errorMsgs = NULL;
	};

	template <typename T>
	T* compileShaderFromFile(ID3D11Device* _device, const CompileFromFileArgs& _args)
	{
		ID3DBlob* buffer;
		HRESULT result = D3DCompileFromFile(_args.fileName, _args.defines, _args.inlcude, _args.entryPoint, _args.target, _args.flags1, _args.flags2, &buffer, _args.errorMsgs);

		if (FAILED(result))
		{
			MessageBox(0, "Couldn't create shader from file.", (LPCSTR)_args.fileName, MB_OK);
			return nullptr;
		}

		T* shader = createShaderFromBytes<T>(_device, (const char*)buffer->GetBufferPointer(), buffer->GetBufferSize());
		RELEASE(buffer);
		return shader;
	}

	template <typename T>
	T* createShaderFromCSO(ID3D11Device* _device, const char* _fpath)
	{
		T* shader = nullptr;
		char* bytes = nullptr;
		int len = 0;

		if (!readBytesFromFile(_fpath, &bytes, &len))
			return nullptr;

		shader = createShaderFromBytes(_device, bytes, len);

		delete[] bytes;

		return shader;
	}


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

		static inline void remove(ID3D11DeviceContext* _dc)
		{
			_dc->VSSetShader(nullptr, 0, 0);
			_dc->HSSetShader(nullptr, 0, 0);
			_dc->DSSetShader(nullptr, 0, 0);
			_dc->GSSetShader(nullptr, 0, 0);
			_dc->PSSetShader(nullptr, 0, 0);
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
};