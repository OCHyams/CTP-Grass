#pragma once
#include <d3d11_2.h>
#include <map>
#include <vector>
#include "objLoader.h"
#include "RendererData.h"
#include "DrawData.h"
#include "GameObject.h"

class ArcCamera;
class MeshInfo;
class RenderInfo;

class MeshObject : public GameObject
{
public:
	unsigned int	m_renderFlags;
	MESH			m_meshID;
};


class Renderer
{
public:

	bool load(ID3D11Device* _device);
	void cleanup();

	void render(const DrawData& data);

	ObjModel* getObjModel(MESH meshIdx);
protected:

	std::map<MESH, MeshInfo*>	m_meshes;
	std::map<MESH, ObjModel*>	m_objModels;
	std::map<FX, RenderInfo*>	m_fx;
	std::vector<MeshObject*>	m_meshObjects;

	MeshInfo* Renderer::loadMesh(const std::string& _fpath, ObjModel::MESH_TOPOLOGY inputTopology, MESH idx, ID3D11Device* _device);
	
private:
	MeshInfo* Renderer::loadMeshHelper(const ObjModel& model, ID3D11Device* _device);
};
