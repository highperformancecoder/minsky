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
// Implementations of canvas items representing operations and variables.

// invert the display of the power operator so that y is on top, and x
// below, for ticket #327
#define DISPLAY_POW_UPSIDE_DOWN

#define BOOST_GEOMETRY_DISABLE_DEPRECATED_03_WARNING
#include "cairoItems.h"
#include <pango.h>                                // for Pango
#include <boost/geometry/core/static_assert.hpp>  // for geometry
#include <boost/geometry/geometry.hpp>  // lines 26-26
#include <boost/locale/encoding_utf.hpp>          // for utf_to_utf
#include <cmath>                                  // for cos, sin, isnan, M_PI
#include <iosfwd>                                 // for std
#include <limits>                                 // for numeric_limits
#include <string>                                 // for operator+
#include "cairo.h"                                // for cairo_line_to, cair...
#include "cairo_base.h"                           // for Surface
#include "error.h"                                // for ecolab, error
#include "geometry.h"                             // for minsky
#include "latexMarkup.h"                          // for latexToPango
#include "plot.h"                                 // for Colour
#include "variable.h"                             // for VariableBase
#include "variableType.h"                         // for VariableType, Varia...
#include "variableValue.h"                        // for EngNotation, expMul...
#include "wire.h"                                 // for error

#include "lassoBox.h"
#include "selection.h"
#include "SVGItem.h"
#include "minsky_epilogue.h"

#include <boost/locale.hpp>
using boost::locale::conv::utf_to_utf;

using namespace ecolab;
using namespace std;
using namespace minsky;
using namespace boost::geometry;

namespace
{
  cairo::Surface dummySurf(cairo_image_surface_create(CAIRO_FORMAT_A1, 100,100));
}

RenderVariable::RenderVariable(const VariableBase& var, cairo_t* cairo):
  Pango(cairo? cairo: dummySurf.cairo()), var(var), cairo(cairo)
{
  setFontSize(12);
  if (var.type()==VariableType::constant)
    {
      try
        {
          auto val=var.engExp();
          if (val.engExp==-3) val.engExp=0; //0.001-1.0
          setMarkup(var.mantissa(val)+expMultiplier(val.engExp));
        }
      catch (const error&)
        {
          setMarkup("0");
        }
      w=0.5*Pango::width();
      h=0.5*Pango::height();
    }
  else
    {
      setMarkup(latexToPango(var.name()));
      w=0.5*Pango::width(); 
      h=0.5*Pango::height();
      if (!var.ioVar())
        { // add additional space for numerical display 
          w+=12; 
          h+=4;
        }
    }
  hoffs=Pango::top();
}

void RenderVariable::draw()
{
  var.draw(cairo);

}

bool RenderVariable::inImage(float x, float y)
{
  float dx=x-var.x(), dy=y-var.y();
  float rx=dx*cos(var.rotation()*M_PI/180)-dy*sin(var.rotation()*M_PI/180);
  float ry=dy*cos(var.rotation()*M_PI/180)+dx*sin(var.rotation()*M_PI/180);
  return rx>=-w && rx<=w && ry>=-h && ry <= h;
}

double RenderVariable::handlePos() const
{
  if (var.sliderStep<std::numeric_limits<double>::min() || std::isnan(var.sliderStep)) var.initSliderBounds();   // this should only be used when sliderStep's value has not been set or is a nonsensicl
  var.adjustSliderBounds();
  return (w<0.5*var.iWidth()? 0.5*var.iWidth() : w)*(var.value()-0.5*(var.sliderMin+var.sliderMax))/(var.sliderMax-var.sliderMin);
}

void minsky::drawTriangle
(cairo_t* cairo, double x, double y, const cairo::Colour& col, double angle)
{
  cairo_save(cairo);
  cairo_new_path(cairo);
  cairo_set_source_rgba(cairo,col.r,col.g,col.b,col.a);
  cairo_translate(cairo,x,y);
  cairo_rotate(cairo, angle);
  cairo_move_to(cairo,10,0);
  cairo_line_to(cairo,0,-3);
  cairo_line_to(cairo,0,3);
  cairo_fill(cairo);
  cairo_restore(cairo);
}

