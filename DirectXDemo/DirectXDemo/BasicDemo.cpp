#include "BasicDemo.h"
#include "SimpleGrass.h"
#include "Camera.h"
#include "Triangle.h"
#include "DrawData.h"
#include "Field.h"
#include "AntTweakBar.h"
#include "objLoader.h"
#include "WindCuboid.h"
#include "WindSphere.h"
#include "Input.h"
/*Make large INTs easier to read in code*/
#define NUM(x0) x0
#define NUM(x0, x1) x0 ## x1
#define NUM(x0, x1, x2) x0 ## x1 ## x2

BasicDemo::BasicDemo()
{}

BasicDemo::~BasicDemo()
{}

#define CHECK_FAIL(x) if (!x) return false
bool BasicDemo::load()
{	
	/*Load data shared by all wind managers (though there should only be one anyway)*/
	WindManager::loadShared(m_d3dDevice);
	CHECK_FAIL(m_windManager.load(m_d3dDevice, 2, 1));
	/*Create a static wind volume, no need to keep track of mem, system does that*/
	WindCuboid* windCuboid = m_windManager.createWindCuboid();
	windCuboid->m_extents = { 100.0f, 100.0f, 100.0f };
	windCuboid->m_initalVelocity = { 0.3f, 0.f, 0.f };
	windCuboid->m_position = { 0.f, 0.f, 0.f };
	/*Create a static wind volume, no need to keep track of mem, system does that*/
	m_demoSphere = m_windManager.createWindSphere();
	m_demoSphere->m_fallOffPow = 5;
	m_demoSphere->m_initalStrength = 0.f;;
	m_demoSphere->m_position = { 5,0,0 };
	m_demoSphere->m_radius = 2.0f;

	//Tweak bar
	TwBar* GUI = TwNewBar("Settings");
	TwDefine(" Settings position='10 10' ");
	TwDefine(" Settings size='100 200' ");
	TwDefine(" Settings movable= false ");
	TwDefine(" Settings resizable= true ");
	//TwAddVarRW(GUI, "str", TwType::TW_TYPE_FLOAT, &m_demoSphere->m_initalStrength, "step = 0.05");
	//TwAddVarRW(GUI, "rad", TwType::TW_TYPE_FLOAT, &m_demoSphere->m_radius, "step = 0.05");
	//TwAddVarRW(GUI, "pow", TwType::TW_TYPE_FLOAT, &m_demoSphere->m_fallOffPow, "step = 0.05");
	TwAddVarRW(GUI, "draw octree", TwType::TW_TYPE_BOOL32, &m_field.drawOctree, "");
	TwAddVarRO(GUI, "FPS", TwType::TW_TYPE_FLOAT, &m_fps, "");
	TwAddVarRO(GUI, "Total Blade count", TwType::TW_TYPE_INT32, &m_numBlades, "");
	TwAddVarRO(GUI, "Blades Drawn", TwType::TW_TYPE_INT32, &m_numDrawnBlades, "");

	using namespace DirectX;
	m_fps = 0;

	/*Load data shared by all fields*/
	CHECK_FAIL(Field::loadShared(m_d3dDevice));

	/*Set up demo field*/
	m_field.m_halfGrassWidth = 0.02f;
	m_field.m_windManager = &m_windManager;

	/*Load hills model for grass*/
	ObjModel model;
	XMMATRIX transform = XMMatrixScalingFromVector(VEC3(1, 0, 1));
	//CHECK_FAIL(model.loadOBJ("../Resources/plane.txt"));
	CHECK_FAIL(model.loadPlane(50, 50, 5, 5));
	CHECK_FAIL(m_field.load(m_d3dDevice, &model, NUM(25), XMFLOAT3(0, 0, 0), {5.f, 25, 5.f}/*, transform*/));
	m_numBlades = m_field.getMaxNumBlades();
	/*Only needed the hill model to place the grass (FOR NOW ANYWAY)*/
	model.release();

	m_cam = new ArcCamera({ 0.f, 0.f, 0.f });
	m_objects.push_back(m_cam);
	CHECK_FAIL(m_cam->load(m_d3dDevice));

	return true;
}
#undef CHECK_FAIL

void BasicDemo::unload()
{
	WindManager::unloadShared();
	Field::unloadShared();
	m_field.unload();
	m_windManager.unload();

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

	/*Move the demo wind sphere around*/
	using namespace DirectX;
	XMVECTOR windSphereNewPos = VEC3(	(int)m_input->getKey(DIK_LEFT) * -1 + (int)m_input->getKey(DIK_RIGHT),
										(int)m_input->getKey(DIK_DOWN) * -1 + (int)m_input->getKey(DIK_UP),
										0);
	windSphereNewPos *= m_time.deltaTime * 0.5f;
	//windSphereNewPos += LF3(&m_demoSphere->m_position);
	//STOREF3(&m_demoSphere->m_position, windSphereNewPos);
	
	
	Field::updateCameraPosition(m_cam->getPos());
	m_field.s_viewproj = GameObject::getViewProj();//@move this out onto camera, and call this function in draw instead of out here...
	m_field.update();

	if (m_time.deltaTime > 0)
	{
		m_fps = 1 / m_time.deltaTime;
	}
	m_numDrawnBlades = m_field.getCurNumBlades();
}

void BasicDemo::render()
{
	if (m_d3dContext == 0) return;

	float col[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_d3dContext->ClearRenderTargetView(m_backBufferTarget, col);
	m_d3dContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0xff);

	DrawData data = { m_cam, m_d3dContext };

	/*Update wind resources*/
	m_windManager.updateResources(m_d3dContext);

	using namespace DirectX;
	m_field.draw(data);

	/*for (auto obj : m_objects)
	{
		obj->draw(data);
	}*/

	TwDraw();
	m_swapChain->Present(0, 0);
}
