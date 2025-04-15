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
#include "windowInformation.h"
#include <Appkit/NSGraphicsContext.h>
#include <Appkit/NSWindow.h>
#include <AppKit/NSView.h>
#include <cairo/cairo-quartz.h>
#include <iostream>
#include <exception>
#include "minsky_epilogue.h"

using namespace std;

@interface CairoView: NSView
{
  minsky::WindowInformation* winfo;
}
-(void) drawRect: (NSRect)rect;
// required to prevent the subview from stealing mouse events
-(NSView*) hitTest: (NSPoint)aPoint;
-(void) setWinfo: (minsky::WindowInformation*)winfo;
@end

namespace minsky
{

  struct ViewImpl
  {
    CairoView* cairoView;
    ViewImpl(): cairoView([[CairoView alloc] init]) {[cairoView retain];}
    ~ViewImpl() {[cairoView removeFromSuperview]; [cairoView release];}
    ViewImpl(const ViewImpl&)=delete;
    void operator=(const ViewImpl&)=delete;
  };

  NSContext::NSContext(NSContext&&)=default;

  
  namespace
  {
    struct SetSignal
    {
      sig_t segvHandler;
      SetSignal() {segvHandler=signal(SIGSEGV,signal_caught);}
      ~SetSignal() {signal(SIGSEGV,segvHandler);}
      static void signal_caught(int) {
        throw runtime_error("Different process space not supported on MacOSX");
      }
    };
  }
  
  NSContext::NSContext(void* nativeHandle,int xoffs,int yoffs,int width,int height,
                       WindowInformation& winfo)
    
  {
    NSView* view=reinterpret_cast<NSView*>(nativeHandle);
    // check that we are passed a valid NSView pointer (must be an in-process call).
    @try {
      SetSignal sigHandler;
      [view class];
    }
    @catch (NSException*)
      {
        throw runtime_error("Different process space not supported on MacOSX");
      }

    impl=make_unique<ViewImpl>();

    // do not overwrite scrollbar
    if (winfo.hasScrollBars)
      {
        width-=20;
      }
    [impl->cairoView setFrameSize: NSMakeSize(width,height)];
    [impl->cairoView setWinfo: &winfo];
    [view addSubview: impl->cairoView];
  }

  NSContext::~NSContext()=default;
  
  void NSContext::requestRedraw()
  {
   [impl->cairoView setNeedsDisplay: true];
  }

}

@implementation CairoView
-(void) drawRect: (NSRect)rect
{
  auto context = [[NSGraphicsContext currentContext] CGContext];
  auto frame=[self frame];
  CGContextTranslateCTM(context,winfo->offsetLeft,winfo->childHeight+(winfo->hasScrollBars?20:0)); 
  CGContextScaleCTM(context,1,-1); //CoreGraphics's y dimension is opposite to everybody else's
  winfo->bufferSurface=make_shared<ecolab::cairo::Surface>(cairo_quartz_surface_create_for_cg_context(context, NSWidth(frame), NSHeight(frame)));
  if (winfo->hasScrollBars)
    cairo_surface_set_device_offset(winfo->bufferSurface->surface(), 0, 20);
  winfo->draw();
  winfo->bufferSurface.reset();
  winfo->lock.reset(); // unlock any mutex attached to this window
}
- (NSView *) hitTest: (NSPoint) aPoint
{
  return nil;
}

-(void) setWinfo: (minsky::WindowInformation*)w
{
  winfo=w;
}
@end
