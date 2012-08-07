


#include "cinder/Camera.h"
#include "cinder/ObjLoader.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Vbo.h"
#include "cinder/TriMesh.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"

#include <vector>
#include <list>


#include "AppTouch.h"
#include "TouchObject.h"
#include "Trackball.h"
#include "Card.h"
#include "CatchAll.h"
#include "PivotRenderer.h"

#include "Resources.h"

using namespace ci;
using namespace ci::app;
using namespace std;



class ProductTrackballApp : public Pivot::AppTouch {
public:
	void prepareSettings( Settings *settings );
	void setup();
	
	void	touchesEnded( TouchEvent event );
	void	touchesBegan( TouchEvent event );
	void	touchesMoved( TouchEvent event );
    void	touchesCancelled( TouchEvent event );
	
	void update();
	void draw();
	
	
	ci::CameraPersp mCam;
	Quatf			mSceneQuat;
	
	gl::VboMesh		mVBO;
	TriMesh			mMesh;
    gl::Texture		mTexture;
	gl::GlslProg	mShader;
	
	float	mPrevTime;
	
	Pivot::Trackball		mTrackball;
	Pivot::CatchAll	mCatchAll; // for debug drawing leftover touches
	
	list<Pivot::TouchObject*>	mTouchObjects;
};




void ProductTrackballApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 1280, 800 );
	settings->setFrameRate( 60.0f );
    settings->setResizable( false );
	settings->enableMultiTouch();
}



void ProductTrackballApp::setup()
{
	gl::enableAlphaBlending();
	
	// shader
	mShader = gl::GlslProg( loadResource( RES_PHONG_VERT ), loadResource( RES_PHONG_FRAG ) );
	
    // object
    mMesh.read( loadResource( RES_3D_MESH ) );
    mVBO = gl::VboMesh( mMesh );
    
    
    // texture
    gl::Texture::Format format;
	format.enableMipmapping(true);
	mTexture = gl::Texture( loadImage( loadResource( RES_MESH_TEX ) ), format );
    
    
	// setup camera
	mCam = CameraPersp( getWindowWidth(), getWindowHeight(), 45.0f );
	
	// create UI
	mTrackball = Pivot::Trackball( Vec2f( 640.0f, 400.0f ), 350.0f );
	mTrackball.setDebugColor( Color( 0.3f, 0.3f, 0.3f ) );
	mCatchAll.setDebugColor( Color( 0.8f, 0.8f, 0.8f ) );
	
	// pushing these back in the order of interactive depth
	mTouchObjects.push_back( &mTrackball );
	mTouchObjects.push_back( &mCatchAll );
	
	mSceneQuat = mTrackball.getOrientation();
}



void ProductTrackballApp::touchesBegan( TouchEvent event )
{
	Pivot::TouchList touchesList = Pivot::toList( event.getTouches() );
	for( list<Pivot::TouchObject*>::iterator it = mTouchObjects.begin(); it != mTouchObjects.end(); ++it ) {
		if ( touchesList.size() == 0 ) break;
		(*it)->touchesBegan( &touchesList );
	}
}

void ProductTrackballApp::touchesMoved( TouchEvent event )
{
	Pivot::TouchList touchesList = Pivot::toList( event.getTouches() );
	for( list<Pivot::TouchObject*>::iterator it = mTouchObjects.begin(); it != mTouchObjects.end(); ++it ) {
		if ( touchesList.size() == 0 ) break;
		(*it)->touchesMoved( &touchesList );
	}
}

void ProductTrackballApp::touchesEnded( TouchEvent event )
{
	Pivot::TouchList touchesList = Pivot::toList( event.getTouches() );
	for( list<Pivot::TouchObject*>::iterator it = mTouchObjects.begin(); it != mTouchObjects.end(); ++it ) {
		if ( touchesList.size() == 0 ) break;
		(*it)->touchesEnded( &touchesList );
	}
}

void ProductTrackballApp::touchesCancelled( TouchEvent event )
{
	Pivot::TouchList touchesList = Pivot::toList( event.getTouches() );
	for( list<Pivot::TouchObject*>::iterator it = mTouchObjects.begin(); it != mTouchObjects.end(); ++it ) {
		if ( touchesList.size() == 0 ) break;
		(*it)->touchesCancelled( &touchesList );
	}
}



void ProductTrackballApp::update()
{
	float mDeltaTime = getElapsedSeconds() - mPrevTime;
	mPrevTime = getElapsedSeconds();
	
	for( list<Pivot::TouchObject*>::reverse_iterator it = mTouchObjects.rbegin(); it != mTouchObjects.rend(); ++it )
		(*it)->update( mDeltaTime );
}



void ProductTrackballApp::draw()
{
	gl::clear( Color( 0.9f, 0.9f, 0.9f ) );
	
	float ratio = mTrackball.getRadius() / 3.5f;
	
	// bg circle
	gl::color( Color( 1, 1, 1 ) );
	gl::drawSolidCircle( Vec2f( 640.0f, 400.0f ), mTrackball.getRadius() * 1.1f + 10.0f );
	
	// prep for 3d scene
	gl::enableDepthRead();
	gl::enableDepthWrite();
	gl::color( Color( 1, 1, 1) );
	glDisable( GL_CULL_FACE );
	
	gl::pushMatrices();
	
	// set cam
	gl::setMatrices( mCam );
	
	// center scene
	gl::translate( Vec3f( 640.0f, 400.0f, 0.0f ) );
	
	// apply scale to scene
	Vec3f v = Vec3f( ratio, -ratio, ratio );
	gl::scale( v );
	
	// apply Trackball orientation to scene
	mSceneQuat = mSceneQuat.slerp( 0.4f, mTrackball.getOrientation() );
	gl::rotate( mSceneQuat );
    gl::rotate( Vec3f( 0.0f, 0.0f, 180.0f ) );
	
	// center object
	gl::translate( Vec3f( 0.0f, -2.0f, 0.6f ) );
	
	// bind texture, bind shader, draw
    mTexture.bind();
	mShader.bind();
    mShader.uniform("tex0", 0);
	gl::draw( mVBO );
    mShader.unbind();
    mTexture.unbind();
	
	gl::popMatrices();
	
    
    // DEBUG DRAW TRACKBALL //////////////////////////
	gl::disableDepthRead();
	gl::disableDepthWrite();
	//Pivot::draw( mTrackball );
	for( list<Pivot::TouchObject*>::reverse_iterator it = mTouchObjects.rbegin(); it != mTouchObjects.rend(); ++it ) {
		Pivot::drawTouches( *(*it) );
		Pivot::drawPivot( *(*it) );
	}
}



CINDER_APP_BASIC( ProductTrackballApp, RendererGl )

