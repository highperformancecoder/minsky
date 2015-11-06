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

#ifndef GROUP_H
#define GROUP_H
#include "intrusiveMap.h"
#include "item.h"
#include "wire.h"

namespace minsky
{
  class Group;
  class GroupPtr: public ItemPtr
  {
    void setself();
  public:
    virtual int id() const {return -1;}

    template <class... A> GroupPtr(A... x):
      ItemPtr(std::forward<A>(x)...) {setself();}
    virtual ~GroupPtr() {}

    Group& operator*() const;
    Group* operator->() const;
  };

  typedef IntrusiveMap<int, GroupPtr> Groups;

  class Group: public Item
  {
  public:
    Items items;
    Groups groups;
    Wires wires;
    std::weak_ptr<Group> self;

    // add item, ownership is passed
    ItemPtr& addItem(int id, Item* it) {return addItem(id,std::shared_ptr<Item>(it));}
    ItemPtr& addItem(int id, const std::shared_ptr<Item>&);
    ItemPtr& addItem(const ItemPtr& it) {
      assert(it.id()>=0);
      return addItem(it.id(), it);
    }
    
    GroupPtr& addGroup(int id, const std::shared_ptr<Group>&);
    GroupPtr& addGroup(int id, Group* g) {return addGroup(id,std::shared_ptr<Group>(g));}

    WirePtr& addWire(int id, const std::shared_ptr<Wire>&);
    WirePtr& addWire(int id, Wire* w) {return addWire(id, std::shared_ptr<Wire>(w));}

    // finds item within this group or subgroups. Returns null if not found
    const ItemPtr& findItem(int id) const;
    /// returns true if this is higher in the heirarchy than \a id
    bool higher(const GroupPtr&) const;

    /// top level group
    Group& globalGroup() const;

  };

  inline void GroupPtr::setself() {
    if (auto g=dynamic_cast<Group*>(get())) 
      g->self=std::dynamic_pointer_cast<Group>(*this);
  }

}

#include "group.cd"
#endif
