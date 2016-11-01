#include "BasicDemo.h"
#include "SimpleGrass.h"
#include "Camera.h"
#include "Triangle.h"
#include "DrawData.h"
#include "AntTweakBar.h"
BasicDemo::BasicDemo()
{}

BasicDemo::~BasicDemo()
{}

#define CHECK_FAIL(x) if (!x) return false
bool BasicDemo::load()
{
	//Tweak bar
	TwBar* GUI = TwNewBar("Wind");
	TwDefine(" Wind position='10 10' ");
	TwDefine(" Wind size='100 150' ");
	TwDefine(" Wind movable= false ");
	TwDefine(" Wind resizable= true ");
	TwAddVarRW(GUI, "str", TwType::TW_TYPE_FLOAT, &m_windStr, "step = 0.05");
	TwAddVarRW(GUI, "dir", TwType::TW_TYPE_DIR3F, &m_wind, "opened = true axisz = -z showval = false");

	using namespace DirectX;
	m_wind = { -1.f, 0.0f, 0.0f };
	m_windStr = 0.4;

	SimpleGrass* grass = new SimpleGrass();
	m_objects.push_back(grass);
	m_grass.push_back(grass);
	CHECK_FAIL(grass->load(m_d3dDevice));
	grass->setPos(XMFLOAT3(0.1,-0.2f,0.f));
	grass->setRot(XMFLOAT3(0.f, 1.2 * XM_PIDIV4, 0.f));
	grass->setWind(m_wind);

	grass = new SimpleGrass();
	m_objects.push_back(grass);
	m_grass.push_back(grass);
	CHECK_FAIL(grass->load(m_d3dDevice));
	grass->setPos(XMFLOAT3(0.f, -0.2f, 0.f));
	grass->setRot(XMFLOAT3(0.f, 0.8 * XM_PIDIV4, 0.f));
	grass->setWind(m_wind);

	grass = new SimpleGrass();
	m_objects.push_back(grass);
	m_grass.push_back(grass);
	CHECK_FAIL(grass->load(m_d3dDevice));
	grass->setPos(XMFLOAT3(-0.1f, -0.2f, 0.f));
	grass->setRot(XMFLOAT3(0.f, 0.6 * XM_PIDIV4, 0.f));
	grass->setWind(m_wind);

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
	for (auto grass : m_grass)
	{
		using namespace DirectX;
		XMVECTOR windv = XMVectorScale(XMLoadFloat3(&m_wind), m_windStr);
		XMFLOAT3 windf3;
		XMStoreFloat3(&windf3, windv);
		grass->setWind(windf3);
	}
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

	TwDraw();
	m_swapChain->Present(0, 0);
}
