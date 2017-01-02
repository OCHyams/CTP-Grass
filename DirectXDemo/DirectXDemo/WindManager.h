#pragma once
#include <vector>
#include <d3d11_2.h>

#include "WindCuboid.h"
#include "WindSphere.h"

class WindManager
{
public:
	const int m_threadsPerGroupX = 1024;

	/////////////////////////////////////////////////
	/// Should be called once per frame after all 
	///	wind zones have been modified. No need to call 
	///	if all zones are immobile and none are created
	/// or destroyed.
	/////////////////////////////////////////////////
	void updateResources(ID3D11DeviceContext*);

	/////////////////////////////////////////////////
	/// Load wind shader and shared resources.
	/////////////////////////////////////////////////
	static bool loadShared(ID3D11Device*);

	/////////////////////////////////////////////////
	/// Unload wind shader and shared resources.
	/////////////////////////////////////////////////
	static void unloadShared();

	/////////////////////////////////////////////////
	/// Call once to initalise resources.
	/////////////////////////////////////////////////
	bool load(ID3D11Device*, int _maxRects, int _maxSpheres);

	/////////////////////////////////////////////////
	/// Called in destructor but can be called 
	///	earlier if required.
	/////////////////////////////////////////////////
	void unload();

	/////////////////////////////////////////////////
	/// No memory management required. Returns nullptr //@This sort of stuff should get hidden behind some user interface classes
	/// if there are already too many WindRects.
	/////////////////////////////////////////////////
	WindCuboid* CreateWindCuboid();

	/////////////////////////////////////////////////
	/// No memory management required. Returns nullptr
	/// if there are already too many WindSpheres.
	/////////////////////////////////////////////////
	WindSphere* CreateWindSphere();

	/////////////////////////////////////////////////
	/// Remove a specific wind zone.
	/////////////////////////////////////////////////
	void remove(WindCuboid*);

	/////////////////////////////////////////////////
	/// Remove a specific wind zone.
	/////////////////////////////////////////////////
	void remove(WindSphere*);

	/////////////////////////////////////////////////
	/// Remove all wind zones.
	/////////////////////////////////////////////////
	void removeAll();

	void applyWindForces(ID3D11UnorderedAccessView* _grass, ID3D11DeviceContext* _dc, int _numGroupsX);

	ID3D11ShaderResourceView* getRectSRV() { return m_cuboidSRV;  }
	ID3D11ShaderResourceView* getSphereSRV() { return m_sphereSRV; }
	const std::vector<WindCuboid>& getCuboids() const { return m_cuboids; }
	const std::vector<WindSphere>& getSpheres() const { return m_spheres; }
protected:
private:
	std::vector<WindCuboid>	m_cuboids;
	std::vector<WindSphere> m_spheres;
	int m_maxCuboids;
	int m_maxSpheres;

	ID3D11Buffer*				m_cuboidBuffer; //@Not sure if i need the buffers?
	ID3D11Buffer*				m_sphereBuffer;
	ID3D11ShaderResourceView*	m_cuboidSRV;
	ID3D11ShaderResourceView*	m_sphereSRV;

	static ID3D11ComputeShader*	s_cs;
};