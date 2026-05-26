/*
  Abstract interface for Cairo operations to enable testing and mocking
  @copyright Steve Keen 2012
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
#ifndef ICAIROSHIM_H
#define ICAIROSHIM_H

#include <cmath>
#include <memory>
#include <string>
#include <cairo.h>

namespace minsky
{
  class SVGRenderer;

  struct TextProperties
  {
    // markup overrides the text entry
    std::string markup, plainText;
    double fontSize=std::nan("");
    double angle=0;
    std::string fontFamily;
    TextProperties(const std::string& markup="", const std::string& plainText="", double fontSize=std::nan("")):
      markup(markup), plainText(plainText), fontSize(fontSize) {}
    TextProperties(const std::string& markup, double fontSize):
      markup(markup), fontSize(fontSize) {}
  };

  struct TextExtents
  {
    double left, top, width, height;
  };

  /// cache results of font rendering to amortise setup costs
  class ICacheRender
  {
  public:
    virtual ~ICacheRender()=default;
    virtual void show()=0;
    virtual TextExtents extents() const=0;
    virtual void* context() const=0;
  };

  class ICairoShim;
  
  /// a simple minded implementation that doesn't cache
  class NonCachedRenderer: public ICacheRender
  {
    TextProperties text;
    const ICairoShim& shim;
  public:
    NonCachedRenderer(const TextProperties& text, const ICairoShim& shim):
      text(text), shim(shim) {}
    void show() override;
    TextExtents extents() const override;
    void* context() const override;
  };
  
  /// Abstract interface for Cairo drawing operations
  class ICairoShim
  {
  public:
    virtual ~ICairoShim() = default;

    /// @{ Drawing operations
    virtual void moveTo(double x, double y) const = 0;
    virtual void lineTo(double x, double y) const = 0;
    virtual void relMoveTo(double x, double y) const = 0;
    virtual void relLineTo(double x, double y) const = 0;
    virtual void arc(double x, double y, double radius, double start, double end) const = 0;
    virtual void curveTo(double x1, double y1, double x2, double y2, double x3, double y3) const = 0;
    virtual void rectangle(double x, double y, double width, double height) const = 0;
    /// @}
    
    /// @{ Path operations
    virtual void newPath() const = 0;
    virtual void newSubPath() const = 0;
    virtual void closePath() const = 0;
    virtual void getCurrentPoint(double& x, double& y) const = 0;
    /// @}
    
    /// @{ Fill and stroke operations
    virtual void fill() const = 0;
    virtual void fillPreserve() const = 0;
    virtual void stroke() const = 0;
    virtual void strokePreserve() const = 0;
    virtual void clip() const = 0;
    virtual void resetClip() const = 0;
    virtual void paint() const = 0;
    /// @}
    
    /// @{ Line properties
    virtual void setLineWidth(double width) const = 0;
    virtual double getLineWidth() const = 0;
    virtual void setDash(const double* dashes, int num_dashes, double offset) const = 0;
    virtual void setFillRule(cairo_fill_rule_t fill_rule) const = 0;
    /// @}
    
    /// @{ Color operations
    virtual void setSourceRGB(double r, double g, double b) const = 0;
    virtual void setSourceRGBA(double r, double g, double b, double a) const = 0;
    /// @}
    
    /// render text
    virtual void showText(const TextProperties& text) const = 0;
    /// show markup text
    void showText(const std::string& s, double fs=std::nan("")) const
    {showText(TextProperties(s,fs));}
    /// show text with no markup interpretation
    void showPlainText(const std::string& s, double fs=std::nan("")) const
    {showText(TextProperties("",s,fs));}
//    virtual void setFontSize(double size) const = 0;
//    virtual void selectFontFace(const std::string& family, cairo_font_slant_t slant, cairo_font_weight_t weight) const = 0;
    /// return metrics for a given bit of text
    virtual TextExtents textExtents(const TextProperties& text) const = 0;
    
    /// @{ Transformation operations
    virtual void identityMatrix() const = 0;
    virtual void translate(double x, double y) const = 0;
    virtual void scale(double sx, double sy) const = 0;
    virtual void rotate(double angle) const = 0;
    virtual void userToDevice(double& x, double& y) const = 0;
    /// @}
    
    /// @{ Context state operations
    virtual void save() const = 0;
    virtual void restore() const = 0;
    /// @}
    
    /// Tolerance
    virtual void setTolerance(double tolerance) const = 0;

    // SVG rendering support
    /// Render an SVG resource into a region of size width x height
    /// @param svgRenderer - Reference to SVGRenderer containing the loaded SVG resource
    /// @param width - target width for rendering
    /// @param height - target height for rendering
    virtual void renderSVG(const SVGRenderer& svgRenderer, double width, double height) const = 0;

    /// returns reference to underlying context for caching purposes
    virtual void* context() const=0;

    /// return a cached object of rendered text
    virtual std::unique_ptr<ICacheRender> cachedRender(const TextProperties& tp) const
    {return std::make_unique<NonCachedRenderer>(tp,*this);}
  };

  /// RAII wrapper around save/restore
  struct CairoShimSave
  {
    const ICairoShim& shim;
    CairoShimSave(const ICairoShim& shim): shim(shim) {shim.save();}
    ~CairoShimSave() {shim.restore();}
  };
  
  inline void NonCachedRenderer::show()
  {shim.showText(text);}
  inline TextExtents NonCachedRenderer::extents() const
  {return shim.textExtents(text);}
  inline void* NonCachedRenderer::context() const
  {return shim.context();}

}

#include "ICairoShim.xcd"
#endif // ICAIROSHIM_H
