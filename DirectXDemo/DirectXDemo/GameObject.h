#pragma once
#include <DirectXMath.h>
#include "DrawData.h"
class ID3D11Device;


/////////////////////////////////////////////////
/// Base class for all in game objects
/////////////////////////////////////////////////
class GameObject
{
public:
	GameObject() : m_scale (DirectX::XMFLOAT3(1,1,1)) {}
	virtual ~GameObject() {}

	virtual bool load(ID3D11Device*) =0;
	virtual void unload() = 0;

	virtual void update();
	virtual void draw(const DrawData&) =0;

	static void setViewProjMat(const DirectX::XMFLOAT4X4& _viewProj);

	void setPos(const DirectX::XMFLOAT3& _pos) { m_pos = _pos; }
	void setRot(const DirectX::XMFLOAT3& _rot) { m_rot = _rot; }
	void setScale(const DirectX::XMFLOAT3& _scale) { m_scale = _scale; }

	const DirectX::XMFLOAT3& getPos() const {return m_pos;}
	const DirectX::XMFLOAT3& getRot() const { return m_rot; }
	static const DirectX::XMFLOAT4X4& getViewProj() { return s_viewProj; }
	const DirectX::XMFLOAT4X4& getWVPTranspose() const { return m_worldViewProj; }

protected:
	DirectX::XMFLOAT3 m_pos;
	DirectX::XMFLOAT3 m_rot;
	DirectX::XMFLOAT3 m_scale;

	DirectX::XMFLOAT4X4 m_worldViewProj;
	static DirectX::XMFLOAT4X4 s_viewProj;
	
};