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
#include "lasso.h"
#include "plotWidget.h"
#include <cairo_base.h>
#include <pango.h>
#include "minsky_epilogue.h"

using namespace minsky;
using namespace ecolab;
using namespace std;

// width of draggable border 
const float border=10;

Sheet::Sheet()
{
  m_ports.emplace_back(make_shared<InputPort>(*this));
  iWidth(100);
  iHeight(100);	  
}

bool Sheet::inItem(float xx, float yy) const
{
  auto z=zoomFactor();	 					
  return abs(xx-x())<0.5*width()-border*z && abs(yy-y())<0.5*height()-border*z;
}

ClickType::Type Sheet::clickType(float x, float y)
{
  double dx=fabs(x-this->x()), dy=fabs(y-this->y());
  double w=0.5*width(), h=0.5*height();  
  if (onResizeHandle(x,y)) return ClickType::onResize;
  if (inItem(x,y)) return ClickType::inItem;                     
  if (dx < w && dy < h)
    return ClickType::onItem;
  return ClickType::outside;  
  if (auto item=select(x,y))
    return item->clickType(x,y);      
  return Item::clickType(x,y);  
}

namespace
{
  struct ElisionRowChecker
  {
    size_t startRow, numRows, tailStartRow;
    double rowHeight;
    ElisionRowChecker(ShowSlice slice, double height, double rowHeight, size_t size): rowHeight(rowHeight) {
      switch (slice)
        {
        case ShowSlice::head:
          tailStartRow=startRow=0;
          numRows=height/rowHeight+1;
          break;
        case ShowSlice::headAndTail:
          startRow=0;
          numRows=0.5*height/rowHeight;
          tailStartRow=size-numRows - ((tailStartRow+numRows)*rowHeight>height? -1: 0);
          break;
        case ShowSlice::tail:
          numRows=height/rowHeight-1;
          tailStartRow=startRow=size-numRows;
          break;
        }
    }
    bool operator()(size_t& row, double& y) {
      if (row==startRow+numRows && row<tailStartRow)
        {
          row=tailStartRow; // for middle elision
          y+=rowHeight;
        }
      return row>tailStartRow+numRows;
    }
  };
}

void Sheet::draw(cairo_t* cairo) const
{
  auto z=zoomFactor();
  m_ports[0]->moveTo(x()-0.5*m_width*z,y());
  if (mouseFocus)
    {
      drawPorts(cairo);
      displayTooltip(cairo,tooltip);
      // Resize handles always visible on mousefocus. For ticket 92.
      drawResizeHandles(cairo);
    }

  cairo_scale(cairo,z,z);
    
  cairo_rectangle(cairo,-0.5*m_width+border,-0.5*m_height+border,m_width-2*border,m_height-2*border);
  cairo_stroke_preserve(cairo);
  cairo_rectangle(cairo,-0.5*m_width,-0.5*m_height,m_width,m_height);
  cairo_stroke_preserve(cairo);
  // draw border
  if (onBorder)
    { // shadow the border when mouse is over it
      cairo::CairoSave cs(cairo);
      cairo_set_source_rgba(cairo,0.5,0.5,0.5,0.5);
      cairo_set_fill_rule(cairo,CAIRO_FILL_RULE_EVEN_ODD);
      cairo_fill(cairo);
    }
  cairo_new_path(cairo);
  cairo_rectangle(cairo,-0.5*m_width+border,-0.5*m_height+border,m_width-2*border,m_height-2*border);
  cairo_clip(cairo);
  
  if (selected) drawSelected(cairo);

  try
    {
      auto value=m_ports[0]->getVariableValue();
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
          double x0=-0.5*m_width+border, y0=-0.5*m_height+border;
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
              double rowHeight=pango.height();

              double colWidth=0;
              double x=x0, y=y0;   // make sure row labels are aligned with corresponding values. for ticket 1281
              string format=value->hypercube().xvectors[0].dimension.units;
              // calculate label column width
              for (auto& i: value->hypercube().xvectors[0])
                {
                  pango.setText(trimWS(str(i,format)));
                  colWidth=std::max(colWidth,5+pango.width());
                }                

              if (value->hypercube().rank()==2)
                {
                  y+=2*rowHeight; // allow room for header row               
                  if (!value->hypercube().xvectors[1].name.empty())
                    {
                      cairo::CairoSave cs(cairo);
                      pango.setMarkup(value->hypercube().xvectors[1].name);
                      cairo_move_to(cairo,0.5*(x0+colWidth+0.5*m_width-pango.width()), y0);
                      y0+=pango.height();
                      pango.show();
                    }
              
                  { // draw horizontal grid lines
                    cairo::CairoSave cs(cairo);
                    cairo_set_source_rgba(cairo,0,0,0,0.5);
                    cairo_move_to(cairo,-0.5*m_width,y0-2.5);
                    cairo_line_to(cairo,0.5*m_width,y0-2.5);
                    cairo_stroke(cairo);
                    cairo_move_to(cairo,x,y0+rowHeight-2.5);
                    cairo_line_to(cairo,0.5*m_width,y0+rowHeight-2.5);
                    cairo_stroke(cairo);
                    
                  }
                }

              auto dims=value->hypercube().dims();
              double dataHeight=m_height-(dims.size()==2?2*rowHeight:0);
              ElisionRowChecker adjustRowAndFinish(showSlice,dataHeight,rowHeight,dims[0]);
              
              // draw in label column
              auto& xv=value->hypercube().xvectors[0];
              for (size_t i=adjustRowAndFinish.startRow; i<xv.size(); ++i)
                {
                  if (adjustRowAndFinish(i,y)) break;
                  cairo_move_to(cairo,x,y);
                  pango.setText(trimWS(str(xv[i],format)));
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
                  for (size_t i=adjustRowAndFinish.startRow; i<value->size(); ++i)
                    {
                      if (adjustRowAndFinish(i,y)) break;
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
                      colWidth=std::max(colWidth, 5+pango.width());
                      for (size_t j=adjustRowAndFinish.startRow; j<dims[0]; ++j)
                        {
                          if (adjustRowAndFinish(j,y)) break;
                          y+=rowHeight;
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
  cairo_reset_clip(cairo);
  cairo_rectangle(cairo,-0.5*m_width,-0.5*m_height,m_width,m_height);
  cairo_clip(cairo);
}

