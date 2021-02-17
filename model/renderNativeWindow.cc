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

/* We have created a struct `WindowInformation` that stores the `childWindowId` along with other details like display and window attributes. This information is reused across multiple calls to `renderFrame`. 

The flow for code will be -- when minsky starts, a call to /minsky/canvas/initializeNativeWindow will be made, with parentWindowId (and offsets) as the parameters (creating child window in electron did not work as expected, so we need to work with offsets). Subsequent repaints can be requested with /minsky/canvas/renderFrame

As of now, we create the cairo surface with each call to `renderFrame`, though I think the surface can also be reused. I have a placeholder for pointer to cairo::SurfacePtr (not sure we should have pointer to pointer) but it didn't work as expected, so for now I am recreating the surface in `renderFrame`

Please especially review the lifecycle (constructors, desctructors and copy constructors) that I have defined in `renderNativeWindow.cc `. I think the WindowInformation object that is destroyed in the destructor for RenderNativeWindow can be reused (perhaps it can be made a static object?). Also - am not sure how to distinguish between destructor for RenderNativeWindow that will be called with each call to load model (or undo/redo as you mentioned), and the final call when minsky is closed.
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
  inline cairo::SurfacePtr createNativeWindowSurface(WindowInformation *winInfo)
  { /* TODO */
  }
#elif defined(MAC_OSX_TK)
  inline cairo::SurfacePtr createNativeWindowSurface(WindowInformation *winInfo)
  { /* TODO */
  }
#else

  int throwOnXError(Display *, XErrorEvent *ev)
  {
    char errorMessage[256];
    XGetErrorText(ev->display, ev->error_code, errorMessage, sizeof(errorMessage));
    throw runtime_error(errorMessage);
  }

  inline cairo::SurfacePtr createNativeWindowSurface(WindowInformation *wi)
  {
    cairo::SurfacePtr childSurface(new cairo::Surface(cairo_xlib_surface_create(wi->display, wi->childWindowId, wi->wAttr.visual, wi->childWidth, wi->childHeight), wi->childWidth, wi->childHeight));
    cairo_surface_set_device_offset(childSurface->surface(), -wi->wAttr.x, -wi->wAttr.y);
    return childSurface;
  }
#endif

  WindowInformation::WindowInformation()
  {
  }
  WindowInformation::~WindowInformation()
  {
    cout << "Delete called for window information" << endl;
  }

  void WindowInformation::initialize(unsigned long parentWin, int left, int top, int cWidth, int cHeight)
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
    
    // Todo:: currently width, height parameters are not getting passed properly
    // Eventually, we need those in order to ensure we don't need to worry about
    // paddings, scrollbars etc
    if(cWidth > 0) {
      childWidth = min(childWidth, cWidth);
    }
    if(cHeight > 0) {
      childHeight = min(childHeight, cHeight);
    }

    cout << childWidth << "::" << childHeight << "::" << offsetLeft << "::" << offsetTop << endl;
    childWindowId = XCreateSimpleWindow(display, parentWin, offsetLeft, offsetTop, childWidth, childHeight, 0, 0, 0); //TODO:: Should we pass visual and attributes at the end?
    XMapWindow(display, childWindowId);
  }

  void WindowInformation::copy(WindowInformation *winInfo)
  {
    this->parentWindowId = winInfo->parentWindowId;
    this->childWindowId = winInfo->childWindowId;
    this->offsetLeft = winInfo->offsetLeft;
    this->offsetTop = winInfo->offsetTop;
    this->display = winInfo->display;
    this->wAttr = winInfo->wAttr;
    // this->childSurface = winInfo->childSurface;// TODO:: Later, try reusing the surface also in winInfo
  }

  RenderNativeWindow::RenderNativeWindow()
  {
    winInfo = new WindowInformation(); // TODO:: Try to reuse the WindowInformation object instead of creating a new one.... perhaps it should be a static data member?
  }

  RenderNativeWindow::~RenderNativeWindow()
  {
    delete winInfo;
  }

  RenderNativeWindow &RenderNativeWindow::operator=(const RenderNativeWindow &a)
  {
    cout << "Calling assignment on renderNativeWindow" << endl;
    a.winInfo->copy(this->winInfo);
    return *this;
  }

  RenderNativeWindow::RenderNativeWindow(const RenderNativeWindow &a)
  {
    cout << "Copy constructor for RenderNativeWindow called... TODO:: implement this" << endl;
    //a.winInfo->copy(this->winInfo); // reverse?
  };

  void RenderNativeWindow::renderFrame()
  {
    auto tmp = createNativeWindowSurface(winInfo);

    //TODO:: Review if this paint (below 3 lines) is really needed with each frame
    cairo_move_to(tmp->cairo(), 0, 0);
    cairo_set_source_rgb(tmp->cairo(), 1, 1, 1);
    cairo_paint(tmp->cairo());

    tmp.swap(surface);
    redraw(0, 0, surface->width(), surface->height());
    tmp.swap(surface);
  }

  void RenderNativeWindow::initializeNativeWindow(unsigned long parentWindowId, int offsetLeft, int offsetTop, int childWidth, int childHeight)
  {
    cout << parentWindowId << "::" << offsetLeft << "::" << offsetTop << "::" << childWidth << "::" << childHeight << endl;
    // TODO:: Only the first argument is getting passed here, rest values are 0s
    offsetLeft = max(20, offsetLeft);
    offsetTop = max(180, offsetTop);
    this->winInfo->initialize(parentWindowId, offsetLeft, offsetTop, childWidth, childHeight);
  }


  void RenderNativeWindow::resizeWindow(int offsetLeft, int offsetTop, int childWidth, int childHeight) {
    // TODO:: To be implemented... need to recreate child window
  }
} // namespace minsky
