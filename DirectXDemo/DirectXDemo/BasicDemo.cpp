#include "BasicDemo.h"
#include "SimpleGrass.h"
#include "Camera.h"
#include "Triangle.h"
BasicDemo::BasicDemo()
{}

BasicDemo::~BasicDemo()
{}

#define CHECK_FAIL(x) if (!x) return false
bool BasicDemo::load()
{
	using namespace DirectX;

	GameObject* grass = new SimpleGrass();
	m_objects.push_back(grass);
	CHECK_FAIL(grass->load(m_d3dDevice));
	grass->setPos(XMFLOAT3(0.1,0.f,0.f));

	grass = new SimpleGrass();
	m_objects.push_back(grass);
	CHECK_FAIL(grass->load(m_d3dDevice));
	grass->setPos(XMFLOAT3(0.f, 0.f, 0.f));

	grass = new SimpleGrass();
	m_objects.push_back(grass);
	CHECK_FAIL(grass->load(m_d3dDevice));
	grass->setPos(XMFLOAT3(-0.1f, 0.f, 0.f));

	GameObject* cam = new Camera();
	cam->setPos({ 0.0f, 0.6f, 0.5f });
	m_objects.push_back(cam);
	CHECK_FAIL(cam->load(m_d3dDevice));

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

void BasicDemo::update(float _dt)
{
	DX11Demo::update(_dt);

	for (auto obj : m_objects)
	{
		obj->update(_dt);
	}
}

void BasicDemo::render()
{
	if (m_d3dContext == 0) return;
	
	float col[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_d3dContext->ClearRenderTargetView(m_backBufferTarget, col);
	m_d3dContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0xFF);

	for (auto obj : m_objects)
	{
		obj->draw(m_d3dContext);
	}

	m_swapChain->Present(0, 0);
}
