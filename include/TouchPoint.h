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

namespace Pivot {
		
	class TouchPoint {
	  public:
		TouchPoint( const ci::Vec2f &pos, const ci::Vec2f &prevPos, const ci::Vec2f &originPos, uint32_t id, double time, const void *native )
		: mPos( pos ), mPrevPos( prevPos ), mId( id ), mTime( time ), mPrevTime( time ), mNative( native ) {}
		
		//! Returns the position of the touch measured in pixels
		ci::Vec2f	getPos() const { return mPos; }
		//! Sets the position of the touch measured in pixels
		void		setPos( const ci::Vec2f &pos ) { mPos = pos; }
		//! Returns the previous position of the touch measured in pixels
		ci::Vec2f	getPrevPos() const { return mPrevPos; }
		//! Sets the previous position of the touch measured in pixels
		void		setPrevPos( const ci::Vec2f &prevPos ) { mPrevPos = prevPos; }
		//! Returns the original position of the touch measured in pixels
		ci::Vec2f	getOriginPos() const { return mOriginPos; }
		//! Sets the original position of the touch measured in pixels
		void		setOriginPos( const ci::Vec2f &originPos ) { mOriginPos = originPos; }
		//! Returns the position velocity of the touch measured in pixels per second
		ci::Vec2f	getPosVel() const { return ( mPos - mPrevPos ) / ( mTime - mPrevTime ); }
		//! Returns the timestamp associated with the touch, measured in seconds
		double		getTime() const { return mTime; }
		//! Sets the timestamp associated with the touch, measured in seconds
		void		setTime( double time ) { mTime = time; }
		//! Returns the timestamp associated with the previous touch position, measured in seconds
		double		getPrevTime() const { return mPrevTime; }
		//! Sets the timestamp associated with the previous touch position, measured in seconds
		void		setPrevTime( double time ) { mPrevTime = time; }
		//! Returns an ID unique for the lifetime of the touch
		uint32_t	getId() const { return mId; }
		//! Returns a pointer to the OS-native object. This is a UITouch* on Cocoa Touch and a TOUCHPOINT* on MSW.
		const void*	getNative() const { return mNative; }
		
	  private:
		ci::Vec2f	mPos, mPrevPos, mOriginPos;
		uint32_t	mId;
		double		mTime, mPrevTime;
		const void	*mNative;
	};

}

