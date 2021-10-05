/*
  @copyright Steve Keen 2021
  @author Russell Standish
  This file is part of Minsky.

  Minsky is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Minsky is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Minsky.  If not, see <http://www.gnu.org/licenses/>.
*/

/// Process an electron native window handle into a CGContext for Cairo
#include "getContext.h"
#include <Appkit/NSGraphicsContext.h>
#include <Appkit/NSWindow.h>
#include <iostream>

namespace minsky
{
  
  NSContext::NSContext(void* nativeHandle,int xoffs,int yoffs)
  {
    std::cout << nativeHandle << std::endl;
    std::cout << [reinterpret_cast<NSObject*>(nativeHandle) class] << std::endl;
    std::cout << [NSStringFromClass([reinterpret_cast<NSView*>(nativeHandle) class]) cStringUsingEncoding:NSUTF8StringEncoding] << std::endl;
    NSWindow* w=[reinterpret_cast<NSView*>(nativeHandle) window];
    std::cout << w << std::endl;
    NSGraphicsContext* g=[NSGraphicsContext graphicsContextWithWindow: w];
    std::cout << g << std::endl;
    graphicsContext=g;
    [g retain];
    context=[g CGContext];
    std::cout << context << std::endl;
    NSRect contentRect=[w contentRectForFrameRect: w.frame]; // allow for title bar
    CGContextTranslateCTM(context,xoffs,contentRect.size.height-yoffs);
    CGContextScaleCTM(context,1,-1); //CoreGraphics's y dimension is opposite to everybody else's
    std::cout << nativeHandle << " " << w << " " << g << " " << context << std::endl;
  }

  NSContext::~NSContext()
  {
    [(NSGraphicsContext*)graphicsContext release];
  }
 
}
