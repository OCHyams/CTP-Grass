#pragma once
#include <d3d11_2.h>
#include "objLoader.h"
#include "FieldRenderData.h"
#include <vector>
namespace field
{
	//Like version from field except there's no octree (  :c  )
	void generateField(ID3D11Device* _device, ObjModel* _model, float _density, std::vector<field::Instance>& _instances);
	float triangleSurfaceArea(float* verts);
	void addPatch(std::vector<field::Instance>& _field, float* verts, unsigned int _numBlades);
}