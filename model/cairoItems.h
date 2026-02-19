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
  class RenderVariable: public ecolab::Pango
  {
    const VariableBase& var;
    cairo_t *cairo;
    float w, h, hoffs;
  public:
    // render a variable to a given cairo context
    RenderVariable(const VariableBase& var, cairo_t* cairo=NULL);
    /// render the cairo image
    void draw();
    /// half width of unrotated image
    float width() const {return w;}
    /// half height of unrotated image
    float height() const {return h;}
    /// return the boost geometry corresponding to this variable's shape
    //Polygon geom() const;
    bool inImage(float x, float y); ///< true if (x,y) within rendered image
    /// x coordinate of the slider handle in the unrotated/unscaled
    /// frame of reference
    double handlePos() const;
  };

  void drawTriangle(cairo_t* cairo, double x, double y, const ecolab::cairo::Colour& col, double angle=0);
  void drawTriangle(const ICairoShim& cairoShim, double x, double y, const ecolab::cairo::Colour& col, double angle=0);
}
#endif
