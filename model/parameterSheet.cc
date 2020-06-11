/*
  @copyright Steve Keen 2019
  @author Russell Standish
  @author Wynand Dednam
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
#include "parameterSheet.h"
#include "latexMarkup.h"
#include "selection.h"
#include "group.h"
#include <pango.h>
#include "minsky_epilogue.h"
#include "minsky.h"
using namespace std;
using ecolab::cairo::Surface;
using ecolab::Pango;
using ecolab::cairo::CairoSave;

namespace minsky{
	
namespace
{    
  struct CroppedPango: public Pango
  {
    cairo_t* cairo;
    double w, x=0, y=0;
    CroppedPango(cairo_t* cairo, double width): Pango(cairo), cairo(cairo), w(width) {}
    void setxy(double xx, double yy) {x=xx; y=yy;}
    void show() {
      CairoSave cs(cairo);
      cairo_rectangle(cairo,x,y,w,height());
      cairo_clip(cairo);
      cairo_move_to(cairo,x,y);
      Pango::show();
    }
  };
}

void ParameterSheet::populateItemvector() {
  itemvector.clear();	
  minsky().canvas.selection.recursiveDo(&GroupItems::items,
                                        [&](Items&, Items::iterator i) {                                 
                                          itemvector.emplace_back(*i);
                                          return false;
                                        });   	
}	

void ParameterSheet::draw(cairo_t* cairo) const
{   
  cairo_rectangle(cairo,0,0,m_width,m_height);
  cairo_stroke_preserve(cairo);
  cairo_clip(cairo);

  try
    {	
      		
      //auto& value=ports[0]->getVariableValue();
      if (!itemvector.empty())
      //if (item)
        if (auto v=itemvector[0]->variableCast())
        //if (auto v=item->variableCast())
          if (v->type()==VariableType::parameter)
            {
	          auto value=v->vValue();
              Pango pango(cairo);
              if (value->hypercube().rank()>2)
                {
                  pango.setMarkup("Error: rank>2");
                  cairo_move_to(cairo,0.5*pango.width(),0.5*pango.height());
                  pango.show();
                }
              else
                {
                  float x0=0.0, y0=0.0;//+pango.height();
                  float x=x0, y=y0;
                  double colWidth=0;
                  pango.setMarkup("9999");
                  //float rowHeight=pango.height();
                  if (value->hypercube().rank()==0)
                    {
                      cairo_move_to(cairo,x,y);
                      pango.setMarkup(str(value->value(0)));
                      pango.show();
                    }
                  else
                    {
                      if (value->hypercube().rank()==2)
                        y+=rowHeight; // allow room for header row

                      // draw in label column
                      string format=value->hypercube().xvectors[0].timeFormat();
                      for (auto& i: value->hypercube().xvectors[0])
                        {
                          cairo_move_to(cairo,x,y);
                          pango.setText(trimWS(str(i,format)));
                          pango.show();
                          y+=rowHeight;
                          colWidth=std::max(colWidth,5+pango.width());
                        }
                      y=y0;
                      x+=colWidth;
                      if (value->hypercube().rank()==1)
                        for (size_t i=0; i<value->size(); ++i)
                          {
                            if (!value->index().empty())
                              y=y0+value->index()[i]*rowHeight;
                            cairo_move_to(cairo,x,y);
                            auto v=value->value(i);
                            if (!std::isnan(v))
                              {
                                pango.setMarkup(str(v));
                                pango.show();
                              }
                            y+=rowHeight;
                          }
                      else
                        {
                          format=value->hypercube().xvectors[1].timeFormat();
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
                                cairo_move_to(cairo,x-2.5,0.0);
                                cairo_line_to(cairo,x-2.5,m_height);
                                cairo_stroke(cairo);
                              }
                              colWidth=std::max(colWidth, 5+pango.width());
                              for (size_t j=0; j<dims[0]; ++j)
                                {
                                  y+=rowHeight;
                                  if (y>1.5*m_height) break;
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
                              if (x>1.5*m_width) break;
                            }
                        }
                      // draw grid
                      {
                        cairo::CairoSave cs(cairo);
                        cairo_set_source_rgba(cairo,0,0,0,0.2);
                        for (y=y0+0.8*rowHeight; y<1.5*m_height; y+=2*rowHeight)
                          {
                            cairo_rectangle(cairo,0.0,y,1.5*m_width,rowHeight);
                            cairo_fill(cairo);
                          }
                      }
                
                    }
                }
            } 
    }
  catch (...) {throw;/* exception most likely invalid variable value */}
}    

void ParameterSheet::redraw(int, int, int width, int height)
{
  cairo_t* cairo=surface->cairo();
  CroppedPango pango(cairo, colWidth);
  rowHeight=15;
  pango.setFontSize(5.0*rowHeight);
  
  {
    //Pango pango(cairo);
    //pango.setText("Watch this space");
    //cairo_move_to(cairo,0.5*m_width,0);
    //pango.show();
    if (!minsky().canvas.selection.empty()) {	  
      populateItemvector();			
    }
    
    draw(cairo);  
  }
}

}
