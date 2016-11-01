#include "BasicDemo.h"
#include "SimpleGrass.h"
#include "Camera.h"
#include "Triangle.h"
#include "DrawData.h"
BasicDemo::BasicDemo()
{}

BasicDemo::~BasicDemo()
{}

#define CHECK_FAIL(x) if (!x) return false
bool BasicDemo::load()
{
	using namespace DirectX;
	XMFLOAT3 wind = { 0.5f, 0.0f, 0.0f };

	SimpleGrass* grass = new SimpleGrass();
	m_objects.push_back(grass);
	CHECK_FAIL(grass->load(m_d3dDevice));
	grass->setPos(XMFLOAT3(0.1,-0.2f,0.f));
	grass->setWind(wind);

	grass = new SimpleGrass();
	m_objects.push_back(grass);
	CHECK_FAIL(grass->load(m_d3dDevice));
	grass->setPos(XMFLOAT3(0.f, -0.2f, 0.f));
	grass->setWind(wind);

	grass = new SimpleGrass();
	m_objects.push_back(grass);
	CHECK_FAIL(grass->load(m_d3dDevice));
	grass->setPos(XMFLOAT3(-0.1f, -0.2f, 0.f));
	grass->setRot(XMFLOAT3(0.f, XM_PIDIV2, 0.f));
	grass->setWind(wind);

	m_cam = new Camera();
	m_cam->setPos({ 0.0f, 0.6f, -0.5f });
	m_objects.push_back(m_cam);
	CHECK_FAIL(m_cam->load(m_d3dDevice));

	return true;
}
#undef CHECK_FAIL

void BasicDemo::unload()
{
	for (auto obj : m_objects)
	{
		if (obj)
		{
			delete obj;
		}
	}
}

void BasicDemo::update()
{
	DX11Demo::update();

	for (auto obj : m_objects)
	{
		obj->update();
	}
}

void BasicDemo::render()
{
	if (m_d3dContext == 0) return;
	
	float col[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_d3dContext->ClearRenderTargetView(m_backBufferTarget, col);
	m_d3dContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0xff);

	DrawData data = { m_cam, m_d3dContext };

	for (auto obj : m_objects)
	{
		obj->draw(data);
	}

	m_swapChain->Present(0, 0);
}
