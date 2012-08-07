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

#include "cinder/gl/gl.h"

#include "Trackball3D.h"
#include "Trackball.h"
#include "Card.h"
#include "CatchAll.h"
#include "TouchPivot.h"
#include "TouchPoint.h"

namespace Pivot {
	
	typedef std::shared_ptr<class Renderer> RendererRef;

	class Renderer {
	public:
		
		~Renderer();
		
	private:
		static RendererRef		getInstance();
		Renderer();
		
		friend void				draw( Trackball3D &trackball );
		void					draw( Trackball3D &trackball );
		
		friend void				draw( Trackball &trackball );
		void					draw( Trackball &trackball );
		
		friend void				draw( Card &card );
		void					draw( Card &card );
		
		friend void				draw( CatchAll &catchAll );
		void					draw( CatchAll &catchAll );
		
		friend void				drawTouches( TouchObject &touchObject );
		void					drawTouches( TouchObject &touchObject );
		friend void				drawPivot( TouchObject &touchObject );
		void					drawPivot( TouchObject &touchObject );
	};
	
	void draw( Trackball3D &trackball );
	void draw( Trackball &trackball );
	void draw( Card &card );
	void draw( CatchAll &catchAll );
	
	void drawTouches( TouchObject &touchObject );
	void drawPivot( TouchObject &touchObject );
}

