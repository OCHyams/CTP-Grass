#pragma once
#include <d3d11.h>
#include <string>
namespace DXHelper
{

	bool createBasicConstBuffer(ID3D11Buffer** _dst, ID3D11Device* _device, int _sizeOfBuffer, const std::wstring& _errMsg = L"");

}

#ifndef RELEASE
#define RELEASE(x) if (x) { x->Release(); x = nullptr; }
#endif