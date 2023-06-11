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

#include "minsky.h"
#include "variableInstanceList.h"
#include "group.h"
#include "selection.h"
#include "variableInstanceList.h"
#include "variableInstanceList.rcd"
#include "variableInstanceList.xcd"
#include "minsky_epilogue.h"

using namespace std;

namespace minsky
{
  // should not really be used, but compiler requuires it
  VariableInstanceList::VariableInstanceList(): model(*minsky().model) {}
  
  VariableInstanceList::VariableInstanceList(Group& model, const string& valueId):
    model(model)
  {
    model.recursiveDo
      (&Group::items,
       [this,&valueId](const Items&, Items::const_iterator i) {
         if (const auto* v=(*i)->variableCast())
           if (v->valueId()==valueId)
             {
               if (auto controller=v->controller.lock())
                 bookmarks.emplace_back(this->model.x()-controller->left()+50, this->model.y()-controller->top()+50, controller->zoomFactor(),
                                      v->name()+"@("+to_string(int(v->x()))+","+to_string(int(v->y()))+")");
               else
                 bookmarks.emplace_back(this->model.x()-v->left()+50, this->model.y()-v->top()+50, v->zoomFactor(),
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

CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::VariableInstanceList);
