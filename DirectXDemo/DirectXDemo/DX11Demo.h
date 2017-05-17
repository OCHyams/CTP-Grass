/*----------------------------------------------------------------
Author:			Orlando Cazalet-Hyams
Description :	Base class for all DX11 demos to make it quick
				and easy to set up.
				(from Beginning DirectX 11 Game Programming)
----------------------------------------------------------------*/

#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d11_2.h>
#include <d3d11.h>
#include "Time.h"
class Input;
/////////////////////////////////////////////////
/// Base class for DirectX demo program
/////////////////////////////////////////////////
class DX11Demo
{
public:
	DX11Demo();
	virtual ~DX11Demo();

	bool init(HINSTANCE _hInstance, HWND _hwnd);
	void shutdown();

	virtual bool load();
	virtual void unload();

	virtual void update();
	virtual void render() = 0;

protected:
	/////////////////////////////////////////////////
	/// Windows
	/////////////////////////////////////////////////
	HINSTANCE m_hInstance;
	HWND m_hwnd;
	/////////////////////////////////////////////////
	/// Features
	/////////////////////////////////////////////////
	D3D_DRIVER_TYPE m_driverType;
	D3D_FEATURE_LEVEL m_featureLevel;
	/////////////////////////////////////////////////
	/// Rendering
	/////////////////////////////////////////////////
	ID3D11Device* m_d3dDevice;
	ID3D11DeviceContext* m_d3dContext;
	IDXGISwapChain* m_swapChain;
	ID3D11RenderTargetView* m_backBufferTarget;
	/////////////////////////////////////////////////
	/// Depth stencil 
	/////////////////////////////////////////////////
	ID3D11Texture2D* m_depthTexture;
	ID3D11DepthStencilView* m_depthStencilView;
	/////////////////////////////////////////////////
	/// Input 
	/////////////////////////////////////////////////
	Input* m_input;
	/////////////////////////////////////////////////
	/// Time 
	/////////////////////////////////////////////////
	Clock m_clock;
	Time m_time;
};