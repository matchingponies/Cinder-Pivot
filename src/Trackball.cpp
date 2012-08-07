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
#include "cinder/gl/gl.h"

#include "Trackball.h"

namespace Pivot {
	
	using namespace ci;
	using namespace ci::app;
	using namespace std;
	
	Trackball::Trackball( Vec2f center, float radius, bool captureTouches )
	: mCenter( center ), mRadius( radius), TouchObject( captureTouches )
	{
		mVelDecay = 0.99f;
		mOriginRadius = mPivotResetRadius = mRadius;
		mOffsetQuat = Quatf( 0.0f, 0.0f, 0.0f, 1.0f );
		mRotQuat = Quatf( 0.0f, 0.0f, 0.0f, 1.0f );
		mPrevBaseAxis = Vec3f::zAxis();
		reset();
		mBaseArcball.setOrigin( mCenter, getOrientation(), mCenter, mRadius );
		mOrientationVel = Vec3f::zero();
		
		mMinRadius = 1.0f;
		mMaxRadius = 10000.0f;
		mPullResistance = 0.6f;
		mReleaseRetraction = 0.2f;
	}
	

	bool Trackball::hitTest( TouchEvent::Touch touch )
	{
		return bool( touch.getPos().distance( mCenter ) < mRadius );
	}


	void Trackball::pivotBegan( TouchPivot *touchPivot )
	{
		mPivotResetRadius = mRadius;
		mBaseArcball.setOrigin( touchPivot->getPos(), getOrientation(), mCenter, mRadius );
		mRot = touchPivot->getRot();
		mRotVel = 0.0f;
		mPrevBaseAxis = mBaseArcball.getToAxis();
		mPrevOrientation = mBaseArcball.getQuat();
		mOrientationVel = Vec3f::zero();
		mIsInMotion = false;
	}

	void Trackball::pivotMoved( TouchPivot *touchPivot )
	{
		if ( touchPivot->numTouchPoints() > 1 ) {
			mRot = touchPivot->getRot();
			mRadius = limit( mPivotResetRadius * touchPivot->getScale(), mPullResistance, mMinRadius, mMaxRadius );
		}
		
		mBaseArcball.move( touchPivot->getPos(), mCenter, mRadius );

		if ( mBaseArcball.getToAxis() != mPrevBaseAxis ) {
			mBaseAxisDirection = mBaseArcball.getToAxis() - mPrevBaseAxis;
			mPrevBaseAxis = mBaseArcball.getToAxis();
		}
		
		mIsInMotion = true;
	}

	void Trackball::pivotReset( TouchPivot *touchPivot )
	{
		mPivotResetRadius = mRadius;
		mBaseArcball.setOrigin( touchPivot->getPos(), getOrientation(), mCenter, mRadius );
		mPrevBaseAxis = mBaseArcball.getToAxis();
		mPrevOrientation = mBaseArcball.getQuat();
		mRot = touchPivot->getRot();
		
		if ( touchPivot->numTouchPoints() == 1 ) {
			// set rotation/scale velocities
			mRotVel = touchPivot->getReleaseRotVel();
			// TODO: mRadiusVel
		} else {
			mRotVel = 0.0f;
			// TODO: mRadiusVel
		}
		
		mIsInMotion = true;
	}

	void Trackball::pivotEnded( TouchPivot *touchPivot )
	{
		mPivotResetRadius = mRadius;
		mBaseArcball.setOrigin( touchPivot->getPos(), getOrientation(), mCenter, mRadius );
		mRot = touchPivot->getRot();

		if ( mBaseArcball.getToAxis() != mPrevBaseAxis ) {
			mBaseAxisDirection = mBaseArcball.getToAxis() - mPrevBaseAxis;
			mPrevBaseAxis = mBaseArcball.getToAxis();
		}
		
		// set position/rotation/scale velocities
		if ( touchPivot->getReleaseRotVel() != 0.0f )
			mRotVel = touchPivot->getReleaseRotVel();
		
		if ( touchPivot->getReleaseScaleVel() != 0.0f ) {
			// TODO: mRadiusVel
		}
		
		
		float thetaVel = touchPivot->getReleasePosVel().length() / mRadius;
		
		if ( thetaVel == 0.0f || mBaseAxisDirection == Vec3f::zero()  ) {
			mOrientationVel = Vec3f::zero();
		} else {
			// get rotational direction
			Quatf diffQuat = Quatf( mPrevBaseAxis, mPrevBaseAxis + mBaseAxisDirection );
			mOrientationVel = Vec3f( diffQuat.getPitch(), diffQuat.getYaw(), diffQuat.getRoll() );
			mOrientationVel.normalize();
			// multiply by rotational speed
			mOrientationVel *= thetaVel;
		}
		
		mIsInMotion = true;
	}

	void Trackball::pivotCancelled( TouchPivot *touchPivot )
	{
		mPivotResetRadius = mRadius;
		mBaseArcball.setOrigin( touchPivot->getPos(), getOrientation(), mCenter, mRadius );
		mPrevBaseAxis = mBaseArcball.getToAxis();
		mRot = touchPivot->getRot();
		mRotVel = 0.0f;
		mOrientationVel = Vec3f::zero();
		mIsInMotion = false;
	}


	// Dear community, I'm not very good with vector and quaternion math, can this be optimized?
	Quatf Trackball::getOrientation() { 
		// normalize to z axis
		mOffsetQuat.set( mBaseArcball.getToAxis(), Vec3f::zAxis() );
		Quatf normalizedQuat = mBaseArcball.getQuat() * (mOffsetQuat * mOffsetQuat);
		// rotate around z axis
		mRotQuat.set( Vec3f::zAxis(), mRot );
		normalizedQuat = normalizedQuat * mRotQuat;
		// restore orientation
		mOffsetQuat.set( Vec3f::zAxis(), mBaseArcball.getToAxis() );
		normalizedQuat = normalizedQuat * (mOffsetQuat * mOffsetQuat);
		return normalizedQuat.normalized();
	}

	void Trackball::reset()
	{
		mPivotResetRadius = mRadius;
		mRot = mRotVel = 0.0f;
		mScale = 1.0f;
		mScaleVel = 0.0f;
		mRadiusVel = 0.0f;
		mBaseArcball.resetQuat();
		mPrevOrientation = mBaseArcball.getQuat();
		mOrientationVel = Vec3f::zero();
	}


	void Trackball::update( float deltaTime )
	{
		if ( mIsInMotion && mTouchPivot.numTouchPoints() < 2 ) {
			
			// velocity decay
			float velDecay = math<float>::clamp( mVelDecay - ( mVelDecay * deltaTime ), 0.0f, 1.0f );
			mOrientationVel *= velDecay;
			mRotVel *= velDecay;
			mScaleVel *= velDecay;
			
			// scale
			//mScale += mScaleVel * elapsedTime;
			//mRadius = mOriginRadius * mScale;
			
			// limit scale/radius
			float prevRadius = mRadius;
			mRadius = limit( mRadius, mReleaseRetraction, mMinRadius, mMaxRadius, deltaTime );
			
			// "position"
			if ( mTouchPivot.numTouchPoints() < 1 )
				mBaseArcball.multQuat( Quatf( mOrientationVel.x * deltaTime, mOrientationVel.y * deltaTime, mOrientationVel.z * deltaTime ) );
			
			// rotation
			mRot += mRotVel * deltaTime;
			
			// check if motion noticable
			if ( mOrientationVel.length() < 0.00001f && math<float>::abs( mRotVel ) < 0.00001f && math<float>::abs( mScaleVel ) < 0.0000001
				&& math<float>::abs( mRadius - prevRadius ) < 0.001f ) {
				mOrientationVel = Vec3f::zero();
				mRotVel = 0.0f;
				mScaleVel = 0.0f;
				mIsInMotion = false;
			}
		}
	}
		
}


