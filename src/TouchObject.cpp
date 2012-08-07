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

#include "cinder/app/App.h"
#include "cinder/Rand.h"
#include "cinder/Utilities.h"
#include "cinder/gl/gl.h"

#include "TouchObject.h"

namespace Pivot {
		
	using namespace ci;
	using namespace ci::app;
	using namespace std;
	
	TouchList toList( const vector<TouchEvent::Touch> &touches )
	{
		TouchList touchesList;
		std::copy( touches.begin(), touches.end(), std::back_inserter( touchesList ) );
		return touchesList;
	}
	
	
	
	TouchObject3D::TouchObject3D( CameraPersp cam, bool captureTouches )
	: TouchObject( captureTouches )
	{
		setCamera( cam );
	}
	
	

	TouchObject::TouchObject()
	{
		enableCaptureMode();
		
		mIsInMotion = false;
		mVelDecay = 0.92f;
		
		// debug color (for debug renderer)
		setDebugColor( Color( CM_HSV, Rand::randFloat(), 1, 1 ) );
	}

	TouchObject::TouchObject( bool capturingTouches )
	{
		enableCaptureMode( capturingTouches );
		
		mIsInMotion = false;
		mVelDecay = 0.92f;
		
		setDebugColor( Color( CM_HSV, Rand::randFloat(), 1, 1 ) );
	}

	void TouchObject::touchesBegan( TouchList *touches )
	{
		bool isPivotReset;
		
		if ( mTouchPoints.size() == 0 ) isPivotReset = false;
		else isPivotReset = true;
		
		// test if touch within interactive zone
		list<TouchEvent::Touch>::iterator touchIt;
		list<TouchPoint> changedTouchPoints;
		
		touchIt = touches->begin();
		while( touchIt != touches->end() ) {
			if( hitTest( *touchIt ) ) {
				mTouchPoints.push_back( TouchPoint( touchIt->getPos(), touchIt->getPos(), touchIt->getPos(), touchIt->getId(), touchIt->getTime(), touchIt->getNative() ) );
				changedTouchPoints.push_back( mTouchPoints.back() );
				if( mCapturingTouches ) touchIt = touches->erase(touchIt);
				else ++touchIt;
			} else {
				++touchIt;
			}
		}
		
		if ( changedTouchPoints.size() > 0 ) {
			mTouchPivot.touchPointsBegan( &mTouchPoints, &changedTouchPoints );
			touchPointsBegan( &mTouchPoints, &changedTouchPoints, &mTouchPivot );
			if (isPivotReset) pivotReset( &mTouchPivot );
			else pivotBegan( &mTouchPivot );
		}
	}

	void TouchObject::touchesMoved( TouchList *touches )
	{
		// test for existing touch and remove from both lists
		list<TouchPoint>::iterator mTouchPointIt;
		list<TouchEvent::Touch>::iterator touchIt;
		list<TouchPoint> changedTouchPoints;
		
		mTouchPointIt = mTouchPoints.begin();
		while( mTouchPointIt != mTouchPoints.end() ) {
			touchIt = touches->begin();
			while( touchIt != touches->end() ) {
				if( touchIt->getId() == mTouchPointIt->getId() ) {
					mTouchPointIt->setPrevPos( touchIt->getPrevPos() );
					mTouchPointIt->setPos( touchIt->getPos() );
					mTouchPointIt->setPrevTime( mTouchPointIt->getTime() );
					mTouchPointIt->setTime( touchIt->getTime() );
					changedTouchPoints.push_back( *mTouchPointIt );
					if( mCapturingTouches ) touchIt = touches->erase( touchIt );
					else ++touchIt;
				} else {
					++touchIt;
				}
			}
			++mTouchPointIt;
		}
		
		if ( changedTouchPoints.size() > 0 ) {
			bool changed = mTouchPivot.touchPointsMoved( &mTouchPoints, &changedTouchPoints );
			if ( changed ) {
                // TODO: If capture mode is DRAG, remove any newly captured touches from TouchList and return a list of cancelled touches
				touchPointsMoved( &mTouchPoints, &changedTouchPoints, &mTouchPivot );
				if ( mTouchPivot.isDragging() ) pivotMoved( &mTouchPivot );
			}
		}
	}

	void TouchObject::touchesEnded( TouchList *touches )
	{
		// test for existing touch and remove from both lists
		list<TouchPoint>::iterator touchPointIt;
		list<TouchEvent::Touch>::iterator touchIt;
		list<TouchPoint> changedTouchPoints;
		
		touchPointIt = mTouchPoints.begin();
		while( touchPointIt != mTouchPoints.end() ) {
			
			touchIt = touches->begin();
			while( touchIt != touches->end() ) {
				
				if( touchPointIt != mTouchPoints.end() && touchIt->getId() == touchPointIt->getId() ) {
					
					touchPointIt->setPrevPos( touchIt->getPrevPos() );
					touchPointIt->setPos( touchIt->getPos() );
					touchPointIt->setPrevTime( touchPointIt->getTime() );
					touchPointIt->setTime( touchIt->getTime() );
					
					changedTouchPoints.push_back( *touchPointIt );
					touchPointIt = mTouchPoints.erase( touchPointIt );
					if ( ! mTouchPoints.empty() ) --touchPointIt;
					if( mCapturingTouches ) touchIt = touches->erase( touchIt );
					else ++touchIt;

				} else {
					++touchIt;
				}
			}

			if ( ! mTouchPoints.empty() ) ++touchPointIt;
		}
		
		if ( changedTouchPoints.size() > 0 ) {
			mTouchPivot.touchPointsEnded( &mTouchPoints, &changedTouchPoints );
			touchPointsEnded( &mTouchPoints, &changedTouchPoints, &mTouchPivot );
			if ( mTouchPoints.size() == 0 ) pivotEnded( &mTouchPivot );
			else pivotReset( &mTouchPivot );
		}
	}

	void TouchObject::touchesCancelled( TouchList *touches )
	{
		// test for existing touch and remove from both lists
		list<TouchPoint>::iterator touchPointIt;
		list<TouchEvent::Touch>::iterator touchIt;
		list<TouchPoint> changedTouchPoints;
		
		touchPointIt = mTouchPoints.begin();
		while( touchPointIt != mTouchPoints.end() ) {
			touchIt = touches->begin();
			while( touchIt != touches->end() ) {
				if( touchPointIt != mTouchPoints.end() && touchIt->getId() == touchPointIt->getId() ) {
                    
                    touchPointIt->setPrevPos( touchIt->getPrevPos() );
					touchPointIt->setPos( touchIt->getPos() );
					touchPointIt->setPrevTime( touchPointIt->getTime() );
					touchPointIt->setTime( touchIt->getTime() );
                    
					changedTouchPoints.push_back( *touchPointIt );
					touchPointIt = mTouchPoints.erase( touchPointIt );
					if ( ! mTouchPoints.empty() ) --touchPointIt;
					if( mCapturingTouches ) touchIt = touches->erase( touchIt );
					else ++touchIt;
				} else {
					++touchIt;
				}
			}
			if ( ! mTouchPoints.empty() ) ++touchPointIt;
		}
		
		if ( changedTouchPoints.size() > 0 ) {
			mTouchPivot.touchPointsCancelled( &mTouchPoints, &changedTouchPoints );
			touchPointsCancelled( &mTouchPoints, &changedTouchPoints, &mTouchPivot );
			pivotCancelled( &mTouchPivot );
		}
	}

	void TouchObject::touchesCancelled( list<TouchPoint> *touchPoints )
	{
		// test for existing touch and remove from both lists
		list<TouchPoint>::iterator mTouchPointIt;
		list<TouchPoint>::iterator touchPointIt;
		list<TouchPoint> changedTouchPoints;
		
		touchPointIt = mTouchPoints.begin();
		while( mTouchPointIt != mTouchPoints.end() ) {
			touchPointIt = touchPoints->begin();
			while( touchPointIt != touchPoints->end() ) {
				if( touchPointIt->getId() == mTouchPointIt->getId() ) {
					changedTouchPoints.push_back( *mTouchPointIt );
					mTouchPointIt = mTouchPoints.erase( mTouchPointIt );
					--mTouchPointIt;
					if( mCapturingTouches ) touchPointIt = touchPoints->erase( touchPointIt );
					else ++touchPointIt;
				} else {
					++touchPointIt;
				}
			}
			++mTouchPointIt;
		}
		
		if ( changedTouchPoints.size() > 0 ) {
			mTouchPivot.touchPointsCancelled( &mTouchPoints, &changedTouchPoints );
			touchPointsCancelled( &mTouchPoints, &changedTouchPoints, &mTouchPivot );
			pivotCancelled( &mTouchPivot );
		}
	}
	
	
}



