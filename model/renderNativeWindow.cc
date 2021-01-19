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

#include "renderNativeWindow.h"
#include "minsky_epilogue.h"

#if defined(CAIRO_HAS_XLIB_SURFACE) && !defined(MAC_OSX_TK)
#include <cairo/cairo-xlib.h>
#include <cairo/cairo-xlib.h>
#include <X11/Xlib.h>
#endif

#if defined(CAIRO_HAS_WIN32_SURFACE) && !defined(__CYGWIN__)
#define USE_WIN32_SURFACE
#endif

#ifdef _WIN32
#undef Realloc
#include <windows.h>
#include <wingdi.h>
#ifdef USE_WIN32_SURFACE
#include <cairo/cairo-win32.h>
#endif
#endif

#if defined(MAC_OSX_TK)
#include <Carbon/Carbon.h>
#include <cairo/cairo-quartz.h>
#include "getContext.h"
#endif

#include <stdexcept>
#include <string>

using namespace std;
using namespace ecolab;

namespace minsky
{
#ifdef USE_WIN32_SURFACE
  inline cairo::SurfacePtr nativeWindowSurface(unsigned long window)
  {/* TODO */}
#elif defined(MAC_OSX_TK)
  inline cairo::SurfacePtr nativeWindowSurface(unsigned long window)
  {/* TODO */}
#else  
  inline cairo::SurfacePtr nativeWindowSurface(unsigned long window)
  {
    XWindowAttributes wAttr;
    auto display=XOpenDisplay(nullptr);
    int err=XGetWindowAttributes(display, window, &wAttr);
    if (err>1)
      throw runtime_error("Invalid window: "+to_string(window));
    cairo::SurfacePtr r(new cairo::Surface(cairo_xlib_surface_create(display,window,wAttr.visual,wAttr.width,wAttr.height),wAttr.width,wAttr.height));
    cairo_surface_set_device_offset(r->surface(), -wAttr.x, -wAttr.y);
    return r;
  }
#endif
  

  void RenderNativeWindow::renderToNativeWindow(unsigned long window)
  {
    auto tmp=nativeWindowSurface(window);
    tmp.swap(surface);
    redraw(0,0,surface->width(),surface->height());
    tmp.swap(surface);
  }
}
