#include "cinder/app/App.h"
#include "cinder/Rand.h"
#include "cinder/Vector.h"
#include "ParticleController.h"
#include "cinder/gl/gl.h"


using namespace ci;
using std::list;

ParticleController::ParticleController()
{
	mPerlin = Perlin( 4 );
}

void ParticleController::applyForceToParticles( float zoneRadius, float lowerThresh, float higherThresh, float attractStrength, float repelStrength, float alignStrength  )
{
	float twoPI = M_PI * 2.0f;
	mParticleCentroid = vec3(0);
	mNumParticles = mParticles.size();
	
	for( list<Particle>::iterator p1 = mParticles.begin(); p1 != mParticles.end(); ++p1 ){
		
		list<Particle>::iterator p2 = p1;
		for( ++p2; p2 != mParticles.end(); ++p2 ) {
			vec3 dir = p1->mPos - p2->mPos;
			float distSqrd = ci::length2(dir);
			float zoneRadiusSqrd = zoneRadius * p1->mCrowdFactor * zoneRadius * p2->mCrowdFactor;
			
			if( distSqrd < zoneRadiusSqrd ){		// Neighbor is in the zone
				float per = distSqrd/zoneRadiusSqrd;
				p1->addNeighborPos( p2->mPos );
				p2->addNeighborPos( p1->mPos );
					
				if( per < lowerThresh ){			// Separation
					float F = ( lowerThresh/per - 1.0f ) * repelStrength;
					dir = ci::normalize(dir);
					dir *= F;
			
					p1->mAcc += dir;
					p2->mAcc -= dir;
				} else if( per < higherThresh ){	// Alignment
					float threshDelta	= higherThresh - lowerThresh;
					float adjPer		= ( per - lowerThresh )/threshDelta;
					float F				= ( 1.0f - ( cos( adjPer * twoPI ) * -0.5f + 0.5f ) ) * alignStrength;
			
					p1->mAcc += p2->mVelNormal * F;
					p2->mAcc += p1->mVelNormal * F;
					
				} else {							// Cohesion (prep)
					float threshDelta	= 1.0f - higherThresh;
					float adjPer		= ( per - higherThresh )/threshDelta;
					float F				= ( 1.0f - ( cos( adjPer * twoPI ) * -0.5f + 0.5f ) ) * attractStrength;
										
					dir = ci::normalize(dir);
					dir *= F;
			
					p1->mAcc -= dir;
					p2->mAcc += dir;
				}
			}
		}
		
		mParticleCentroid += p1->mPos;

		
		// ADD PERLIN NOISE INFLUENCE
		float scale = 0.005f;
		float multi = 0.1f;
		vec3 perlin = mPerlin.dfBm( p1->mPos * scale ) * multi;
		
		p1->mAcc += perlin;
		
		
		//PARTICLE/PREDATOR INTERACTION
		float eatDistSqrd = 50.0f;
		float predatorZoneRadiusSqrd = zoneRadius * zoneRadius * 5.0f;
		for( list<Predator>::iterator predator = mPredators.begin(); predator != mPredators.end(); ++predator ) {

			vec3 dir = p1->mPos - predator->mPos[0];
			float distSqrd = ci::length2(dir);
			
			if( distSqrd < predatorZoneRadiusSqrd ){
				
				if( distSqrd > eatDistSqrd ){
					float F = ( predatorZoneRadiusSqrd/distSqrd - 1.0f ) * 0.1f;
					p1->mFear += F * 0.1f;
					dir = ci::normalize(dir) * F;
					p1->mAcc += dir;
					p1->mColor = ci::Color(0,1,0);
					if( predator->mIsHungry )
						predator->mAcc += dir * 0.04f * predator->mHunger;
				} else {
					p1->mIsDead = true;
					predator->mHunger = 0.0f;
					predator->mIsHungry = false;
				}
			}
		}
		
	}
	mParticleCentroid /= (float)mNumParticles;
}


void ParticleController::applyForceToPredators( float zoneRadius, float lowerThresh, float higherThresh )
{
	float twoPI = M_PI * 2.0f;
	for( list<Predator>::iterator P1 = mPredators.begin(); P1 != mPredators.end(); ++P1 ){
	
		list<Predator>::iterator P2 = P1;
		for( ++P2; P2 != mPredators.end(); ++P2 ) {
			vec3 dir = P1->mPos[0] - P2->mPos[0];
			float distSqrd = ci::length2(dir);
			float zoneRadiusSqrd = zoneRadius * zoneRadius * 4.0f;
			
			if( distSqrd < zoneRadiusSqrd ){		// Neighbor is in the zone
				float per = distSqrd/zoneRadiusSqrd;
				if( per < lowerThresh ){			// Separation
					float F = ( lowerThresh/per - 1.0f ) * 0.01f;
					dir = ci::normalize(dir);
					dir *= F;
			
					P1->mAcc += dir;
					P2->mAcc -= dir;
				} else if( per < higherThresh ){	// Alignment
					float threshDelta	= higherThresh - lowerThresh;
					float adjPer		= ( per - lowerThresh )/threshDelta;
					float F				= ( 1.0f - cos( adjPer * twoPI ) * -0.5f + 0.5f ) * 0.3f;
			
					P1->mAcc += P2->mVelNormal * F;
					P2->mAcc += P1->mVelNormal * F;
					
				} else {							// Cohesion
					float threshDelta	= 1.0f - higherThresh;
					float adjPer		= ( per - higherThresh )/threshDelta;
					float F				= ( 1.0f - ( cos( adjPer * twoPI ) * -0.5f + 0.5f ) ) * 0.1f;
										
					dir = ci::normalize(dir);
					dir *= F;
			
					P1->mAcc -= dir;
					P2->mAcc += dir;
				}
			}
		}
	}
}


void ParticleController::pullToCenter( const ci::vec3 &center )
{
	for( list<Particle>::iterator p = mParticles.begin(); p != mParticles.end(); ++p ){
		p->pullToCenter( center );
	}
	
	for( list<Predator>::iterator p = mPredators.begin(); p != mPredators.end(); ++p ){
		p->pullToCenter( center );
	}
}

void ParticleController::update( bool flatten )
{
	for( list<Particle>::iterator p = mParticles.begin(); p != mParticles.end(); ){
		if( p->mIsDead ){
			p = mParticles.erase( p );
		} else {
			p->update( flatten );
			++p;
		}
	}
	
	for( list<Predator>::iterator p = mPredators.begin(); p != mPredators.end(); ++p ){
		p->update( flatten );
	}
}

void ParticleController::draw()
{	
	// DRAW PREDATOR ARROWS
	for( list<Predator>::iterator p = mPredators.begin(); p != mPredators.end(); ++p ){
		float hungerColor = 1.0f - p->mHunger;
		gl::color( ColorA( 1.0f, hungerColor, hungerColor, 1.0f ) );
		p->draw();
	}
	
	// DRAW PARTICLE ARROWS
	
	//glBegin( GL_LINES );
	for( list<Particle>::iterator p = mParticles.begin(); p != mParticles.end(); ++p ){
		
		if(p->mFear > 1.0f)
		{
			gl::color( ColorA( 0,1,0, 1.0f ) );
		}else
		{
			gl::color( ColorA( 1,1,1, 1.0f ) );
		}
		p->draw();
	}

	

}

void ParticleController::addPredators( int amt )
{
	for( int i=0; i<amt; i++ )
	{
		vec3 pos = Rand::randVec3() * Rand::randFloat( 500.0f, 750.0f );
		vec3 vel = Rand::randVec3();
		mPredators.push_back( Predator( pos, vel ) );
	}
}

void ParticleController::addParticles( int amt )
{
	for( int i=0; i<amt; i++ )
	{
		vec3 pos = Rand::randVec3() * Rand::randFloat( 100.0f, 200.0f );
		vec3 vel = Rand::randVec3();
		
		bool followed = false;
		if( mParticles.size() == 0 ) followed = true;
		
		mParticles.push_back( Particle( pos, vel, followed ) );
	}
}

void ParticleController::removeParticles( int amt )
{
	for( int i=0; i<amt; i++ )
	{
		mParticles.pop_back();
	}
}

vec3 ParticleController::getPos()
{
	return mParticles.begin()->mPos;
}

