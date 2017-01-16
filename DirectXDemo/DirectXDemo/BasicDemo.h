#pragma once
#include "DX11Demo.h"
#include <vector>
#include <DirectXMath.h>
#include "Field.h"
#include "WindManager.h"
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
	float m_fps;
	int m_numBlades;
	int m_numDrawnBlades;
};