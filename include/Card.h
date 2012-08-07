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

#include "cinder/Vector.h"
#include <list>

#include "TouchObject.h"

namespace Pivot {
	
	class Card : public TouchObject {
	  public:
		Card() {}
		Card( ci::Vec2f pos, float width, float height, float rotation, bool captureTouches = true );
		
		bool	hitTest( ci::app::TouchEvent::Touch touch );
		
		void	pivotBegan( TouchPivot *touchPivot );
		void	pivotMoved( TouchPivot *touchPivot );
		void	pivotReset( TouchPivot *touchPivot );
		void	pivotEnded( TouchPivot *touchPivot );
		void	pivotCancelled( TouchPivot *touchPivot );
		
		void	update( float deltaTime );
		
		void		setPos( ci::Vec2f pos ) { mPos = pos; }
		ci::Vec2f	getPos() { return mPos; }
		
		void		setRot( float rotation ) { mRot = rotation; }
		float		getRot() { return mRot; }
		
		void		setScale( float scale ) { mScale = scale; }
		float		getScale() { return mScale; }
		
		void		setWidth( float width ) { mWidth = width; }
		float		getWidth() { return mWidth; }
		
		void		setHeight( float height ) { mHeight = height; }
		float		getHeight() { return mHeight; }
		
		// BEHAVIORAL PARAMS //////////////////////////////////////////////////
		
		//! Minimum Trackball radius
		void		setMinScale( float scale ) { mMinScale = scale; }
		float		getMinScale() { return mMinScale; }
		//! Maximum Trackball radius
		void		setMaxScale( float scale ) { mMaxScale = scale; }
		float		getMaxScale() { return mMaxScale; }
		
		//! Resistance when dragging outsize of allowed range (0 - 1).
		void		setPullResistance( float resistance ) { mPullResistance = resistance; }
		float		getPullResistance() { return mPullResistance; }
		//! Retraction when releasing outsize of allowed range (0 - 1).
		void		setReleaseRetraction( float retraction ) { mReleaseRetraction = retraction; }
		float		getReleaseRetraction() { return mReleaseRetraction; }
		
	  protected:
		float		mWidth, mHeight, mOriginWidth, mOriginHeight, mPivotResetWidth, mPivotResetHeight;
		
		ci::Vec2f	mPos, mPivotOffset, mPivotPos, mPivotPosVel;
		float		mRot, mPivotResetRotation, mRotVel;
		float		mScale, mScaleVel;
		
		float		mMinScale, mMaxScale, mPullResistance, mReleaseRetraction;
	};
	
}

