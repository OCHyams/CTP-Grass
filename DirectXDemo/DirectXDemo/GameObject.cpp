#include "GameObject.h"

DirectX::XMFLOAT4X4 GameObject::s_viewProj = DirectX::XMFLOAT4X4();

void GameObject::update(float _dt)
{
	using namespace DirectX;
	XMMATRIX world = XMMatrixTranslationFromVector(XMLoadFloat3(&m_pos));
	XMMATRIX viewproj = XMLoadFloat4x4(&s_viewProj);
	XMStoreFloat4x4(&m_worldViewProj, XMMatrixMultiply(world, viewproj));
}

void GameObject::setViewProjMat(const DirectX::XMFLOAT4X4 & _viewProj)
{
	s_viewProj = _viewProj;
}
