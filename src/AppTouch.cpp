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

#include "AppTouch.h"

namespace Pivot {
    
    using namespace ci;
	using namespace ci::app;
	using namespace std;
    
    
    
    AppTouch::AppTouch()
    : AppNative()
    {
        const Settings s = getSettings();
        console() << s.isMultiTouchEnabled() << endl;
        
        registerTouchesBegan( this, &AppTouch::preTouchesBegan );
        registerTouchesMoved( this, &AppTouch::preTouchesMoved );
        registerTouchesEnded( this, &AppTouch::preTouchesEnded );
        
        registerMouseDown( this, &AppTouch::preMouseDown );
        registerMouseDrag( this, &AppTouch::preMouseDrag );
        registerMouseUp( this, &AppTouch::preMouseUp );
    }
    
    
    
    bool AppTouch::preTouchesBegan ( TouchEvent event )
    {
        vector<TouchEvent::Touch> emptyTouchVector;
        cancelStaleTouches( emptyTouchVector );
        touchesBegan( event );
        return true;
    }
    
    bool AppTouch::preTouchesMoved ( TouchEvent event )
    {
        touchesMoved( event );
        return true;
    }
    
    bool AppTouch::preTouchesEnded ( TouchEvent event )
    {
        cancelStaleTouches( event.getTouches() );
        touchesEnded( event );
        return true;
    }
    
    
    
    bool AppTouch::preMouseDown( MouseEvent event )
    {
        mMouseDownTimeStamp = getElapsedSeconds();
        mPrevMousePos = event.getPos();
        touchesBegan( TouchEvent( vector<TouchEvent::Touch>( 1, TouchEvent::Touch( mPrevMousePos, mPrevMousePos, 0, 0.0, NULL ) ) ) );
        return false;
    }
    
    bool AppTouch::preMouseDrag( MouseEvent event )
    {
        touchesMoved( TouchEvent( vector<TouchEvent::Touch>( 1, TouchEvent::Touch( event.getPos(), mPrevMousePos, 0, mMouseDownTimeStamp - getElapsedSeconds(), NULL ) ) ) );
        mPrevMousePos = Vec2f( event.getPos() );
        return false;
    }
    
    bool AppTouch::preMouseUp( MouseEvent event )
    {
        touchesEnded( TouchEvent( vector<TouchEvent::Touch>( 1, TouchEvent::Touch( event.getPos(), mPrevMousePos, 0, mMouseDownTimeStamp - getElapsedSeconds(), NULL ) ) ) );
        return false;
    }
    
    
    
    
    void AppTouch::cancelStaleTouches( const vector<TouchEvent::Touch>& excludingTouches )
    {
        vector<TouchEvent::Touch> newActiveTouches = getActiveTouches();
        vector<TouchEvent::Touch> staleTouches = getStaleTouches( mCurrentActiveTouches, newActiveTouches, excludingTouches );
        mCurrentActiveTouches = newActiveTouches;
        if ( ! staleTouches.empty() ) touchesCancelled( TouchEvent( staleTouches ) );
    }
    
    
    vector<TouchEvent::Touch> AppTouch::getStaleTouches( const vector<TouchEvent::Touch>& currentActiveTouches, const vector<TouchEvent::Touch>& newActiveTouches, const vector<TouchEvent::Touch>& excludingTouches )
    {
        vector<TouchEvent::Touch> staleTouches;
        bool found;
        
        for( vector<TouchEvent::Touch>::const_iterator currentTouchIt = currentActiveTouches.begin(); currentTouchIt != currentActiveTouches.end(); ++currentTouchIt ) {
            found = false;
            
            for( vector<TouchEvent::Touch>::const_iterator newTouchIt = newActiveTouches.begin(); newTouchIt != newActiveTouches.end(); ++newTouchIt ) {
                if( newTouchIt->getId() == currentTouchIt->getId() ) {
                    found = true;
                    break;
                }
            }
            
            for( vector<TouchEvent::Touch>::const_iterator excludeTouchIt = excludingTouches.begin(); excludeTouchIt != excludingTouches.end(); ++excludeTouchIt ) {
                if( excludeTouchIt->getId() == currentTouchIt->getId() ) {
                    found = true;
                    break;
                }
            }
            
            if ( ! found ) staleTouches.push_back( *currentTouchIt );
        }
        return staleTouches;
    }
    
}




