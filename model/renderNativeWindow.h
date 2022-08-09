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
#include <plot.h>

namespace minsky
{  
  class WindowInformation;
  class RenderNativeWindow : public ecolab::CairoSurface
  {
  private:
    CLASSDESC_ACCESS(RenderNativeWindow); 
    classdesc::Exclude<std::shared_ptr<WindowInformation>> winInfoPtr;

  public:
    static ecolab::cairo::Colour backgroundColour;
    void disable();
    
    ~RenderNativeWindow() override;
    void resizeWindow(int offsetLeft, int offsetTop, int childWidth, int childHeight);
    void renderFrame(uint64_t parentWindowId, int offsetLeft, int offsetTop, int childWidth, int childHeight, double scalingFactor);
    void renderFrame(uint64_t parentWindowId, int offsetLeft, int offsetTop, int childWidth, int childHeight)
    {renderFrame(parentWindowId,offsetLeft,offsetTop,childWidth,childHeight,0);}
    void destroyFrame();
    void draw();
    void requestRedraw();
    // do not clobber winInfoPtr on load of model
    RenderNativeWindow& operator=(const RenderNativeWindow& x) {ecolab::CairoSurface::operator=(x); return *this;}
    double scaleFactor() const;
    /// return whether this window has scrollbars (needed for MacOSX).
    virtual bool hasScrollBars() const {return false;}
    /// extra vertical translation, required for MacOSX
    virtual int extraVerticalTranslation() const {return 0;}
  };
} // namespace minsky

#include "renderNativeWindow.cd"
#endif
