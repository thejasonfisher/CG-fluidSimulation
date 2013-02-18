//--------------------------------------------------------------------------------------
// File: Particle.h
//
// Particle encapsulation for SPH system.
//--------------------------------------------------------------------------------------

#ifndef PARTICLE
#define	PARTICLE

#pragma once

#include <vector>
#include "d3dx9math.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <string>
#include <time.h>

struct Neighbor
{
	unsigned index;
	float distanceSquared;

	Neighbor( unsigned index, float distanceSquared )
	{
		this->index = index;
		this->distanceSquared = distanceSquared;
	}
};

class Particle
{
public:
	Particle( std::string name, D3DXVECTOR3 pos, D3DXVECTOR3 vel, D3DXVECTOR3 acc, 
				D3DXVECTOR4 color, float mass, float pressure, float density, float iso,
				float radius );

	void PreStep( float fElapsedTime );
	void Step( float fElapsedTime, std::vector<Particle> &particles, 
					const std::vector< std::vector<Neighbor> > &neighborList );
	void Finalize();

	D3DXVECTOR3	m_vPosition;
	D3DXVECTOR3	m_vVelocity;
	D3DXVECTOR3	m_vAcceleration;

	// used for rendering
	D3DXVECTOR4 m_vColor;
	D3DXMATRIX	m_mWorld;

	std::string m_Name;
	float m_fRadius;
	float m_fMass;
	float m_fPressure;
	float m_fDensity;
	float m_fIsoValue;

	// location of this particle in the neighbor list
	unsigned m_uNLocation;
	float m_fGradient;
	float m_fLaplacian;

	bool m_bDelete;

private:
	// temp physics properities used in computation
	D3DXVECTOR3	m_vStepPosition;
	D3DXVECTOR3	m_vStepVelocity;
	D3DXVECTOR3	m_vStepAcceleration;

	void ComputeForce( float fElapsedTime, std::vector<Particle> &particles, 
								const std::vector< std::vector<Neighbor> > &neighborList );
	D3DXVECTOR3 ComputeBoundaryCollision( float fElapsedTime ); 
	D3DXVECTOR3 ComputeCollisionAccel( const D3DXVECTOR3 &norm );
	void Clamp( D3DXVECTOR3 &v, float value );
	void Normalize( D3DXVECTOR3 &v );
};


#endif
