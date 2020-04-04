/*
  @copyright Steve Keen 2020
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

#include "variableInstanceList.h"
#include "group.h"
#include "selection.h"
#include "minsky_epilogue.h"

using namespace std;

namespace minsky
{
  VariableInstanceList::VariableInstanceList(Group& model, const string& valueId):
    model(model)
  {
    model.recursiveDo
      (&Group::items,
       [this,&valueId](const Items&, Items::const_iterator i) {
         if (auto v=(*i)->variableCast())
           if (v->valueId()==valueId)
             {
               bookmarks.emplace_back(this->model.x()-v->x()+50, this->model.y()-v->y()+50, v->zoomFactor(),
                                      v->name()+"@("+to_string(int(v->x()))+","+to_string(int(v->y()))+")");
               items.push_back(*i);
             }
         return false;
       });
  }

  void VariableInstanceList::gotoInstance(size_t i) {
    if (i<bookmarks.size())
      {
        assert(bookmarks.size()==items.size());
        model.gotoBookmark_b(bookmarks[i]);
        items[i]->selected=true;
      }
  }

}

