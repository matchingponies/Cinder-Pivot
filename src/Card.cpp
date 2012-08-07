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

#include "Card.h"

namespace Pivot {
	
	using namespace ci;
	using namespace ci::app;
	using namespace std;
	
	Card::Card( Vec2f pos, float width, float height, float rotation, bool captureTouches )
	: mPos( pos ), mWidth( width ), mHeight( height ), mRot( rotation ), TouchObject( captureTouches )
	{
		mPivotPos = mPos;
		mPivotOffset = Vec2f::zero();
		mOriginWidth = mPivotResetWidth = mWidth;
		mOriginHeight = mPivotResetHeight = mHeight;
		mPivotResetRotation = mRot;
		mScale = 1.0f;
		
		mPivotPosVel = Vec2f::zero();
		mRotVel = 0.0f;
		mScaleVel = 0.0f;
		
		mVelDecay = 0.97f;
		
		mMinScale = 0.1f;
		mMaxScale = 10000.0f;
		mPullResistance = 0.6f;
		mReleaseRetraction = 0.2f;
	}


	bool Card::hitTest( TouchEvent::Touch touch )
	{
		Vec2f v1 = Vec2f( mWidth*cosf( mRot ), mWidth*sinf( mRot ) );
		Vec2f v2 = Vec2f( -mHeight*sinf( mRot ), mHeight*cosf( mRot ) );
		Vec2f v = touch.getPos() - mPos;
		float vv1Dot = v.dot( v1 );
		float vv2Dot = v.dot( v2 );
		return vv1Dot >= 0.0f && vv1Dot <= v1.dot( v1 ) && vv2Dot >= 0.0f && vv2Dot <= v2.dot( v2 );
	}


	void Card::pivotBegan( TouchPivot *touchPivot )
	{
		mPivotResetRotation = mRot;
		mPivotResetWidth = mWidth;
		mPivotResetHeight = mHeight;
		mPivotPos = touchPivot->getPos();
		mPivotOffset = (mPivotPos - mPos) / mScale;
		mPivotOffset = Vec2f( mPivotOffset.x*cosf( -mRot ) - mPivotOffset.y*sinf( -mRot ), mPivotOffset.x*sinf( -mRot ) + mPivotOffset.y*cosf( -mRot ) );
		mPivotPosVel = Vec2f::zero();
		mRotVel = 0.0f;
		mScaleVel = 0.0f;
		mIsInMotion = false;
	}

	void Card::pivotMoved( TouchPivot *touchPivot )
	{
		if ( touchPivot->numTouchPoints() > 1 ) {
			mRot = mPivotResetRotation + touchPivot->getRot();
			mWidth = mPivotResetWidth * touchPivot->getScale();
			mHeight = mPivotResetHeight * touchPivot->getScale();
			mScale = mWidth / mOriginWidth;
			mScale = limit( mScale, mPullResistance, mMinScale, mMaxScale );
			mWidth = mOriginWidth * mScale;
			mHeight = mOriginHeight * mScale;
		}
		mPivotPos = touchPivot->getPos();
		Vec2f v = mPivotOffset * mScale;
		v = Vec2f( v.x*cosf( mRot ) - v.y*sinf( mRot ), v.x*sinf( mRot ) + v.y*cosf( mRot ) );
		mPos = mPivotPos - v;
		mIsInMotion = true;
	}

	void Card::pivotReset( TouchPivot *touchPivot )
	{
		//console() << "pivotReset:   " << touchPivot->getReleasePosVel() << "   " << touchPivot->getReleaseRotVel() << "   " << touchPivot->getReleaseScaleVel() << endl;
		
		mPivotResetRotation = mRot;
		mPivotResetWidth = mWidth;
		mPivotResetHeight = mHeight;
		mPivotPos = touchPivot->getPos();
		mPivotOffset = (mPivotPos - mPos) / mScale;
		mPivotOffset = Vec2f( mPivotOffset.x*cosf( -mRot ) - mPivotOffset.y*sinf( -mRot ), mPivotOffset.x*sinf( -mRot ) + mPivotOffset.y*cosf( -mRot ) );
		
		if ( touchPivot->numTouchPoints() == 1 ) {
			// set rotation/scale vels/accels
			mRotVel = touchPivot->getReleaseRotVel();
			//mScaleVel = touchPivot->getReleaseScaleVel();
		} else {
			mRotVel = 0.0f;
			mScaleVel = 0.0f;
		}
		mIsInMotion = true;
	}

	void Card::pivotEnded( TouchPivot *touchPivot )
	{
		//console() << "pivotEnded:   " << touchPivot->getReleasePosVel().length() << "   " << touchPivot->getReleaseRotVel() << "   " << touchPivot->getReleaseScaleVel() << endl;
		
		mPivotResetRotation = mRot;
		mPivotResetWidth = mWidth;
		mPivotResetHeight = mHeight;
		mPivotPos = touchPivot->getPos();
		mPivotOffset = (mPivotPos - mPos) / mScale;
		mPivotOffset = Vec2f( mPivotOffset.x*cosf( -mRot ) - mPivotOffset.y*sinf( -mRot ), mPivotOffset.x*sinf( -mRot ) + mPivotOffset.y*cosf( -mRot ) );
		
		// set position/rotation/scale vels/accels
		if ( touchPivot->getReleaseRotVel() != 0.0f ) {
			mRotVel = touchPivot->getReleaseRotVel();
		}
		if ( touchPivot->getReleaseScaleVel() != 0.0f ) {
			//mScaleVel = touchPivot->getReleaseScaleVel();
		}
		mPivotPosVel = touchPivot->getReleasePosVel();
		mIsInMotion = true;
	}

	void Card::pivotCancelled( TouchPivot *touchPivot )
	{
		mPivotResetRotation = mRot;
		mPivotResetWidth = mWidth;
		mPivotResetHeight = mHeight;
		mPivotPos = touchPivot->getPos();
		mPivotOffset = (mPivotPos - mPos) / mScale;
		mPivotOffset = Vec2f( mPivotOffset.x*cosf( -mRot ) - mPivotOffset.y*sinf( -mRot ), mPivotOffset.x*sinf( -mRot ) + mPivotOffset.y*cosf( -mRot ) );
		mPivotPosVel = Vec2f::zero();
		mRotVel = 0.0f;
		mScaleVel = 0.0f;
		mIsInMotion = false;
	}



	void Card::update( float deltaTime )
	{
		if ( mIsInMotion && mTouchPivot.numTouchPoints() < 2 ) {
			
			// velocity decay
			float velDecay = math<float>::clamp( mVelDecay - ( mVelDecay * deltaTime ), 0.0f, 1.0f );
			mPivotPosVel *= velDecay;
			mRotVel *= velDecay;
			mScaleVel *= velDecay;
			
			// rotation
			mRot += mRotVel * deltaTime;
			
			// scale
			mScale += mScaleVel * deltaTime;
			mWidth = mOriginWidth * mScale;
			mHeight = mOriginHeight * mScale;
			
			// pivot position
			if ( mTouchPivot.numTouchPoints() < 1 )
				mPivotPos += mPivotPosVel * deltaTime;
			
			// set postion based on rotation/scale/pivotpos
			Vec2f v = mPivotOffset * mScale;
			v = Vec2f( v.x*cosf( mRot ) - v.y*sinf( mRot ), v.x*sinf( mRot ) + v.y*cosf( mRot ) );
			mPos = mPivotPos - v;
			
			// limit scale
			float prevScale = mScale;
			mScale = limit( mScale, mReleaseRetraction, mMinScale, mMaxScale, deltaTime );
			
			// check if motion noticable
			if ( mPivotPosVel.length() < 0.01f && math<float>::abs( mRotVel ) < 0.0001f && math<float>::abs( mScaleVel ) < 0.000001
				&& math<float>::abs( mScale - prevScale ) < 0.001f ) {
				mPivotPosVel = Vec2f::zero();
				mRotVel  = 0.0f;
				mScaleVel = 0.0f;
				mIsInMotion = false;
			}
		}
	}

}
