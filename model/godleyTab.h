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

#ifndef GODLEYTAB_H
#define GODLEYTAB_H
#include <itemTab.h>

namespace minsky
{
	 
  class GodleyTab: public ItemTab
  {	  
  public:
    bool itemSelector(ItemPtr i) override {if (dynamic_cast<GodleyIcon*>(i.get())) return i.get(); return false;}
  };
  
}
#include "godleyTab.cd"
#endif
