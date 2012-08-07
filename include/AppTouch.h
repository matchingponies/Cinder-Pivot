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

#include <vector>

#include "cinder/app/AppNative.h"
#include "cinder/app/TouchEvent.h"
#include "cinder/app/MouseEvent.h"

namespace Pivot {

    class AppTouch : public ci::app::AppNative {
     public:
        AppTouch();
        virtual ~AppTouch() {}
        
        virtual void    touchesBegan( ci::app::TouchEvent event ) {}
        virtual void    touchesMoved( ci::app::TouchEvent event ) {}
        virtual void    touchesEnded( ci::app::TouchEvent event ) {}
        virtual void    touchesCancelled( ci::app::TouchEvent event ) {}
    
     private:
        // for MouseEvent-to-TouchEvent translation. Great if you only have a mouse :)
        bool            preMouseDown( ci::app::MouseEvent event );
        bool            preMouseDrag( ci::app::MouseEvent event );
        bool            preMouseUp( ci::app::MouseEvent event );
        double          mMouseDownTimeStamp;
        ci::Vec2f       mPrevMousePos;
        
        // for cancelling stale touches
        bool            preTouchesBegan ( ci::app::TouchEvent event );
        bool            preTouchesMoved ( ci::app::TouchEvent event );
        bool            preTouchesEnded ( ci::app::TouchEvent event );
        
        void                                        cancelStaleTouches( const std::vector<ci::app::TouchEvent::Touch>& excludingTouches );
        std::vector<ci::app::TouchEvent::Touch>     getStaleTouches( const std::vector<ci::app::TouchEvent::Touch>& currentActiveTouches, 
                                                                     const std::vector<ci::app::TouchEvent::Touch>& newActiveTouches, 
                                                                     const std::vector<ci::app::TouchEvent::Touch>& excludingTouches );
        std::vector<ci::app::TouchEvent::Touch>     mCurrentActiveTouches;
    };

}




