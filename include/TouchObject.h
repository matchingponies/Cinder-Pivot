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

#pragma once

#include "cinder/app/TouchEvent.h"
#include "cinder/Camera.h"
#include "cinder/Color.h"
#include <list>

#include "TouchPivot.h"
#include "TouchPoint.h"

namespace Pivot {
	
	typedef std::list<ci::app::TouchEvent::Touch> TouchList;
	TouchList toList( const std::vector<ci::app::TouchEvent::Touch> &touches );
	
	class TouchObject {
	  public:
		TouchObject();
		TouchObject( bool captureTouches );
		virtual ~TouchObject() {}
		
		//! Most TouchObjects will use this.
		virtual void update( float deltaTime = 0.01667f ) {}
		
		//! Evaluates if touches are within TouchObject's interactive range.
		virtual bool hitTest( ci::app::TouchEvent::Touch touch ) { return false; }
		
		//! Primary touch events -- these should hopefully cover 90% of any touch UI development
		virtual void pivotBegan( TouchPivot *touchPivot ) {}
		virtual void pivotMoved( TouchPivot *touchPivot ) {}
		virtual void pivotReset( TouchPivot *touchPivot ) {}
		virtual void pivotEnded( TouchPivot *touchPivot ) {}
		virtual void pivotCancelled( TouchPivot *touchPivot ) {}    
		
		//! Secondary touch events -- for when you need granular touch point information
		virtual void touchPointsBegan( std::list<TouchPoint> *allTouchPoints, std::list<TouchPoint> *changedTouchPoints, TouchPivot *touchPivot ) {}
		virtual void touchPointsMoved( std::list<TouchPoint> *allTouchPoints, std::list<TouchPoint> *changedTouchPoints, TouchPivot *touchPivot ) {}
		virtual void touchPointsEnded( std::list<TouchPoint> *allTouchPoints, std::list<TouchPoint> *changedTouchPoints, TouchPivot *touchPivot ) {}
		virtual void touchPointsCancelled( std::list<TouchPoint> *allTouchPoints, std::list<TouchPoint> *changedTouchPoints, TouchPivot *touchPivot ) {}
		
		//! TouchEvents must be passed to TouchObject for it to be aware of incoming touches
		void	touchesBegan( TouchList *touches );
		void	touchesMoved( TouchList *touches );
		void	touchesEnded( TouchList *touches );
		void	touchesCancelled( TouchList *touches );
		void	touchesCancelled( std::list<TouchPoint>	*touchPoints );
		
		//! Allows the assignment of different types of TouchPivots (BasicPivot, RotationPivot, FullPivot, AdvancedPivot)
		void		setTouchPivot( TouchPivot touchPivot ) { mTouchPivot = touchPivot; }
		TouchPivot	getTouchPivot() const { return mTouchPivot; }
		
		//! Sets the capture mode of the TouchObject.  Captured touches are removed from the TouchEvent list.
        // TODO: capture mode should be these enums: ALWAYS, DRAG, NEVER
		void	enableCaptureMode( bool captureTouches = true ) { mCapturingTouches = captureTouches; }
		void	disableCaptureMode() { mCapturingTouches = false; }
		bool	getCaptureMode() { return mCapturingTouches; }
		
		//! Returns true if the TouchPivot is currently active
		bool	isActive() const { return mTouchPivot.isActive(); }
		//! Returns true if the TouchPivot has broken it's dragging threshold
		bool	isDragging() const { return mTouchPivot.isDragging(); }
		//! Returns true if the TouchPivot is in motion, from user interaction or momentum
		bool	isInMotion() { return mIsInMotion; }
		
		//! Returns all touches watched by this TouchObject
		std::list<TouchPoint>	getTouchPoints() const { return mTouchPoints; }
		
		//! Velocity decay is a diminishment factor per second.  Must be a value between 0 and 1, not inclusive.
		void			setVelDecay( float velDecay ) { mVelDecay = velDecay; }
		float			getVelDecay() { return mVelDecay; }
		
		//! Debug convenience functions
		void			setDebugColor( ci::Color debugColor ) { mDebugColor = debugColor; }
		ci::Color		getDebugColor() { return mDebugColor; }
		
	  protected:
		std::list<TouchPoint>	mTouchPoints;
		TouchPivot				mTouchPivot;
		
		bool					mCapturingTouches, mIsInMotion;
		ci::Color				mDebugColor;
		
		float					mVelDecay;
		
		static float limit( float value, float strength, float min, float max, float deltaTime = 0.0f )
		{
			if ( value > max ) {
				ci::math<float>::clamp( deltaTime, 0.0f, 1.0f );
				return value + (max - value) * strength * ( 1.0f - deltaTime );
			} else if ( value < min ) {
				ci::math<float>::clamp( deltaTime, 0.0f, 1.0f );
				return value + (min - value) * strength * ( 1.0f - deltaTime );
			} else {
				return value;
			}
		}
		
	};
	
	
	class TouchObject3D : public TouchObject {
	  public:
		TouchObject3D() {}
		TouchObject3D( ci::CameraPersp cam, bool captureTouches );
		
		void				setCamera( ci::CameraPersp cam ) { mCam = cam; }
		ci::CameraPersp		getCamera() { return mCam; }
		
	  protected:
		ci::CameraPersp				mCam;
	};
	
}
