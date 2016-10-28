#pragma once
#include "GameObject.h"
#include <DirectXMath.h>
//basic boring static camera
class Camera : public GameObject
{
public:
	Camera() : Camera(DirectX::XMFLOAT3(0.f, 0.f, 0.f), DirectX::XMFLOAT3(0.f, 1.f, 0.f)) {}
	Camera(const DirectX::XMFLOAT3& _target) : Camera(_target, DirectX::XMFLOAT3(0.f,1.f,0.f)) {}
	Camera(const DirectX::XMFLOAT3& _target, const DirectX::XMFLOAT3& _up) : m_target(_target), m_up(_up) {}
	virtual ~Camera() {}

	virtual bool load(ID3D11Device*) override;
	virtual void unload() override;

	virtual void update() override;
	virtual void draw(ID3D11DeviceContext* _dc) override;

	const DirectX::XMFLOAT4X4& getViewMatrix();
	const DirectX::XMFLOAT4X4& getProjMatrix();

private:
	DirectX::XMFLOAT3 m_target;
protected:
	void updateViewProjForGO();
	
	DirectX::XMFLOAT3 m_up;
	DirectX::XMFLOAT4X4 m_proj;
	DirectX::XMFLOAT4X4 m_view;
};