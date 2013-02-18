//--------------------------------------------------------------------------------------
// File: MarchingCubes.h
//
// Isosurface generation algorithm.
// Used to build a fluid mesh from the SPH model.
//--------------------------------------------------------------------------------------

#ifndef MARCHINGCUBES
#define	MARCHINGCUBES

#include <vector>
#include <algorithm>
#include "Particle.h"

//*******************************************************************************
//					MARCHING CUBES - TABLES ,STRUCTURES AND CLASS
//*******************************************************************************
// Attributes of every vertex in the 3D Grid
struct vertex
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 norm;
	float flux;
	bool inside;
};


//	Structure of vectors to be returned back from PART2 tp PART1 of the project
//	Consists of the vertex and the normals vector as the structure attributes
//	Using the values in vectors mesh of the isosurface can be generated
struct MarchingCubesVertex
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 norm;
};

//	Marching Cubes blue print
class MarchingCubes
{
public:

	//	Marching Cubes Constructor
	MarchingCubes();

	// Builds the 3D Grid of cubes for generating the isosurface
	void Initialization( float start_x, float start_y, float start_z, float end_x, float end_y, float end_z,
			float step_x, float step_y, float step_z );
	void ClearGrid();

	std::vector<MarchingCubesVertex> RunMarchingCubes( const std::vector<Particle> &particles, float isoValue );
	std::vector<MarchingCubesVertex> ConstructIsoSurface();

	//	Calculates how much the vertex in the grid is affected by all the particles in its cube
	void ComputeVertexStatus( const std::vector<Particle> &particles, int x, int y, int z );


	//	x, y, z coordinates of the starting vertex of the grid in world space
	float start_x;
	float start_y;
	float start_z;

	//	x, y, z coordinates of the ending vertex of the grid in world space
	float end_x;
	float end_y;
	float end_z;

	//	Size of the each cube in the grid along each axis
	float step_x;
	float step_y;
	float step_z;

	// Number of cubes along each axis
	int size_x;
	int size_y;
	int size_z;

	// the isolevel we are reconstructing
	float m_Isolevel;

	// pointer to the array of vertices representing the grid in 3D
	//vertex *vertices;
	std::vector<vertex> m_Vertices;

private:
	// helper functions
	unsigned ThreeDimensionalMap( int x, int y, int z );
	vertex InterpolateEdge( const vertex &v1, const vertex &v2 );
	float InterpolateIsoValue( const std::vector<Particle> &particles, vertex &v );
	float InterpolateNormal( float f1, float f2, float iso1, float iso2 );

	//	Table Contains all kinds of combinations of points within or outside of a cube 
	const static int edgeTable[256];

	// contains an overview of the points we must take if a given combination is found in the edge table
	const static int triTable[256][16];

	// array to store all the 12 edges in the cube
	vertex verts[12];
};

#endif