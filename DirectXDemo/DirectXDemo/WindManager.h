/*----------------------------------------------------------------
Author:			Orlando Cazalet-Hyams
Description :	This class manages wind volumes and 
				updates a grass instance buffer with
				per-instance wind vectors. An octree
				is queried such that only visible grass
				instances are added to the output buffer
				used for rendering. All grass instances
				are saved to another buffer so that
				state-based calculations can be used in the
				future.
----------------------------------------------------------------*/

#pragma once
#include <vector>
#include <d3d11.h>
#include "Buffer.h"
#include "ConstantBuffer.h"
#include "ConstantBuffers.h"
#include "WindCuboid.h"
#include "WindSphere.h"

class WindManager
{
public:
	const int THREADS_PER_GROUP_X = 256;

	/////////////////////////////////////////////////
	/// Should be called once per frame to update
	/// GPU resources.
	/////////////////////////////////////////////////
	void updateResources(ID3D11DeviceContext*, unsigned int _numInstances, float _time, float _deltaTime);

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
	bool load(ID3D11Device*, int _maxCuboids, int _maxSpheres);

	/////////////////////////////////////////////////
	/// Called in destructor but can be called 
	///	earlier if required.
	/////////////////////////////////////////////////
	void unload();

	/////////////////////////////////////////////////
	/// No memory management required. Returns nullptr
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

	/////////////////////////////////////////////////
	/// Used by Field objects during rendering.
	/////////////////////////////////////////////////
	void applyWindForces(	ID3D11UnorderedAccessView* _outGrass, ID3D11UnorderedAccessView* _frustumCulled, ID3D11UnorderedAccessView* _indirectArgs,
							ID3D11ShaderResourceView* _inGrass, ID3D11ShaderResourceView* _inOctree, 
							ID3D11DeviceContext* _dc, int _numInstances);

	const Buffer* getCuboidBuffer() { return &m_cuboidBuffer; } const
	const Buffer* getSphereBuffer() { return &m_sphereBuffer; } const
	const std::vector<WindCuboid>& getCuboids() const { return m_cuboids; } 
	const std::vector<WindSphere>& getSpheres() const { return m_spheres; } 

private:
	std::vector<WindCuboid>	m_cuboids;
	std::vector<WindSphere> m_spheres;
	int m_maxCuboids;
	int m_maxSpheres;
	Buffer m_cuboidBuffer;
	Buffer m_sphereBuffer;
	CBuffer<CBWindForceChangesPerFrame> m_CB_changesPerFrame;
	static ID3D11ComputeShader*	s_cs;
};