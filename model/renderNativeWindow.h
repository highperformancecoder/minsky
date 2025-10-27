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

#include "eventInterface.h"

#include <cairoSurfaceImage.h>
#include <plot.h>

#ifndef CLASSDESC_TYPENAME___std__lock_guard__mutex___
#define CLASSDESC_TYPENAME___std__lock_guard__mutex___
namespace classdesc
{
  template <> struct tn<std::lock_guard<std::mutex>>
  {
    static string name() {return "std::lock_guard<std::mutex>";}
  };
}
#endif

namespace minsky
{  
  class WindowInformation;
  class RenderNativeWindow : public ecolab::CairoSurface, public EventInterface
  {
  public:
    static ecolab::cairo::Colour backgroundColour;

    struct RenderFrameArgs
    {
      std::string parentWindowId;
      double offsetLeft;
      double offsetTop;
      double childWidth;
      double childHeight;
      double scalingFactor=0;
    };

    
    const RenderFrameArgs& frameArgs() const {return m_frameArgs;}
    
    ~RenderNativeWindow() override;
    /// perform any initialisation of any subclasses of this 
    virtual void init() {};
    void renderFrame(const RenderFrameArgs& args);
    void destroyFrame();
    void draw();
    void requestRedraw();
    // implemented as a free function to avoid Classdesc exposing this to Typescript
    friend void macOSXRedraw(RenderNativeWindow&,const std::shared_ptr<std::lock_guard<std::mutex>>&);
    // do not clobber winInfoPtr on load of model
    RenderNativeWindow& operator=(const RenderNativeWindow& x) {ecolab::CairoSurface::operator=(x); return *this;}
    RenderNativeWindow()=default;
    RenderNativeWindow(const RenderNativeWindow&)=default;
    static double scaleFactor();
    /// return whether this window has scrollbars (needed for MacOSX).
    virtual bool hasScrollBars() const {return false;}
    
  private:
    CLASSDESC_ACCESS(RenderNativeWindow); 
    classdesc::Exclude<std::shared_ptr<WindowInformation>> winInfoPtr;
    RenderFrameArgs m_frameArgs;

    /// drawing implementation without mutex locking (for MacOSX to avoid deadlock)
    void drawInternal();

    /// a copiable mutex where each copy has a distinct mutex
    struct CopiableMutux: public std::mutex
    {
      CopiableMutux()=default;
      CopiableMutux(const CopiableMutux&) {}
    };
    classdesc::Exclude<CopiableMutux> drawMutex;
  };

  //void macOSXRedraw(RenderNativeWindow&,const std::shared_ptr<std::lock_guard<std::mutex>>&);
  
} // namespace minsky

#include "renderNativeWindow.cd"
#endif
