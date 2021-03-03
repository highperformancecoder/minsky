/*
  @copyright Steve Keen 2021
  @author Janak Porwal
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

#include "windowInformation.h"
#include "minsky_epilogue.h"

#include <stdexcept>
#include <string>

#if defined(CAIRO_HAS_XLIB_SURFACE) && !defined(MAC_OSX_TK)
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

using namespace std;
using namespace ecolab;


#define MINSKY_CANVAS_BACKGROUND_COLOR 0x00ffffff

namespace minsky
{

#ifdef USE_WIN32_SURFACE
#elif defined(MAC_OSX_TK)
#else
  int throwOnXError(Display *, XErrorEvent *ev)
  {
    char errorMessage[256];
    XGetErrorText(ev->display, ev->error_code, errorMessage, sizeof(errorMessage));
    throw runtime_error(errorMessage);
  }
#endif

  unsigned long WindowInformation::getChildWindowId() {
    return childWindowId;
  }

  Display *WindowInformation::getDisplay() {
    return display;
  }

  WindowInformation::~WindowInformation() {
    childSurface.reset();
    XDestroyWindow(display, childWindowId);
  }

  ecolab::cairo::SurfacePtr WindowInformation::getSurface() {
    return childSurface;
  }

  void WindowInformation::createSurface(){
#ifdef USE_WIN32_SURFACE
      {
          /* TODO */
      }
#elif defined(MAC_OSX_TK)

      {
          /* TODO */
      }
#else
      {childSurface.reset(new cairo::Surface(cairo_xlib_surface_create(getDisplay(), getChildWindowId(), wAttr.visual, childWidth, childHeight), childWidth, childHeight));
  cairo_surface_set_device_offset(childSurface->surface(), -wAttr.x, -wAttr.y);
}
#endif
  }

  void WindowInformation::clear() {
    XClearWindow(display, childWindowId);
  }

  WindowInformation::WindowInformation(unsigned long parentWin, int left, int top, int cWidth, int cHeight)
  {
    parentWindowId = parentWin;
    offsetLeft = left;
    offsetTop = top;

    static bool errorHandlingSet = (XSetErrorHandler(throwOnXError), true);
    display = XOpenDisplay(nullptr);
    int err = XGetWindowAttributes(display, parentWin, &wAttr);
    if (err > 1)
      throw runtime_error("Invalid window: " + to_string(parentWin));

    childWidth = wAttr.width - offsetLeft;
    childHeight = wAttr.height - offsetTop;

    // TODO:: Take care of scrollbars

    if (cWidth > 0) {
      childWidth = min(childWidth, cWidth);
    }

    if (cHeight > 0) {
      childHeight = min(childHeight, cHeight);
    }

    childWindowId = XCreateSimpleWindow(display, parentWin, offsetLeft, offsetTop, childWidth, childHeight, 0, 0, MINSKY_CANVAS_BACKGROUND_COLOR);
    XMapWindow(display, childWindowId);
    createSurface();
  }
} // namespace minsky