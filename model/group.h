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
#include <function.h>

namespace minsky
{
  class Group;
  class GroupPtr: public ItemPtr
  {
    void setself();
  public:
    template <class... A> GroupPtr(A... x):
      ItemPtr(std::forward<A>(x)...) {setself();}

    operator std::shared_ptr<Group>() const {
      auto r=std::dynamic_pointer_cast<Group>(*this);
      assert(r);
      return r;
    }
    Group& operator*() const;
    Group* operator->() const;
  };

  typedef IntrusiveMap<int, GroupPtr> Groups;

  class Group: public Item
  {
    int m_id=-1;
    friend class GroupPtr;
  public:
    Items items;
    Groups groups;
    Wires wires;
    std::weak_ptr<Group> self;
    float width{100}, height{100}; // size of icon

    Group* clone() const {return new Group(*this);}

    int id() const {return m_id;}

    void clear() {
      items.clear();
      groups.clear();
      wires.clear();
    }

    // add item, ownership is passed
    ItemPtr addItem(int id, Item* it) {return addItem(id,std::shared_ptr<Item>(it));}
    ItemPtr addItem(int id, const std::shared_ptr<Item>&);
    ItemPtr addItem(const ItemPtr& it) {
      assert(it.id()>=0);
      return addItem(it.id(), it);
    }

    /// move all items from source to this
    void moveContents(Group& source); 

    template <class M>
    bool recusiveDelete(M Group::*map, int id)
    {
      if ((this->*map).erase(id))
        return true;
      for (auto& g: groups)
        if (g->recusiveDelete(map, id))
          return true;
      return false;
    }

    /// delete item from this, or contained group, if it exists
    bool deleteItem(int id) {return recusiveDelete(&Group::items, id);}


    GroupPtr& addGroup(int id, const std::shared_ptr<Group>&);
    GroupPtr& addGroup(int id, Group* g) {return addGroup(id,std::shared_ptr<Group>(g));}
    GroupPtr& addGroup(const GroupPtr& g) {
      assert(g.id()>=0);
      return addGroup(g.id(), g);
    }

    /// delete group from this, or contained group, if it exists
    bool deleteGroup(int id) {return recusiveDelete(&Group::groups, id);}

    WirePtr& addWire(int id, const std::shared_ptr<Wire>&);
    WirePtr& addWire(int id, Wire* w) {return addWire(id, std::shared_ptr<Wire>(w));}
    WirePtr& addWire(const WirePtr& w) {
      assert(w.id()>=0);
      return addWire(w.id(), w);
    }

    /// delete wire from this, or contained group, if it exists
    bool deleteWire(int id) {return recusiveDelete(&Group::wires, id);}

    /// finds item within this group or subgroups. Returns null if not found
    template <class T, class C> const ItemPtr& findItem(C) const;
    const ItemPtr& findItem(int id) const;
    const ItemPtr& findItem(const Item& it) const;

    /// finds all items/wires matching criterion \a c. Found items are transformed by \a tfm
    //TODO - when functional has lambda support, use type deduction to remove the extra template argument
    template <class Return, class R, class C, class M>
    std::vector<Return> findItemsOrWires(C c, M (Group::*m), R tfm) const {
      std::vector<Return> r;
      for (auto& i: this->*m)
        if (c(i)) r.push_back(tfm(i));

      for (auto& i: groups)
        {
          //if (c(i)) r.push_back(tfm(i)); //TODO: Needed or not?
          auto items=i->findItemsOrWires<Return>(c,m,tfm);
          r.insert(r.end(), items.begin(), items.end());
        }
      return r;
    }

    /// returns list of items matching criterion \a c
    template <class C>
    std::vector<ItemPtr> findItems(C c) const {
      return findItemsOrWires<ItemPtr>(c,&Group::items,[](ItemPtr x){return x;});
    }

    /// returns ids of items matching criterion \a c
    template <class C>
    std::vector<int> findItemIds(C c) const {
      return findItemsOrWires<int>(c,&Group::items,[](const ItemPtr& x){return x.id();});
    }
   
    /// returns list of wires matching criterion \a c
    template <class C>
    std::vector<WirePtr> findWires(C c) const {
      return findItemsOrWires<WirePtr>(c,&Group::wires,[](WirePtr x){return x;});
    }

     /// returns list of groups matching criterion \a c
    template <class C>
    std::vector<GroupPtr> findGroups(C c) const {
      return findItemsOrWires<GroupPtr>(c,&Group::groups,[](GroupPtr x){return x;});
    }

   

    /// returns true if this is higher in the heirarchy than the argument
    bool higher(const Group&) const;

    /// top level group
    const Group& globalGroup() const;

    /// returns true if items appear uniquely within the
    /// heirarchy. Note the map structure does not guarantee that an
    /// item doesn't appear within another group
    bool uniqueKeys(std::set<int>& idset) const;
    bool uniqueKeys() const {
      std::set<int> idset;
      return uniqueKeys(idset);
    }
    

    /// returns whether contents should be displayed
    bool displayContents() const {return zoomFactor>displayZoom;}

    /// computes the zoom at which to show contents, given current
    /// contentBounds and width
    float displayZoom{1}; ///< zoom at which contents are displayed
    float computeDisplayZoom();
    float localZoom() const {
      return (displayZoom>0 && zoomFactor>displayZoom)
        ? zoomFactor/displayZoom: 1;
    }
    /// scale used to render io variables. Smoothly interpolates
    /// between the scale at which internal items are displayed, and
    /// the outside zoom factor
    float edgeScale() const {
      return zoomFactor>1? localZoom(): zoomFactor;
    }

    /// return bounding box coordinates for all variables, operators
    /// etc in this group. Returns the zoom scale (aka local zoom) of
    /// the contained items, or 1 if the group is empty.
    float contentBounds(double& x0, double& y0, double& x1, double& y1) const;

    /// for TCL debugging
    ecolab::array<double> cBounds() const {
      ecolab::array<double> r(4);
      contentBounds(r[0],r[1],r[2],r[3]);
      return r;
    }

    /// return maximum id in heirarchy routed at base
    int maxId() const;

  };

  inline void GroupPtr::setself() {
    if (auto g=dynamic_cast<Group*>(get())) 
      {
        g->self=std::dynamic_pointer_cast<Group>(*this);
        g->m_id=id();
      }
  }

}

#include "group.cd"
#endif
