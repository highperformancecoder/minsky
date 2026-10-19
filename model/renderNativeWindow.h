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

#include <IMansoura.h>
#include <mansouraPlot.h>

#include <mutex>

#ifndef CLASSDESC_TYPENAME___std__lock_guard__recursive_mutex___
#define CLASSDESC_TYPENAME___std__lock_guard__recursive_mutex___
namespace classdesc
{
  template <> struct tn<std::lock_guard<std::recursive_mutex>>
  {
    static string name() {return "std::lock_guard<std::recursive_mutex>";}
  };
}
#endif

namespace minsky
{  
  class WindowInformation;
  class RenderNativeWindow : public EventInterface
  {
  public:
    static mansoura::Colour backgroundColour;

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
    
    virtual ~RenderNativeWindow();
    /// perform any initialisation of any subclasses of this 
    virtual void init() {};
    void renderFrame(const RenderFrameArgs& args);
    void destroyFrame();
    void draw();
    void requestRedraw();

    /// @{ export the window's contents to a file
    void renderToPS(const std::string& filename) {renderTo(mansoura::ExportFormat::PS,filename);}
    void renderToPDF(const std::string& filename) {renderTo(mansoura::ExportFormat::PDF,filename);}
    void renderToSVG(const std::string& filename) {renderTo(mansoura::ExportFormat::SVG,filename);}
    void renderToPNG(const std::string& filename) {renderTo(mansoura::ExportFormat::PNG,filename);}
    /// render to an EMF file. Windows only.
    void renderToEMF(const std::string& filename) {renderTo(mansoura::ExportFormat::EMF,filename);}
    /// @}
    /// increase output resolution of pixmap surfaces by this factor
    double resolutionScaleFactor() const {return m_resolutionScaleFactor;}
    double resolutionScaleFactor(double rsf) {return m_resolutionScaleFactor=rsf;}
    /// arrange a callback with the drawing time in seconds
    virtual void reportDrawTime(double) {}


    // implemented as a free function to avoid Classdesc exposing this to Typescript
    friend void macOSXRedraw(RenderNativeWindow&,std::recursive_mutex&);
    // do not clobber winInfoPtr on load of model
    RenderNativeWindow& operator=(const RenderNativeWindow& x) {m_resolutionScaleFactor=x.m_resolutionScaleFactor; return *this;}
    RenderNativeWindow()=default;
    RenderNativeWindow(const RenderNativeWindow&)=default;
    static double scaleFactor();
    /// return whether this window has scrollbars (needed for MacOSX).
    virtual bool hasScrollBars() const {return false;}
    
  protected:
    /// draw the window's contents into \a cairoShim. x0,y0,width,height give the region to redraw
    /// @return true if something drawn
    virtual bool redraw(const mansoura::IMansoura& cairoShim, int x0, int y0, int width, int height)=0;
    /// draw the entire contents of the window, unclipped
    virtual void redrawWithBounds(const mansoura::IMansoura& cairoShim) {redraw(cairoShim,-1e6,-1e6,2e6,2e6);}
    void renderTo(mansoura::ExportFormat format, const std::string& filename);
  private:
    CLASSDESC_ACCESS(RenderNativeWindow); 
    double m_resolutionScaleFactor=1.0;
    classdesc::Exclude<std::shared_ptr<WindowInformation>> winInfoPtr;
    RenderFrameArgs m_frameArgs;

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
