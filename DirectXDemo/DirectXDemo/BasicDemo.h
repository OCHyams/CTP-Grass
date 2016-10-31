#pragma once
#include "DX11Demo.h"
#include <vector>
class GameObject;
class Camera;
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
	
	std::vector<GameObject*> m_objects;
	Camera* m_cam;
};