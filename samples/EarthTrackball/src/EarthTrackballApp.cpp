
/*
 This code is designed for use with the Cinder C++ library, http://libcinder.org
 
 Earthquake sample code repurposed by permssion from Robert Hodgin.
*/

#include "AppTouch.h"

#include "Earth.h"
#include "POV.h"
#include "Resources.h"

#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Xml.h"
#include "cinder/Url.h"
#include "cinder/Vector.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder/gl/TileRender.h"

#include "Trackball3D.h"
#include "CatchAll.h"
#include "PivotRenderer.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#include <vector>
#include <sstream>
using std::vector;
using std::string;
using std::istringstream;
using std::stringstream;


class EarthTrackballApp : public Pivot::AppTouch {
public:
	void prepareSettings( Settings *settings );
	void keyDown( KeyEvent event );
	
	void	touchesBegan( TouchEvent event );
	void	touchesMoved( TouchEvent event );
    void	touchesEnded( TouchEvent event );
    void    touchesCancelled( TouchEvent event );
	
	void parseEarthquakes( const string &url );
	void setup();
	void update();
	void draw();
	
	gl::GlslProg	mEarthShader;
	gl::GlslProg	mQuakeShader;
	
	gl::Texture		mStars;
	
	POV				mPov;
	Earth			mEarth;
    
    Vec3f			mLightDir;
	Vec3f			sBillboardUp, sBillboardRight;
	Quatf			mSceneQuat;
	
	float					mInitRadius;
	Pivot::Trackball3D		mTrackball;
	Pivot::CatchAll			mCatchAll; // for debug drawing leftover touches
    
    float	mPrevTime;
};


void EarthTrackballApp::prepareSettings( Settings *settings )
{
	//settings->setWindowSize( 1920, 1080 );
    settings->setWindowSize( 1280, 720 );
	settings->setFrameRate( 60.0f );
	settings->enableMultiTouch();
	//settings->setResizable( true );
	//settings->setFullScreen( false );
}


void EarthTrackballApp::setup()
{
	mInitRadius = 260.0f;
    
    mPov = POV( this, ci::Vec3f( 0.0f, 0.0f, -1000.0f ), ci::Vec3f( 0.0f, 0.0f, 0.0f ) );
	
	gl::Texture earthDiffuse	= gl::Texture( loadImage( loadResource( RES_EARTHDIFFUSE ) ) );
	gl::Texture earthNormal		= gl::Texture( loadImage( loadResource( RES_EARTHNORMAL ) ) );
	gl::Texture earthMask		= gl::Texture( loadImage( loadResource( RES_EARTHMASK ) ) );
	earthDiffuse.setWrap( GL_REPEAT, GL_REPEAT );
	earthNormal.setWrap( GL_REPEAT, GL_REPEAT );
	earthMask.setWrap( GL_REPEAT, GL_REPEAT );
	
	mStars = gl::Texture( loadImage( loadResource( RES_STARS_PNG ) ) );
	
	mEarthShader = gl::GlslProg( loadResource( RES_PASSTHRU_VERT ), loadResource( RES_EARTH_FRAG ) );
	mQuakeShader = gl::GlslProg( loadResource( RES_QUAKE_VERT ), loadResource( RES_QUAKE_FRAG ) );
	
	mLightDir = Vec3f( 0.025f, 0.25f, 1.0f );
	mLightDir.normalize();
	mEarth = Earth( earthDiffuse, earthNormal, earthMask );
	
    try {
        parseEarthquakes( "http://earthquake.usgs.gov/earthquakes/catalogs/7day-M2.5.xml" );
        mEarth.setQuakeLocTip();
    } catch( const std::exception& e ) {
        console() << "Earthquake data not available!" << endl;
    }
	
	mTrackball = Pivot::Trackball3D( Vec3f( 0.0, 0.0, 0.0f ), mInitRadius, mPov.mCam );
	mTrackball.setDebugColor( Color( 0,1,1 ) );
	mSceneQuat = mTrackball.getOrientation();
}


void EarthTrackballApp::keyDown( KeyEvent event )
{
	
}


void EarthTrackballApp::touchesBegan( TouchEvent event )
{
	Pivot::TouchList touchesList = Pivot::toList( event.getTouches() );
	
	mTrackball.touchesBegan( &touchesList );
	mCatchAll.touchesBegan( &touchesList );
}

void EarthTrackballApp::touchesMoved( TouchEvent event )
{
	Pivot::TouchList touchesList = Pivot::toList( event.getTouches() );
	
	mTrackball.touchesMoved( &touchesList );
	mCatchAll.touchesMoved( &touchesList );
}

void EarthTrackballApp::touchesEnded( TouchEvent event )
{
	Pivot::TouchList touchesList = Pivot::toList( event.getTouches() );
	
	mTrackball.touchesEnded( &touchesList );
	mCatchAll.touchesEnded( &touchesList );
}

void EarthTrackballApp::touchesCancelled( TouchEvent event )
{
	Pivot::TouchList touchesList = Pivot::toList( event.getTouches() );
    
    mTrackball.touchesCancelled( &touchesList );
	mCatchAll.touchesCancelled( &touchesList );
}



void EarthTrackballApp::update()
{
    // update time change since last update
	float deltaTime = getElapsedSeconds() - mPrevTime;
	mPrevTime = getElapsedSeconds();
	
    // update camera
	mPov.update();
	mPov.mCam.getBillboardVectors( &sBillboardRight, &sBillboardUp );
	
    // update Trackball
	mTrackball.setCamera( mPov.mCam );
	mTrackball.update( deltaTime );
	
	// apply Trackball radius to globe
	mEarth.setRadius( mTrackball.getRadius() );
	mEarth.update();
    
    // slerp scene orientation to Trackball orientation, for smoother movement
	mSceneQuat = mSceneQuat.slerp( 0.4f, mTrackball.getOrientation() );
}


void EarthTrackballApp::draw()
{
	gl::clear( Color( 0.5f, 0.5f, 0.5f ) );
	
    // DRAW EARTH SCENE ///////////////////////////
	
	gl::pushMatrices();
	
	gl::enableAlphaBlending();
	gl::enableDepthRead( true );
	gl::enableDepthWrite( true );
	gl::color( 1, 1, 1 );
    
    // apply orientation to scene
	gl::rotate( mSceneQuat );
    
    // draw stars
	mStars.enableAndBind();
	gl::drawSphere( Vec3f( 0, 0, 0 ), 15000.0f, 64 );
	
    // draw earth
    mEarthShader.bind();
    mEarthShader.uniform( "texDiffuse", 0 );
    mEarthShader.uniform( "texNormal", 1 );
    mEarthShader.uniform( "texMask", 2 );
    mEarthShader.uniform( "lightDir", mLightDir );
    mEarth.draw();
    mEarthShader.unbind();
	
    // draw quake cones
    mQuakeShader.bind();
    mQuakeShader.uniform( "lightDir", mLightDir );
    mEarth.drawQuakeVectors();
    mQuakeShader.unbind();
	
    // draw quake labels
    //gl::enableDepthWrite( false );
    mEarth.drawQuakeLabelsOnSphere( mPov.mEyeNormal, mPov.mDist );
	
	gl::popMatrices();
	
	
	// DRAW TRACKBALL DEBUG /////////////////////
	
	gl::pushMatrices();
	
	// reseting everything I can think of...
	gl::enableAlphaBlending();
	gl::enableDepthRead( false );
	gl::enableDepthWrite( false );
	glDisable( GL_TEXTURE_2D );
    gl::color( 1, 1, 1 );
	
    // debug draw Trackball
	//Pivot::draw( mTrackball );
	Pivot::drawTouches( mTrackball );
	Pivot::drawPivot( mTrackball );
	
	// debug draw remaining touches
	Pivot::drawTouches( mCatchAll );
	//Pivot::drawPivot( mCatchAll );
    
	gl::popMatrices();
}


void EarthTrackballApp::parseEarthquakes( const string &url )
{
	const XmlTree xml( loadUrl( Url( url ) ) );
	for( XmlTree::ConstIter itemIter = xml.begin( "feed/entry" ); itemIter != xml.end(); ++itemIter ) {
		string titleLine( itemIter->getChild( "title" ).getValue() );
		size_t firstComma = titleLine.find( ',' );
		float magnitude = fromString<float>( titleLine.substr( titleLine.find( ' ' ) + 1, firstComma - 2 ) );
		string title = titleLine.substr( firstComma + 2 );
		
		istringstream locationString( itemIter->getChild( "georss:point" ).getValue() );
		Vec2f locationVector;
		locationString >> locationVector.x >> locationVector.y;
		
		mEarth.addQuake( locationVector.x, locationVector.y, magnitude, title );		
	}
	//console() << xml << std::endl;
	//mEarth.addQuake( 37.7f, -122.0f, 8.6f, "San Francisco" );
}


CINDER_APP_BASIC( EarthTrackballApp, RendererGl )
