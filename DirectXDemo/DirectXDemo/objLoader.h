/*
    Beginning DirectX 11 Game Programming
    By Allen Sherrod and Wendy Jones

    ObjModel - Used to represent an OBJ model.
*/


#ifndef _OBJ_LOADER_H_
#define _OBJ_LOADER_H_


class ObjModel
{
   public:
      ObjModel( );      
      ~ObjModel( );

      void Release( );
      bool LoadOBJ( char *fileName );

      float *GetVertices()		const { return vertices_; }
      float *GetNormals()		const { return normals_; }
      float *GetTexCoords()		const { return texCoords_; }
      int    GetTotalVerts()	const { return totalVerts_; } 

   private:
      float *vertices_ = nullptr;
      float *normals_ = nullptr;
      float *texCoords_ = nullptr;
      int totalVerts_;
};


#endif