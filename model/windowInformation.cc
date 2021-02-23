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

using namespace std;
using namespace ecolab;

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
  unsigned long WindowInformation::getChildWindowId()
  {
    return childWindowId;
  }

  Display *WindowInformation::getDisplay()
  {
    return display;
  }

  WindowInformation::~WindowInformation()
  {
    childSurface.reset();
    XDestroyWindow(display, childWindowId);
  }

  ecolab::cairo::SurfacePtr WindowInformation::getSurface()
  {
    return childSurface;
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

    // Todo:: currently width, height parameters are not getting passed properly
    // Eventually, we need those in order to ensure we don't need to worry about
    // paddings, scrollbars etc
    if (cWidth > 0)
    {
      childWidth = min(childWidth, cWidth);
    }
    if (cHeight > 0)
    {
      childHeight = min(childHeight, cHeight);
    }

    // std::cout << childWidth << "::" << childHeight << "::" << offsetLeft << "::" << offsetTop << std::endl;
    childWindowId = XCreateSimpleWindow(display, parentWin, offsetLeft, offsetTop, childWidth, childHeight, 0, 0, 0); //TODO:: Should we pass visual and attributes at the end?
    XMapWindow(display, childWindowId);
    // std::cout << "We got child window id = " << childWindowId << std::endl;
  }
} // namespace minsky
