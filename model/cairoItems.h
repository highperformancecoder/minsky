/*
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
#ifndef CAIRO_ITEMS_H
#define CAIRO_ITEMS_H
#include <geometry.h>
#include <plot.h>
#include <pango.h>
#include <cairo_base.h>
#include <cairo/cairo.h>
#include "operation.h"
#include "variable.h"

namespace minsky
{
  /** class that renders a variable into a cairo context. 
      A user can also query the size of the unrotated rendered image
  */
  class RenderVariable
  {
    const VariableBase& var;
    const ICairoShim& cairoShim;
    // caching of text rendering
    std::shared_ptr<ICacheRender> cachedRenderer;
    float w, h, hoffs;
  public:
    RenderVariable(const VariableBase& var);
    RenderVariable(const VariableBase& var, const ICairoShim& shim);
    /// render the cairo image
    void draw() {var.draw(cairoShim);}
    /// render cached text
    void show() const {cachedRenderer->show();}
    /// half width of unrotated image
    float width() const {return w;}
    /// half height of unrotated image
    float height() const {return h;}
    /// vertical offset
    float top() const {return hoffs;}
    bool inImage(float x, float y); ///< true if (x,y) within rendered image
    /// x coordinate of the slider handle in the unrotated/unscaled
    /// frame of reference
    double handlePos() const;
    void* context() const {return cairoShim.context();}
  };

  void drawTriangle(cairo_t* cairo, double x, double y, const ecolab::cairo::Colour& col, double angle=0);
  void drawTriangle(const ICairoShim& cairoShim, double x, double y, const ecolab::cairo::Colour& col, double angle=0);
}
#endif
