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

#include "cinder/app/App.h"
#include <boost/circular_buffer.hpp>
#include <list>

#include "TouchPoint.h"

namespace Pivot {

	// TODO: make this a base class that more advanced TouchPivots can extend
	class TouchPivot {
	  public:
		TouchPivot();
		virtual	~TouchPivot() {}
		
		virtual void touchPointsBegan( std::list<TouchPoint> *currentTouchPoints, std::list<TouchPoint> *addedTouchPoints );
		virtual bool touchPointsMoved( std::list<TouchPoint> *currentTouchPoints, std::list<TouchPoint> *changedTouchPoints );
		virtual void touchPointsEnded( std::list<TouchPoint> *currentTouchPoints, std::list<TouchPoint> *removedTouchPoints );
		virtual void touchPointsCancelled( std::list<TouchPoint> *currentTouchPoints, std::list<TouchPoint> *cancelledTouchPoints ) { touchPointsEnded( currentTouchPoints, cancelledTouchPoints ); }
		
		// TODO: remove this when things are in a better state
		virtual void	draw();
		
		//! Returns number of TouchPoints currently being watched
		int				numTouchPoints() const { return mNumTouchPoints; }
		//! Returns true if the pivot is currently active
		bool			isActive() const { return mIsActive; }
		//! Returns true if the pivot has broken it's dragging threshold
		bool			isDragging() const { return mIsDragging; }
		
		//! Returns the timestamp associated with the pivot, measured in seconds from last activation
		double	getTime() const { 
			if ( ! isActive() ) return 0.0;
			return ci::app::getElapsedSeconds() - mBeginTime;
		}
		//! Returns the timestamp associated with the previous pivot position, measured in seconds from last activation
		double			getPrevTime() const { return mPrevTime; }
		
		
		// EXTENDED GETTERS ///////////////////////////////////////////////////
		
		//! Returns the position of the pivot measured in pixels
		ci::Vec2f		getPos() const { return mPos; }
		//! Returns	the position of last pivot reset, measured in pixels
		ci::Vec2f		getResetPos() const { return mResetPos; }
		//! Returns the release position velocity of the pivot measured in pixels per second
		ci::Vec2f		getReleasePosVel() const { return mReleasePosVel; }
		
		//! Returns the rotation of the pivot measured in radians
		float			getRot() const { return mRot; }
		//! Returns the scale of the last pivot reset, measured in radians
		float			getResetRot() const { return mResetRot; }
		//! Returns the release rotation velocity of the pivot measured in radians per second
		float			getReleaseRotVel() const { return mReleaseRotVel; }
		
		//! Returns the scale of the pivot measured as ratio
		float			getScale() const { return mScale; }
		//! Returns the scale of the last pivot reset, measured as ratio
		float			getResetScale() const { return mResetScale; }
		//! Returns the release scale velocity of the pivot measured as ratio per second
		float			getReleaseScaleVel() const { return mReleaseScaleVel; }
		
//		//! Returns the spread of the pivot measured as ratio
//		float			getSpread() const { return mSpread; }
//		//! Returns the spread of the last pivot reset, measured as ratio
//		float			getResetSpread() const { return mResetSpread; }
//		//! Returns the release spread velocity of the pivot measured as ratio per second. Spread.
//		float			getReleaseSpreadVel() const { return mReleaseSpreadVel; }
		
		
		// BEHAVIORAL PARAMS //////////////////////////////////////////////////
		
		// DRAG
		//! Distance deviation needed to favor dragging movement.  If exceded, also triggers isDragging.
		void			setStartDragThreshold( float distance ) { mStartDragThreshold = distance; }
		float			getStartDragThreshold() { return mStartDragThreshold; }
		//! Maximum increase of drag threshold while in motion.
		void			setStartDragThresholdAugment( float distance ) { mStartDragThresholdAugment = distance; }
		float			getStartDragThresholdAugment() { return mStartDragThresholdAugment; }
		//! Minimum value needed for release drag velocity.
		void			setReleaseDragVelThreshold( float velocity ) { mReleaseDragVelThreshold = velocity; }
		float			getReleaseDragVelThreshold() { return mReleaseDragVelThreshold; }
		//! Maximum value for release drag velocity.
		void			setReleaseDragVelMax( float velocity ) { mReleaseDragVelMax = velocity; }
		float			getReleaseDragVelMax() { return mReleaseDragVelMax; }
		
		// ROTATION
		//! Rotate deviation needed to favor rotational movement.  If exceded, also triggers isDragging.
		void			setStartRotThreshold( float radians ) { mStartRotThreshold = radians; }
		float			getStartRotThreshold() { return mStartRotThreshold; }
		//! Maximum increase of rotation threshold while in motion.
		void			setStartRotThresholdAugment( float radians ) { mStartRotThresholdAugment = radians; }
		float			getStartRotThresholdAugment() { return mStartRotThresholdAugment; }
		//! Minimum value needed for release rotational velocity.
		void			setReleaseRotVelThreshold( float velocity ) { mReleaseRotVelThreshold = velocity; }
		float			getReleaseRotVelThreshold() { return mReleaseRotVelThreshold; }
		//! Maximum value for release rotational velocity.
		void			setReleaseRotVelMax( float velocity ) { mReleaseRotVelMax = velocity; }
		float			getReleaseRotVelMax() { return mReleaseRotVelMax; }
		
		// SPREAD (removing scale, can be inferred through spread these)
		//! Scale deviation needed to favor scaling movement.  If exceded, also triggers isDragging.
		void			setStartScaleThreshold( float ratio ) { mStartScaleThreshold = ratio; }
		float			getStartScaleThreshold() { return mStartScaleThreshold; }
		//! Maximum increase of scale threshold while in motion.
		void			setStartScaleThresholdAugment( float ratio ) { mStartScaleThresholdAugment = ratio; }
		float			getStartScaleThresholdAugment() { return mStartScaleThresholdAugment; }
		//! Minimum value needed for release scale velocity.
		void			setReleaseScaleVelThreshold( float velocity ) { mReleaseScaleVelThreshold = velocity; }
		float			getReleaseScaleVelThreshold() { return mReleaseScaleVelThreshold; }
		//! Maximum value for release scale velocity.
		void			setReleaseScaleVelMax( float velocity ) { mReleaseScaleVelMax = velocity; }
		float			getReleaseScaleVelMax() { return mReleaseScaleVelMax; }
		
		// SPREAD
		//! Spread deviation needed to favor scaling movement.  If exceded, also triggers isDragging.
		void			setStartSpreadThreshold( float distance ) { mStartSpreadThreshold = distance; }
		float			getStartSpreadThreshold() { return mStartSpreadThreshold; }
		//! Maximum increase of spread threshold while in motion.
		void			setStartSpreadThresholdAugment( float distance ) { mStartSpreadThresholdAugment = distance; }
		float			getStartSpreadThresholdAugment() { return mStartSpreadThresholdAugment; }
		//! Minimum value needed for release spread velocity.
		void			setReleaseSpreadVelThreshold( float velocity ) { mReleaseSpreadVelThreshold = velocity; }
		float			getReleaseSpreadVelThreshold() { return mReleaseSpreadVelThreshold; }
		//! Maximum value for release spread velocity.
		void			setReleaseSpreadVelMax( float velocity ) { mReleaseSpreadVelMax = velocity; }
		float			getReleaseSpreadVelMax() { return mReleaseSpreadVelMax; }
		
		
		
	  protected:
		void								setBufferLengths( int len );
		boost::circular_buffer<ci::Vec2f>	mPosAvgVel;
		boost::circular_buffer<float>		mRotAvgVel, mScaleAvgVel;
		boost::circular_buffer<double>		mDeltaTimeAvg;
		
		ci::Vec2f		mPos, mResetPos, mReleasePosVel;
		float			mRot, mResetRot, mReleaseRotVel;
		float			mScale, mResetScale, mReleaseScaleVel;
		float			mSpread, mResetSpread, mReleaseSpreadVel;
		
		double			mBeginTime, mTime, mPrevTime;
		
		int				mNumTouchPoints;
		bool			mIsActive, mIsDragging;
		
		float			mStartDragThreshold, mStartDragThresholdAugment, mReleaseDragVelThreshold, mReleaseDragVelMax;
		float			mStartRotThreshold, mStartScaleThresholdAugment, mReleaseRotVelThreshold, mReleaseRotVelMax;
		float			mStartScaleThreshold, mStartRotThresholdAugment, mReleaseScaleVelThreshold, mReleaseScaleVelMax;
		float			mStartSpreadThreshold, mStartSpreadThresholdAugment, mReleaseSpreadVelThreshold, mReleaseSpreadVelMax;
		double			mReleaseDeltaTimeThreshold, mReleaseDeltaTimeMax, mReleaseDeltaTimeMin;
		
		void			setPivotMoved( std::list<TouchPoint> *touchPoints );
		float			calcVecAngleDiff( ci::Vec2f a, ci::Vec2f b );
		float			calcVecScaleDiff( ci::Vec2f a, ci::Vec2f b );
		float			calcNumAvg( boost::circular_buffer<float> numBuffer );
		ci::Vec2f		calcPointAvg( boost::circular_buffer<ci::Vec2f> vecBuffer );
		double			calcDeltaTimeAvg( boost::circular_buffer<double> numBuffer );
		
		ci::Vec2f		mNode1, mResetNode1, mNode2, mResetNode2;
	};

}
