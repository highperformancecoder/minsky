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

#include <plotTab.h>
#include <pango.h>
#include "minsky_epilogue.h"
#include "minsky.h"
using namespace std;
using ecolab::cairo::Surface;
using ecolab::Pango;
using ecolab::cairo::CairoSave;

namespace minsky
{

  bool PlotTab::itemSelector(const ItemPtr& i)
  {
	if (auto p=i->plotWidgetCast()) return p->plotOnTab();
	return false;
  }	

  void PlotTab::togglePlotDisplay() const      
  {
    if (auto p=itemFocus->plotWidgetCast()) p->togglePlotTabDisplay();
  }	
	 
  void PlotTab::draw(cairo_t* cairo)
  {   
     for (auto& it: itemVector)
       {
         assert(it->plotWidgetCast());
         cairo::CairoSave cs(cairo);   
         if (it==itemFocus) {
           cairo_translate(cairo,xItem,yItem);  		    				    
           itemCoords[itemFocus]=move(make_pair(xItem,yItem));
         } else cairo_translate(cairo,itemCoords[it].first,itemCoords[it].second);      
         it->draw(cairo);
       }              
  }
  
}
