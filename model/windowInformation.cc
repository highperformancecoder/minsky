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

#if defined(CAIRO_HAS_WIN32_SURFACE) && !defined(__CYGWIN__)
#define USE_WIN32_SURFACE
#elif defined(CAIRO_HAS_XLIB_SURFACE) && !defined(MAC_OSX_TK)
#define USE_X11
#include <cairo/cairo-xlib.h>
#include <X11/Xlib.h>
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

  WindowInformation::~WindowInformation()
  {
    bufferSurface.reset();
#ifdef USE_WIN32_SURFACE
    SelectObject(hdcMem, hOld);
    DeleteObject(hbmMem);
    DeleteDC(hdcMem);
#elif defined(MAC_OSX_TK)
#elif defined(USE_X11)
    XFreeGC(display, graphicsContext);
    XDestroyWindow(display, childWindowId);
    XDestroyWindow(display, bufferWindowId);
#endif
  }

  const ecolab::cairo::SurfacePtr& WindowInformation::getBufferSurface()
  {
    return bufferSurface;
  }

  void WindowInformation::copyBufferToMain()
  {
    cairo_surface_flush(bufferSurface->surface());
#ifdef USE_WIN32_SURFACE
    HDC hdc=GetDC(parentWindowId);
    BitBlt(hdc, offsetLeft, offsetTop, childWidth,childHeight,hdcMem,0,0,SRCCOPY);
    ReleaseDC(parentWindowId, hdc);
#elif defined(USE_X11)
    XCopyArea(display, bufferWindowId, childWindowId, graphicsContext, 0, 0, childWidth, childHeight, 0, 0);
    XCopyArea(display, bufferWindowId, childWindowId, graphicsContext, 0, 0, childWidth, childHeight, 0, 0);
    XFlush(display);
#endif
  }

  void WindowInformation::setRenderingFlag(bool value)
  {
    isRendering = value;
  }

  bool WindowInformation::getRenderingFlag()
  {
    return isRendering;
  }

  WindowInformation::WindowInformation(uint64_t parentWin, int left, int top, int cWidth, int cHeight)
#ifdef MAC_OSX_TK
    : nsContext(reinterpret_cast<void*>(parentWin),left,top)
#endif
  {

    offsetLeft = left;
    offsetTop = top;

    childWidth = cWidth;
    childHeight = cHeight;

#ifdef USE_WIN32_SURFACE
    parentWindowId = reinterpret_cast<HWND>(parentWin);
    HDC hdc=GetDC(parentWindowId);
    hdcMem=CreateCompatibleDC(hdc);
    hbmMem=CreateCompatibleBitmap(hdc, childWidth, childHeight);
    ReleaseDC(parentWindowId, hdc);
    hOld=SelectObject(hdcMem, hbmMem);
    bufferSurface.reset(new cairo::Surface(cairo_win32_surface_create(hdcMem),childWidth, childHeight));
#elif defined(MAC_OSX_TK)
#elif defined(USE_X11)
    parentWindowId = parentWin;
    static bool errorHandlingSet = (XSetErrorHandler(throwOnXError), true);
    display = XOpenDisplay(nullptr);
    int err = XGetWindowAttributes(display, parentWin, &wAttr);
    if (err > 1)
      throw runtime_error("Invalid window: " + to_string(parentWin));

    // TODO:: Do some sanity checks on dimensions 

    childWindowId = XCreateSimpleWindow(display, parentWin, offsetLeft, offsetTop, childWidth, childHeight, 0, 0, MINSKY_CANVAS_BACKGROUND_COLOR);
    bufferWindowId = XCreatePixmap(display, parentWin, childWidth, childHeight, wAttr.depth);
    graphicsContext=XCreateGC(display, childWindowId, 0, nullptr);
    
    XMapWindow(display, childWindowId);
    bufferSurface.reset(new cairo::Surface(cairo_xlib_surface_create(display, bufferWindowId, wAttr.visual, childWidth, childHeight), childWidth, childHeight));
#endif
    
    isRendering = false;
  }
} // namespace minsky
