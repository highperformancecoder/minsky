/*
  @copyright Steve Keen 2015
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

#include "group.h"
#include "wire.h"
#include <ecolab_epilogue.h>
using namespace std;

namespace minsky
{
  Group& GroupPtr::operator*() const {return dynamic_cast<Group&>(ItemPtr::operator*());}
  Group* GroupPtr::operator->() const {return dynamic_cast<Group*>(ItemPtr::operator->());}


  ItemPtr& Group::addItem(int id, const shared_ptr<Item>& it)
  {
    if (auto x=dynamic_pointer_cast<Group>(it))
      return addGroup(id,x);
   
    auto origGroup=it->group.lock();
    assert(origGroup);
    if (origGroup.get()!=this)
      origGroup->items.erase(id);

    if (self.lock())
      it->group=self;

    // move wire to highest common group
    // TODO add in I/O variables if needed, and move wires to same group
    for (auto& p: it->ports)
      {
        assert(p);
        for (auto& w: p->wires)
          {
            assert(w);
            GroupPtr otherGroup;
            if (p->input())
              {
                if (auto from=w->from())
                  otherGroup=from->group();
                assert(p==w->to());
              }
            else
              {
                if (auto to=w->to())
                  otherGroup=to->group();
                assert(p==w->from());
              }
            // if origGroup is null, then it is global
            if (origGroup && otherGroup)
              {
                if (origGroup->higher(otherGroup))
                  {
                    if (higher(otherGroup))
                      w->moveGroup(*origGroup,*this);
                    else
                      w->moveGroup(*origGroup,*otherGroup);
                  }
                else
                  if (higher(otherGroup))
                    w->moveGroup(*otherGroup,*this);
              }
          }
      }

    return *items.insert(Items::value_type(id,it)).first;
  }

  GroupPtr& Group::addGroup(int id, const std::shared_ptr<Group>& g)
  {
    assert(g->group.lock());
    if (auto origGroup=g->group.lock())
      if (origGroup.get()!=this)
        origGroup->groups.erase(id);
    g->group=self;
    g->self=g;
    return *groups.insert(Groups::value_type(id,g)).first;
  }

  WirePtr& Group::addWire(int id, const std::shared_ptr<Wire>& w)
  {
    return *wires.insert(Wires::value_type(id,w)).first;
  }

  const ItemPtr& Group::findItem(int id) const 
  {
    for (auto& i: items)
      if (i.id()==id)
        return i;

    for (auto& g: groups)
      if (g.id()==id)
        return g;

    for (auto& g: groups)
      if (auto& i=g->findItem(id))
        return i;

    static ItemPtr nullItem;
    return nullItem;
  }

  
  bool Group::higher(const GroupPtr& x) const
  {
    if (!x) return false; // global group x is always higher
    for (auto i: groups)
      if (i==x) return true;
    for (auto i: groups)
      if (higher(i))
        return true;
    return false;
  }

}
