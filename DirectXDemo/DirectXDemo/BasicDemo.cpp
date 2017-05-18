#include "BasicDemo.h"
#include "Camera.h"
#include "DrawData.h"
#include "Field.h"
#include "AntTweakBar.h"
#include "objLoader.h"
#include "WindCuboid.h"
#include "WindSphere.h"
#include "Input.h"
#include "GPUOctreeDebugger.h"

DX11GrassDemo::DX11GrassDemo()
{}

DX11GrassDemo::~DX11GrassDemo()
{}

static const float GEN_HILL_HEIGHT = 20.f;
bool DX11GrassDemo::load()
{	
	RETURN_IF_FAILED(m_renderer.load(m_d3dDevice));
	/*Models!*/
	using namespace DirectX;
	/*XMMATRIX v44_transform = XMMatrixScalingFromVector(VEC3(0.2, 0.2, 0.2));
	XMMATRIX v44_translation = XMMatrixTranslation(0.0f, -3.0f, 0.0f);
	v44_transform = XMMatrixMultiply(v44_transform, v44_translation);
	XMFLOAT4X4 f44_transform;
	XMStoreFloat4x4(&f44_transform, v44_transform);*/
	//CHECK_FAIL(m_renderer.registerMesh((int)MESH::HILL, "../Resources/hill_tris.txt", ObjModel::QUAD_STRIP, f44_transform, m_d3dDevice));

	
	/*v44_transform = XMMatrixScalingFromVector(VEC3(4, 2, 4));
	v44_translation = XMMatrixTranslation(0.0f, -2, 4);
	v44_transform = XMMatrixMultiply(v44_transform, v44_translation);
	XMStoreFloat4x4(&f44_transform, v44_transform);*/
	//CHECK_FAIL(m_renderer.registerMesh((int)MESH::SMALL_HILLS, "../Resources/SmallHills.obj", ObjModel::TRIANGLE_STRIP, f44_transform, m_d3dDevice));

	ObjModel plane;
	plane.loadPlane(125,125,20,20);
	RETURN_IF_FAILED(m_renderer.registerMeshFromObjModel((int)MESH::PLANE, plane, m_d3dDevice));
	
	ObjModel hill;
	hill.loadHill(150, 150, 10, 10, GEN_HILL_HEIGHT);
	RETURN_IF_FAILED(m_renderer.registerMeshFromObjModel((int)MESH::GEN_HILL, hill, m_d3dDevice));

	RETURN_IF_FAILED(GPUOctreeDebugger::loadShared(m_d3dDevice));

	/*Load data shared by all wind managers (though there should only be one anyway)*/
	WindManager::loadShared(m_d3dDevice);
	RETURN_IF_FAILED(m_windManager.load(m_d3dDevice, 1, 1));
	/* Create a static wind volume, fire and forget on the cuboid, don't care about it anymore,
		createWindCuboid() added it to an internal container. */
	WindCuboid* windCuboid = m_windManager.createWindCuboid();
	windCuboid->m_extents = { 1000.0f, 1000.0f, 1000.0f };
	windCuboid->m_initalVelocity = { 0.3f, 0.f, 0.f };
	windCuboid->m_position = { 0.f, 0.f, 0.f };
	/* Create a static wind volume */
	m_demoSphere = m_windManager.createWindSphere();
	m_demoSphere->m_fallOffPow = 5;
	m_demoSphere->m_initalStrength = 0.f;;
	m_demoSphere->m_position = { 0,0,0 };
	m_demoSphere->m_radius = 2.0f;

	//AntTweakBar
	TwBar* GUI = TwNewBar("Settings");
	TwDefine(" Settings position='10 10' ");
	TwDefine(" Settings size='300 200' ");
	TwDefine(" Settings movable= false ");
	TwDefine(" Settings resizable= true ");
	TwAddVarRW(GUI, "draw gpu  octree", TwType::TW_TYPE_BOOLCPP, &m_field.m_drawGPUOctree, "");
	TwAddVarRW(GUI, "no frustum culling", TwType::TW_TYPE_BOOLCPP, &m_field.m_noCulling, "");
	TwAddVarRW(GUI, "str", TwType::TW_TYPE_FLOAT, &m_demoSphere->m_initalStrength, "step = 0.05");
	TwAddVarRW(GUI, "rad", TwType::TW_TYPE_FLOAT, &m_demoSphere->m_radius, "step = 0.05");
	TwAddVarRW(GUI, "pow", TwType::TW_TYPE_FLOAT, &m_demoSphere->m_fallOffPow, "step = 0.05");
	TwAddVarRO(GUI, "FPS", TwType::TW_TYPE_FLOAT, &m_fps, "");
	TwAddVarRO(GUI, "Total Blade count", TwType::TW_TYPE_INT32, &m_numBlades, "");

	using namespace DirectX;
	m_fps = 0;

	/*Load data shared by all fields*/
	RETURN_IF_FAILED(Field::loadShared(m_d3dDevice));

	/*Set up demo field*/
	MESH meshToUse = MESH::GEN_HILL;
	ObjModel* meshObj = &hill;
	RETURN_IF_FAILED(m_field.load(m_d3dDevice, meshObj, 125, XMFLOAT3(0, 0, 0), { 6.f, 6.f, 6.f}, &m_windManager));
	//Update LOD and width settings like this...
	CBField_RarelyChanges fieldData = *m_field.getCurrentLODAndWidth();
	fieldData.halfGrassWidth = 0.02f;
	m_field.updateLODAndWidth(fieldData);

	m_numBlades = m_field.getMaxNumBlades();

	/* Free mem of intermidiate representation */
	plane.release();
	hill.release();

	MeshObject* mesh = new MeshObject();
	mesh->m_meshID = meshToUse;
	mesh->m_renderFlags = (int)FX::DEFAULT;
	m_renderer.addToRenderList(mesh);
	m_objects.push_back(mesh);

	m_cam = new ArcCamera({ 0.f, GEN_HILL_HEIGHT + 0.2f, -3.f });
	m_cam->m_arcSpeed = 1;
	m_objects.push_back(m_cam);
	RETURN_IF_FAILED(m_cam->load(m_d3dDevice));

	return true;
}
#undef CHECK_FAIL

void DX11GrassDemo::unload()
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
	
	GPUOctreeDebugger::unloadShared();
	m_renderer.cleanup();
}

void DX11GrassDemo::update()
{
	DX11Demo::update();

	for (auto obj : m_objects)
	{
		obj->update();
	}

	/*Move the demo wind sphere around*/
	using namespace DirectX;
	XMVECTOR windSphereNewPos = VEC3(	(int)m_input->getKey(DIK_LEFT) * -1 + (int)m_input->getKey(DIK_RIGHT),
										0,
										(int)m_input->getKey(DIK_DOWN) * -1 + (int)m_input->getKey(DIK_UP));

	windSphereNewPos *= m_time.deltaTime * 0.5f;
	windSphereNewPos += LF3(&m_demoSphere->m_position);
	STOREF3(&m_demoSphere->m_position, windSphereNewPos);
	m_demoSphere->m_position.y = GEN_HILL_HEIGHT;

	if (m_time.deltaTime > 0)
	{
		m_fps = 1 / m_time.deltaTime;
	}
}

void DX11GrassDemo::render()
{
	if (m_d3dContext == 0) return;

	float col[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_d3dContext->ClearRenderTargetView(m_backBufferTarget, col);
	m_d3dContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0xff);

	DrawData data = { m_cam, m_d3dContext, m_time.time, m_time.deltaTime };

	m_field.draw(data);
	m_renderer.render(data);

	TwDraw();
	m_swapChain->Present(0, 0);
}
