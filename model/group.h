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
#include "variable.h"
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

  typedef std::vector<GroupPtr> Groups;
  template <class G, class M, class O>
  bool GroupRecursiveDo(G& gp, M Group::*map, O op) 
    {
      for (auto i=(gp.*map).begin(); i!=(gp.*map).end(); ++i)
        if (op(gp.*map,i))
          return true;
      for (auto& g: gp.groups)
        if (g->recursiveDo(map, op))
          return true;
      return false;
    }

  

  class Group: public Item
  {
    friend class GroupPtr;
    std::vector<VariablePtr> inVariables, outVariables;
  public:
    std::string classType() const override {return "Group";}
    int id=-1; // unique id used for variable scoping
    std::string title;
    Items items;
    Groups groups;
    Wires wires;
    std::weak_ptr<Group> self;
    float width{100}, height{100}; // size of icon

    Group* clone() const {return new Group(*this);}

    static SVGRenderer svgRenderer;


    void draw(cairo_t*) const override;

    /// draw representations of edge variables around group icon
    void drawEdgeVariables(cairo_t*) const;
    /// draw notches in the I/O region to indicate docability of variables there
    void drawIORegion(cairo_t*) const;

    void clear() {
      items.clear();
      groups.clear();
      wires.clear();
    }

    /// Perform action heirarchically on elements of map \a map. If op returns true, the operation terminates.
    /// returns true if operation terminates early, false if every element processed.
    template <class M, class O>
    bool recursiveDo(M Group::*map, O op) const 
    {return GroupRecursiveDo(*this,map,op);}
    template <class M, class O>
    bool recursiveDo(M Group::*map, O op)
    {return GroupRecursiveDo(*this,map,op);}

    /// search for the first item in the heirarchy of \a map for which
    /// \a c is true. M::value_type must evaluate in a boolean
    /// environment to false if not valid
    template <class M, class C>
    const typename M::value_type findAny(M Group::*map, C c) const
    {
      for (auto& i: this->*map)
        if (c(i))
          return i;
      for (auto& g: groups)
        if (auto& r=g->findAny(map, c))
          return r;
      return typename M::value_type();
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



    ItemPtr removeItem(const Item&);
    WirePtr removeWire(const Wire&);
    GroupPtr removeGroup(const Group&);

    /// finds item within this group or subgroups. Returns null if not found
    ItemPtr findItem(const Item& it) const; 

    /// finds group within this group or subgroups. Returns null if not found
    GroupPtr findGroup(const Group& it) const 
    {return findAny(&Group::groups, [&](const GroupPtr& x){return x.get()==&it;});}

    /// finds wire within this group or subgroups. Returns null if not found
    WirePtr findWire(const Wire& it) const 
    {return findAny(&Group::wires, [&](const WirePtr& x){return x.get()==&it;});}

    /// returns list of items matching criterion \a c
    template <class C>
    std::vector<ItemPtr> findItems(C c) const {
      return findAll<ItemPtr>(c,&Group::items,[](ItemPtr x){return x;});
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
    ItemPtr addItem(Item* it) {return addItem(std::shared_ptr<Item>(it));}
    ItemPtr addItem(const std::shared_ptr<Item>&);

    GroupPtr addGroup(const std::shared_ptr<Group>&);
    GroupPtr addGroup(Group* g) {return addGroup(std::shared_ptr<Group>(g));}

    WirePtr addWire(const std::shared_ptr<Wire>&);
    WirePtr addWire(Wire* w) {return addWire(std::shared_ptr<Wire>(w));}

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
    bool uniqueItems(std::set<void*>& idset) const;
    bool uniqueItems() const {
      std::set<void*> idset;
      return uniqueItems(idset);
    }
    

    /// returns whether contents should be displayed. Top level group always displayed
    bool displayContents() const {return !group.lock() || zoomFactor>displayZoom;}

    /// margin sizes to allow space for edge variables. 
    void margins(float& left, float& right) const;

    /// computes the zoom at which to show contents, given current
    /// contentBounds and width
    float displayZoom{1}; ///< zoom at which contents are displayed
    float computeDisplayZoom();
    float localZoom() const {
      return (displayZoom>0 && zoomFactor>displayZoom)
        ? zoomFactor/displayZoom: 1;
    }
    /// sets the zoom factor to \a factor. Recursively set the
    /// zoomfactor on contained objects to the computed localzoom
    void setZoom(float factor);
    void zoom(float xOrigin, float yOrigin,float factor) override;

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

    /// returns the smallest group whose icon completely encloses the
    /// rectangle given by the argument. If no candidate group found,
    /// returns nullptr. Weak reference returned, no ownership.
    Group* minimalEnclosingGroup(float x0, float y0, float x1, float y1);
    const Group* minimalEnclosingGroup(float x0, float y0, float x1, float y1) const;

    /// scaling factor to allow a rotated icon to fit on the bitmap
    float rotFactor() const;

  };

  inline void GroupPtr::setself() {
    if (auto g=dynamic_cast<Group*>(get())) 
      {
        g->self=std::dynamic_pointer_cast<Group>(*this);
      }
  }

  /// find the closest (in or out) port to \a x or \a y.
  struct ClosestPort: public std::shared_ptr<Port>
  {
    enum InOut {in, out};
    ClosestPort(const Group&, InOut, float x, float y); 
  };

}

#ifdef CLASSDESC
// omit these, because weak/shared pointers cause problems, and its
// not needed anyway
#pragma omit pack minsky::Group
#pragma omit unpack minsky::Group
#pragma omit TCL_obj minsky::GroupPtr
#endif
namespace classdesc_access
{
  template <> struct access_pack<minsky::Group>: 
    public classdesc::NullDescriptor<classdesc::pack_t> {};
  template <> struct access_unpack<minsky::Group>: 
    public classdesc::NullDescriptor<classdesc::unpack_t> {};
  template <> struct access_TCL_obj<minsky::GroupPtr>
  {
    template <class U>
    void operator()(classdesc::TCL_obj_t& t, const classdesc::string& d, U& a)
    {
      if (auto g=dynamic_cast<minsky::Group*>(a.get())) 
        TCL_obj(t,d,*g);
    }
  };
}
#include "group.cd"
#endif
