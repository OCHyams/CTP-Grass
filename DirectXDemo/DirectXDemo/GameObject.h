#pragma once
#include <DirectXMath.h>

class ID3D11DeviceContext;
class ID3D11Device;


/////////////////////////////////////////////////
/// Base class for all in game objects
/////////////////////////////////////////////////
class GameObject
{
public:
	GameObject() {}
	virtual ~GameObject() {}

	virtual bool load(ID3D11Device*) =0;
	virtual void unload() = 0;

	virtual void update(float _dt); //should really do matrix updates here! see codebase from Simon's projects
	virtual void draw(ID3D11DeviceContext* _dc) =0;

	static void setViewProjMat(const DirectX::XMFLOAT4X4& _viewProj);

	void setPos(const DirectX::XMFLOAT3& _pos) { m_pos = _pos; }
	void setRot(const DirectX::XMFLOAT3& _rot) { m_rot = _rot; }
protected:
	DirectX::XMFLOAT3 m_pos;
	DirectX::XMFLOAT3 m_rot;

	DirectX::XMFLOAT4X4 m_worldViewProj;
	static DirectX::XMFLOAT4X4 s_viewProj;
	
};