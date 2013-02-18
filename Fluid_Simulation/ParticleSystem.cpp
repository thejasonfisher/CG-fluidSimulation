//--------------------------------------------------------------------------------------
// File: ParticleSystem.cpp
//
// Particle system abstraction for SPH system.
//--------------------------------------------------------------------------------------
#include "stdafx.h"
#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(void)
{
	// seed random number generator
	srand( (unsigned) time( NULL ) );

	legendreCoeff = 315.0f / ( 64.0f * (float) M_PI * (float) pow( 0.66f, 9 ) );

	// init nearest neighbor grid
	m_Grid.size = 20 * 20 * 20;
	m_Grid.length = 20.0f;
	m_fTime = 0.0f;
	m_fTargetFrameTime = 1.0f / 60.0f;
	m_fTimeSinceLastParticles = 0.0f;

	// allocate vectors
	for( unsigned i = 0; i < m_Grid.size - 1; ++i )
	{
		std::vector<int> newVector;
		m_Grid.particles.push_back( newVector ); 
	}	
}

void ParticleSystem::AddParticle( std::string name, D3DXVECTOR3 pos, D3DXVECTOR3 vel, D3DXVECTOR3 acc, 
									D3DXVECTOR4 color, float mass, float pressure, float density, float iso,
									float rad )
{
	// create a new particle and add it to the list
	m_Particles.push_back( Particle( name, pos, vel, acc, color, mass, pressure, density, iso, rad ) );
}

void ParticleSystem::RemoveParticle( std::string name )
{
	std::vector<Particle>::iterator i;
	for( i = m_Particles.begin(); i != m_Particles.end(); ++i )
	{
		if( i->m_Name == name )
		{
			m_Particles.erase( i );
			break;
		}
	}
}

const std::vector<Particle>& ParticleSystem::GetParticles()
{
	return m_Particles;
}

void ParticleSystem::Step( float elapsedTime )
{
	/*
	m_fTimeSinceLastParticles += elapsedTime;

	if( m_Particles.size() > 250 )
		m_fTimeSinceLastParticles = 0.0f;

	while( m_fTimeSinceLastParticles > 0.1f )
	{
		// add particles
		m_fTimeSinceLastParticles -= 0.1f;

		std::string particle = "Particle ";
		particle.append( FloatToString( (float) m_Particles.size() + 1.0f ) );

		// build a random position
		D3DXVECTOR3 position;
		position = D3DXVECTOR3( float( rand() % 75 ), 75.0f, float( rand() % 75 ) );
		
		int sign;
		sign = rand();
		if( sign % 2 == 0 )
			position.x = -position.x;
		sign = rand();
		if( sign % 2 == 0 )
			position.z = -position.z;
		
		// build a random color
		D3DXVECTOR4 color;
		color = D3DXVECTOR4( ( float( rand() % 100 ) ) / 100.0f, ( float( rand() % 100 ) ) / 100.0f, ( float( rand() % 100 ) ) / 100.0f, ( float( rand() % 100 ) ) / 400.0f );

		float particleMass = 0.00020543f;

		D3DXVECTOR3 velocity;
		velocity = D3DXVECTOR3( float( rand() % 100 ), float( rand() % 100 ), float( rand() % 100 ) );
		sign = rand();
		if( sign % 2 == 0 )
			velocity.x = -velocity.x;
		sign = rand();
		if( sign % 2 == 0 )
			velocity.y = -velocity.y;
		sign = rand();
		if( sign % 2 == 0 )
			velocity.z = -velocity.z;

		AddParticle( particle, position, 
						D3DXVECTOR3( 0.0f, 0.0f, 0.0f ), D3DXVECTOR3( 0.0f, 0.0f, 0.0f ), 
						color,
						particleMass, 0.0f, 0.0f, 50.0f, 15.0f );
	}
	*/


	m_fTime += elapsedTime;
	// update 60 times per second.
	while( m_fTime >= m_fTargetFrameTime )
	{
		ClearGrid();
		SetGrid();
		GetNearestNeighbors();
		ComputeDensity();

		std::vector<Particle>::iterator i;
		for( i = m_Particles.begin(); i != m_Particles.end(); ++i )
		{
			i->PreStep( m_fTargetFrameTime + 0.2f );
		}

		for( i = m_Particles.begin(); i != m_Particles.end(); ++i )
		{
			i->Step( m_fTargetFrameTime + 0.2f, m_Particles, m_NeighborList );
		}

		for( i = m_Particles.begin(); i != m_Particles.end(); ++i )
		{
			i->Finalize();
		}
	
		/*
		for( i = m_Particles.begin(); i != m_Particles.end(); ++i )
		{
			if( i->m_bDelete == true )
			{
				m_Particles.erase( i );
				i = m_Particles.begin();
			}
		}
		*/

		m_fTime -= m_fTargetFrameTime;
	}
}

void ParticleSystem::Reset()
{
	m_Particles.clear();

	// set up particles
	for( unsigned i = 0; i < 200; ++i )
	{
		std::string particle = "Particle ";
		particle.append( FloatToString( (float) m_Particles.size() + 1.0f ) );

		// build a random position
		D3DXVECTOR3 position;
		position = D3DXVECTOR3( float( rand() % 15 ), float( rand() % 75 ), float( rand() % 15 ) );
		
		int sign;
		sign = rand();
		if( sign % 2 == 0 )
			position.x = -position.x;
		sign = rand();
		if( sign % 2 == 0 )
			position.y = -position.y;
		sign = rand();
		if( sign % 2 == 0 )
			position.z = -position.z;
		
		// build a random color
		D3DXVECTOR4 color;
		color = D3DXVECTOR4( ( float( rand() % 100 ) ) / 100.0f, ( float( rand() % 100 ) ) / 100.0f, ( float( rand() % 100 ) ) / 100.0f, ( float( rand() % 100 ) ) / 400.0f );

		float particleMass = 0.00020543f;

		D3DXVECTOR3 velocity;
		velocity = D3DXVECTOR3( float( rand() % 100 ), float( rand() % 100 ), float( rand() % 100 ) );
		sign = rand();
		if( sign % 2 == 0 )
			velocity.x = -velocity.x;
		sign = rand();
		if( sign % 2 == 0 )
			velocity.y = -velocity.y;
		sign = rand();
		if( sign % 2 == 0 )
			velocity.z = -velocity.z;

		AddParticle( particle, position, 
						D3DXVECTOR3( 0.0f, 0.0f, 0.0f ), D3DXVECTOR3( 0.0f, 0.0f, 0.0f ), 
						color,
						particleMass, 0.0f, 0.0f, 50.0f, 15.0f );
	}
}

// reset the NN grid
void ParticleSystem::ClearGrid()
{
	float minX;
	float maxX;
	float minY;
	float maxY;
	float minZ;
	float maxZ;

	minX = minY = minZ = 100;
	maxX = maxY = maxZ = -100;

	// find boundaries
	std::vector<Particle>::iterator i;
	for( i = m_Particles.begin(); i != m_Particles.end(); ++i )
	{
		// find the actual boundaries
		if( minX > i->m_vPosition.x )
			minX = i->m_vPosition.x;
		if( maxX < i->m_vPosition.x )
			maxX = i->m_vPosition.x; 

		if( minY > i->m_vPosition.y )
			minY = i->m_vPosition.y;
		if( maxY < i->m_vPosition.y )
			maxY = i->m_vPosition.y; 

		if( minZ > i->m_vPosition.z )
			minZ = i->m_vPosition.z;
		if( maxZ < i->m_vPosition.z )
			maxZ = i->m_vPosition.z; 
	}

	m_Grid.width	= (int) ( ( maxX - minX + m_Grid.length ) / m_Grid.length );
	m_Grid.height	= (int) ( ( maxY - minY + m_Grid.length ) / m_Grid.length );
	m_Grid.depth	= (int) ( ( maxZ - minZ + m_Grid.length ) / m_Grid.length );

	m_Grid.x = minX;
	m_Grid.y = minY;
	m_Grid.z = minZ;

	// clear grid values
	std::vector< std::vector<int> >::iterator j;
	for( j = m_Grid.particles.begin(); j != m_Grid.particles.end(); ++j )
	{
		j->clear();
	}

	m_NeighborList.clear();
}

// place particles into the grid
void ParticleSystem::SetGrid()
{
	float invLength = 1.0f / m_Grid.length;

	std::vector<Particle>::iterator i;
	unsigned j = 0;
	for( i = m_Particles.begin(); i != m_Particles.end(); ++i, ++j )
	{
		unsigned x,y,z;

		x = (unsigned) ( ( i->m_vPosition.x - m_Grid.x ) * invLength );
		y = (unsigned) ( ( i->m_vPosition.y - m_Grid.y ) * invLength );
		z = (unsigned) ( ( i->m_vPosition.z - m_Grid.z ) * invLength );

		unsigned index = ThreeDimensionalMap( x, y, z );

		if( index > m_Grid.particles.size() - 1 )
		{
			continue;
		}

		m_Grid.particles[ index ].push_back( j );
	}
}

void ParticleSystem::GetNearestNeighbors()
{
	float invLength = 1.0f / m_Grid.length;
	float searchRadius = 100.0f;

	std::vector<Particle>::iterator i;
	unsigned j = 0;
	for( i = m_Particles.begin(); i != m_Particles.end(); ++i, ++j )
	{
		std::vector<Neighbor> neighbors;
		neighbors.push_back( Neighbor( j, 0.0f ) );

		int x,y,z;

		x = (unsigned) ( ( i->m_vPosition.x - m_Grid.x ) * invLength );
		y = (unsigned) ( ( i->m_vPosition.y - m_Grid.x ) * invLength );
		z = (unsigned) ( ( i->m_vPosition.z - m_Grid.x ) * invLength );

		unsigned index = ThreeDimensionalMap( x, y, z );

		for ( z = -1; z <= 1; z++ )
		{
			for ( y = -1; y <= 1; y++ )
			{
				for ( x = -1; x <= 1; x++ )
				{
					int neighborIndex = index + m_Grid.width * m_Grid.height * z + m_Grid.width *y + x;

					// bounds check
					if ( ( neighborIndex < 0 ) || ( neighborIndex >= (int) ( m_Grid.width * m_Grid.depth * m_Grid.height ) ) )
					{
						continue;
					}

					// store this index in the particle class
					i->m_uNLocation = j;

					std::vector<int>::iterator k;
					for( k = m_Grid.particles[ neighborIndex ].begin(); k != m_Grid.particles[ neighborIndex ].end(); ++k )
					{
						D3DXVECTOR3 vDistance;
						vDistance = i->m_vPosition - m_Particles[ *k ].m_vPosition;

						float distanceSquared;
						distanceSquared = vDistance.x * vDistance.x + vDistance.y * vDistance.y + vDistance.z * vDistance.z;
						
						// within search distance
						if( distanceSquared < searchRadius && distanceSquared != 0.0f )
						{
							neighbors.push_back( Neighbor( (*k), distanceSquared ) );
						}
					}
				}
			}	
		}

		// add the neighbors to the list
		m_NeighborList.push_back( neighbors );
	}

}

void ParticleSystem::ComputeDensity()
{
	// clear densities
	std::vector<Particle>::iterator i;
	for( i = m_Particles.begin(); i != m_Particles.end(); ++i )
	{
		i->m_fDensity = 0.0f;
	}

	unsigned count = 0;
	for ( i = m_Particles.begin(); i != m_Particles.end(); ++i, ++count )
	{
		std::vector<Neighbor>::iterator j;
		for ( j = m_NeighborList[ count ].begin(); j < m_NeighborList[ count ].end(); ++j )
		{
			float h = 50.0f;

			if (h > j->distanceSquared )
			{
				float r;
				r = h - j->distanceSquared;

				float density;
				density = r * r * r;
				
				i->m_fDensity += m_Particles[ j->index ].m_fMass * density;
				
				if ( count != j->index )
				{
					m_Particles[ j->index ].m_fDensity += i->m_fMass * density; 
				}
			}
		}
	}
	
	for ( i = m_Particles.begin(); i != m_Particles.end(); ++i )
	{
		i->m_fDensity	*=	legendreCoeff;
		i->m_fPressure	=	1.5f * ( i->m_fDensity - 1000.0f );
		i->m_fDensity	=	1.0f / i->m_fDensity;
	}
}

unsigned ParticleSystem::ThreeDimensionalMap( unsigned x, unsigned y, unsigned z )
{
	return ( x + ( m_Grid.width * y ) + ( m_Grid.width * m_Grid.height * z ) );
}


// Helper Function
std::string ParticleSystem::FloatToString( const float &f )
{
	std::stringstream ss;
	ss << f;
	std::string result;
	ss >> result;
	return result;
};
