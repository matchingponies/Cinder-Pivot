/*
 *  BasicTrackballDemoApp.cpp
 *
 *  Created by Chris McKenzie on 12/15/11.
 *  Copyright 2011. All rights reserved.
 *
 */

#include "cinder/Camera.h"
#include <vector>
#include <list>

#include "AppTouch.h"
#include "Trackball3D.h"
#include "Trackball.h"
#include "Card.h"
#include "CatchAll.h"
#include "TouchObject.h"
#include "PivotRenderer.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class BasicTrackballDemoApp : public Pivot::AppTouch {
public:
	void	prepareSettings( Settings *settings );
	void	setup();
	
	void	keyDown( KeyEvent event );
	
	void	touchesBegan( TouchEvent event );
	void	touchesMoved( TouchEvent event );
    void	touchesEnded( TouchEvent event );
    void	touchesCancelled( TouchEvent event );
	
	void	update();
	void	draw();
	
	Pivot::Card				mCard;
	Pivot::Trackball3D		mTrackball;
	Pivot::Trackball3D		mTrackball2;
	Pivot::Trackball		mTrackball3;
	Pivot::CatchAll			mCatchAll; // for debug drawing leftover touches
	
	list<Pivot::TouchObject*>	mTouchObjects;
	
	float	mPrevTime;
	
	bool	mDrawTouches, mDrawPivot, mDrawObjects, mInteractObjects;
	
	CameraPersp mCamPersp;
	CameraOrtho mCamOrtho;
};



void BasicTrackballDemoApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize(768,1024);
	settings->setFrameRate(60.0f);
	settings->enableMultiTouch();
}



void BasicTrackballDemoApp::setup()
{
	// create demo cameras
	mCamPersp = CameraPersp( getWindowWidth(), getWindowHeight(), 60.0f );
	mCamOrtho = CameraOrtho( 0, getWindowWidth(), getWindowHeight(), 0, -1, 1 );
	
	// create UI
	mCard = Pivot::Card( Vec2f( 100.0f, 700.0f ), 300.0f, 200.0f, 0.0f );
	mCard.setMinScale( 0.6f );
	mCard.setMaxScale( 3.0f );
	mTrackball = Pivot::Trackball3D( Vec3f( 384.0f, 512.0f, 0.0f ), 150.0f, mCamPersp );
	mTrackball.setDebugColor( Color( 0,1,1 ) );
	mTrackball.setMinRadius( 150.0f );
	mTrackball.setMaxRadius( 400.0f );
	mTrackball2 = Pivot::Trackball3D( Vec3f( 200.0f, 800.0f, 0.0f ), 200.0f, mCamPersp );
	mTrackball3 = Pivot::Trackball( Vec2f( 570.0f, 800.0f ), 150.0f );
	
	// pushing these back in the order of interactive depth
	mTouchObjects.push_back( &mCard );
	mTouchObjects.push_back( &mTrackball );
	mTouchObjects.push_back( &mTrackball2 );
	mTouchObjects.push_back( &mTrackball3 );
	mTouchObjects.push_back( &mCatchAll );
	
	mPrevTime = getElapsedSeconds();
	
	mDrawTouches = mDrawPivot = mDrawObjects = mInteractObjects = true;
    
    //mDrawObjects = mInteractObjects = false; // temp
}



void BasicTrackballDemoApp::keyDown( KeyEvent event )
{
	int ch = toupper( event.getChar() );
	if ( ch == 'Q' ) {
		quit();
	} else if ( ch == 'T' ) {
		mDrawTouches = !mDrawTouches;
	} else if ( ch == 'P' ) {
		mDrawPivot = !mDrawPivot;
	} else if ( ch == 'O' ) {
		mDrawObjects = !mDrawObjects;
	} else if ( ch == 'I' ) {
		mInteractObjects = !mInteractObjects;
	}
}



void BasicTrackballDemoApp::touchesBegan( TouchEvent event )
{
	Pivot::TouchList touchesList = Pivot::toList( event.getTouches() );
    
	if ( mInteractObjects ) {
		for( list<Pivot::TouchObject*>::iterator it = mTouchObjects.begin(); it != mTouchObjects.end(); ++it ) {
			if ( touchesList.empty() ) break;
			(*it)->touchesBegan( &touchesList );
		}
	} else {
		mCatchAll.touchesBegan( &touchesList );
	}
}

void BasicTrackballDemoApp::touchesMoved( TouchEvent event )
{ 
	Pivot::TouchList touchesList = Pivot::toList( event.getTouches() );
    
	if ( mInteractObjects ) {
		for( list<Pivot::TouchObject*>::iterator it = mTouchObjects.begin(); it != mTouchObjects.end(); ++it ) {
			if ( touchesList.empty() ) break;
			(*it)->touchesMoved( &touchesList );
		}
	} else {
		mCatchAll.touchesMoved( &touchesList );
	}
}

void BasicTrackballDemoApp::touchesEnded( TouchEvent event )
{
    Pivot::TouchList touchesList = Pivot::toList( event.getTouches() );
    
    if ( mInteractObjects ) {
		for( list<Pivot::TouchObject*>::iterator it = mTouchObjects.begin(); it != mTouchObjects.end(); ++it ) {
			if ( touchesList.empty() ) break;
			(*it)->touchesEnded( &touchesList );
		}
	} else {
		mCatchAll.touchesEnded( &touchesList );
	}
}

void BasicTrackballDemoApp::touchesCancelled( TouchEvent event )
{
    console() << "touchesCancelled" << endl;
    Pivot::TouchList touchesList = Pivot::toList( event.getTouches() );
    
    if ( mInteractObjects ) {
		for( list<Pivot::TouchObject*>::iterator it = mTouchObjects.begin(); it != mTouchObjects.end(); ++it ) {
			if ( touchesList.empty() ) break;
			(*it)->touchesCancelled( &touchesList );
		}
	} else {
		mCatchAll.touchesCancelled( &touchesList );
	}
}



void BasicTrackballDemoApp::update()
{
	float deltaTime = getElapsedSeconds() - mPrevTime;
	mPrevTime = getElapsedSeconds();
	
	for( list<Pivot::TouchObject*>::reverse_iterator it = mTouchObjects.rbegin(); it != mTouchObjects.rend(); ++it )
		(*it)->update( deltaTime );
}



void BasicTrackballDemoApp::draw()
{
	gl::clear( Color( 0.0f, 0.0f, 0.0f ) );
	
	if ( mDrawObjects ) {
		Pivot::draw( mCatchAll );
		Pivot::draw( mTrackball3 );
		Pivot::draw( mTrackball2 );
		Pivot::draw( mTrackball );
		Pivot::draw( mCard );
	}
	
	for( list<Pivot::TouchObject*>::reverse_iterator it = mTouchObjects.rbegin(); it != mTouchObjects.rend(); ++it ) {
		if ( mDrawTouches ) Pivot::drawTouches( *(*it) );
		if ( mDrawPivot ) Pivot::drawPivot( *(*it) );
	}
}



CINDER_APP_NATIVE( BasicTrackballDemoApp, RendererGl );

