#include "GameObject.h"

DirectX::XMFLOAT4X4 GameObject::s_viewProj = DirectX::XMFLOAT4X4();

void GameObject::update()
{
	using namespace DirectX;
	XMMATRIX scale = XMMatrixScalingFromVector(XMLoadFloat3(&m_scale));
	XMMATRIX rotation = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_rot));
	XMMATRIX translation = XMMatrixTranslationFromVector(XMLoadFloat3(&m_pos));


	XMMATRIX world = XMMatrixMultiply(scale, rotation);
	world = XMMatrixMultiply(world, translation);

	XMMATRIX viewproj = XMLoadFloat4x4(&s_viewProj);
	XMMATRIX worldviewproj = XMMatrixMultiply(world, viewproj);


	worldviewproj = XMMatrixTranspose(worldviewproj);
	XMStoreFloat4x4(&m_worldViewProj, worldviewproj);
}

void GameObject::setViewProjMat(const DirectX::XMFLOAT4X4 & _viewProj)
{
	s_viewProj = _viewProj;
}
