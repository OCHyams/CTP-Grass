/*----------------------------------------------------------------
Author:			Orlando Cazalet-Hyams
Description :	Grass demo class, kind of like a 'scene'.
				It owns the camera and the grass etc... 
----------------------------------------------------------------*/
#pragma once
#include "DX11Demo.h"
#include <vector>
#include <DirectXMath.h>
#include "Field.h"
#include "WindManager.h"
#include "Renderer.h"

class GameObject;
class ArcCamera;
class SimpleGrass;
class DX11GrassDemo : public DX11Demo
{
public:
	DX11GrassDemo();
	virtual ~DX11GrassDemo();

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
	float m_fps;
	int m_numBlades;
	Renderer m_renderer;
};