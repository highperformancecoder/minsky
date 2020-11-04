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

void Sheet::draw(cairo_t* cairo) const
{
  auto z=zoomFactor();
  ports[0]->moveTo(x()-0.5*m_width*z,y());
  if (mouseFocus)
    {
      drawPorts(cairo);
      displayTooltip(cairo,tooltip);
      // Resize handles always visible on mousefocus. For ticket 92.
      drawResizeHandles(cairo);
    }

  cairo_scale(cairo,z,z);
    
  cairo_rectangle(cairo,-0.5*m_width,-0.5*m_height,m_width,m_height);
  cairo_stroke_preserve(cairo);
  cairo_clip(cairo);
  if (selected) drawSelected(cairo);

  try
    {
      auto value=ports[0]->getVariableValue();
      if (!value) return;
      Pango pango(cairo);
      if (value->hypercube().rank()>2)
        {
          pango.setMarkup("Error: rank>2");
          cairo_move_to(cairo,-0.5*pango.width(),-0.5*pango.height());
          pango.show();
        }
      else
        {
          float x0=-0.5*m_width, y0=-0.5*m_height;//+pango.height();
          if (value->hypercube().rank()==0)
            {
              cairo_move_to(cairo,x0,y0);
              pango.setMarkup(str((*value)[0]));
              pango.show();
            }
          else
            {
              if (!value->hypercube().xvectors[0].name.empty())
                {
                  cairo::CairoSave cs(cairo);
                  pango.setMarkup(value->hypercube().xvectors[0].name);
                  x0+=pango.height();
                  cairo_move_to(cairo,x0, -0.5*pango.width());
                  pango.angle=0.5*M_PI;
                  pango.show();
                  pango.angle=0;
                  { // draw vertical grid line
                    cairo::CairoSave cs(cairo);
                    cairo_set_source_rgba(cairo,0,0,0,0.5);
                    cairo_move_to(cairo,x0,-0.5*m_height);
                    cairo_line_to(cairo,x0,0.5*m_height);
                    cairo_stroke(cairo);
                  }                  				
                }

              pango.setMarkup("9999");
              float rowHeight=pango.height();

              double colWidth=0;
              float x=x0, y=y0;
              string format=value->hypercube().xvectors[0].dimension.units;
              // calculate label column width
              for (auto& i: value->hypercube().xvectors[0])
                {
                  pango.setText(trimWS(str(i,format)));
                  colWidth=std::max(colWidth,5+pango.width()/z);
                }                

              if (value->hypercube().rank()==2)
                {
                  y+=rowHeight; // allow room for header row               
                  if (!value->hypercube().xvectors[1].name.empty())
                    {
                      cairo::CairoSave cs(cairo);
                      pango.setMarkup(value->hypercube().xvectors[1].name);
                      cairo_move_to(cairo,0.5*(x0+colWidth+0.5*m_width-pango.width()), y0);
                      y0+=pango.height();
                      pango.show();
                    }
                }
              
              { // draw horizontal grid line
                cairo::CairoSave cs(cairo);
                cairo_set_source_rgba(cairo,0,0,0,0.5);
                cairo_move_to(cairo,-0.5*m_width,y0-2.5);
                cairo_line_to(cairo,0.5*m_width,y0-2.5);
                cairo_stroke(cairo);
              }                    
              // draw in label column
              for (auto& i: value->hypercube().xvectors[0])
                {
                  cairo_move_to(cairo,x,y);
                  pango.setText(trimWS(str(i,format)));
                  pango.show();
                  y+=rowHeight;
                }                
              y=y0;          
              x+=colWidth;            
              if (value->hypercube().rank()==1)
                {
                  { // draw vertical grid line
                    cairo::CairoSave cs(cairo);
                    cairo_set_source_rgba(cairo,0,0,0,0.5);
                    cairo_move_to(cairo,x,-0.5*m_height);
                    cairo_line_to(cairo,x,0.5*m_height);
                    cairo_stroke(cairo);
                  }                  				
                  for (size_t i=0; i<value->size(); ++i)
                    {
                      if (!value->index().empty())
                        y=y0+value->index()[i]*rowHeight;
                      cairo_move_to(cairo,x,y);
                      auto v=(*value)[i];
                      if (!std::isnan(v))
                        {
                          pango.setMarkup(str(v));
                          pango.show();
                        }                       
                      y+=rowHeight;
                    }
                }
              else
                {
                  format=value->hypercube().xvectors[1].dimension.units;
                  auto dims=value->hypercube().dims();
                  for (size_t i=0; i<dims[1]; ++i)
                    {
                      colWidth=0;
                      y=y0;
                      cairo_move_to(cairo,x,y);
                      pango.setText(trimWS(str(value->hypercube().xvectors[1][i],format)));
                      pango.show();
                      { // draw vertical grid line
                        cairo::CairoSave cs(cairo);
                        cairo_set_source_rgba(cairo,0,0,0,0.5);
                        cairo_move_to(cairo,x-2.5,y0);
                        cairo_line_to(cairo,x-2.5,0.5*m_height);
                        cairo_stroke(cairo);
                      }
                      colWidth=std::max(colWidth, 5+pango.width()/z);
                      for (size_t j=0; j<dims[0]; ++j)
                        {
                          y+=rowHeight;
                          if (y>0.5*m_height) break;
                          cairo_move_to(cairo,x,y);
                          auto v=value->atHCIndex(j+i*dims[0]);
                          if (!std::isnan(v))
                            {
                              pango.setText(str(v));
                              pango.show();
                            }
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
                    cairo_rectangle(cairo,x0,y,m_width,rowHeight);
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
