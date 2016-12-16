#include "DX11Demo.h"
#include "dxerr.h"
#include "ServiceLocator.h" 
#include "Input.h"
#include "AntTweakBar.h"
#include "DXHelper.h"
DX11Demo::DX11Demo() :	m_driverType(D3D_DRIVER_TYPE_NULL), 
						m_featureLevel(D3D_FEATURE_LEVEL_11_1),
						m_d3dDevice(nullptr), 
						m_d3dContext(nullptr), 
						m_swapChain(nullptr), 
						m_backBufferTarget(nullptr),
						m_input(nullptr)
{}

DX11Demo::~DX11Demo()
{
	
}

void DX11Demo::shutdown()
{
	unload();

	OCH::ServiceLocator<Time>::remove(&m_time);
	if (m_input)
	{
		OCH::ServiceLocator<Input>::remove(m_input);
		delete m_input;
	}

	RELEASE(m_depthTexture);
	RELEASE(m_depthStencilView);
	RELEASE(m_backBufferTarget);
	RELEASE(m_swapChain);
	RELEASE(m_d3dContext);
	RELEASE(m_d3dDevice);

	TwTerminate();
}

bool DX11Demo::load()
{
	return true;
}

void DX11Demo::unload()
{
	//override with derrived functionality
}

void DX11Demo::update()
{
	m_clock.updateGameTime(m_time);
	m_input->update();
}


bool DX11Demo::init(HINSTANCE _hInstance, HWND _hwnd)
{
	m_hInstance = _hInstance;
	m_hwnd = _hwnd;

	RECT dim;
	GetClientRect(_hwnd, &dim);
	unsigned int w = dim.right - dim.left;
	unsigned int h = dim.bottom - dim.top;

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_SOFTWARE
	};
	unsigned int totalDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] = 
	{
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0
	};
	unsigned int totalFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SAMPLE_DESC msaaDesc;
	ZeroMemory(&msaaDesc, sizeof(msaaDesc));
	msaaDesc.Count = 8;
	msaaDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;

	DXGI_SWAP_CHAIN_DESC scDesc;
	ZeroMemory(&scDesc, sizeof(scDesc));
	scDesc.BufferCount = 1;
	scDesc.BufferDesc.Width = w;
	scDesc.BufferDesc.Height = h;
	scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.BufferDesc.RefreshRate.Numerator = 60;
	scDesc.BufferDesc.RefreshRate.Denominator = 1;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.OutputWindow = _hwnd;
	scDesc.Windowed = true;
	scDesc.SampleDesc = msaaDesc;

	unsigned int creationFlags = 0;

#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	HRESULT result;
	unsigned int driver = 0;

	//loop through driver types, starting at most desirable and attempt to create device & sc
	for (driver = 0; driver < totalDriverTypes; ++driver)
	{
		result = D3D11CreateDeviceAndSwapChain(	NULL, driverTypes[driver], NULL,
												creationFlags, featureLevels, totalFeatureLevels,
												D3D11_SDK_VERSION, &scDesc, &m_swapChain,
												&m_d3dDevice, &m_featureLevel, &m_d3dContext);

		if (SUCCEEDED(result))
		{
			m_driverType = driverTypes[driver];
			break;
		}
	}
	if (FAILED(result))
	{
		DXTRACE_MSG(L"failed to created the d3d device.");
		return false;
	}

	ID3D11Texture2D* backBufferTex=nullptr;
	result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferTex);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Failed to get the swap chain back buffer");
		return false;
	}

	result = m_d3dDevice->CreateRenderTargetView(backBufferTex, nullptr, &m_backBufferTarget);
	if (backBufferTex) backBufferTex->Release();
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Failed to create render target view.");
		return false;
	}

	//depth stencil 
	D3D11_TEXTURE2D_DESC depthTexDesc;
	ZeroMemory(&depthTexDesc, sizeof(depthTexDesc));
	depthTexDesc.Width = w;
	depthTexDesc.Height = h;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.ArraySize = 1;
	depthTexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthTexDesc.SampleDesc = msaaDesc;
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthTexDesc.CPUAccessFlags = 0;
	depthTexDesc.MiscFlags = 0;
	result = m_d3dDevice->CreateTexture2D(&depthTexDesc, NULL, &m_depthTexture);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Failed to create depth texture.");
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));
	dsvDesc.Format = depthTexDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;//D3D11_DSV_DIMENSION_TEXTURE2D; changed when adding MSAA
	dsvDesc.Texture2D.MipSlice = 0;
	result = m_d3dDevice->CreateDepthStencilView(m_depthTexture, &dsvDesc, &m_depthStencilView);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"Failed to create depth stencil view.");
		return false;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////
	////testing depth stuff!
	//D3D11_DEPTH_STENCIL_DESC dsDesc;
	//// Depth test parameters
	//dsDesc.DepthEnable = true;
	//dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	//dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	//// Stencil test parameters
	//dsDesc.StencilEnable = true;
	//dsDesc.StencilReadMask = 0xFF;
	//dsDesc.StencilWriteMask = 0xFF;
	//// Stencil operations if pixel is front-facing
	//dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	//dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	//dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	//dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	//// Stencil operations if pixel is back-facing
	//dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	//dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	//dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	//dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	//// Create depth stencil state
	//ID3D11DepthStencilState * pDSState;
	//m_d3dDevice->CreateDepthStencilState(&dsDesc, &pDSState);
	//m_d3dContext->OMSetDepthStencilState(pDSState, 1);
	////////////////////////////////////////////////////////////////////////////////////////////

	m_d3dContext->OMSetRenderTargets(1, &m_backBufferTarget, m_depthStencilView);
	
	D3D11_VIEWPORT viewport;
	viewport.Width = static_cast<float>(w);
	viewport.Height = static_cast<float>(h);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	m_d3dContext->RSSetViewports(1, &viewport);

	bool success;
	//input
	m_input = new Input();
	success = m_input->init(m_hInstance,m_hwnd);
	OCH::ServiceLocator<Input>::add(m_input);

	//time
	OCH::ServiceLocator<Time>::add(&m_time);
	m_clock.reset();
	m_clock.updateGameTime(m_time);

	//Anttweak
	//set up GUI library
	TwInit(TW_DIRECT3D11, m_d3dDevice);
	TwWindowSize(w, h);

	return success && load();
}
