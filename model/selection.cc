/*
  @copyright Steve Keen 2019
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

#include "selection.h"
#include "lasso.h"
#include "plotWidget.h"
#include "selection.rcd"
#include "minsky_epilogue.h"
using namespace std;

namespace minsky
{
  void Selection::clear()
  {
    for (auto& i: items) i->selected=false;
    for (auto& i: groups) i->selected=false;
    Group::clear();
  }

  void Selection::toggleItemMembership(const ItemPtr& item)
  {
    if (!item) return;
    if (removeItem(*item))
      {
        item->selected=false;
        item->removeControlledItems(*this);
      }
    else if (auto gPtr=std::dynamic_pointer_cast<Group>(item))
      {
        auto it=find(groups.begin(), groups.end(),gPtr);
        if (it!=groups.end())
          {
            groups.erase(it);
            gPtr->selected=false;
          }
        else
          insertGroup(gPtr);
      }
    else
      insertItem(item);
  }
    
  void Selection::ensureItemInserted(const ItemPtr& item)
  {
    if (!item) return; //nothing to do
    if (auto g=dynamic_pointer_cast<Group>(item))
      {
        ensureGroupInserted(g);
        return;
      }
    auto i=find(items.begin(), items.end(), item);
    if (i==items.end())
      insertItem(item);
  }

  void Selection::ensureGroupInserted(const GroupPtr& item)
  {
    if (!item) return; //nothing to do
    auto i=find(groups.begin(), groups.end(), item);
    if (i==groups.end())
      insertGroup(item);
  }

  void Selection::insertItem(const ItemPtr& item)
  {
    items.push_back(item);
    item->insertControlled(*this);
    item->selected=true;
    // insert any attached wires that connect to already selected items
    if (auto g=item->group.lock())
      for (size_t i=0; i<item->portsSize(); ++i)
        {
          auto p=item->ports(i).lock();
          for (auto w: p->wires())
            {
              auto& other_end=p->input()? w->from()->item(): w->to()->item();
              if (find_if(items.begin(), items.end(),
                          [&](const ItemPtr& i) {return i.get()==&other_end;})
                  !=items.end())
                wires.push_back(g->findWire(*w));
            }
        }
    }
  
  bool Selection::contains(const ItemPtr& item) const
  {
    if (!item) return false;
    if (auto g=std::dynamic_pointer_cast<Group>(item))
      {
        if (find(groups.begin(), groups.end(), g)!=groups.end())
          return true;
      }
    else if (find(items.begin(), items.end(), item)!=items.end())
      return true;
    // at this point, we need to check if item is contained in any of
    // the selected groups
    if (auto gi=item->group.lock())
      for (auto& g: groups)
        if (g==gi || g->higher(*gi))
          return true;
    return false;
  }
  
}

CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::Selection);
