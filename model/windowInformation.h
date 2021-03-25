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

#ifndef WINDOW_INFORMATION_H
#define WINDOW_INFORMATION_H

#include <cairoSurfaceImage.h>
#include <X11/Xlib.h>

namespace minsky
{
  class WindowInformation
  {
    bool isRendering;
    unsigned long parentWindowId;
    unsigned long childWindowId;
    
    Display*	display; // Weak reference, returned by system
    ecolab::cairo::SurfacePtr windowSurface;
    ecolab::cairo::SurfacePtr bufferSurface;

  private:
      void createSurfaces();
  public: 
      void clear();
      int childWidth;
      int childHeight;
      int offsetLeft;
      int offsetTop;
      
      XWindowAttributes wAttr;
      Display* getDisplay();
      bool getRenderingFlag();
      void setRenderingFlag(bool value);
      void copyBufferToMain();
      
  public:
    ~WindowInformation();
    WindowInformation(unsigned long parentWin, int left, int top, int cWidth, int cHeight);
    
    ecolab::cairo::SurfacePtr getBufferSurface();

    WindowInformation(const WindowInformation&)=delete;
    void operator=(const WindowInformation&)=delete;
  };
} // namespace minsky

#endif
