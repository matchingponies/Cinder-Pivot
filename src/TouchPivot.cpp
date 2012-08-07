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

#include "cinder/gl/gl.h"
#include "cinder/CinderMath.h"

#include "TouchPivot.h"

namespace Pivot {
	
	using namespace ci;
	using namespace ci::app;
	using namespace std;
	
	TouchPivot::TouchPivot()
	{
		mIsActive = mIsDragging = false;
		mNumTouchPoints = 0;
		
		// init values and velocities
		mPos = mResetPos = mReleasePosVel = Vec2f::zero();
		mRot = mResetRot = mReleaseRotVel = mReleaseScaleVel = 0.0f;
		mScale = mResetScale = 1.0f;
		
		// init time values
		mBeginTime = mTime = mPrevTime = 0.0;
		
		// these help calculate pivot rotation and scale
		mNode1 = mResetNode1 = mNode2 = mResetNode2 = Vec2f::zero();
		
		
		// BEHAVIORAL PRESETS //////////////////////////////////////////////////
		
		// init average buffers
		setBufferLengths( 5 );
		
		// drag params
		mStartDragThreshold = 6.0f; // pixels
		mStartDragThresholdAugment = 0.0f; // pixels
		mReleaseDragVelThreshold = 70.0f; // pixels per second
		mReleaseDragVelMax = 5000.0f; // pixels per second
		
		// rot params
		mStartRotThreshold = 0.8f; // radians
		mStartRotThresholdAugment = 0.0f; // radians
		mReleaseRotVelThreshold = 0.3f; // radians per second
		mReleaseRotVelMax = 22.0f; // radians per second
		
		// scale params //////////////////////////////////////////////////////////////// <----- might be removing these
		mStartScaleThreshold = 0.09; // ratio
		mStartScaleThresholdAugment = 0.0f; // ratio
		mReleaseScaleVelThreshold = 1.0f; // ratio per second
		mReleaseScaleVelMax = 2.0f; // ratio per second
		
		// spread params
		mStartSpreadThreshold = 8.0f; // pixels
		mStartSpreadThresholdAugment = 0.0f; // pixels
		mReleaseSpreadVelThreshold = 70.0f; // pixels per second
		mReleaseSpreadVelMax = 2000.0f; // pixels per second
		
		// delta time params (in seconds. probably won't ever need to tweak these)
		mReleaseDeltaTimeMax = 10.0;
		mReleaseDeltaTimeMin = 0.011;
		mReleaseDeltaTimeThreshold = 0.04;
	}
	
	
	void TouchPivot::touchPointsBegan( list<TouchPoint> *currentTouchPoints, list<TouchPoint> *addedTouchPoints )
	{
		mNumTouchPoints = currentTouchPoints->size();
		
		Vec2f compositePos;
		for (list<TouchPoint>::iterator it = currentTouchPoints->begin(); it != currentTouchPoints->end(); ++it) {
			if ( it == currentTouchPoints->begin() ) compositePos = it->getPos();
			compositePos = ( compositePos + it->getPos() ) / 2;
		}
		
		list<TouchPoint>::iterator touchPointsIt = currentTouchPoints->begin();
		if ( mNumTouchPoints < 2 ) {
			mNode1 = mResetNode1 = touchPointsIt->getPos();
			mNode2 = mResetNode2 = touchPointsIt->getPos();
		} else {
			mNode1 = mResetNode1 = touchPointsIt->getPos();
			mNode2 = mResetNode2 = (++touchPointsIt)->getPos();
		}
		
		// reset position/rotation/scale velocities
		mReleasePosVel = Vec2f::zero();
		mReleaseRotVel = 0.0f;
		mReleaseScaleVel = 0.0f;
		
		// clear circular buffers
		mPosAvgVel.clear();
		mRotAvgVel.clear();
		mScaleAvgVel.clear();
		
		// reset position/rotation/scale
		mResetPos = mPos = compositePos;
		mResetRot = mRot = 0.0f;
		mResetScale = mScale = 1.0f;
		
		// check if active
		if ( mIsActive ) {
			mPrevTime = mTime;
			mTime = getTime();
			mDeltaTimeAvg.push_back( mTime - mPrevTime );
		} else {
			mDeltaTimeAvg.clear();
			mBeginTime = getElapsedSeconds();
			mPrevTime = mTime = 0.0;
			mIsDragging = false;
			mIsActive = true;
		}
	}

	bool TouchPivot::touchPointsMoved( list<TouchPoint> *currentTouchPoints, list<TouchPoint> *changedTouchPoints )
	{
		mPrevTime = mTime;
		mTime = getTime();
		mDeltaTimeAvg.push_back( mTime - mPrevTime );
		
		mNumTouchPoints = currentTouchPoints->size();
		
		Vec2f p = mPos;
		float r	= mRot;
		float s = mScale;

		setPivotMoved( currentTouchPoints );

		bool changed;
		if ( p != mPos || r != mRot || s != mScale ) changed = true;
		else changed = false;
		
		if ( changed ) {
			// TODO: trigger isDragging only if any thresholds passed
			mIsDragging = true; // temp, should happen after one of the thresholds is passed
		}

		return changed;
	}
	
	
	void TouchPivot::setPivotMoved( list<TouchPoint> *touchPoints )
	{
		int num = touchPoints->size();
		
		Vec2f compositePos;
		for (list<TouchPoint>::iterator it = touchPoints->begin(); it != touchPoints->end(); ++it) {
			if ( it == touchPoints->begin() ) compositePos = it->getPos();
			compositePos = ( compositePos + it->getPos() ) / 2;
		}
		
		list<TouchPoint>::iterator touchPointsIt = touchPoints->begin();
		if ( num < 2 ) {
			mNode1 = touchPointsIt->getPos();
			mNode2 = touchPointsIt->getPos();
		} else {
			mNode1 = touchPointsIt->getPos();
			mNode2 = (++touchPointsIt)->getPos();
			
			// move rotation
			float prevRot = mRot;
			mRot = calcVecAngleDiff( mResetNode2 - mResetNode1, mNode2 - mNode1 );
			
			// move scale
			float prevScale = mScale;
			mScale = calcVecScaleDiff( mResetNode2 - mResetNode1, mNode2 - mNode1 );
			
			// calc rotation/scale velocities
			mRotAvgVel.push_back( mRot - prevRot );
			mScaleAvgVel.push_back( mScale - prevScale );
		}
		
		// move position
		Vec2f prevPos = mPos;
		mPos = compositePos;
		// calc position velocity
		mPosAvgVel.push_back( mPos - prevPos );
	}
	

	void TouchPivot::touchPointsEnded( list<TouchPoint> *currentTouchPoints, list<TouchPoint> *removedTouchPoints )
	{
		mPrevTime = mTime;
		mTime = getTime();
		double deltaTime = mTime - mPrevTime;
		mDeltaTimeAvg.push_back( deltaTime );
		mDeltaTimeAvg.push_back( deltaTime );
		deltaTime = math<double>::clamp( calcDeltaTimeAvg( mDeltaTimeAvg ), mReleaseDeltaTimeMin, mReleaseDeltaTimeMax );
		
		mNumTouchPoints = currentTouchPoints->size();
		
		// if no starting thesholds have been met, there is no release velocity
		if ( ! mIsDragging ) {
			mPosAvgVel.clear();
			mRotAvgVel.clear();
			mScaleAvgVel.clear();
			mDeltaTimeAvg.clear();
		}
		
		Vec2f compositePos;
		list<TouchPoint>::iterator touchPointsIt = currentTouchPoints->begin();
		list<TouchPoint>::iterator removedPointsIt = removedTouchPoints->begin();
		
		if ( mNumTouchPoints == 0 ) {
			
			// reset and clear pivot
			for (list<TouchPoint>::iterator it = removedTouchPoints->begin(); it != removedTouchPoints->end(); ++it) {
				if ( it == removedTouchPoints->begin() ) compositePos = it->getPos();
				compositePos = ( compositePos + it->getPos() ) / 2;
			}
			
			if ( removedTouchPoints->size() < 2 ) {
				mNode1 = mResetNode1 = removedPointsIt->getPos();
				mNode2 = mResetNode2 = removedPointsIt->getPos();
			} else {
				mNode1 = mResetNode1 = removedPointsIt->getPos();
				mNode2 = mResetNode2 = (++removedPointsIt)->getPos();
			}
			
			// set new release velocities
			mReleasePosVel = ( mPosAvgVel.empty() || deltaTime > mReleaseDeltaTimeThreshold ) ? Vec2f::zero() : calcPointAvg( mPosAvgVel ) / deltaTime;
			mReleaseRotVel = ( mRotAvgVel.empty() || deltaTime > mReleaseDeltaTimeThreshold ) ? 0.0f : calcNumAvg( mRotAvgVel ) / deltaTime;
			mReleaseScaleVel = ( mScaleAvgVel.empty() || deltaTime > mReleaseDeltaTimeThreshold ) ? 0.0f : calcNumAvg( mScaleAvgVel ) / deltaTime;
			
			// zero out position velocity if below threshold. Clamp if above max.
			float releasePosVelLen = mReleasePosVel.length();
			if ( releasePosVelLen < mReleaseDragVelThreshold ) {
				mReleasePosVel = Vec2f::zero();
			} else {
				releasePosVelLen = math<float>::clamp( releasePosVelLen, -mReleaseDragVelMax, mReleaseDragVelMax );
				mReleasePosVel.normalize();
				mReleasePosVel *= releasePosVelLen;
			}
			// zero out rotation velocity if below threshold. Clamp if above max.
			if ( math<float>::abs( mReleaseRotVel ) < mReleaseRotVelThreshold ) mReleaseRotVel = 0.0f;
			else mReleaseRotVel = math<float>::clamp( mReleaseRotVel, -mReleaseRotVelMax, mReleaseRotVelMax );
			// zero out scale velocity if below threshold. Clamp if above max.
			if ( math<float>::abs( mReleaseScaleVel ) < mReleaseScaleVelThreshold ) mReleaseScaleVel = 0.0f;
			else mReleaseScaleVel = math<float>::clamp( mReleaseScaleVel, -mReleaseScaleVelMax, mReleaseScaleVelMax );
			
			// reset position/rotation/scale
			mResetPos = mPos = compositePos;
			mResetRot = mRot = 0.0f;
			mResetScale = mScale = 1.0f;
			
			// clear velocity buffers
			mPosAvgVel.clear();
			mRotAvgVel.clear();
			mScaleAvgVel.clear();
			mDeltaTimeAvg.clear();
			
			// set inactive
			mIsActive = false;
			mIsDragging = false;
			mBeginTime = mTime = mPrevTime = 0.0;
			
		} else {
			
			for (list<TouchPoint>::iterator it = currentTouchPoints->begin(); it != currentTouchPoints->end(); ++it) {
				if ( it == currentTouchPoints->begin() ) compositePos = it->getPos();
				compositePos = ( compositePos + it->getPos() ) / 2;
			}
			
			if ( mNumTouchPoints < 2 ) {
				mNode1 = mResetNode1 = touchPointsIt->getPos();
				mNode2 = mResetNode2 = touchPointsIt->getPos();
			} else {
				mNode1 = mResetNode1 = touchPointsIt->getPos();
				mNode2 = mResetNode2 = (++touchPointsIt)->getPos();
			}
			
			// set new release velocities
			mReleasePosVel = Vec2f::zero(); // because at least one touch is still holding down positon
			if ( mNumTouchPoints == 1 ) {
				// set new release velocities
				mReleaseRotVel = ( mRotAvgVel.empty() || deltaTime > mReleaseDeltaTimeThreshold ) ? 0.0f : calcNumAvg( mRotAvgVel ) / deltaTime;
				mReleaseScaleVel = ( mScaleAvgVel.empty() || deltaTime > mReleaseDeltaTimeThreshold ) ? 0.0f : calcNumAvg( mScaleAvgVel ) / deltaTime;
				// zero out rotation velocity if below threshold. Clamp if above max.
				if ( math<float>::abs( mReleaseRotVel ) < mReleaseRotVelThreshold ) mReleaseRotVel = 0.0f;
				else mReleaseRotVel = math<float>::clamp( mReleaseRotVel, -mReleaseRotVelMax, mReleaseRotVelMax );
				// zero out scale velocity if below threshold. Clamp if above max.
				if ( math<float>::abs( mReleaseScaleVel ) < mReleaseScaleVelThreshold ) mReleaseScaleVel = 0.0f;
				else mReleaseScaleVel = math<float>::clamp( mReleaseScaleVel, -mReleaseScaleVelMax, mReleaseScaleVelMax );
			} else {
				mReleaseRotVel = 0.0f;
				mReleaseScaleVel = 0.0f;
			}
			
			// reset position/rotation/scale
			mResetPos = mPos = compositePos;
			mResetRot = mRot = 0.0f;
			mResetScale = mScale = 1.0f;
			
			// clear velocity buffers
			mRotAvgVel.clear();
			mScaleAvgVel.clear();
		}
		
	}

	
	
	// TODO: remove this when things are in a better state
	void TouchPivot::draw()
	{
//		gl::drawStrokedRect( Rectf( -10, -10, 10, 10) + mNode1 );
//		gl::drawStrokedRect( Rectf( -10, -10, 10, 10) + mNode2 );
//		gl::drawLine( mNode1, mNode2 );
//		gl::drawLine( mResetNode1, mResetNode2 );
	}
	
	
	void TouchPivot::setBufferLengths( int len )
	{
		mPosAvgVel = boost::circular_buffer<Vec2f>( len );
		mRotAvgVel = boost::circular_buffer<float>( len );
		mScaleAvgVel = boost::circular_buffer<float>( len );
		mDeltaTimeAvg = boost::circular_buffer<double>( len );
	}
	

	float TouchPivot::calcVecAngleDiff( Vec2f a, Vec2f b )
	{
		float theta = atan2( a.x, a.y ) - atan2( b.x, b.y );
		if ( theta > 3.14f ) return theta - 6.28f;
		else if ( theta < -3.14f ) return 6.28f + theta;
		else return theta;
	}

	float TouchPivot::calcVecScaleDiff( Vec2f a, Vec2f b )
	{
		return b.length() / a.length();
	}
	
	
	float TouchPivot::calcNumAvg( boost::circular_buffer<float> numBuffer )
	{
		float total = 0.0f;
		int count = 0;
		for( boost::circular_buffer<float>::iterator it = numBuffer.begin(); it != numBuffer.end(); ++it ) {
			total += (*it);
			count++;
		}
		return total / float( count );
	}

	Vec2f TouchPivot::calcPointAvg( boost::circular_buffer<Vec2f> vecBuffer )
	{
		Vec2f total = Vec2f::zero();
		int count = 0;
		for( boost::circular_buffer<Vec2f>::iterator it = vecBuffer.begin(); it != vecBuffer.end(); ++it ) {
			total += (*it);
			count++;
		}
		return total / float( count );
	}
	
	double TouchPivot::calcDeltaTimeAvg( boost::circular_buffer<double> numBuffer )
	{
		float total = 0.0;
		int count = 0;
		for( boost::circular_buffer<double>::iterator it = numBuffer.begin(); it != numBuffer.end(); ++it ) {
			total += (*it);
			count++;
		}
		return total / double( count );
	}
	
}


