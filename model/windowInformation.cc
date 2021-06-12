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

//#define MINSKY_CANVAS_BACKGROUND_COLOR 0xee5a5add
//#define MINSKY_CANVAS_BACKGROUND_COLOR_OTHER 0xee5add5a

#define MINSKY_CANVAS_BACKGROUND_COLOR 0x00ffffff

// TODO:: Child surface should be transparent --- or -- there has to be provision to set background color as FE has that option

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

  Display *WindowInformation::getDisplay()
  {
    return display;
  }

  WindowInformation::~WindowInformation()
  {
    windowSurface.reset();
    bufferSurface.reset();
    XDestroyWindow(display, childWindowId);
  }

  ecolab::cairo::SurfacePtr WindowInformation::getBufferSurface()
  {
    return bufferSurface;
  }

  void WindowInformation::copyBufferToMain()
  {
    cairo_set_source_surface(windowSurface->cairo(), bufferSurface->surface(), 0, 0);
    cairo_paint(windowSurface->cairo());
  }

  void WindowInformation::createSurfaces()
  {
#ifdef USE_WIN32_SURFACE
    {
      /* TODO */
    }
#elif defined(MAC_OSX_TK)

    {
      /* TODO */
    }
#else
    {
      windowSurface.reset(new cairo::Surface(cairo_xlib_surface_create(getDisplay(), childWindowId, wAttr.visual, childWidth, childHeight), childWidth, childHeight));

      bufferSurface.reset(new cairo::Surface(cairo_surface_create_similar(windowSurface->surface(), CAIRO_CONTENT_COLOR_ALPHA, childWidth, childHeight), childWidth, childHeight));
    }

#endif
  }

  void WindowInformation::clear()
  {
    XClearWindow(display, childWindowId);
  }

  void WindowInformation::setRenderingFlag(bool value)
  {
    isRendering = value;
  }

  bool WindowInformation::getRenderingFlag()
  {
    return isRendering;
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

    // TODO:: Do some sanity checks on dimensions 

    childWidth = cWidth;
    childHeight = cHeight;

    childWindowId = XCreateSimpleWindow(display, parentWin, offsetLeft, offsetTop, childWidth, childHeight, 0, 0, MINSKY_CANVAS_BACKGROUND_COLOR);

    XMapWindow(display, childWindowId);

    createSurfaces();
    isRendering = false;
  }
} // namespace minsky
