#include "Camera.h"
#include <d3d11_2.h>
#include "ServiceLocator.h"
#include "Input.h"
#include "Time.h"
#include "AntTweakBar.h"
#include "Shorthand.h"
bool ArcCamera::load(ID3D11Device*)
{
	//Tweak bar
	TwBar* GUI = TwNewBar("Cam");
	TwDefine(" Cam position='10 200' ");
	TwDefine(" Cam size='100 150' ");
	TwDefine(" Cam movable= false ");
	TwDefine(" Cam resizable= true ");
	TwAddVarRW(GUI, "x", TwType::TW_TYPE_FLOAT, &(m_pos.x), "");
	TwAddVarRW(GUI, "y", TwType::TW_TYPE_FLOAT, &(m_pos.y), "");
	TwAddVarRW(GUI, "z", TwType::TW_TYPE_FLOAT, &(m_pos.z), "");

	m_rot = { 0,0,0 };

	using namespace DirectX;
	XMStoreFloat4x4(&m_view, XMMatrixLookAtLH(XMLoadFloat3(&m_pos), XMLoadFloat3(&m_target), XMLoadFloat3(&m_up)));
	XMStoreFloat4x4(&m_proj, DirectX::XMMatrixPerspectiveFovLH(XM_PIDIV4, 640 /480 , 0.1f, 50));

	updateViewProjForGO();
	return true;
}

void ArcCamera::unload()
{

}

void ArcCamera::update()
{
	//panning for testing grass - this should all get ripped out into it's own class
	using namespace DirectX;
	using TIME = OCH::ServiceLocator<Time>;
	using INPUT = OCH::ServiceLocator<Input>;
	float speed = 0.3 * TIME::get()->deltaTime;

	XMFLOAT3 euler = m_rot;

	if (INPUT::get()->getKey(DIK_A))
	{
		euler.y += speed;
		
	}
	else if (INPUT::get()->getKey(DIK_D))
	{
		euler.y -= speed;

	}

	if (INPUT::get()->getKey(DIK_W))
	{
		euler.x += speed;
	
	}
	else if (INPUT::get()->getKey(DIK_S))
	{
		euler.x -= speed;
	}

	if (INPUT::get()->getKey(DIK_Q))
	{
		m_offset.z += speed;
	}
	else if (INPUT::get()->getKey(DIK_E))
	{
		m_offset.z -= speed;
	}

	XMVECTOR offset = XMVector3Rotate(LF3(&m_offset), QUAT(LF3(&euler)));
	XMStoreFloat3(&m_pos, LF3(&m_target) + offset);
	m_rot = euler;

	XMVECTOR eye = XMLoadFloat3(&m_pos);
	XMVECTOR target = XMLoadFloat3(&m_target);
	XMVECTOR up = XMLoadFloat3(&m_up);
	XMMATRIX lookat = XMMatrixLookAtLH(eye, target, up);
	XMStoreFloat4x4(&m_view, lookat);

	updateViewProjForGO();
	return;
}

void ArcCamera::draw(const DrawData&)
{
	return;
}

const DirectX::XMFLOAT4X4& ArcCamera::getViewMatrix()
{
	return m_view;
}

const DirectX::XMFLOAT4X4& ArcCamera::getProjMatrix()
{
	return m_proj;
}

void ArcCamera::updateViewProjForGO()
{
	using namespace DirectX;
	XMMATRIX view = XMLoadFloat4x4(&m_view);
	XMMATRIX proj = XMLoadFloat4x4(&m_proj);
	XMMATRIX viewprojmat = XMMatrixMultiply(view, proj);
	XMFLOAT4X4 viewproj4x4;
	XMStoreFloat4x4(&viewproj4x4, viewprojmat);
	GameObject::setViewProjMat(viewproj4x4);
}
