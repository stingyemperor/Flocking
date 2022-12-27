#include "Particle.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"
#include "cinder/app/App.h"

using namespace ci;

Particle::Particle()
{
	
}

Particle::Particle(vec2 loc)
{
	mLoc = loc;
	mDir = Rand::randVec2();
	mVel = 0.0f;
	mRadius = 0.0f;
	mScale = 3.0f;
	mDirToCursor = vec2(0);
}

void Particle::update( const Channel32f &channel, const vec2 &mouseLoc)
{
	mDirToCursor= mouseLoc - mLoc;

	float distToCursor	= mDirToCursor.length();
	float time			= app::getElapsedSeconds();
	float dist			= distToCursor * 0.025f;
	float sinOffset		= sin( dist - time ) + 1.0f;

	mDirToCursor = ci::normalize(mDirToCursor);
	mDirToCursor		*= sinOffset * 100.0f;
	
	vec2 newLoc		= mLoc + mDirToCursor;
	newLoc.x			= constrain( newLoc.x, 0.0f, channel.getWidth() - 1.0f );
	newLoc.y			= constrain( newLoc.y, 0.0f, channel.getHeight() - 1.0f );
	
	mRadius				= channel.getValue( newLoc ) * mScale;
}

void Particle::draw()
{
	gl::drawSolidCircle( mLoc + mDirToCursor * 0.2f, mRadius );
}


