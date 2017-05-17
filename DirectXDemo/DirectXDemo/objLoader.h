/*
	Originally taken from:

    Beginning DirectX 11 Game Programming
    By Allen Sherrod and Wendy Jones

    ObjModel - Used to represent an OBJ model.

	This file has been modified for the purposes of the prototype.
*/

#ifndef _OBJ_LOADER_H_
#define _OBJ_LOADER_H_
#include <d3d11.h>
#include "SimpleMath.h"

class ObjModel
{
   public:
	   enum MESH_TOPOLOGY { TRIANGLE_LIST, TRIANGLE_STRIP, QUAD_LIST, QUAD_STRIP };
      ObjModel( );      
      ~ObjModel( );

      void release( );

	  //Right now, only works with TRIANGLE_STRIP and QUAD_STRIP inputs and TRIANGLE_LIST output
      bool loadOBJ(const char *fileName, const DirectX::XMFLOAT4X4& _transform, MESH_TOPOLOGY inputTopology = TRIANGLE_STRIP );
	  bool loadOBJ(const char *fileName,  MESH_TOPOLOGY inputTopology = TRIANGLE_STRIP);
	  bool loadPlane( float x, float z, float subSizeX, float subSizeZ );
	  bool ObjModel::loadHill(float _x, float _z, float _subSizeX, float _subSizeZ, float _height);

      float *getVertices()		const { return m_vertices; }
      float *getNormals()		const { return m_normals; }
      float *getTexCoords()		const { return m_texCoords; }
      int    getTotalVerts()	const { return m_totalVerts; } 

   private:
      float*	m_vertices	= nullptr;
      float*	m_normals	= nullptr;
      float*	m_texCoords = nullptr;
      int		m_totalVerts;
};


#endif