#include "DXHelper.h"
#include "dxerr.h"
#include <windows.h>
bool DXHelper::createBasicConstBuffer(ID3D11Buffer** _dst, ID3D11Device* _device,int _sizeOfBuffer, const std::wstring& _errMsg)
{
	D3D11_BUFFER_DESC constDesc;
	ZeroMemory(&constDesc, sizeof(constDesc));
	constDesc.Usage = D3D11_USAGE_DYNAMIC;
	constDesc.ByteWidth = _sizeOfBuffer;
	constDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constDesc.MiscFlags = 0;
	constDesc.StructureByteStride = 0;
	HRESULT result = _device->CreateBuffer(&constDesc, NULL, _dst);
	if (FAILED(result))
	{
		std::wstring concatted_stdstr = L"Error: " + _errMsg;
		LPCWSTR concatted = concatted_stdstr.c_str();
		DXTRACE_MSG(concatted);
		return false;
	}

	return true;
}
