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

#include "minsky.h"
#include <plotTab.h>
#include <pango.h>
#include "itemTab.xcd"
#include "plotTab.rcd"
#include "plotTab.xcd"
#include "minsky_epilogue.h"
using namespace std;
using ecolab::cairo::CairoSave;

namespace minsky
{

  bool PlotTab::itemSelector(const ItemPtr& i)
  {
	if (auto* p=i->plotWidgetCast()) return p->plotTabDisplay;
	return false;
  }	

  void PlotTab::togglePlotDisplay() const      
  {
    if (item)
      if (auto p=item->plotWidgetCast())
        p->togglePlotTabDisplay();
  }	
	 
  void PlotTab::draw(cairo_t* cairo)
  {   
     for (auto& it: itemVector)
       {
         assert(it->plotWidgetCast());
         cairo::CairoSave cs(cairo);   
         cairo_translate(cairo,it->itemTabX, it->itemTabY);  		    				    
         it->draw(cairo);
       }              
  }
  
}
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::PlotTab);
