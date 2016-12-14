#pragma once
#include "DX11Demo.h"
#include <vector>
#include <DirectXMath.h>
#include "Field.h"
class GameObject;
class Camera;
class SimpleGrass;
class BasicDemo : public DX11Demo
{
public:
	BasicDemo();
	virtual ~BasicDemo();

	bool load() override;
	void unload() override;

	void update() override;
	void render() override;

private:
	DirectX::XMFLOAT3 m_wind;
	float m_windStr;
	std::vector<GameObject*> m_objects;
	Camera* m_cam;
	Field m_field;
	float m_fps;
};