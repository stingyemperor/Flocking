#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "cinder/Camera.h"
#include "cinder/Utilities.h"
#include "cinder/params/Params.h"
#include "cinder/Perlin.h"
#include "../vc2019/ParticleController.h"


#define NUM_INITIAL_PARTICLES 2000
#define NUM_INITIAL_PREDATORS 10
#define NUM_PARTICLES_TO_SPAWN 15


using namespace ci;
using namespace ci::app;

class FlockingApp : public App {
  public:
	void prepareSettings( Settings *settings );
	void keyDown( KeyEvent event );
	void setup();
	void update();
	void draw();
	
	// PARAMS
	params::InterfaceGlRef	mParams;
	
	// CAMERA
	CameraPersp			mCam;
	quat				mSceneRotation;
	vec3				mEye, mCenter, mUp;
	float				mCameraDistance;
	
	ParticleController	mParticleController;
	float				mZoneRadius;
	float				mLowerThresh, mHigherThresh;
	float				mAttractStrength, mRepelStrength, mOrientStrength;
	
	bool				mCentralGravity;
	bool				mFlatten;
	bool				mSaveFrames;
	bool				mIsRenderingPrint;
};

void prepareSettings( App::Settings* settings )
{
	settings->setWindowSize( 2560, 1440 );
	settings->setFrameRate(60.0f);
}

void FlockingApp::setup()
{
	Rand::randomize();
	
	mCenter			= vec3( getWindowWidth() * 0.5f, getWindowHeight() * 0.5f, 0.0f );
	mCentralGravity = true;
	mFlatten		= false;
	mSaveFrames		= false;
	mIsRenderingPrint = false;
	mZoneRadius		= 50.0f;
	mLowerThresh	= 0.5f;
	mHigherThresh	= 0.8f;
	mAttractStrength	= 0.154f;
	mRepelStrength		= 0.5f;
	mOrientStrength		= 0.01f;
	
	// SETUP CAMERA
	mCameraDistance = 1750.0f;
	mEye			= vec3( 0.0f, 0.0f, mCameraDistance );
	mCenter			= vec3(0);
	mUp				= vec3(0,1,0);
	mCam.setPerspective( 75.0f, getWindowAspectRatio(), 5.0f, 5000.0f );

	// SETUP PARAMS
	mParams = params::InterfaceGl::create( "Flocking", ivec2( 200, 310 ) );
	mParams->addParam( "Scene Rotation", &mSceneRotation, "opened=1" );

	mParams->addParam( "Flatten", &mFlatten, "keyIncr=f" );

	
	// CREATE PARTICLE CONTROLLER
	mParticleController.addParticles( NUM_INITIAL_PARTICLES );
	mParticleController.addPredators( NUM_INITIAL_PREDATORS );

}

void FlockingApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'p' ){
		mParticleController.addParticles( NUM_PARTICLES_TO_SPAWN );
	} else if( event.getChar() == ' ' ){
		mSaveFrames = !mSaveFrames;
	}
}


void FlockingApp::update()
{
	if( mLowerThresh > mHigherThresh ) mHigherThresh = mLowerThresh;

	mParticleController.applyForceToPredators( mZoneRadius, 0.4f, 0.7f );	
	mParticleController.applyForceToParticles( mZoneRadius, mLowerThresh, mHigherThresh, mAttractStrength, mRepelStrength, mOrientStrength );
	if( mCentralGravity ) mParticleController.pullToCenter( mCenter );
	mParticleController.update( mFlatten );
	
	mEye	= vec3( 0.0f, 0.0f, mCameraDistance );
	mCam.lookAt( mEye, mCenter, mUp );
	gl::setMatrices( mCam );
	gl::rotate( mSceneRotation );
}

void FlockingApp::draw()
{
	gl::clear( Color( 0, 0, 0 ), true );
	gl::enableDepthRead();
	gl::enableDepthWrite();

	gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
	mParticleController.draw();

	// DRAW PARAMS WINDOW
	mParams->draw();

}



CINDER_APP( FlockingApp, RendererGl, prepareSettings )
