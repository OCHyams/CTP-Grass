#include "BasicDemo.h"
#include "SimpleGrass.h"
#include "Camera.h"
#include "Triangle.h"
#include "DrawData.h"
#include "Plane.h"
#include "Field.h"
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
	TwAddVarRO(GUI, "FPS", TwType::TW_TYPE_FLOAT, &m_fps, "");

	using namespace DirectX;
	m_wind = { 0.f, 0.0f, 1.0f };
	m_windStr = 0.2;

	m_fps = 0;

	Field::loadShared(m_d3dDevice);
	m_field.m_halfGrassWidth = 0.05f;//0.012f;
							//1,000,000
	m_field.load(m_d3dDevice, 1000000, { 50, 50 }, {-25,0,-25});
	
	
	m_cam = new Camera({0,0.5f, 3.f});
	m_cam->setPos({ 0.0f, 1.f, -3.f });
	m_objects.push_back(m_cam);
	CHECK_FAIL(m_cam->load(m_d3dDevice));

	return true;
}
#undef CHECK_FAIL

void BasicDemo::unload()
{
	Field::unloadShared();

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
	
	using namespace DirectX;
	XMVECTOR windv = XMVectorScale(XMLoadFloat3(&m_wind), m_windStr);
	XMFLOAT3 windf3;
	DirectX::XMStoreFloat3(&windf3, windv);
	m_field.setWind(windf3);

	for (auto obj : m_objects)
	{
		obj->update();
	}

	Field::updateCameraPosition(m_cam->getPos());
	m_field.s_viewproj = GameObject::getViewProj();//move this out onto camera, and call this function in draw instead of out here...
	m_field.update();

	if (m_time.deltaTime > 0)
	{
		m_fps = 1 / m_time.deltaTime;
	}
}

void BasicDemo::render()
{
	if (m_d3dContext == 0) return;

	float col[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_d3dContext->ClearRenderTargetView(m_backBufferTarget, col);
	m_d3dContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0xff);

	DrawData data = { m_cam, m_d3dContext };

	using namespace DirectX;
	m_field.draw(data);

	/*for (auto obj : m_objects)
	{
		obj->draw(data);
	}*/

	TwDraw();
	m_swapChain->Present(0, 0);
}
