/*
    Beginning DirectX 11 Game Programming
    By Allen Sherrod and Wendy Jones

    ObjModel - Used to represent an OBJ model.

	Modified for this simulation.
*/


#ifndef _OBJ_LOADER_H_
#define _OBJ_LOADER_H_


class ObjModel
{
   public:
	   enum MESH_TOPOLOGY { TRIANGLE_LIST, TRIANGLE_STRIP, QUAD_LIST, QUAD_STRIP };
      ObjModel( );      
      ~ObjModel( );

      void release( );

      bool loadOBJ( char *fileName, MESH_TOPOLOGY inputTopology = TRIANGLE_STRIP);
	  bool loadPlane( float x, float z, float subSizeX, float subSizeZ );


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