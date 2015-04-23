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
#include "variable.h"
#include "operation.h"
#include "portManager.h"
#include <plot.h>
#include <cairo/cairo.h>
#include "geometry.h"
#include "clickType.h"

namespace minsky
{
  /** class that randers a variable into a cairo context. 
      A user can also query the size of the unrotated rendered image
  */
  class RenderOperation
  {
    const OperationBase& op;
    cairo_t *cairo;
    float w, h, hoffs;

  public:
    // render a variable to a given cairo context
    RenderOperation(const OperationBase& var, cairo_t* cairo=NULL);
    /// render the cairo image
    void draw();
    /// half width of unrotated image
    float width() const {return w;}
    /// half height of unrotated image
    float height() const {return h;}

    Polygon geom() const;
    bool inImage(float x, float y); ///< true if (x,y) within rendered image
  };


  /** class that randers a variable into a cairo context. 
      A user can also query the size of the unrotated rendered image
  */
  class RenderVariable
  {
    const VariableBase& var;
    cairo_t *cairo;
    float w, h, hoffs;
  public:
    // render a variable to a given cairo context
    RenderVariable(const VariableBase& var, cairo_t* cairo=NULL);
    /// render the cairo image
    void draw();
    /// compute and update port locations
    void updatePortLocs();
    /// half width of unrotated image
    float width() const {return w;}
    /// half height of unrotated image
    float height() const {return h;}
    /// return the boost geometry corresponding to this variable's shape
    Polygon geom() const;
    bool inImage(float x, float y); ///< true if (x,y) within rendered image
  };

  void drawTriangle(cairo_t* cairo, double x, double y, cairo::Colour& col, double angle=0);

  template <class Item, class Ports>
  void drawPorts(const Item& item, const Ports& ports, 
                 cairo_t* cairo)
  {
    cairo_save(cairo);
    for (size_t i=0; i<ports.size(); ++i)
      {
        Port p=cportManager().ports[ports[i]];
        cairo_new_sub_path(cairo);
        cairo_arc(cairo, p.x()-item.x(), p.y()-item.y(), portRadius*item.zoomFactor, 0, 2*M_PI);
      }
    cairo_set_source_rgb(cairo, 0,0,0);
    cairo_set_line_width(cairo,1);
    cairo_stroke(cairo);
    cairo_restore(cairo);
  }

  /// mark icon as selected.
  void drawSelected(cairo_t* cairo);

}
