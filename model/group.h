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
#include "SVGItem.h"

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
    std::string title;
    Items items;
    Groups groups;
    Wires wires;
    std::weak_ptr<Group> self;
    float width{100}, height{100}; // size of icon

    Group* clone() const {return new Group(*this);}

    int id() const {return m_id;}

    static SVGRenderer svgRenderer;


    void draw(cairo_t* cairo) const override {}

    void clear() {
      items.clear();
      groups.clear();
      wires.clear();
    }

    /// Perform action heirarchically on elements of map \a map. If op returns true, the operation terminates.
    /// returns true if operation terminates early, false if every element processed.
    template <class M, class O>
    bool recursiveDo(M Group::*map, O op)
    {
      for (auto i=(this->*map).begin(); i!=(this->*map).end(); ++i)
        if (op(this->*map,i))
          return true;
      for (auto& g: groups)
        if (g->recursiveDo(map, op))
          return true;
      return false;
    }

    // find item \a id, and perform op on it. Returns whatever op returns
    template <class R, class M, class O>
    R recursiveDo(M Group::*map, int id, O op)
    {
      auto i=(this->*map).find(id);
      if (i!=(this->*map).end())
        return op(this->*map,i);
      for (auto& g: groups)
        if (auto r=g->recursiveDo<R>(map, id, op))
          return r;
      return R();
    }

    /// search for the first item in the heirarchy of \a map for which
    /// \a c is true. M::value_type must evaluate in a boolean
    /// environment to false if not valid
    template <class M, class C>
    const typename M::value_type& findAny(M Group::*map, C c) const
    {
      for (auto& i: this->*map)
        if (c(i))
          return i;
      for (auto& g: groups)
        if (auto& r=g->findAny(map, c))
          return r;
      static typename M::value_type null{-1};
      return null;
    }

    /// search heirarchy for item with id \a id.
    template <class M>
    const typename M::value_type& findId(M Group::*map, int id) const
    {
      auto i=(this->*map).find(id);
      if (i!=(this->*map).end())
        return *i;
      for (auto& g: groups)
        if (auto& r=g->findId(map, id))
          return r;
      static typename M::value_type null{-1};
      return null;
    }

    /// finds all items/wires matching criterion \a c. Found items are transformed by \a xfm
    //TODO - when functional has lambda support, use type deduction to remove the extra template argument
    template <class R, class M, class C, class X>
    std::vector<R> findAll(C c, M (Group::*m), X xfm) const {
      std::vector<R> r;
      for (auto& i: this->*m)
        {
          assert(i);
          if (c(i)) r.push_back(xfm(i));
        }

      for (auto& i: groups)
        {
          assert(i);
          auto items=i->findAll<R>(c,m,xfm);
          r.insert(r.end(), items.begin(), items.end());
        }
      return r;
    }



    /// delete item from this, or contained group, if it exists
    template <class M>
    typename M::mapped_type removeItem(M Group::*map, int id) {
      return recursiveDo<typename M::mapped_type>
        (map, id, [&](M& map, typename M::iterator& x)
         {typename M::mapped_type r=*x; map.erase(x); return r;});
    }
    ItemPtr removeItem(int id) {return removeItem(&Group::items,id);}
    GroupPtr removeGroup(int id) {return removeItem(&Group::groups,id);}
    WirePtr removeWire(int id) {return removeItem(&Group::wires,id);}

    ItemPtr removeItem(const Item&);
    WirePtr removeWire(const Wire&);
    GroupPtr removeGroup(const Group&);

    /// finds item within this group or subgroups. Returns null if not found
    const ItemPtr& findItem(int id) const {return findId(&Group::items, id);}
    const ItemPtr& findItem(const Item& it) const; 

    /// finds group within this group or subgroups. Returns null if not found
    const GroupPtr& findGroup(int id) const {return findId(&Group::groups, id);}
    const GroupPtr& findGroup(const Group& it) const 
    {return findAny(&Group::groups, [&](const GroupPtr& x){return x.get()==&it;});}

    /// finds wire within this group or subgroups. Returns null if not found
    const WirePtr& findWire(int id) const {return findId(&Group::wires, id);}
    const WirePtr& findWire(const Wire& it) const 
    {return findAny(&Group::wires, [&](const WirePtr& x){return x.get()==&it;});}

    /// returns list of items matching criterion \a c
    template <class C>
    std::vector<ItemPtr> findItems(C c) const {
      return findAll<ItemPtr>(c,&Group::items,[](ItemPtr x){return x;});
    }
    /// returns ids of items matching criterion \a c
    template <class C>
    std::vector<int> findItemIds(C c) const {
      return findAll<int>(c,&Group::items,[](const ItemPtr& x){return x.id();});
    }
   
    /// returns list of wires matching criterion \a c
    template <class C>
    std::vector<WirePtr> findWires(C c) const {
      return findAll<WirePtr>(c,&Group::wires,[](WirePtr x){return x;});
    }

     /// returns list of groups matching criterion \a c
    template <class C>
    std::vector<GroupPtr> findGroups(C c) const {
      return findAll<GroupPtr>(c,&Group::groups,[](GroupPtr x){return x;});
    }

    // add item, ownership is passed
    ItemPtr& addItem(int id, Item* it) {return addItem(id,std::shared_ptr<Item>(it));}
    ItemPtr& addItem(int id, const std::shared_ptr<Item>&);
    ItemPtr& addItem(const ItemPtr& it) {
      assert(it.id()>=0);
      return addItem(it.id(), it);
    }

    GroupPtr& addGroup(int id, const std::shared_ptr<Group>&);
    GroupPtr& addGroup(int id, Group* g) {return addGroup(id,std::shared_ptr<Group>(g));}
    GroupPtr& addGroup(const GroupPtr& g) {
      assert(g.id()>=0);
      return addGroup(g.id(), g);
    }

    WirePtr& addWire(int id, const std::shared_ptr<Wire>&);
    WirePtr& addWire(int id, Wire* w) {return addWire(id, std::shared_ptr<Wire>(w));}
    WirePtr& addWire(const WirePtr& w) {
      assert(w.id()>=0);
      return addWire(w.id(), w);
    }

    /// adjust wire's group to be the least common ancestor of its ports
    void adjustWiresGroup(Wire& w);

    /// move all items from source to this
    void moveContents(Group& source); 

    /// returns true if this is higher in the heirarchy than the argument
    bool higher(const Group&) const;
    /// return level in the heirarchy
    unsigned level() const;

    /// top level group
    const Group& globalGroup() const;
    Group& globalGroup();

    /// returns true if items appear uniquely within the
    /// heirarchy. Note the map structure does not guarantee that an
    /// item doesn't appear within another group
    bool uniqueKeys(std::set<int>& idset) const;
    bool uniqueKeys() const {
      std::set<int> idset;
      return uniqueKeys(idset);
    }
    

    /// returns whether contents should be displayed. Top level group always displayed
    bool displayContents() const {return !group.lock() || zoomFactor>displayZoom;}

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
