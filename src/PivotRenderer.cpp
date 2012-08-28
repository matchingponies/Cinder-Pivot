/*
 This code is designed for use with the Cinder C++ library, http://libcinder.org 
 
 Copyright (c) 2012, Chris McKenzie
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and
 the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 the following disclaimer in the documentation and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 The views and conclusions contained in the software and documentation are those of the authors and should not
 be interpreted as representing official policies, either expressed or implied, of the FreeBSD Project.
 */

#include "cinder/Camera.h"
#include "cinder/Utilities.h"

#include "PivotRenderer.h"

namespace Pivot {
	
	using namespace ci;
	using namespace app;
	using namespace std;

	// Singleton implementation
	RendererRef Renderer::getInstance() {
		static RendererRef renderer;
		if ( !renderer ) {
			renderer = RendererRef( new Renderer() );
		}
		return renderer;
	}
	
	Renderer::Renderer() {}
	Renderer::~Renderer() {}
	
	
	
	// Debug draw trackball
	void Renderer::draw( Trackball3D &trackball ) {
		gl::enableAlphaBlending();
		glLineWidth( 1.0f );
		gl::pushMatrices();
		{
			CameraPersp cam = trackball.getCamera();
			
			
			// for PanoSphere
			//cam.setNearClip( cam.getEyePoint().z - trackball.getCenter().z - 1.0f );
			//cam.setFarClip( trackball.getCenter().z - cam.getEyePoint().z );
			
			
			cam.setFarClip( cam.getEyePoint().z - trackball.getCenter().z );
			gl::setMatrices( cam );
			gl::color( ColorAf( trackball.getDebugColor() ) * ColorAf( 1.0f, 1.0f, 1.0f, 0.4f ) );
			
			// Draw interactive area
			gl::draw( trackball.getSphere(), 30 );
			
			// Draw three circles, one on each axis
			gl::pushMatrices();
			{
				gl::color( trackball.getDebugColor() );
				gl::translate( trackball.getCenter() );
				gl::rotate( trackball.getOrientation() );
				gl::pushMatrices();
				{
					gl::drawLine( Vec3f::zero(), Vec3f( 0.0f, trackball.getRadius() * 1.3f, 0.0f ) );
					gl::drawLine( Vec3f::zero(), Vec3f( 0.0f, 0.0f, trackball.getRadius() * 1.3f ) );
					gl::drawStrokedCircle( Vec2f::zero(), trackball.getRadius() );
					gl::rotate( Vec3f( 0, 90, 0 ) );
					gl::drawStrokedCircle( Vec2f::zero(), trackball.getRadius() );
					gl::rotate( Vec3f( 90, 0, 0 ) );
					gl::drawStrokedCircle( Vec2f::zero(), trackball.getRadius() );
				}
				gl::popMatrices();
			}
			gl::popMatrices();
		}
		gl::popMatrices();
	}
	
	
	
	// Debug draw trackball
	void Renderer::draw( Trackball &trackball ) {
		gl::enableAlphaBlending();
		glLineWidth( 1.0f );
		gl::pushMatrices();
		{
			// Set scene to screen coords
			gl::setMatrices( CameraOrtho( 0, getWindowWidth(), getWindowHeight(), 0, trackball.getRadius() * -1.1f, 0.0f ) );
			
			gl::color( ColorAf( trackball.getDebugColor() ) * ColorAf( 1.0f, 1.0f, 1.0f, 0.4f ) );
			
			// Draw interactive area
			gl::drawSolidCircle( trackball.getCenter(), trackball.getRadius() );
			gl::color( trackball.getDebugColor() );
			gl::translate( trackball.getCenter() );
			
			// Draw three circles, one on each axis
			gl::pushMatrices();
			{
				gl::rotate( trackball.getOrientation() );
				gl::pushMatrices();
				{
					gl::drawLine( Vec3f::zero(), Vec3f( 0.0f, trackball.getRadius() * 1.3f, 0.0f ) );
					gl::drawLine( Vec3f::zero(), Vec3f( 0.0f, 0.0f, trackball.getRadius() * 1.3f ) );
					gl::drawStrokedCircle( Vec2f::zero(), trackball.getRadius() );
					gl::rotate( Vec3f( 0, 90, 0 ) );
					gl::drawStrokedCircle( Vec2f::zero(), trackball.getRadius() );
					gl::rotate( Vec3f( 90, 0, 0 ) );
					gl::drawStrokedCircle( Vec2f::zero(), trackball.getRadius() );
				}
				gl::popMatrices();
			}
			gl::popMatrices();
		}
		gl::popMatrices();
	}
	
	
	// draw TouchCard
	void Renderer::draw( Card &card )
	{
		gl::enableAlphaBlending();
		glLineWidth( 1.0f );
		gl::pushMatrices();
		// set ortho cam
		gl::setMatrices( CameraOrtho( 0, getWindowWidth(), getWindowHeight(), 0, -1, 1 ) );
		gl::translate( card.getPos() );
		gl::rotate( card.getRot() * ( 180.0f/M_PI ) );
		// interactive area
		gl::color( ColorAf( card.getDebugColor() ) * ColorAf( 1.0f, 1.0f, 1.0f, 0.4f ) );
		gl::drawSolidRect( Rectf( 0.0f, 0.0f, card.getWidth(), card.getHeight() ) );
		// border
		gl::color( card.getDebugColor() );
		gl::drawStrokedRect( Rectf( 0.0f, 0.0f, card.getWidth(), card.getHeight() ) );
		gl::drawLine( Vec2f::zero(), Vec2f( 0.0f, card.getHeight() * 1.3f ) );
		gl::popMatrices();
	}
	
	// draw CatchAll
	void Renderer::draw( CatchAll &catchAll )
	{}
	
	// Debug draw touches
	void Renderer::drawTouches( TouchObject &touchObject ) {
		list<TouchPoint> touchPoints = touchObject.getTouchPoints();
		
		gl::pushMatrices();
		
		// Set up ortho camera to get projection matrix
		gl::setMatrices( CameraOrtho( 0, getWindowWidth(), getWindowHeight(), 0, -1, 1 ) );
		
		// draw listened touches
		gl::color( touchObject.getDebugColor() );
		
		if ( touchObject.getCaptureMode() ) {
			for( list<TouchPoint>::iterator touchPointIt = touchPoints.begin(); touchPointIt != touchPoints.end(); ++touchPointIt ) {
                //glLineWidth( 0.0f );
                //gl::drawString( ci::toString( touchPointIt->getId() ), touchPointIt->getPos() + Vec2f( 30.0f, -5.0f ), touchObject.getDebugColor(), Font( "Helvetica", 16.0f ) );
				glLineWidth( 2.5f );
				gl::drawStrokedCircle( touchPointIt->getPos(), 20.0f );
			}
		} else {
			for( list<TouchPoint>::iterator touchPointIt = touchPoints.begin(); touchPointIt != touchPoints.end(); ++touchPointIt ) {
				glLineWidth( 1.0f );
				gl::drawStrokedCircle( touchPointIt->getPos(), 25.0f );
			}
		}
		
		gl::popMatrices();
	}
	
	
	void Renderer::drawPivot( TouchObject &touchObject )
	{
		TouchPivot touchPivot = touchObject.getTouchPivot();
		
		if ( touchPivot.isActive() ) {
			gl::pushMatrices();
			gl::setMatrices( CameraOrtho( 0, getWindowWidth(), getWindowHeight(), 0, -1, 1 ) );
			glLineWidth( 1.0f );
			gl::color( touchObject.getDebugColor() );
			// TODO: remove this when things are in a better place /////////////////////////////////////////////////
			touchPivot.draw();
			// /////////////////////////////////////////////////////////////////////////////////////////////////////
			gl::translate( touchPivot.getPos() );
			gl::rotate( Vec3f( 0, 0, ( touchPivot.getRot()*180.0f ) / M_PI ) );
			for ( int i = 0; i < 25; ++i) gl::drawLine( Vec2f( i*10.0f*touchPivot.getScale(), 0.0f ), Vec2f( (i*10.0f + 4)*touchPivot.getScale(), 0.0f ) );
			gl::rotate( Vec3f( 0, 0, 90 ) );
			for ( int i = 0; i < 25; ++i) gl::drawLine( Vec2f( i*10.0f*touchPivot.getScale(), 0.0f ), Vec2f( (i*10.0f + 4)*touchPivot.getScale(), 0.0f ) );
			gl::rotate( Vec3f( 0, 0, 90 ) );
			for ( int i = 0; i < 25; ++i) gl::drawLine( Vec2f( i*10.0f*touchPivot.getScale(), 0.0f ), Vec2f( (i*10.0f + 4)*touchPivot.getScale(), 0.0f ) );
			gl::rotate( Vec3f( 0, 0, 90 ) );
			for ( int i = 0; i < 25; ++i) gl::drawLine( Vec2f( i*10.0f*touchPivot.getScale(), 0.0f ), Vec2f( (i*10.0f + 4)*touchPivot.getScale(), 0.0f ) );
			gl::popMatrices();
			
			
		}
	}
	
	// static calls
	void draw( Trackball3D &trackball ) { Renderer::getInstance()->draw( trackball ); }
	void draw( Trackball &trackball ) { Renderer::getInstance()->draw( trackball ); }
	void draw( Card &card ) { Renderer::getInstance()->draw( card ); }
	void draw( CatchAll &catchAll ) { Renderer::getInstance()->draw( catchAll ); }
	
	void drawTouches( TouchObject &touchObject ) { Renderer::getInstance()->drawTouches( touchObject ); }
	void drawPivot( TouchObject &touchObject ) { Renderer::getInstance()->drawPivot( touchObject ); }
}


