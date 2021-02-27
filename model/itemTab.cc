/*
  @copyright Steve Keen 2020
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
#include "itemTab.h"
#include <assert.h>         // for assert
#include <pango.h>          // for Pango
#include <stddef.h>         // for size_t, NULL
#include <algorithm>        // for max, upper_bound
#include <cmath>            // for isnan, fabs
#include <limits>           // for numeric_limits
#include <sstream>          // for ostringstream, ostream, std
#include <utility>          // for make_pair
#include "canvas.h"         // for Canvas, Canvas::Model
#include "dimension.h"      // for Dimension
#include "equations.h"      // for VariableDAG, SystemOfEquations, WeakNodePtr
#include "geometry.h"       // for sqr
#include "group.h"          // for Group, GroupItems, GroupItems::items, Groups
#include "hypercube.h"      // for Hypercube
#include "index.h"          // for Index
#include "latexMarkup.h"    // for latexToPango
#include "minsky.h"         // for minsky, cminsky, Minsky
#include "str.h"            // for str, trimWS
#include "variable.h"       // for VariableBase
#include "variableType.h"   // for VariableType, VariableType::parameter
#include "variableValue.h"  // for VariableValue
#include "xvector.h"        // for str, XVector

#include <capiRenderer.h>
#include "minsky_epilogue.h"
using namespace std;
using namespace MathDAG;
using ecolab::cairo::Surface;
using ecolab::Pango;
using ecolab::cairo::CairoSave;

namespace minsky
{

  void ItemTab::populateItemVector() {
    itemVector.clear();	
    minsky().canvas.model->recursiveDo(&GroupItems::items,
                                       [&](Items&, Items::iterator i) {                                 
                                         if (itemSelector(*i)) 
                                           {		                                 
                                             itemVector.emplace_back(*i);
                                             itemCoords.emplace(make_pair(*i,make_pair((*i)->x(),(*i)->y()))); 
                                           }
                                         return false;
                                       });   	
  }
  
  int ItemTab::colX(double x) const
  { 
    if (itemVector.empty() || colLeftMargin.empty()) return -1;
    size_t c=-1;
    for (auto& i: colLeftMargin)
      {
        auto p=std::upper_bound(i.second.begin(), i.second.end(), (x-offsx));
        c=p-i.second.begin();
      }
    if (c<0) c=-1; // out of bounds, invalidate
    return c;
  }

  int ItemTab::rowY(double y) const
  {
    if (itemVector.empty() || rowTopMargin.empty()) return -1;     
    auto p=std::upper_bound(rowTopMargin.begin(), rowTopMargin.end(), (y-offsy));
    size_t r=p-rowTopMargin.begin(); 
    if (r<0) r=-1; // out of bounds, invalidate
    return r;
  }
  
  void ItemTab::moveTo(float x, float y)
  {   
    if (itemFocus) {    
      xItem=x;	                               
      yItem=y;
      assert(abs(x-xItem)<1 && abs(y-yItem)<1);
    }
  }  
  
  ItemTab::ClickType ItemTab::clickType(double x, double y) const
  {
    int c=colX(x), r=rowY(y);

    if (c>=0 && c<int(colLeftMargin.size())&& r>=0 && r<int(itemVector.size()))
      return internal;
        
    if (itemFocus) return internal;   
  
    return background;
  }
      
  void ItemTab::mouseDownCommon(float x, float y)
  {
    if (itemFocus=itemAt(x,y))
      switch (clickType(x,y))
        {
        case internal:
          moveOffsX=x-itemCoords[itemFocus].first;
          moveOffsY=y-itemCoords[itemFocus].second;
          break;
        case background:
          itemFocus.reset();
          break;
        default:
          break;  
        }
  }  
  
  void ItemTab::mouseUp(float x, float y)
  {
    mouseMove(x,y);
    itemFocus.reset();
  }
  
  void ItemTab::mouseMove(float x, float y)
  {
	    
    try
      {
        if (itemFocus)
          switch (clickType(x,y))
            {
            case internal:
              moveTo(x-moveOffsX,y-moveOffsY);
              requestRedraw();
              return;
            default:
              break;
            }
      }
    catch (...) {/* absorb any exceptions, as they're not useful here */}  
  }
  
  void ItemTab::displayDelayedTooltip(float x, float y)
  {
    if (auto item=itemAt(x,y))
      {
        item->displayDelayedTooltip(x,y);
        requestRedraw();
      }
  } 
  
  ItemPtr ItemTab::itemAt(float x, float y)
  {
    ItemPtr item;                    
    auto minD=numeric_limits<float>::max();
    for (auto& i: itemCoords)
      {
        float xx=(i.second).first+offsx, yy=(i.second).second+offsy;  
        float d=sqr(xx-x)+sqr(yy-y);
        float z=i.first->zoomFactor();
        float w=0.5*i.first->iWidth()*z,h=0.5*i.first->iHeight()*z;
        if (d<minD && fabs(xx-x)<w && fabs(yy-y)<h)
          {
            minD=d;
            item=i.first;
          }
      }
    
    return item;
  }
  
  namespace
  {
 
    std::string definition(const VariableBase& v)
    {
      SystemOfEquations system(cminsky());	  
      ostringstream o;
	
      auto varDAG=system.getNodeFromVar(v);
    
      if (v.type()!=VariableType::parameter)
        {
          if (varDAG && varDAG->rhs && varDAG->type!=VariableType::constant && varDAG->type!=VariableType::integral)
            o << varDAG->rhs->matlab();
	  else return system.getDefFromIntVar(v).str();
        }
          
      return o.str();	  
    }
  }  
	
  void ItemTab::draw(cairo_t* cairo)
  {   
    try
      {	
      		
        if (!itemVector.empty())
          {
            float x0, y0=1.5*rowHeight;//+pango.height();	
            double w=0,h=0,h_prev,lh; 
            colLeftMargin.clear();                
            rowTopMargin.clear();
            std::string def;
            int iC=0;  // keep track of number of Pars and Vars as distinct from Godleys and Plots
            size_t lastRank=1; // needed to space parameters and variables of different rank properly on the tabs.                                            
            for (auto& it: itemVector)
              {
                if (auto v=it->variableCast())
                  {
                    auto value=v->vValue();
                    auto rank=value->hypercube().rank();
                    auto dims=value->hypercube().dims();                
                    Pango pango(cairo);      
                    x0=0.0;
                    if (rank>0 && lastRank==0) y0+=2*rowHeight;                    
                    float x=x0, y=y0;
                    def=definition(*v);                      
                    pango.setMarkup("9999");
                    if (rank==0)
                      {   
                        varAttribVals.clear();
                        varAttribVals.push_back(v->name());
                        varAttribVals.push_back(def);                    
                        varAttribVals.push_back(v->init());
                        varAttribVals.push_back(it->tooltip);
                        varAttribVals.push_back(it->detailedText);
                        varAttribVals.push_back(str(v->sliderStep));
                        varAttribVals.push_back(str(v->sliderMin));
                        varAttribVals.push_back(str(v->sliderMax));
                        varAttribVals.push_back(str(v->value()));
                        
                        size_t vACtr=0;                           
                        colWidths.resize(varAttribVals.size());                 
                    
                        if (&it==&itemVector[0] || lastRank>0) {
                          for (auto& i:varAttrib) 
                            {
                              cairo_move_to(cairo,x,y-1.5*rowHeight);                    
                              pango.setMarkup(i);
                              pango.show();                  
                              colWidths[vACtr]=std::max(colWidths[vACtr],5+pango.width());
                              x+=colWidths[vACtr];
                              colLeftMargin[iC].push_back(x);         
                              vACtr++;               				    
                            }
                        }
                        x=0;
                        vACtr=0;                      
                        for (auto& i : varAttribVals)
                          {
                            cairo_move_to(cairo,x,y-0.5*rowHeight);                    
                            pango.setMarkup(latexToPango(i));
                            pango.show();                    
                            colWidths[vACtr]=std::max(colWidths[vACtr],5+pango.width()); 
                            x+=colWidths[vACtr];
                            vACtr++;
                          }
                        x=x0;                      
                        h_prev=h;
                        w=0;h=0;      
                        cairo_get_current_point (cairo,&w,&h);   
                        if (h<h_prev) h+=h_prev;                                                                         
                        // draw grid
                        float y1=(&it==&itemVector[0] || lastRank>0)?-1.5*rowHeight: rowHeight;
                        {
				      		
                          cairo::CairoSave cs(cairo);
                          cairo_set_source_rgba(cairo,0,0,0,0.2);
                          for (y=y0+y1; y<h+rowHeight; y+=2*rowHeight)
                            {
                              cairo_rectangle(cairo,x0,y,w+colWidths.back(),rowHeight);
                              cairo_fill(cairo);
                            }

                        }
                        { // draw vertical grid lines
                          cairo::CairoSave cs(cairo);
                          cairo_set_source_rgba(cairo,0,0,0,0.5);
                          y1=(&it==&itemVector[0] || lastRank>0)? 0.5*rowHeight: 0;
                          cairo_move_to(cairo,x,y-2*rowHeight);
                          cairo_line_to(cairo,x,y+y1);                          
                          cairo_stroke(cairo);                                                        
                          for (auto& i : colWidths)
                            {						
                              x+=i;	
                              cairo_move_to(cairo,x,y-2*rowHeight);
                              cairo_line_to(cairo,x,y+y1);
                              cairo_stroke(cairo);
                            }
                        }
                        if (&it==&itemVector[0] || lastRank>0)                                            
                          { // draw horizontal grid line
                            cairo::CairoSave cs(cairo);
                            cairo_set_source_rgba(cairo,0,0,0,0.5);
                            cairo_move_to(cairo,x0,y0-0.5*rowHeight);
                            cairo_line_to(cairo,w+colWidths.back(),y0-0.5*rowHeight);
                            cairo_stroke(cairo);
                          }                                  
                        cairo::CairoSave cs(cairo);
                        // make sure rows have right height
                        y1=(&it==&itemVector[0] || lastRank>0)? -0.5*rowHeight: 0;
						cairo_rectangle(cairo,x0,y0+y1-rowHeight,w+colWidths.back(),y-y0-2*y1+rowHeight);
                        cairo_stroke(cairo);                          	          
                        cairo_clip(cairo);	                                     
                        rowTopMargin.push_back(y);                        
                      }
                    else if (rank==1)
                      {
                        cairo_move_to(cairo,x,y-1.5*rowHeight);
                        pango.setMarkup(latexToPango(value->name)+"="+latexToPango(def));
                        pango.show();                                  
                        string format=value->hypercube().xvectors[0].dimension.units;
                        for (auto& i: value->hypercube().xvectors[0])
                          {
                            cairo_move_to(cairo,x,y);
                            pango.setText(trimWS(str(i,format)));
                            pango.show();
                            y+=rowHeight;
                            colWidth=std::max(colWidth,5+pango.width());
                          }
                        colLeftMargin[iC].push_back(x);                         
                        y=y0;
                        lh=0;                        
                        for (size_t j=0; j<dims[0]; ++j)
                          lh+=rowHeight;                    
                        { // draw vertical grid line
                          cairo::CairoSave cs(cairo);
                          cairo_set_source_rgba(cairo,0,0,0,0.5);
                          cairo_move_to(cairo,colWidth-2.5,y0);
                          cairo_line_to(cairo,colWidth-2.5,y0+lh);
                          cairo_stroke(cairo);
                        }                                       
                        x+=colWidth;
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
                            colWidth=std::max(colWidth,5+pango.width());                            
                          } 
                        h_prev=h;
                        w=0;h=0;      
                        cairo_get_current_point (cairo,&w,&h);   
                        if (h<h_prev) h+=h_prev;                                                                        
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
                        cairo::CairoSave cs(cairo);
                        float rectHeight=0;
                        // make sure rectangle has right height
                        if ((value->size()&1)!=0) rectHeight= y-y0;
                        else rectHeight=y-y0-rowHeight;                    
                        cairo_rectangle(cairo,0.0,y0,w+colWidth,rectHeight);    
                        cairo_stroke(cairo);                          
                        cairo_clip(cairo);             
                        rowTopMargin.push_back(y);                    
                        colLeftMargin[iC].push_back(x);                   
                        y0=h+3.1*rowHeight;                 
                      }
                    else
                      { 
                        cairo_move_to(cairo,x,y-1.5*rowHeight);
                        pango.setMarkup(latexToPango(value->name)+"="+latexToPango(def));
                        pango.show();                
                        size_t labelDim1=0, labelDim2=1; 					    
                        string vName;
                        if (v->type()==VariableType::parameter)
                          for (size_t k=0; k<rank; k++)  
                            {
                              vName=static_cast<string>(value->hypercube().xvectors[k].name);
                              if (v->getDimLabelsPicked().first==vName) labelDim1=k;
                              if (v->getDimLabelsPicked().second==vName) labelDim2=k;
                              else if (v->getDimLabelsPicked().second=="") labelDim2=labelDim1+1;
                            }
						
                        if ((labelDim1&1)==0) y+=rowHeight; // allow room for header row
                        string format=value->hypercube().xvectors[labelDim1].dimension.units;
                        for (auto& i: value->hypercube().xvectors[labelDim1])
                          {
                            cairo_move_to(cairo,x,y);
                            pango.setText(trimWS(str(i,format)));
                            pango.show();
                            y+=rowHeight;
                            colWidth=std::max(colWidth,5+pango.width());
                          }                                             
                        y=y0;  
                        x+=colWidth;
                        lh=0;                 
                        for (size_t j=0; j<dims[labelDim1]; ++j)
                          lh+=rowHeight;                         
                        format=value->hypercube().xvectors[labelDim2].timeFormat();
                        for (size_t i=0; i<dims[labelDim2]; ++i)
                          {
                            y=y0;
                            cairo_move_to(cairo,x,y);
                            pango.setText(trimWS(str(value->hypercube().xvectors[labelDim2][i],format)));
                            pango.show();
                            { // draw vertical grid line
                              cairo::CairoSave cs(cairo);
                              cairo_set_source_rgba(cairo,0,0,0,0.5);
                              cairo_move_to(cairo,x-2.5,y0);
                              cairo_line_to(cairo,x-2.5,y0+lh+1.1*rowHeight);
                              cairo_stroke(cairo);
                            }
                            colWidth=std::max(colWidth, 5+pango.width());
                            for (size_t j=0; j<dims[labelDim1]; ++j)
                              {
                                y+=rowHeight;
                                if (y>2e09) break;
                                cairo_move_to(cairo,x,y);
                                auto v=value->atHCIndex(j+i*dims[labelDim1]);
                                if (!std::isnan(v))
                                  {
                                    pango.setText(str(v));
                                    pango.show();
                                  }
                                colWidth=std::max(colWidth, pango.width());
                              }
                            colLeftMargin[iC].push_back(x);                     
                            x+=colWidth;
                            if (x>2e09) break;
                          }      
                        h_prev=h;
                        w=0;h=0;      
                        cairo_get_current_point (cairo,&w,&h);   
                        if (h<h_prev) h+=h_prev;                                                                         
                        // draw grid
                        {
				      		
                          cairo::CairoSave cs(cairo);
                          cairo_set_source_rgba(cairo,0,0,0,0.2);
                          for (y=y0+rowHeight; y<h+rowHeight; y+=2*rowHeight)
                            {
                              cairo_rectangle(cairo,x0,y,w+colWidth,rowHeight);
                              cairo_fill(cairo);
                            }
                        }
                        { // draw horizontal grid line
                          cairo::CairoSave cs(cairo);
                          cairo_set_source_rgba(cairo,0,0,0,0.5);
                          cairo_move_to(cairo,x0,y0+1.1*rowHeight);
                          cairo_line_to(cairo,w+colWidth,y0+1.1*rowHeight);
                          cairo_stroke(cairo);
                        }                         
                        cairo::CairoSave cs(cairo);
                        float rectHeight=0;
                        // make sure rectangle has right height
                        if ((labelDim1&1)==0) rectHeight= y-y0;
                        else rectHeight=y-y0-rowHeight;
                        cairo_rectangle(cairo,x0,y0,w+colWidth,rectHeight);    
                        cairo_stroke(cairo);                          	        
                        cairo_clip(cairo);	
                        rowTopMargin.push_back(y);    	        
                        x+=0.25*colWidth;      
                        y=y0;                	
			
						
                      }               
                    if (rank>0) y0=h+4.1*rowHeight;
                    else y0+=2.1*rowHeight;   
                    lastRank=rank;                       
                    iC++;
              
                  }
              }              
          }         
      }
    catch (...) {throw;/* exception most likely invalid variable value */}
  }

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

  void ItemTab::redraw(int, int, int width, int height)
  {
    if (surface.get()) {
      cairo_t* cairo=surface->cairo();  
      CroppedPango pango(cairo, colWidth);
      rowHeight=15;
      pango.setFontSize(5.0*rowHeight);
	    
      if (!minsky().canvas.model->empty()) {	  
        populateItemVector();			               
        cairo_translate(cairo,offsx,offsy); 
        draw(cairo); 
        ecolab::cairo::Surface surf
          (cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,NULL));            
        draw(surf.cairo());      
        m_width=surf.width();
        m_height=surf.height();
      }     
    }
  }    

}
