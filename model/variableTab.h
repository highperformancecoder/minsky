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

#ifndef VARIABLETAB_H
#define VARIABLETAB_H
#include "itemTab.h"
#include "pannableTab.h"

namespace minsky
{
	 
  class VariableTab: public PannableTab<ItemTab>
  {	  
  public:
    bool itemSelector(const ItemPtr& i) override {if (auto v=i->variableCast()) return v->type()!=VariableType::parameter /*&& v->attachedToDefiningVar() this filtered variables based on varTabDisplay boolean */; return false;}  
  };
  
}
#include "variableTab.cd"
#endif
