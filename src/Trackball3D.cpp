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
#include "cinder/Camera.h"
#include "cinder/Ray.h"

#include "Trackball3D.h"

namespace Pivot {
	
	using namespace ci;
	using namespace ci::app;
	using namespace std;
	
	Trackball3D::Trackball3D( Vec3f center, float radius, CameraPersp cam, bool captureTouches )
	: TouchObject3D( cam, captureTouches )
	{
		mSphere = Sphere( center, radius );
		mVelDecay = 0.99f;
		mOriginRadius = mPivotResetRadius = mSphere.getRadius();
		mOffsetQuat = Quatf( 0.0f, 0.0f, 0.0f, 1.0f );
		mRotQuat = Quatf( 0.0f, 0.0f, 0.0f, 1.0f );
		mPrevBaseAxis = Vec3f::zAxis();
		reset();
		mBaseArcball.setOrigin( Vec2f( mSphere.getCenter().x, mSphere.getCenter().y ), getOrientation(), mSphere, mCam, getWindowWidth(), getWindowHeight() ); // TODO: fix this ...................................
		mOrientationVel = Vec3f::zero();
		
		mMinRadius = 1.0f;
		mMaxRadius = 10000.0f;
		mPullResistance = 0.6f;
		mReleaseRetraction = 0.2f;
	}
	
	
	bool Trackball3D::hitTest( TouchEvent::Touch touch )
	{
		float u = touch.getPos().x / (float) getWindowWidth();
		float v = touch.getPos().y / (float) getWindowHeight();
		Ray ray = mCam.generateRay(u , 1.0f - v, mCam.getAspectRatio() );
		return mSphere.intersects( ray );
	}
	
	
	void Trackball3D::pivotBegan( TouchPivot *touchPivot )
	{
		mPivotResetRadius = mSphere.getRadius();
		mBaseArcball.setOrigin( touchPivot->getPos(), getOrientation(), mSphere, mCam, getWindowWidth(), getWindowHeight() );
		mRot = touchPivot->getRot();
		mRotVel = 0.0f;
		mPrevBaseAxis = mBaseArcball.getToAxis();
		mPrevOrientation = mBaseArcball.getQuat();
		mBaseAxisDirection = mOrientationVel = Vec3f::zero();
		mIsInMotion = false;
	}
	
	void Trackball3D::pivotMoved( TouchPivot *touchPivot )
	{
		// casting example
		//AdvancedPivot pivot = &(*touchPivot);
		
		// TODO: centerOffset
		if ( touchPivot->numTouchPoints() > 1 ) {
			mRot = touchPivot->getRot();
			float maxRadius = mCam.getEyePoint().distance( mSphere.getCenter() ) - mCam.getNearClip();
			float radDistRatio = math<float>::clamp( mSphere.getRadius() / maxRadius, 0.0f, 1.0f );
			float scaleOffset = ( 1.0f - touchPivot->getScale() ) * radDistRatio;
            
			//scaleOffset = math<float>::clamp( scaleOffset, -1.0f, 1.0f );
			float newRadius = math<float>::clamp( mPivotResetRadius * ( touchPivot->getScale() + scaleOffset ), 0.0f, maxRadius );
			
            //console() << radDistRatio << "       " << touchPivot->getScale() << "   " << scaleOffset << "    " << (touchPivot->getScale() + scaleOffset) << endl;
            
            newRadius = limit( newRadius, mPullResistance, mMinRadius, mMaxRadius );
			mSphere.setRadius( newRadius ); //////////////////////////////////////////////////////////////////////////////////////////////////////////////
		}

		mBaseArcball.move( touchPivot->getPos(), mSphere, mCam, getWindowWidth(), getWindowHeight() );
		
		if ( mBaseArcball.getToAxis() != mPrevBaseAxis )
			mBaseAxisDirection = mBaseArcball.getToAxis() - mPrevBaseAxis;
		mPrevBaseAxis = mBaseArcball.getToAxis();

		mIsInMotion = true;
	}
	
	void Trackball3D::pivotReset( TouchPivot *touchPivot )
	{
		mPivotResetRadius = mSphere.getRadius();
		mBaseArcball.setOrigin( touchPivot->getPos(), getOrientation(), mSphere, mCam, getWindowWidth(), getWindowHeight() );
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
	
	void Trackball3D::pivotEnded( TouchPivot *touchPivot )
	{
		mPivotResetRadius = mSphere.getRadius();
		mBaseArcball.setOrigin( touchPivot->getPos(), getOrientation(), mSphere, mCam, getWindowWidth(), getWindowHeight() );
		mRot = touchPivot->getRot();

		if ( mBaseArcball.getToAxis() != mPrevBaseAxis )
			mBaseAxisDirection = mBaseArcball.getToAxis() - mPrevBaseAxis;
		mPrevBaseAxis = mBaseArcball.getToAxis();
		
		// set position/rotation/scale velocities
		if ( touchPivot->getReleaseRotVel() != 0.0f )
			mRotVel = touchPivot->getReleaseRotVel();
		
		if ( touchPivot->getReleaseScaleVel() != 0.0f ) {
			// TODO: mRadiusVel
		}
		
		float thetaVel = touchPivot->getReleasePosVel().length() / mSphere.getRadius();
		
		if ( thetaVel == 0.0f || mBaseAxisDirection == Vec3f::zero() ) {
			mOrientationVel = Vec3f::zero();
		} else {
			// adjust thetaVel based on cam distance
			float distToCam = mCam.getEyePoint().distance( mSphere.getCenter() );
			float radiusCamDistRatio = 1.0f - math<float>::min( 1.0f, mSphere.getRadius() / distToCam );
			thetaVel *= radiusCamDistRatio;
			// adjust thetaVel based on perspective
			float eyeY 		= getWindowHeight() / 2.0f;
			float halfFov 	= ( 3.14159f * mCam.getFov() ) / 360.0f;
			float theTan 	= math<float>::tan( halfFov );
			float focalDist = eyeY / theTan;
			float camDepth	= -mCam.worldToEyeDepth( mSphere.getCenter() );
			float ratio = camDepth / focalDist;
			thetaVel *= ratio;
			
			// get rotational direction
			Quatf diffQuat = Quatf( mPrevBaseAxis, mPrevBaseAxis + mBaseAxisDirection );
			mOrientationVel = Vec3f( diffQuat.getPitch(), diffQuat.getYaw(), diffQuat.getRoll() );
			mOrientationVel.normalize();
			// multiply by rotational speed
			mOrientationVel *= thetaVel;
		}
		
		mIsInMotion = true;
	}
	
	void Trackball3D::pivotCancelled( TouchPivot *touchPivot )
	{
		mPivotResetRadius = mSphere.getRadius();
		mBaseArcball.setOrigin( touchPivot->getPos(), getOrientation(), mSphere, mCam, getWindowWidth(), getWindowHeight() );
		mPrevBaseAxis = mBaseArcball.getToAxis();
		mRot = touchPivot->getRot();
		mRotVel = 0.0f;
		mBaseAxisDirection = mOrientationVel = Vec3f::zero();
		mIsInMotion = false;
	}
	
	
	// Dear community, I'm not very good with vector and quaternion math, can this be optimized?
	Quatf Trackball3D::getOrientation() { 
		// normalize to z axis
		mOffsetQuat.set( mBaseArcball.getToAxis(), -Vec3f::zAxis() );
		Quatf normalizedQuat = mBaseArcball.getQuat() * (mOffsetQuat * mOffsetQuat);
		// rotate around z axis
		mRotQuat.set( -Vec3f::zAxis(), mRot );
		normalizedQuat = normalizedQuat * mRotQuat;
		// restore orientation
		mOffsetQuat.set( -Vec3f::zAxis(), mBaseArcball.getToAxis() );
		normalizedQuat = normalizedQuat * (mOffsetQuat * mOffsetQuat);
		return normalizedQuat.normalized();
	}
	
	void Trackball3D::reset()
	{
		mPivotResetRadius = mSphere.getRadius();
		mRot = mRotVel = 0.0f;
		mScale = 1.0f;
		mScaleVel = 0.0f;
		mRadiusVel = 0.0f;
		mBaseArcball.resetQuat();
		mPrevOrientation = mBaseArcball.getQuat();
		mOrientationVel = Vec3f::zero();
	}
	
	
	void Trackball3D::update( float deltaTime )
	{
		if ( mIsInMotion && mTouchPivot.numTouchPoints() < 2 ) {
			
			// velocity decay
			float velDecay = math<float>::clamp( mVelDecay - ( mVelDecay * deltaTime ), 0.0f, 1.0f );
			mOrientationVel *= velDecay;
			mRotVel *= velDecay;
			mScaleVel *= velDecay;
			
			// scale
			//mScale += mScaleVel * elapsedTime;
			//mSphere.setRadius( mOriginRadius * mScale );
			
			// limit scale/radius
			float prevRadius = mSphere.getRadius();
			mSphere.setRadius( limit( prevRadius, mReleaseRetraction, mMinRadius, mMaxRadius, deltaTime ) );
			
			// "position"
			if ( mTouchPivot.numTouchPoints() < 1 )
				mBaseArcball.multQuat( Quatf( mOrientationVel.x * deltaTime, mOrientationVel.y * deltaTime, mOrientationVel.z * deltaTime ) );
			
			// rotation
			mRot += mRotVel * deltaTime;
			
			// check if motion noticable
			if ( mOrientationVel.length() < 0.00001f && math<float>::abs( mRotVel ) < 0.00001f && math<float>::abs( mScaleVel ) < 0.0000001
				 && math<float>::abs( mSphere.getRadius() - prevRadius ) < 0.001f ) {
				mOrientationVel = Vec3f::zero();
				mRotVel = 0.0f;
				mScaleVel = 0.0f;
				mIsInMotion = false;
			}
		}
	}
	
}


