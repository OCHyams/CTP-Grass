#pragma once
#include "DX11Demo.h"
#include <vector>
#include <DirectXMath.h>
#include "Field.h"
#include "WindManager.h"
#include "Field_14_03.h"
#include "WindManager_14_03.h"
#include "Renderer.h"
class GameObject;
class ArcCamera;
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
	std::vector<GameObject*> m_objects;
	ArcCamera* m_cam;
	Field m_field;
	WindManager m_windManager;
	WindSphere* m_demoSphere;
	Field_14_03 m_field_14_03;
	WindManager_14_03 m_windManager_14_03;
	float m_fps;
	int m_numBlades;
	int m_numDrawnBlades;
	Renderer m_renderer;
};