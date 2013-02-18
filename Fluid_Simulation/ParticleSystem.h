//--------------------------------------------------------------------------------------
// File: ParticleSystem.h
//
// Particle system abstraction for SPH system.
//--------------------------------------------------------------------------------------

#ifndef PARTICLESYSTEM
#define	PARTICLESYSTEM

#pragma once

#include <vector>
#include <string>
#include <sstream>
#define _USE_MATH_DEFINES
#include <math.h>
#include "Particle.h"
#include <time.h>

class ParticleSystem
{
public:
	ParticleSystem(void);

	void AddParticle( std::string name, D3DXVECTOR3 pos, D3DXVECTOR3 vel, D3DXVECTOR3 acc, 
						D3DXVECTOR4 color, float mass, float pressure, float density, float iso,
						float rad );
	void RemoveParticle( std::string name );
	const std::vector<Particle>& GetParticles();
	void Reset();
	// increment the simulation by 1 time step
	void Step( float elapsedTime );
	

	float m_fTime;
	float m_fTimeSinceLastParticles;
	float m_fTargetFrameTime;


private:
	// consts
	float legendreCoeff;

	// the particles in the system
	std::vector<Particle> m_Particles;

	// nearest neighbor grid
	struct ParticleGrid
	{
		// dimensions of grid
		unsigned width;
		unsigned height;
		unsigned depth;

		// positive vertex of grid
		float x;
		float y;
		float z;

		// # of squares in grid
		unsigned size;
		float length;
		// index to particles per contained per square
		std::vector< std::vector<int> > particles;
	};

	ParticleGrid m_Grid;

	// grid functions
	void ClearGrid();
	void SetGrid();
	void GetNearestNeighbors();
	void ComputeDensity();

	unsigned ThreeDimensionalMap( unsigned x, unsigned y, unsigned z );

	std::vector< std::vector<Neighbor> > m_NeighborList;


	std::string FloatToString( const float &f );

};


#endif
