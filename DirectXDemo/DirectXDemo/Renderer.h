#pragma once
#include <d3d11_2.h>
#include <map>
#include <vector>
#include "objLoader.h"
#include "RendererData.h"
#include "DrawData.h"
#include "GameObject.h"
#include <SimpleMath.h>
class ArcCamera;
class MeshInfo;
class RenderInfo;

class MeshObject : public GameObject
{
public:
	unsigned int	m_renderFlags;
	MESH			m_meshID;

	bool load(ID3D11Device*) override { return true; }
	void unload() override { }
	void draw(const DrawData&) override {}
};


class Renderer
{
public:

	bool load(ID3D11Device* _device);
	void cleanup();

	void render(const DrawData& data);
	void addObj(MeshObject* _obj) { m_meshObjects.push_back(_obj); }

	bool registerMesh(int _id, const std::string& _fpath, ObjModel::MESH_TOPOLOGY _inputTopology, DirectX::XMFLOAT4X4 _transform, ID3D11Device* _device);
	//Creates vertex buffer etc for given obj model but does not store a reference to the objmodel as the other registerMesh() overload does.
	bool registerMesh(int _id, const ObjModel& model, ID3D11Device* _device);

	ObjModel* getObjModel(int meshIdx);
protected:

	std::map<int, MeshInfo*>	m_meshes;
	std::map<int, ObjModel*>	m_objModels;
	std::map<FX, RenderInfo*>	m_fx;
	std::vector<MeshObject*>	m_meshObjects;

	MeshInfo* Renderer::loadMesh(const std::string& _fpath, ObjModel::MESH_TOPOLOGY inputTopology, int idx, ID3D11Device* _device, const DirectX::XMFLOAT4X4& _transform);
	
private:
	MeshInfo* Renderer::loadMeshHelper(const ObjModel& model, ID3D11Device* _device);
};
