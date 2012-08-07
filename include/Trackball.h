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

#include "cinder/Quaternion.h"
#include "cinder/Vector.h"
#include <list>

#include "TouchObject.h"
#include "TouchPivot.h"

namespace Pivot {
	
	class Trackball : public TouchObject {
	  public:
		Trackball() {}
		Trackball( ci::Vec2f center, float radius, bool captureTouches = true );
		
		bool	hitTest( ci::app::TouchEvent::Touch touch );
		
		void	pivotBegan( TouchPivot *touchPivot );
		void	pivotMoved( TouchPivot *touchPivot );
		void	pivotReset( TouchPivot *touchPivot );
		void	pivotEnded( TouchPivot *touchPivot );
		void	pivotCancelled( TouchPivot *touchPivot );
		
		void	update( float deltaTime );
		
		ci::Quatf	getOrientation();
		ci::Vec2f	getCenter() { return mCenter; }
		float		getRadius() { return mRadius; }
		void		reset();
		
		
		// BEHAVIORAL PARAMS //////////////////////////////////////////////////
		
		//! Minimum Trackball radius
		void		setMinRadius( float radius ) { mMinRadius = radius; }
		float		getMinRadius() { return mMinRadius; }
		//! Maximum Trackball radius
		void		setMaxRadius( float radius ) { mMaxRadius = radius; }
		float		getMaxRadius() { return mMaxRadius; }
		
		//! Resistance when dragging outsize of allowed range (0 - 1).
		void		setPullResistance( float resistance ) { mPullResistance = resistance; }
		float		getPullResistance() { return mPullResistance; }
		//! Retraction when releasing outsize of allowed range (0 - 1).
		void		setReleaseRetraction( float retraction ) { mReleaseRetraction = retraction; }
		float		getReleaseRetraction() { return mReleaseRetraction; }
		
		
	  protected:
		class Arcball {
		  public:
			Arcball() {}
			
			void setOrigin( const ci::Vec2f &pos, ci::Quatf currentQuat, ci::Vec2f center, float radius )
			{
				mInitialPosQuat = mCurrentQuat = currentQuat;
				mToAxis = mFromAxis = pointOnSphere( pos, center, radius );
			}
			
			void move( const ci::Vec2f &pos, ci::Vec2f center, float radius )
			{
				mToAxis = pointOnSphere( pos, center, radius );
				ci::Vec3f axis = mFromAxis.cross( mToAxis );
				mCurrentQuat = mInitialPosQuat * ci::Quatf( mFromAxis.dot( mToAxis ), axis.x, axis.y, axis.z );
				mCurrentQuat.normalize();
			}
			
			void multQuat( ci::Quatf velQuat ) { 
				mCurrentQuat = mCurrentQuat * velQuat; 
				mCurrentQuat.normalize();
			}
			
			void		resetQuat() { mCurrentQuat = mInitialPosQuat = ci::Quatf( ci::Vec3f::yAxis(), 0 ); }
			
			ci::Quatf	getQuat() const { return mCurrentQuat; }
			
			ci::Vec3f	getToAxis() const { return mToAxis; }
			ci::Vec3f	getFromAxis() const { return mFromAxis; }
			
			static ci::Vec3f pointOnSphere( const ci::Vec2f &point, ci::Vec2f center, float radius ) {
				ci::Vec3f result;
				result.x = ( point.x - center.x ) / ( radius * 2 );
				result.y = ( point.y - center.y ) / ( radius * 2 );
				result.z = 0.0f;
				
				float mag = result.lengthSquared();
				if( mag > 1.0f ) {
					result.normalize();
				} else {
					result.z = ci::math<float>::sqrt( 1.0f - mag );
					result.normalize();
				}
				
				return result;
			}
			
		  private:
			ci::Vec3f		mFromAxis, mToAxis;
			ci::Quatf		mCurrentQuat, mInitialPosQuat;
		};
		
		float					mRot, mRotVel;
		float					mScale, mScaleVel;
		float					mRadius, mOriginRadius, mPivotResetRadius, mRadiusVel;
		
		ci::Vec2f				mCenter;
		
		ci::Quatf				mRotQuat, mOffsetQuat, mPrevOrientation;
		Trackball::Arcball		mBaseArcball;
		
		ci::Vec3f				mBaseAxisDirection, mPrevBaseAxis, mOrientationVel;
		
		float					mMinRadius, mMaxRadius, mPullResistance, mReleaseRetraction;
	};
	
}
