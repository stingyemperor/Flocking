#include "cinder/app/App.h"
#include "cinder/Rand.h"
#include "cinder/Vector.h"
#include "ParticleController.h"


using namespace ci;
using std::list;

ParticleController::ParticleController(){}

ParticleController::ParticleController(int res)
{
	mXRes = app::getWindowWidth()/res;
	mYRes = app::getWindowHeight()/res;
	
	for( int y=0; y<mYRes; y++ ){
		for( int x=0; x<mXRes; x++ ){
			addParticle( x, y, res );
		}
	}
}

void ParticleController::update(const ci::Channel32f &channel, const ci::vec2 &mouseLoc)
{
	for( list<Particle>::iterator p = mParticles.begin(); p != mParticles.end(); ++p ){
		p->update( channel, mouseLoc );
	}
}

void ParticleController::draw()
{
	for(list<Particle>::iterator p = mParticles.begin(); p != mParticles.end(); ++p )
	{
		p->draw();
	}
}


void ParticleController::addParticle(int xi, int yi, int res)
{
	float x = ( xi + 0.5f ) * (float)res;
	float y = ( yi + 0.5f ) * (float)res;
	mParticles.emplace_back( Particle( vec2(x,y) ) );
}

void ParticleController::addParticles(int amt)
{
	for(int i = 0; i < amt; i++)
	{
		float x = Rand::randFloat(app::getWindowWidth());
		float y = Rand::randFloat(app::getWindowHeight());
		mParticles.emplace_back(Particle(vec2(x,y)));
	}
}

void ParticleController::removeParticles(int amt)
{
	for(int i = 0; i < amt; i++)
	{
		mParticles.pop_back();
	}
}



