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
#include <boost/geometry/geometry.hpp>

#include "minsky.h"

#include "cairoItems.h"
#include "operation.h"
#include "latexMarkup.h"
#include <arrays.h>
#include <pango.h>
#include "minsky_epilogue.h"

#include <boost/locale.hpp>

using namespace ecolab;
using ecolab::cairo::CairoSave;
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
      catch (const std::exception& ex)
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
  const float dx=x-var.x(), dy=y-var.y();
  const float rx=dx*cos(var.rotation()*M_PI/180)-dy*sin(var.rotation()*M_PI/180);
  const float ry=dy*cos(var.rotation()*M_PI/180)+dx*sin(var.rotation()*M_PI/180);
  return rx>=-w && rx<=w && ry>=-h && ry <= h;
}

double RenderVariable::handlePos() const
{
  if (var.sliderStep()<std::numeric_limits<double>::min() || std::isnan(var.sliderStep())) var.initSliderBounds();   // this should only be used when sliderStep's value has not been set or is a nonsensicl
  var.adjustSliderBounds();
  return (w<0.5*var.iWidth()? 0.5*var.iWidth() : w)*(var.value()-0.5*(var.sliderMin()+var.sliderMax()))/(var.sliderMax()-var.sliderMin());
}

void minsky::drawTriangle
(cairo_t* cairo, double x, double y, const cairo::Colour& col, double angle)
{
  const CairoSave cs(cairo);
  cairo_new_path(cairo);
  cairo_set_source_rgba(cairo,col.r,col.g,col.b,col.a);
  cairo_translate(cairo,x,y);
  cairo_rotate(cairo, angle);
  cairo_move_to(cairo,10,0);
  cairo_line_to(cairo,0,-3);
  cairo_line_to(cairo,0,3);
  cairo_fill(cairo);
}

