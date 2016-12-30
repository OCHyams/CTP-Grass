#pragma once
#include <vector>
#include <d3d11_2.h>

#include "WindRect.h"
#include "WindSphere.h"

class WindManager
{
public:
	/////////////////////////////////////////////////
	/// Updates shader resources, doesn't need to be 
	///	called if all the wind zones are immobile.
	/////////////////////////////////////////////////
	void updateResources();

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
	/// No memory management required. Returns nullptr
	/// if there are already too many WindRects.
	/////////////////////////////////////////////////
	WindRect* CreateWindRect();

	/////////////////////////////////////////////////
	/// No memory management required. Returns nullptr
	/// if there are already too many WindSpheres.
	/////////////////////////////////////////////////
	WindSphere* CreateWindSphere();

	/////////////////////////////////////////////////
	/// Remove a specific wind zone.
	/////////////////////////////////////////////////
	void remove(WindRect*);

	/////////////////////////////////////////////////
	/// Remove a specific wind zone.
	/////////////////////////////////////////////////
	void remove(WindSphere*);

	/////////////////////////////////////////////////
	/// Remove all wind zones.
	/////////////////////////////////////////////////
	void removeAll();

	ID3D11ShaderResourceView* getRectSRV() { return m_rectSRV;  }
	ID3D11ShaderResourceView* getSphereSRV() { return m_sphereSRV; }
protected:
private:
	std::vector<WindRect>	m_rects;
	std::vector<WindSphere> m_spheres;
	int m_maxRects;
	int m_maxSpheres;

	ID3D11ShaderResourceView* m_rectSRV;
	ID3D11ShaderResourceView* m_sphereSRV;
};