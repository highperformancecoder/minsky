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

#ifndef RENDER_NATIVE_WINDOW_H
#define RENDER_NATIVE_WINDOW_H

#include <cairoSurfaceImage.h>

namespace minsky
{
  struct WindowInformation
  {
    unsigned long parentWindowId;
    unsigned long childWindowId;
    int childWidth;
    int childHeight;
    int offsetLeft;
    int offsetTop;
    Display*	display;
    XWindowAttributes wAttr;
    ecolab::cairo::SurfacePtr *childSurface; // TODO:: Review right way to store surface ptr

    WindowInformation();
    ~WindowInformation();
    void initialize(unsigned long parentWin, int offsetLeft, int offsetTop, int childWidth, int childHeight);
    void copy(WindowInformation *winInfo);
  };

  class RenderNativeWindow : public ecolab::CairoSurface
  {
  private:
    WindowInformation *winInfo;

  public:
    RenderNativeWindow();
    ~RenderNativeWindow();
    RenderNativeWindow& operator=(const RenderNativeWindow &a);
    RenderNativeWindow(const RenderNativeWindow &a);

  public:
    void initializeNativeWindow(unsigned long parentWindowId, int offsetLeft, int offsetTop, int childWidth, int childHeight);
    
    void resizeWindow(int offsetLeft, int offsetTop, int childWidth, int childHeight);

    void renderFrame();
  };
} // namespace minsky

#include "renderNativeWindow.cd"
#endif
