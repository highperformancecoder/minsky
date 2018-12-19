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
#include <ecolab_epilogue.h>

using namespace minsky;
using namespace ecolab;

Sheet::Sheet()
{
  ports.emplace_back(new Port(*this, Port::inputPort));
}

void Sheet::draw(cairo_t* cairo) const
{
  if (mouseFocus)
    {
      drawPorts(cairo);
      displayTooltip(cairo,tooltip);
    }

  cairo::CairoSave cs(cairo);
  cairo_scale(cairo,zoomFactor,zoomFactor);
    
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
          float x0=-0.5*m_width, y0=-0.5*m_height+pango.height();
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
                  pango.setMarkup(str(i,format));
                  pango.show();
                  y+=rowHeight;
                  colWidth=std::max(colWidth,pango.width());
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
                      pango.setMarkup(str(value.xVector[1][i],format));
                      pango.show();
                      colWidth=std::max(colWidth, pango.width());
                      for (size_t j=0; j<dims[0]; ++j)
                        {
                          y+=rowHeight;
                          if (y>0.5*m_height) break;
                          cairo_move_to(cairo,x,y);
                          pango.setMarkup(str(value.xVector[1][i],format));
                          pango.show();
                          colWidth=std::max(colWidth, pango.width());
                        }
                      x+=colWidth;
                      if (x>0.5*m_width) break;
                    }
                }
            }
        }
    }
  catch (...) {/* exception most like invalid variable value */}
}
