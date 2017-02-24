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
#include "MeshInfo.h"
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
	m_renderer.load(m_d3dDevice);

	/*Load data shared by all wind managers (though there should only be one anyway)*/
	WindManager::loadShared(m_d3dDevice);
	CHECK_FAIL(m_windManager.load(m_d3dDevice, 1, 1));
	/*Create a static wind volume, no need to keep track of mem, system does that*/
	WindCuboid* windCuboid = m_windManager.createWindCuboid();
	windCuboid->m_extents = { 100.0f, 100.0f, 100.0f };
	windCuboid->m_initalVelocity = { 0.3f, 0.f, 0.f };
	windCuboid->m_position = { 0.f, 0.f, 0.f };


	using namespace DirectX;

	/*Load data shared by all fields*/
	CHECK_FAIL(GrassObject::loadShared(m_d3dDevice));

	/*Set up demo field*/
	m_field.m_halfGrassWidth = 0.02f;
	m_field.m_windManager = &m_windManager;

	/*Load hills model for grass*/
	MeshInfo* pMesh = m_renderer.getMeshInfo(MESH::HILL);
	CHECK_FAIL(pMesh);

	CHECK_FAIL(m_field.load(m_d3dDevice, pMesh->m_vbs.at(0), pMesh->m_vCount, 100));


	m_cam = new ArcCamera({ 0.f, 0.f, 0.f });
	m_objects.push_back(m_cam);
	CHECK_FAIL(m_cam->load(m_d3dDevice));

	return true;
}
#undef CHECK_FAIL

void BasicDemo::unload()
{
	WindManager::unloadShared();
	m_windManager.unload();
	Field::unloadShared();
	m_field.unload();

	for (auto obj : m_objects)
	{
		if (obj)
		{
			obj->unload();
			delete obj;
		}
	}

	OctreeDebugger::unloadShared();
	m_renderer.cleanup();
}

void BasicDemo::update()
{
	DX11Demo::update();

	for (auto obj : m_objects)
	{
		obj->update();
	}

	/*Move the demo wind sphere around*/
	/*using namespace DirectX;
	XMVECTOR windSphereNewPos = VEC3(	(int)m_input->getKey(DIK_LEFT) * -1 + (int)m_input->getKey(DIK_RIGHT),
										(int)m_input->getKey(DIK_DOWN) * -1 + (int)m_input->getKey(DIK_UP),
										0);
	windSphereNewPos *= m_time.deltaTime * 0.5f;
	windSphereNewPos += LF3(&m_demoSphere->m_position);*/
	
	
	//Field::updateCameraPosition(m_cam->getPos());
	//m_field.s_viewproj = GameObject::getViewProj();//@move this out onto camera, and call this function in draw instead of out here...
	//m_field.update();
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
	m_field.draw(data);

	m_renderer.render(data);

	TwDraw();
	m_swapChain->Present(0, 0);
}
