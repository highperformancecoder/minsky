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

  int throwOnXError(Display*, XErrorEvent* ev)
  {
    char errorMessage[256];
    XGetErrorText(ev->display, ev->error_code, errorMessage, sizeof(errorMessage));
    throw runtime_error(errorMessage);
  }
  
  inline cairo::SurfacePtr nativeWindowSurface(unsigned long window)
  {
    // ensure errors are thrown, rather than exit() being called
    static bool errorHandlingSet=(XSetErrorHandler(throwOnXError), true);
    XWindowAttributes wAttr;
    auto display=XOpenDisplay(nullptr);
    int err=XGetWindowAttributes(display, window, &wAttr);
    if (err>1)
      throw runtime_error("Invalid window: "+to_string(window));


    int padding = 10;
    int yOffset = 150;
    int childWindowWidth = wAttr.width - 2*padding;
    int childWindowHeight = wAttr.height - yOffset - padding;

    unsigned long childWindow = XCreateSimpleWindow(display, window, padding, yOffset, childWindowWidth, childWindowHeight, 0, 0, 0);
    XMapWindow(display, childWindow);

    cout << "Child Window ID ::"<< childWindow << endl;
    //XSelectInput(d, da, ButtonPressMask | KeyPressMask);
    cout << "wAttr(x, y)::" << wAttr.x << "," << wAttr.y << endl;

    cairo::SurfacePtr r(new cairo::Surface(cairo_xlib_surface_create(display, childWindow,wAttr.visual, childWindowWidth, childWindowHeight), childWindowWidth, childWindowHeight));
    cairo_surface_set_device_offset(r->surface(), -wAttr.x, -wAttr.y);

    cairo_move_to(r->cairo(), 0, 0);
    cairo_set_source_rgb(r->cairo(), 1, 1, 1);
    cairo_paint(r->cairo());
    return r;
  }
#endif


  void RenderNativeWindow::initializeNativeWindow(unsigned long window) {

  }

  void RenderNativeWindow::renderFrame() {

  }

  void RenderNativeWindow::renderToNativeWindow(unsigned long window)
  {
    auto tmp=nativeWindowSurface(window);
    tmp.swap(surface);
    redraw(0,0,surface->width(),surface->height());
    tmp.swap(surface);
  }
}
