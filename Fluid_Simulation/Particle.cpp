//--------------------------------------------------------------------------------------
// File: Particle.cpp
//
// Particle encapsulation for SPH system.
//--------------------------------------------------------------------------------------
#include "stdafx.h"
#include "Particle.h"

Particle::Particle( std::string name, D3DXVECTOR3 pos, D3DXVECTOR3 vel, D3DXVECTOR3 acc, 
						D3DXVECTOR4 color, float mass, float pressure, float density, float iso,
						float radius)
{
	m_fIsoValue = iso;
	m_fDensity = density;
	m_fMass = mass;
	m_fPressure = pressure;

	m_vPosition =		pos;
	m_vVelocity =		vel;
	m_vAcceleration =	acc;
	m_vColor =			color;
	m_fRadius =			radius;

	m_bDelete =			false;

	m_uNLocation = 0;
	m_fGradient = -45.0f / ( (float) M_PI * 0.66f * 0.66f * 0.66f * 0.66f * 0.66f * 0.66f );
	m_fLaplacian = 45.0f / ( (float) M_PI * 0.66f * 0.66f * 0.66f * 0.66f * 0.66f * 0.66f );

	m_Name = name;

	m_vStepPosition			= D3DXVECTOR3(0.0f,0.0f,0.0f);
	m_vStepVelocity			= D3DXVECTOR3(0.0f,0.0f,0.0f);
	m_vStepAcceleration		= D3DXVECTOR3(0.0f,0.0f,0.0f);

	D3DXMatrixTranslation( &m_mWorld, m_vPosition.x, m_vPosition.y, m_vPosition.z );
}

void Particle::PreStep( float fElapsedTime )
{
	// update gravity
	m_vStepAcceleration.y = -7.0f;
}

void Particle::Step( float fElapsedTime, std::vector<Particle> &particles, 
					const std::vector< std::vector<Neighbor> > &neighborList )
{
	// update based on density and pressure
	ComputeForce( fElapsedTime, particles, neighborList );

	// enforce max accel
	Clamp( m_vStepAcceleration, 30.0f );
	// Normalize( m_vStepAcceleration );

	m_vStepVelocity = m_vVelocity + ( m_vStepAcceleration * fElapsedTime );
	m_vStepPosition = m_vPosition + ( m_vStepVelocity * fElapsedTime );

	// then modify the result by collision
	D3DXVECTOR3 deltaAccel;
	deltaAccel = ComputeBoundaryCollision( fElapsedTime );
	if( deltaAccel != D3DXVECTOR3( 0.0f, 0.0f, 0.0f ) )
	{
		m_vStepAcceleration = deltaAccel;
		Clamp( m_vStepAcceleration, 30.0f );
		// Normalize( m_vStepAcceleration );

		m_vStepVelocity = m_vStepAcceleration * fElapsedTime;
		m_vStepPosition = m_vPosition + ( m_vStepVelocity * fElapsedTime );
	}
}

void Particle::Finalize()
{
	// Finalize position, vel, and accel.
	m_vPosition =		m_vStepPosition;
	m_vVelocity =		m_vStepVelocity;
	m_vAcceleration =	m_vStepAcceleration;

	// clear the steps
	m_vStepPosition			= D3DXVECTOR3(0.0f,0.0f,0.0f);
	m_vStepVelocity			= D3DXVECTOR3(0.0f,0.0f,0.0f);
	m_vAcceleration			= D3DXVECTOR3(0.0f,0.0f,0.0f);

	// update world transform
	D3DXMatrixTranslation( &m_mWorld, m_vPosition.x, m_vPosition.y, m_vPosition.z ); 

	// remove crazy valued particles
	/*
	if( -100.0f > m_vPosition.x )
	{
		m_bDelete = true;
	}
	if( 100.0f < m_vPosition.x )
	{
		m_bDelete = true;
	}

	if( -100.0f > m_vPosition.y )
	{
		m_bDelete = true;
	}
	if( 100.0f < m_vPosition.y )
	{
		m_bDelete = true;
	}

	if( -100.0f > m_vPosition.z )
	{
		m_bDelete = true;
	}
	if( 100.0f < m_vPosition.z )
	{
		m_bDelete = true;
	}
	*/
}

void Particle::ComputeForce( float fElapsedTime, std::vector<Particle> &particles, 
									const std::vector< std::vector<Neighbor> > &neighborList )
{
	float h = 50.0f;

	std::vector<Neighbor>::const_iterator i;
	for( i = neighborList[ m_uNLocation ].begin(); i != neighborList[ m_uNLocation ].end(); ++i )
	{
		float r = sqrt( i->distanceSquared );

		if( r == 0.0f ) 
			continue;

		if (r < h)
		{
			// Compute force due to pressure and viscosity
			float fDiff;
			fDiff = h - r;
			
			D3DXVECTOR3 vDiff;
			vDiff = m_vPosition - particles[ i->index ].m_vPosition;

            float pressure;
			pressure = max( m_fPressure, 0.0f ) + max( particles[ i->index ].m_fPressure, 0.0f );

			D3DXVECTOR3 vForce( 0.0f, 0.0f, 0.0f );
			
			float scale = -0.5f * pressure * m_fGradient * fDiff / r;
			vForce = scale * vDiff;

			vDiff = particles[ i->index ].m_vVelocity - m_vVelocity;

			// viscosity * laplace operator
			scale = 0.2f * m_fLaplacian;
			vDiff = scale * vDiff;

			vForce += vDiff;

			scale = fDiff * m_fDensity * particles[ i->index ].m_fDensity * 1000.0f;
			vForce = scale * vForce;

			// update the accelerations based on force
			m_vStepAcceleration = m_vStepAcceleration + ( particles[ i->index ].m_fMass * vForce );
			particles[ i->index ].m_vStepAcceleration = particles[ i->index ].m_vStepAcceleration +
				( -m_fMass * vForce );
		}
	}
}

D3DXVECTOR3 Particle::ComputeBoundaryCollision( float fElapsedTime )
{
	// acceleration result
	D3DXVECTOR3 aResult = m_vStepAcceleration;

	// jitter prevention
	const float epsilon = 0.001f;	

	// Inner Boundary to Simulate Fluid flowing through something
	//if( m_vStepPosition.x < 75.0f + epsilon && m_vStepPosition.y > 25.0f + epsilon ||
	//	m_vStepPosition.x < 75.0f - epsilon && m_vStepPosition.y > 25.0f - epsilon )
	//{
	//	if( m_vStepPosition.y < 30.0f + epsilon || m_vStepPosition.y < 30.0f - epsilon )
	//	{
	//		D3DXVECTOR3 norm( 0.0f, 1.0f, 0.0f );
	//		D3DXVECTOR3 vCollisionResponse;
	//		vCollisionResponse = ComputeCollisionAccel( norm );

	//		// adjust only the y value of the acceleration based on this collision
	//		aResult.y = vCollisionResponse.y * fElapsedTime;
	//	}
	//}

	// Outter Boundaries

	// check collision against floor
	if ( m_vStepPosition.y < -90.0f + epsilon || m_vStepPosition.y < -90.0f - epsilon )
	{
		D3DXVECTOR3 norm( 0.0f, 1.0f, 0.0f );
		D3DXVECTOR3 vCollisionResponse;
		vCollisionResponse = ComputeCollisionAccel( norm );

		// adjust only the y value of the acceleration based on this collision
		aResult.y = vCollisionResponse.y * fElapsedTime;
	}
	// against ceiling
	else if(  m_vStepPosition.y > 90.0f + epsilon || m_vStepPosition.y > 90.0f - epsilon )
	{
		D3DXVECTOR3 norm( 0.0f, -1.0f, 0.0f );
		D3DXVECTOR3 vCollisionResponse;
		vCollisionResponse = ComputeCollisionAccel( norm );

		// adjust only the y value of the acceleration based on this collision
		aResult.y = vCollisionResponse.y * fElapsedTime;
	}

	// check collision against walls
	if ( m_vStepPosition.x < -90.0f + epsilon || m_vStepPosition.x < -90.0f - epsilon )
	{
		D3DXVECTOR3 norm( 1.0f, 0.0f, 0.0f );
		D3DXVECTOR3 vCollisionResponse;
		vCollisionResponse = ComputeCollisionAccel( norm );

		aResult.x = vCollisionResponse.x * fElapsedTime;
	}
	else if(  m_vStepPosition.x > 90.0f + epsilon || m_vStepPosition.x > 90.0f - epsilon )
	{
		D3DXVECTOR3 norm( -1.0f, 0.0f, 0.0f );
		D3DXVECTOR3 vCollisionResponse;
		vCollisionResponse = ComputeCollisionAccel( norm );

		aResult.x = vCollisionResponse.x * fElapsedTime;
	}

	if ( m_vStepPosition.z < -90.0f + epsilon || m_vStepPosition.z < -90.0f - epsilon )
	{
		D3DXVECTOR3 norm( 0.0f, 0.0f, 1.0f );
		D3DXVECTOR3 vCollisionResponse;
		vCollisionResponse = ComputeCollisionAccel( norm );

		aResult.z = vCollisionResponse.z * fElapsedTime;
	}
	else if(  m_vStepPosition.z > 90.0f + epsilon || m_vStepPosition.z > 90.0f - epsilon )
	{
		D3DXVECTOR3 norm( 0.0f, 0.0f, -1.0f );
		D3DXVECTOR3 vCollisionResponse;
		vCollisionResponse = ComputeCollisionAccel( norm );

		aResult.z = vCollisionResponse.z * fElapsedTime;
	}

	return aResult;
}

D3DXVECTOR3 Particle::ComputeCollisionAccel( const D3DXVECTOR3 &norm )
{
	// acceleration result
	D3DXVECTOR3 aResult;

	float dot;
	dot = -D3DXVec3Dot( &norm, &m_vStepVelocity );

	aResult = dot * norm;

	return aResult;
}

void Particle::Clamp( D3DXVECTOR3 &v, float value )
{
	if( v.x > value )
		v.x = value;
	else if( v.x < -value )
		v.x = -value;

	if( v.y > value )
		v.y = value;
	else if( v.y < -value )
		v.y = -value;

	if( v.z > value )
		v.z = value;
	else if( v.z < -value )
		v.z = -value;
}

void Particle::Normalize( D3DXVECTOR3 &v )
{
	float magnitude = v.x * v.x + v.y * v.y + v.z * v.z;

	v /= sqrt( magnitude );
}

