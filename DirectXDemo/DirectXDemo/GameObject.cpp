#include "GameObject.h"

DirectX::XMFLOAT4X4 GameObject::s_viewProj = DirectX::XMFLOAT4X4();

void GameObject::update()
{
	using namespace DirectX;
	XMMATRIX world = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_rot));
	world = XMMatrixMultiply(world, XMMatrixTranslationFromVector(XMLoadFloat3(&m_pos)));
	XMMATRIX viewproj = XMLoadFloat4x4(&s_viewProj);
	XMStoreFloat4x4(&m_worldViewProj, XMMatrixMultiply(world, viewproj));
}

void GameObject::setViewProjMat(const DirectX::XMFLOAT4X4 & _viewProj)
{
	s_viewProj = _viewProj;
}
