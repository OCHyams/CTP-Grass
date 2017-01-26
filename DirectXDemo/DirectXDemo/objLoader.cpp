/*
    Beginning DirectX 11 Game Programming
    By Allen Sherrod and Wendy Jones

    ObjModel - Used to represent an OBJ model.
*/

#include <d3d11_2.h>
#include <SimpleMath.h>
#include<fstream>
#include<vector>
#include<string>
#include"objLoader.h"
#include"TokenStream.h"
#include "Shorthand.h"

ObjModel::ObjModel( )
{
    m_vertices = 0;
    m_normals = 0;
    m_texCoords = 0;
    m_totalVerts = 0;
}

      
ObjModel::~ObjModel( )
{
    release( );
}


void ObjModel::release( )
{
    m_totalVerts = 0;

    if( m_vertices != 0 ) delete[] m_vertices;
    if( m_normals != 0 ) delete[] m_normals;
    if( m_texCoords != 0 ) delete[] m_texCoords;

    m_vertices = 0;
    m_normals = 0;
    m_texCoords = 0;
}


bool ObjModel::loadOBJ( char *fileName )
{
    std::ifstream fileStream;
    int fileSize = 0;

    fileStream.open( fileName, std::ifstream::in );
   
    if( fileStream.is_open( ) == false )
        return false;

    fileStream.seekg( 0, std::ios::end );
    fileSize = ( int )fileStream.tellg( );
    fileStream.seekg( 0, std::ios::beg );

    if( fileSize <= 0 )
        return false;

    char *buffer = new char[fileSize];

    if( buffer == 0 )
        return false;

    memset( buffer, '\0', fileSize );

    TokenStream tokenStream, lineStream, faceStream;
    std::string tempLine, token;

    fileStream.read( buffer, fileSize );
    tokenStream.SetTokenStream( buffer );

    delete[] buffer;

    tokenStream.ResetStream( );

    std::vector<float> verts, norms, texC;
    std::vector<int> faces;

    char lineDelimiters[2] = { '\n', ' ' };

    while( tokenStream.MoveToNextLine( &tempLine ) )
    {
        lineStream.SetTokenStream( ( char* )tempLine.c_str( ) );
        tokenStream.GetNextToken( 0, 0, 0 );

        if( !lineStream.GetNextToken( &token, lineDelimiters, 2 ) )
            continue;

        if( strcmp( token.c_str( ), "v" ) == 0 )
        {
            lineStream.GetNextToken( &token, lineDelimiters, 2 );
            verts.push_back( ( float )atof( token.c_str( ) ) );

            lineStream.GetNextToken( &token, lineDelimiters, 2 );
            verts.push_back( ( float )atof( token.c_str( ) ) );

            lineStream.GetNextToken( &token, lineDelimiters, 2 );
            verts.push_back( ( float )atof( token.c_str( ) ) );
        }
        else if( strcmp( token.c_str( ), "vn" ) == 0 )
        {
            lineStream.GetNextToken( &token, lineDelimiters, 2 );
            norms.push_back( ( float )atof( token.c_str( ) ) );

            lineStream.GetNextToken( &token, lineDelimiters, 2 );
            norms.push_back( ( float )atof( token.c_str( ) ) );

            lineStream.GetNextToken( &token, lineDelimiters, 2 );
            norms.push_back( ( float )atof( token.c_str( ) ) );
        }
        else if( strcmp( token.c_str( ), "vt" ) == 0 )
        {
            lineStream.GetNextToken( &token, lineDelimiters, 2 );
            texC.push_back( ( float )atof( token.c_str( ) ) );

            lineStream.GetNextToken( &token, lineDelimiters, 2 );
            texC.push_back( ( float )atof( token.c_str( ) ) );
        }
        else if( strcmp( token.c_str( ), "f" ) == 0 )
        {
            char faceTokens[3] = { '\n', ' ', '/' };
            std::string faceIndex;

            faceStream.SetTokenStream( ( char* )tempLine.c_str( ) );
            faceStream.GetNextToken( 0, 0, 0 );

            for( int i = 0; i < 3; i++ )
            {
                faceStream.GetNextToken( &faceIndex, faceTokens, 3 );
                faces.push_back( ( int )atoi( faceIndex.c_str( ) ) );

				faceStream.GetNextToken( &faceIndex, faceTokens, 3 );
                faces.push_back( ( int )atoi( faceIndex.c_str( ) ) );

                faceStream.GetNextToken( &faceIndex, faceTokens, 3 );
                faces.push_back( ( int )atoi( faceIndex.c_str( ) ) );
            }
        }
        else if( strcmp( token.c_str( ), "#" ) == 0 )
        {
            int a = 0;
            int b = a;
        }

        token[0] = '\0';
    }

    // "Unroll" the loaded obj information into a list of triangles.

    int vIndex = 0, nIndex = 0, tIndex = 0;
    int numFaces = ( int )faces.size( ) / 9;

    m_totalVerts = numFaces * 3;

    m_vertices = new float[m_totalVerts * 3];

    if( ( int )norms.size( ) != 0 )
    {
        m_normals = new float[m_totalVerts * 3];
    }

    if( ( int )texC.size( ) != 0 )
    {
        m_texCoords = new float[m_totalVerts * 2];
    }

    for( int f = 0; f < ( int )faces.size( ); f+=3 )
    {
        m_vertices[vIndex + 0] = verts[( faces[f + 0] - 1 ) * 3 + 0];
        m_vertices[vIndex + 1] = verts[( faces[f + 0] - 1 ) * 3 + 1];
        m_vertices[vIndex + 2] = verts[( faces[f + 0] - 1 ) * 3 + 2];
        vIndex += 3;

        if(m_texCoords)
        {
            m_texCoords[tIndex + 0] = texC[( faces[f + 1] - 1 ) * 2 + 0];
            m_texCoords[tIndex + 1] = texC[( faces[f + 1] - 1 ) * 2 + 1];
            tIndex += 2;
        }

        if(m_normals)
        {
			int index = (faces[f + 2] - 1) * 3;
            m_normals[nIndex + 0] = norms[index + 0];
            m_normals[nIndex + 1] = norms[index + 1];
            m_normals[nIndex + 2] = norms[index + 2];
            nIndex += 3;
        }
    }

    verts.clear( );
    norms.clear( );
    texC.clear( );
    faces.clear( );

    return true;
}

bool ObjModel::loadPlane(float _x, float _z, float _subSizeX, float _subSizeZ)
{
	assert(_x >= _subSizeX, "Subdivision min size must be smaller than size");
	assert(_z >= _subSizeZ, "Subdivision min size must be smaller than size");
	if (_x < _subSizeX || _z < _subSizeZ) return false;


	using namespace DirectX;
	XMFLOAT2 size = { _x, _z };
	XMFLOAT2 halfSize = { size.x / 2, size.y / 2 };
	XMFLOAT2 numDivs = { floor(size.x / _subSizeX), floor(size.y / _subSizeZ) };
	XMFLOAT2 subSize = { size.x / numDivs.x, size.y / numDivs.y };

	XMVECTOR vec;
	XMVECTOR halfSizeVec = VEC3(halfSize.x, 0, halfSize.y);
	XMFLOAT3 vert;
	XMFLOAT3 norm = { 0, 1, 0 };

	m_totalVerts = (int)numDivs.x * (int)numDivs.y * 6;
	m_vertices = new float[m_totalVerts * 3];
	m_normals = new float[m_totalVerts * 3];
	float* verPtr = m_vertices;
	float* normPtr = m_normals;
	for (int x = 0; x < (int)numDivs.x; ++x)
	{
		for (int y = 0; y < (int)numDivs.y; ++y)
		{
			vec = VEC3((subSize.x  * (x + 1)), 0, (subSize.y * y)) - halfSizeVec;
			STOREF3(&vert, vec);
			memcpy(verPtr, &vert, sizeof(float) * 3);
			memcpy(normPtr, &norm, sizeof(float) * 3);
			verPtr += 3;
			normPtr += 3;

			vec = VEC3((subSize.x  * x), 0, (subSize.y * y)) - halfSizeVec;
			STOREF3(&vert, vec);
			memcpy(verPtr, &vert, sizeof(float) * 3);
			memcpy(normPtr, &norm, sizeof(float) * 3);
			verPtr += 3;
			normPtr += 3;
			
			vec = VEC3((subSize.x  * x), 0, (subSize.y * (y + 1))) - halfSizeVec;
			STOREF3(&vert, vec);
			memcpy(verPtr, &vert, sizeof(float) * 3);
			memcpy(normPtr, &norm, sizeof(float) * 3);
			verPtr += 3;
			normPtr += 3;

			vec = VEC3((subSize.x  * x), 0, (subSize.y * (y + 1))) - halfSizeVec;
			STOREF3(&vert, vec);
			memcpy(verPtr, &vert, sizeof(float) * 3);
			memcpy(normPtr, &norm, sizeof(float) * 3);
			verPtr += 3;
			normPtr += 3;

			vec = VEC3((subSize.x  * (x + 1)), 0, (subSize.y * (y + 1))) - halfSizeVec;
			STOREF3(&vert, vec);
			memcpy(verPtr, &vert, sizeof(float) * 3);
			memcpy(normPtr, &norm, sizeof(float) * 3);
			verPtr += 3;
			normPtr += 3;

			vec = VEC3((subSize.x  * (x + 1)), 0, (subSize.y * y)) - halfSizeVec;
			STOREF3(&vert, vec);
			memcpy(verPtr, &vert, sizeof(float) * 3);
			memcpy(normPtr, &norm, sizeof(float) * 3);
			verPtr += 3;
			normPtr += 3;
		}
	}

	
	return true;
}
