#pragma once

#include <d3d11_2.h>
#include "Buffer.h"

class DoubleBuffer
{
public:

	void swap();
	//use front buffer for processing
	const Buffer* front();
	//write to backbuffer
	const Buffer* back();


	bool init(	ID3D11Device* _device, 
				const D3D11_BUFFER_DESC* _bufferDesc, 
				const D3D11_SUBRESOURCE_DATA* _initialData, 
				const D3D11_UNORDERED_ACCESS_VIEW_DESC* _UAVDesc, 
				const D3D11_SHADER_RESOURCE_VIEW_DESC* _SRVDesc);

	void cleanup();
	
private:
	
	Buffer m_first;
	Buffer m_second;
	Buffer* m_front = nullptr;
	Buffer* m_back = nullptr;
};