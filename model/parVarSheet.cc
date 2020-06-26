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
#include "parVarSheet.h"
#include "latexMarkup.h"
#include "group.h"
#include <pango.h>
#include "minsky_epilogue.h"
#include "minsky.h"
using namespace std;
using ecolab::cairo::Surface;
using ecolab::Pango;
using ecolab::cairo::CairoSave;

namespace minsky
{
	
  void ParVarSheet::populateItemVector() {
    itemVector.clear();	
    minsky().canvas.model->recursiveDo(&GroupItems::items,
                                       [&](Items&, Items::iterator i) {                                 
                                         if (variableSelector(*i))		                                 
                                           itemVector.emplace_back(*i);
                                         return false;
                                       });   	
  }	
	
  void ParVarSheet::draw(cairo_t* cairo) const
  {   
    try
      {	
      		
        if (!itemVector.empty())
          {
            float x0=0.0, y0=0.0;//+pango.height();	
            double w,h,h_prev,lh; 
            for (auto& it: itemVector)
              {
                auto value=it->variableCast()->vValue();
                Pango pango(cairo);
                if (value->hypercube().rank()>2)
                  {
                    pango.setMarkup("Error: rank>2");
                    cairo_move_to(cairo,0.5*pango.width(),0.5*pango.height());
                    pango.show();
                  }
                else
                {  	 
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
                          {
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
                          }
                        else
                          {
                            format=value->hypercube().xvectors[1].timeFormat();
                            auto dims=value->hypercube().dims();
                            for (size_t i=0; i<dims[1]; ++i)
                              {
                                colWidth=0;
                                y=y0;
                                lh=0;
                                cairo_move_to(cairo,x,y);
                                for (size_t j=0; j<dims[0]; ++j)
                                  lh+=rowHeight;
                                pango.setText(trimWS(str(value->hypercube().xvectors[1][i],format)));
                                pango.show();
                                { // draw vertical grid line
                                  cairo::CairoSave cs(cairo);
                                  cairo_set_source_rgba(cairo,0,0,0,0.5);
                                  cairo_move_to(cairo,x-2.5,y0);
                                  cairo_line_to(cairo,x-2.5,y0+lh+1.1*rowHeight);
                                  cairo_stroke(cairo);
                                }
                                colWidth=std::max(colWidth, 5+pango.width());
                                for (size_t j=0; j<dims[0]; ++j)
                                  {
                                    y+=rowHeight;
                                    if (y>20000) break;
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
                                if (x>20000) break;
                              }      
                          }
                        h_prev=h;  
                        w=0;h=0;      
                        cairo_get_current_point (cairo,&w,&h);   
                        if (h<h_prev) h+=h_prev;                                                                          
                        //cout << " " << w << " " << h << " " << y0 << " " << h_prev << endl;
                        // draw grid
                        {
                          cairo::CairoSave cs(cairo);
                          cairo_set_source_rgba(cairo,0,0,0,0.2);
                          for (y=y0+rowHeight; y<h+rowHeight; y+=2*rowHeight)
                            {
                              cairo_rectangle(cairo,0.0,y,w+colWidth,rowHeight);
                              cairo_fill(cairo);
                            }
                        }
                
                      }
                    auto rank=value->hypercube().rank();
                    if (rank>0) {  
                      cairo::CairoSave cs(cairo);
                      auto dims=value->hypercube().dims();    
                      //cout << " " << dims[dims.size()-1] << " " << rank << " " << y0 << endl;
                      //cout << " " << width() << " " << height() << endl;
                      float rectHeight=0;
                      // make sure rectangle has right height
                      if ((dims[dims.size()-1])%2!=0 || rank%2==0) rectHeight= y-y0;
                      else rectHeight=y-y0-rowHeight;
                      cairo_rectangle(cairo,x0,y0,w+colWidth,rectHeight);    
                      cairo_rectangle(cairo,x0,y0,w+colWidth,y-y0);    
                      cairo_stroke(cairo);                          
                      cairo_clip(cairo);
                      y0=h+2.1*rowHeight;
				    } else y0+=2.1*rowHeight;
                       
                  }    
              }
          }
      }
    catch (...) {throw;/* exception most likely invalid variable value */}
  } 

}
