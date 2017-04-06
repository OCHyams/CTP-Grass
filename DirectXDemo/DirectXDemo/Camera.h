#pragma once
#include "GameObject.h"
#include <DirectXMath.h>
#include "Shorthand.h"

//basic boring static camera
class ArcCamera : public GameObject
{
public:
	bool m_lockedTarget = true;
	ArcCamera() : m_target(0.f, 0.f, 0.f), m_up(0.f, 1.f, 0.f), m_offset(0.f, 0.f, -2.f) {}
	ArcCamera(const DirectX::XMFLOAT3& _target) : m_target(_target), m_up(0.f,1.f,0.f), m_offset(0.f, 0.f, -2.f) {}
	ArcCamera(const DirectX::XMFLOAT3& _target, const DirectX::XMFLOAT3& _up) : m_target(_target), m_up(_up), m_offset(0.f, 0.f, -2.f) {}
	ArcCamera(const DirectX::XMFLOAT3& _target, const DirectX::XMFLOAT3& _up, const DirectX::XMFLOAT3& _offset) : m_target(_target), m_up(_up), m_offset(_offset) {}
	virtual ~ArcCamera() {}

	float m_arcSpeed;

	virtual bool load(ID3D11Device*) override;
	virtual void unload() override;

	virtual void update() override;
	virtual void draw(const DrawData&) override;

	const DirectX::XMFLOAT4X4& getViewMatrix() const;
	const DirectX::XMFLOAT4X4& getProjMatrix() const;
	const DirectX::XMMATRIX calcLargeProjMatrix() const;

	DirectX::XMVECTOR calcViewDir() const
	{
		return DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(LF3(&m_target), LF3(&m_pos)));
	}
private:
	DirectX::XMFLOAT3 m_target;
	DirectX::XMFLOAT3 m_offset;

protected:
	void updateViewProjForGO();
	
	DirectX::XMFLOAT3 m_up;
	DirectX::XMFLOAT4X4 m_proj;
	DirectX::XMFLOAT4X4 m_view;
};