#pragma once
#pragma once
#include <vector>
#include <d3d11_2.h>

#include "WindCuboid.h"
#include "WindSphere.h"
#include "Buffer.h"
#include "ConstantBuffer.h"
#include "ConstantBuffers.h"
class WindManager_14_03
{
public:
	
	/////////////////////////////////////////////////
	/// Should be called once per frame after all 
	///	wind zones have been modified. No need to call 
	///	if all zones are immobile and none have been created
	/// or destroyed.
	/////////////////////////////////////////////////
	void updateResources(ID3D11DeviceContext*);

	/////////////////////////////////////////////////
	/// Call once to initalise resources.
	/////////////////////////////////////////////////
	bool load(ID3D11Device*, unsigned int _maxCuboids, unsigned int _maxSpheres);

	/////////////////////////////////////////////////
	/// Called in destructor but can be called 
	///	earlier if required.
	/////////////////////////////////////////////////
	void unload();

	/////////////////////////////////////////////////
	/// No memory management required. Returns nullptr //@This sort of stuff should get hidden behind some user interface classes
	/// if there are already too many WindRects.
	/////////////////////////////////////////////////
	WindCuboid* createWindCuboid();

	/////////////////////////////////////////////////
	/// No memory management required. Returns nullptr
	/// if there are already too many WindSpheres.
	/////////////////////////////////////////////////
	WindSphere* createWindSphere();

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

	const Buffer& getCuboidBuffer() const { return m_cuboidBuffer; }
	const Buffer& getSphereBuffer() const { return m_sphereBuffer; }

	const std::vector<WindCuboid>& getCuboids() const { return m_cuboids; }
	const std::vector<WindSphere>& getSpheres() const { return m_spheres; }

	int getMaxSpheres() const { return m_maxSpheres; }
	int getMaxCuboids() const { return m_maxCuboids; }
protected:
private:
	std::vector<WindCuboid>	m_cuboids;
	std::vector<WindSphere> m_spheres;
	int m_maxCuboids;
	int m_maxSpheres;

	Buffer m_cuboidBuffer;
	Buffer m_sphereBuffer;
};