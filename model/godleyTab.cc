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
#include "godleyTab.rcd"
#include "godleyTab.xcd"
#include "itemTab.xcd"
#include "minsky_epilogue.h"
#include "minsky.h"
using namespace std;
using ecolab::Pango;
using ecolab::cairo::CairoSave;

namespace minsky
{
	
  bool GodleyTab::itemSelector(const ItemPtr& i)
  {
    return dynamic_cast<GodleyIcon*>(i.get());
  }
  
  ItemTab::ClickType GodleyTab::clickType(double x, double y) const
  {
    return internal;
  }
  
  
  ItemPtr GodleyTab::itemAt(float x, float y)
  {
    ItemPtr item;                    
    auto minD=numeric_limits<float>::max();
    for (auto& i: itemVector)
      {
        if (auto g=dynamic_pointer_cast<GodleyIcon>(i))
          {
            
            float xx=i->itemTabX+offsx, yy=i->itemTabY+offsy;   
            float w=0.5*g->editor.width();
            float h=0.5*g->editor.height();
            xx+=w;
            yy+=h;
            float d=sqr(xx-x)+sqr(yy-y);

            // add an extra room to allow grabbing  the title. For ticket #1326.
            if (d<minD && fabs(xx-x)<w && fabs(yy-y)<h+g->editor.rowHeight)
              {
                minD=d;
                item=i;
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
            cairo::CairoSave cs(cairo);   
            cairo_translate(cairo,it->itemTabX,it->itemTabY);  		    				   
            g->editor.disableButtons();
            g->editor.draw(cairo);
            
            // draw title
            if (!g->table.title.empty())
              {
                CairoSave cs(cairo);
                Pango pango(cairo);
                pango.setMarkup("<b>"+latexToPango(g->table.title)+"</b>");
                pango.setFontSize(12);
                cairo_move_to(cairo,0.5*g->editor.colLeftMargin[g->editor.colLeftMargin.size()-1],g->editor.topTableOffset-2*g->editor.rowHeight);
                pango.show();
              }                    
          }			   
      }              
  }

}
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::GodleyTab);
