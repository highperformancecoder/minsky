/*
  @copyright Steve Keen 2018
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
#include "sheet.h"
#include "str.h"
#include "selection.h"
#include <cairo_base.h>
#include <pango.h>
#include "minsky_epilogue.h"

using namespace minsky;
using namespace ecolab;

Sheet::Sheet()
{
  ports.emplace_back(new Port(*this, Port::inputPort));
}

ClickType::Type Sheet::clickType(float x, float y)
{
  double dx=x-this->x(), dy=y-this->y();
  auto z=zoomFactor();
  double w=0.5*m_width*z, h=0.5*m_height*z;
  // check if (x,y) is within portradius of the 4 corners
  if (fabs(fabs(dx)-w) < portRadius*z &&
      fabs(fabs(dy)-h) < portRadius*z &&
      fabs(hypot(dx,dy)-hypot(w,h)) < portRadius*z)
    return ClickType::onResize;
  return Item::clickType(x,y);
}

void Sheet::draw(cairo_t* cairo) const
{
  auto z=zoomFactor();
  ports[0]->moveTo(x()-0.5*m_width*z,y());
  if (mouseFocus)
    {
      drawPorts(cairo);
      displayTooltip(cairo,tooltip);
    }
  if (onResizeHandles) drawResizeHandles(cairo);

  cairo_scale(cairo,z,z);
    
  cairo_rectangle(cairo,-0.5*m_width,-0.5*m_height,m_width,m_height);
  cairo_stroke_preserve(cairo);
  cairo_clip(cairo);
  if (selected) drawSelected(cairo);

  try
    {
      auto& value=ports[0]->getVariableValue();
      Pango pango(cairo);
      if (value.xVector.size()>2)
        {
          pango.setMarkup("Error: rank>2");
          cairo_move_to(cairo,-0.5*pango.width(),-0.5*pango.height());
          pango.show();
        }
      else
        {
          float x0=-0.5*m_width, y0=-0.5*m_height;//+pango.height();
          float x=x0, y=y0;
          double colWidth=0;
          pango.setMarkup("9999");
          float rowHeight=pango.height();
          if (value.xVector.empty())
            {
              cairo_move_to(cairo,x,y);
              pango.setMarkup(str(value.value()));
              pango.show();
            }
          else
            {
              if (value.xVector.size()==2)
                y+=rowHeight; // allow room for header row

              // draw in label column
              string format=value.xVector[0].timeFormat();
              for (auto& i: value.xVector[0])
                {
                  cairo_move_to(cairo,x,y);
                  pango.setText(trimWS(str(i,format)));
                  pango.show();
                  y+=rowHeight;
                  colWidth=std::max(colWidth,5+pango.width()/z);
                }
              y=y0;
              x+=colWidth;
              if (value.xVector.size()==1)
                for (auto v: value)
                  {
                    cairo_move_to(cairo,x,y);
                    pango.setMarkup(str(v));
                    pango.show();
                    y+=rowHeight;
                  }
              else
                {
                  format=value.xVector[1].timeFormat();
                  auto dims=value.dims();
                  for (size_t i=0; i<dims[1]; ++i)
                    {
                      colWidth=0;
                      y=y0;
                      cairo_move_to(cairo,x,y);
                      pango.setText(trimWS(str(value.xVector[1][i],format)));
                      pango.show();
                      { // draw vertical grid line
                        cairo::CairoSave cs(cairo);
                        cairo_set_source_rgba(cairo,0,0,0,0.5);
                        cairo_move_to(cairo,x-2.5,-0.5*m_height);
                        cairo_line_to(cairo,x-2.5,0.5*m_height);
                        cairo_stroke(cairo);
                      }
                      colWidth=std::max(colWidth, 5+pango.width()/z);
                      for (size_t j=0; j<dims[0]; ++j)
                        {
                          y+=rowHeight;
                          if (y>0.5*m_height) break;
                          cairo_move_to(cairo,x,y);
                          pango.setText(str(value.value(j+i*dims[0])));
                          pango.show();
                          colWidth=std::max(colWidth, pango.width());
                        }
                      x+=colWidth;
                      if (x>0.5*m_width) break;
                    }
                }
              // draw grid
              {
                cairo::CairoSave cs(cairo);
                cairo_set_source_rgba(cairo,0,0,0,0.2);
                for (y=y0+0.8*rowHeight; y<0.5*m_height; y+=2*rowHeight)
                  {
                    cairo_rectangle(cairo,-0.5*m_width,y,m_width,rowHeight);
                    cairo_fill(cairo);
                 }
              }
                
            }
        }
    }
  catch (...) {/* exception most likely invalid variable value */}
}

void Sheet::resize(const LassoBox& b)
{
  auto invZ=1/zoomFactor();
  m_width=abs(b.x1-b.x0)*invZ;
  m_height=abs(b.y1-b.y0)*invZ;
  moveTo(0.5*(b.x0+b.x1), 0.5*(b.y0+b.y1));
  bb.update(*this);
}
