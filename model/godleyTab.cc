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
#include "godleyTab.h"
#include "godleyTableWindow.h"
#include <pango.h>
#include "minsky_epilogue.h"
#include "minsky.h"
using namespace std;
using ecolab::cairo::Surface;
using ecolab::Pango;
using ecolab::cairo::CairoSave;

namespace minsky
{
	
  bool GodleyTab::itemSelector(const ItemPtr& i)
  {
    return dynamic_cast<GodleyIcon*>(i.get());
  }	
  
  ItemPtr GodleyTab::itemAt(float x, float y)
  {
    ItemPtr item;                    
    auto minD=numeric_limits<float>::max();
    for (auto& i: itemCoords)
      {
        if (auto g=dynamic_pointer_cast<GodleyIcon>(i.first))
          {
            float xx=(i.second).first+offsx, yy=(i.second).second+offsy;   
            if (!g->godleyT) g->godleyT.reset(new GodleyTableEditor(g));
            ecolab::cairo::Surface surf
              (cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,NULL));			
            try
              {
                g->godleyT->draw(surf.cairo());
              }
            catch (const std::exception& e) 
              {cerr<<"illegal exception caught in draw(): "<<e.what()<<endl;}
            catch (...) {cerr<<"illegal exception caught in draw()";}
            float w=0.5*g->godleyT->colLeftMargin[g->godleyT->colLeftMargin.size()-1];
            float h=0.5*(g->godleyT->godleyIcon->table.rows())*g->godleyT->rowHeight;
            xx+=w;
            yy+=h;
            float d=sqr(xx-x)+sqr(yy-y);
          
            if (d<minD && fabs(xx-x)<w && fabs(yy-y)<h)
              {
                minD=d;
                item=i.first;
              }
          }
      }
    
    return item;
  }
	
  void GodleyTab::draw(cairo_t* cairo)
  {   
    for (auto& it: itemVector)
      {
        if (auto g=dynamic_pointer_cast<GodleyIcon>(it))
          {
            if (!g->godleyT) g->godleyT.reset(new GodleyTableEditor(g));  
            cairo::CairoSave cs(cairo);   
            if (it==itemFocus) {
              cairo_translate(cairo,xItem,yItem);  		    				   
              itemCoords[itemFocus]=move(make_pair(xItem,yItem));         
            } else cairo_translate(cairo,itemCoords[it].first,itemCoords[it].second);
            g->godleyT->disableButtons();
            g->godleyT->displayValues=true;   
            g->godleyT->draw(cairo);
            
            // draw title
            if (!g->table.title.empty())
              {
                CairoSave cs(cairo);
                Pango pango(cairo);
                pango.setMarkup("<b>"+latexToPango(g->table.title)+"</b>");
                pango.setFontSize(12);
                cairo_move_to(cairo,0.5*g->godleyT->colLeftMargin[g->godleyT->colLeftMargin.size()-1],g->godleyT->topTableOffset-2*g->godleyT->rowHeight);
                pango.show();
              }                    
          }			   
      }              
  }

}
