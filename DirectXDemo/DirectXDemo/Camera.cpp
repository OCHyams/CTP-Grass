#include "Camera.h"
#include <d3d11_2.h>

bool Camera::load(ID3D11Device*)
{
	using namespace DirectX;
	XMStoreFloat4x4(&m_view, XMMatrixLookAtLH(XMLoadFloat3(&m_pos), XMLoadFloat3(&m_target), XMLoadFloat3(&m_up)));
	XMStoreFloat4x4(&m_proj, DirectX::XMMatrixPerspectiveFovLH(XM_PIDIV4, 640 / 480, 0.001f, 300.f)); //make this less magic number-y 


	updateViewProjForGO();
	return true;
}

void Camera::unload()
{

}

void Camera::update(float _dt)
{

	return;
}

void Camera::draw(ID3D11DeviceContext* _dc)
{
	return;
}

const DirectX::XMFLOAT4X4& Camera::getViewMatrix()
{
	return m_view;
}

const DirectX::XMFLOAT4X4& Camera::getProjMatrix()
{
	return m_proj;
}

void Camera::updateViewProjForGO()
{
	using namespace DirectX;
	XMMATRIX view = XMLoadFloat4x4(&m_view);
	XMMATRIX proj = XMLoadFloat4x4(&m_proj);
	XMMATRIX viewprojmat = XMMatrixMultiply(view, proj);
	XMFLOAT4X4 viewproj4x4;
	XMStoreFloat4x4(&viewproj4x4, viewprojmat);
	GameObject::setViewProjMat(viewproj4x4);
}
