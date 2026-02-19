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
#include "SVGItem.h"
#include "callableFunction.h"

#include <assert.h>         // for assert
#include <string.h>         // for std::size_t, memcpy
#include <memory>           // for shared_ptr, weak_ptr, __shared_ptr_access
#include <set>              // for set
#include <string>           // for string, basic_string, operator+, operator==
#include <vector>           // for vector
#include "arrays.h"         // for array
#include "bookmark.h"       // for Bookmark
#include "cairo.h"          // for cairo_t
#include "classdesc.h"      // for Exclude, NullDescriptor
#include "error.h"          // for error
#include "item.h"           // for ItemPtr, Item, ItemPortVector, ItemT, Items
#include "operationType.h"  // for operator<<
#include "port.h"           // for GroupPtr, Port (ptr only)
#include "tensorVal.h"      // for operator<<
#include "variable.h"       // for VariablePtr, VariableBase
#include "variableType.h"   // for operator<<
#include "variableSummary.h"
#include "wire.h"           // for WirePtr, Wires, Wire, error
namespace classdesc { class pack_t; }
namespace classdesc_access { template <class T> struct access_pack; }
namespace classdesc_access { template <class T> struct access_unpack; }
namespace minsky { class PlotWidget; }
namespace minsky { struct LassoBox; }

namespace minsky
{
  typedef std::vector<GroupPtr> Groups;

  // items broken out in a separate structure, as copying is non-default
  class GroupItems
  {
    bool inDestructor=false;
  protected:
    /// add a wire from item \a from, to item \a to, connecting to the
    /// toIdx port of \a to, with \a coordinates
    WirePtr addWire(const Item& from, const Item& to, unsigned toPortIdx, 
                const std::vector<float>& coords)
    {
      if (toPortIdx>=to.portsSize()) return WirePtr();
      return addWire(from.ports(0), to.ports(toPortIdx), coords);
    }

    WirePtr addWire(const std::weak_ptr<Port>& from,
                    const std::weak_ptr<Port>& to, 
                    const std::vector<float>& coords);
    /// rename variable so that it maintains most general scope possible 
    void renameVar(const GroupPtr& origGroup, VariableBase& v);
    CLASSDESC_ACCESS(GroupItems);
    friend class Canvas;
  public:
    Items items;
    Groups groups;
    Wires wires;
    std::set<Bookmark> bookmarks;
    std::vector<VariablePtr> inVariables, outVariables;
    std::vector<VariablePtr> createdIOvariables;
    GroupItems() {}
    virtual ~GroupItems() {inDestructor=true; clear();}
    // copy operations not deleted to allow ItemT<Group> to compile
    GroupItems(const GroupItems& x) {};
    GroupItems& operator=(const GroupItems&) {return *this;}
    classdesc::Exclude<std::weak_ptr<Group>> self; ///< weak ref to this
    
    void clear() {
      // controlled items need to be removed from a copy
      auto itemsCopy=items;
      for (auto& i: itemsCopy) i->removeControlledItems(*this);
      items.clear();
      groups.clear();
      wires.clear();
      inVariables.clear();
      outVariables.clear();
      bookmarks.clear();
    }
    bool empty() const {return items.empty() && groups.empty() && wires.empty();}

    /// tests that groups are arranged heirarchically without any recurrence
    virtual bool nocycles() const=0; 

    /// @{ Perform action heirarchically on elements of map \a map. If op returns true, the operation terminates.
    /// returns true if operation terminates early, false if every element processed.
    /// O has signature bool(M, M::const_iterator)
    template <class M, class O>
    bool recursiveDo(M GroupItems::*map, O op) const 
    {return GroupRecursiveDo(*this,map,op);}
    /// O has signature bool(M&, M::iterator)
    template <class M, class O>
    bool recursiveDo(M GroupItems::*map, O op)
    {return GroupRecursiveDo(*this,map,op);}
    /// @}
    
    /// search for the first item in the heirarchy of \a map for which
    /// \a c is true. M::value_type must evaluate in a boolean
    /// environment to false if not valid
    /// C is of signature bool(M::value_type)
    template <class M, class C>
    const typename M::value_type findAny(M GroupItems::*map, C c) const;

    /// finds all items/wires matching criterion \a c. Found items are transformed by \a xfm
    //TODO - when functional has lambda support, use type deduction to remove the extra template argument
    template <class R, class M, class C, class X>
    std::vector<R> findAll(C c, M (GroupItems::*m), X xfm) const;

    WirePtr removeWire(const Wire&);
    GroupPtr removeGroup(const Group&);

    
    /// finds item within this group or subgroups. Returns null if not found
    ItemPtr findItem(const Item& it) const; 

    /// finds group within this group or subgroups. Returns null if not found
    GroupPtr findGroup(const Group& it) const 
    {return findAny(&GroupItems::groups, [&](const GroupPtr& x){return &*x==&it;});}

    /// finds wire within this group or subgroups. Returns null if not found
    WirePtr findWire(const Wire& it) const 
    {return findAny(&GroupItems::wires, [&](const WirePtr& x){return x.get()==&it;});}

    /// returns list of items matching criterion \a c
    template <class C>
    std::vector<ItemPtr> findItems(C c) const {
      return findAll<ItemPtr>(c,&GroupItems::items,[](ItemPtr x){return x;});
    }
   
    /// returns list of wires matching criterion \a c
    template <class C>
    std::vector<WirePtr> findWires(C c) const {
      return findAll<WirePtr>(c,&GroupItems::wires,[](WirePtr x){return x;});
    }

     /// returns list of groups matching criterion \a c
    template <class C>
    std::vector<GroupPtr> findGroups(C c) const {
      return findAll<GroupPtr>(c,&GroupItems::groups,[](GroupPtr x){return x;});
    }

    /// add item, ownership is passed
    ItemPtr addItem(Item* it, bool inSchema=false) {return addItem(std::shared_ptr<Item>(it),inSchema);}
    /** add item. 
        @param inSchema - if building a group from schema processing, rather than generally. Does not adjust item position within group if true.
    */
    virtual ItemPtr addItem(const std::shared_ptr<Item>&, bool inSchema=false);
    ItemPtr removeItem(const Item&);

    GroupPtr addGroup(const std::shared_ptr<Group>&);
    GroupPtr addGroup(Group* g) {return addGroup(std::shared_ptr<Group>(g));}

    /// add a wire from item \a from, to item \a to, connecting to the
    /// toIdx port of \a to
    WirePtr addWire(const Item& from, const Item& to, unsigned toPortIdx)
    {return addWire(from, to, toPortIdx, {});}
    WirePtr addWire(const std::shared_ptr<Wire>&);
    WirePtr addWire(Wire* w) {return addWire(std::shared_ptr<Wire>(w));}
    WirePtr addWire(const std::weak_ptr<Port>& from, const std::weak_ptr<Port>& to)
    {return addWire(from,to,{});}

    /// adjust wire's group to be the least common ancestor of its ports
    static void adjustWiresGroup(Wire& w);

    /// total number of items in this and child groups
    std::size_t numItems() const; 
    /// total number of wires in this and child groups
    std::size_t numWires() const; 
    /// total number of groups in this and child groups
    std::size_t numGroups() const; 
    /// plot widget used for group icon
    classdesc::Exclude<std::shared_ptr<PlotWidget>> displayPlot;
    /// remove the display plot
    void removeDisplayPlot() {
      displayPlot.reset();
    }
    
  };

  template <class G, class M, class O>
  bool GroupRecursiveDo(G& gp, M GroupItems::*map, O op) 
    {
      for (auto i=(gp.*map).begin(); i!=(gp.*map).end(); ++i)
        {
          assert(*i);
          if (op(gp.*map,i))
            return true;
        }
      for (auto& g: gp.groups)
        {
          assert(g);
          if (g->recursiveDo(map, op))
            return true;
        }
      return false;
    }

  class Group: public ItemT<Group>, public GroupItems, public CallableFunction
  {
    bool m_displayContentsChanged=true;
    VariablePtr addIOVar(const char*);
  protected:
    /// returns the smallest group whose icon completely encloses the
    /// rectangle given by the argument. If no candidate group found,
    /// returns nullptr. Weak reference returned, no ownership.
    /// @param ignore - do not return group \a ignore, even if it encloses the rectangle
    const Group* minimalEnclosingGroup(float x0, float y0, float x1, float y1, const Item*ignore=nullptr) const;
    Group* minimalEnclosingGroup(float x0, float y0, float x1, float y1, const Item*ignore=nullptr)
    {return const_cast<Group*>(const_cast<const Group*>(this)->minimalEnclosingGroup(x0,y0,x1,y1,ignore));}
    friend class Canvas;

    /// return bounding box coordinates for all variables, operators
    /// etc in this group. Returns the zoom scale (aka local zoom) of
    /// the contained items, or 1 if the group is empty.
    float contentBounds(double& x0, double& y0, double& x1, double& y1) const;

  public:
    std::string title;
    std:: string name() const override {return title;}
    /// evaluate function on arbitrary number of arguments (exprtk support)
    double operator()(const std::vector<double>& p) override;
    /// algebraic formula representing value of first output argument of this group
    std::string formula() const;
    /// argument declaration, to be appended to name() for function declaration
    std::string arguments() const;
    
    Group() {iWidth(100); iHeight(100);}
    
    bool nocycles() const override; 

    /// Make a copy of this as a sibling group (owned by
    /// parent). Attempting to copying minsky.model is a null operation.
    GroupPtr copy() const;
    /// make a copy of this, that is not owned by any group
    GroupPtr copyUnowned() const;
    Group* clone() const override {throw error("Groups cannot be cloned");}
    static SVGRenderer svgRenderer;

    using GroupItems::addItem;
    ItemPtr addItem(const std::shared_ptr<Item>& it, bool inSchema=false) override
    {
      if (it && it->bookmark)
        addBookmarkXY(it->left(),it->top(),it->bookmarkId());
      return GroupItems::addItem(it,inSchema);
    }


    /// Make all variables not present in outerscope local to this group
    void makeSubroutine();
    
    void draw(cairo_t*) const override;

    /// draw representations of edge variables around group icon
    void drawEdgeVariables(cairo_t*) const;
    /// draw the variables of one of the edges
    void draw1edge(const std::vector<VariablePtr>& vars, cairo_t* cairo, 
                   float x) const;
    /// draw notches in the I/O region to indicate dockability of
    /// variables there
    void drawIORegion(cairo_t*) const;

    /// move all items from source to this
    void moveContents(Group& source);     

    /// returns which I/O region (x,y) is in if any
    struct IORegion {enum type {none,input,output,topBottom};};
      
    IORegion::type inIORegion(float x, float y) const;
    /// check if item is a variable and located in an I/O region, and add it if it is
    void checkAddIORegion(const ItemPtr& x);
    
    void addInputVar() {inVariables.push_back(addIOVar("input"));}
    void addOutputVar() {outVariables.push_back(addIOVar("output"));}

    /// remove item from group, and also all attached wires.
    void deleteItem(const Item&);

    void deleteAttachedWires() override {
      for (auto& i: inVariables) i->deleteAttachedWires();
      for (auto& i: outVariables) i->deleteAttachedWires();
    }
    
    /// adjust position and size of icon to just cover contents
    void resizeOnContents();

    void resize(const LassoBox& x) override;
    ClickType::Type clickType(float x, float y) const override;

    /// returns true if this is higher in the heirarchy than the argument
    /// this->higher(*this) is false
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
    bool displayContents() const {return !group.lock() || zoomFactor()*relZoom>1;}
    /// true if displayContents status changed on this or any
    /// contained group last zoom
    bool displayContentsChanged() const {return m_displayContentsChanged;}

    /// margin sizes to allow space for edge variables. 
    void margins(float& left, float& right) const;    

    /// for debugging purposes
    std::vector<float> marginsV() const {
      float l, r; margins(l,r);  return {l,r};
    }    
    
    /// computes the zoom at which to show contents, given current
    /// contentBounds and width
    float displayZoom{1}; ///< extra zoom at which contents are displayed
    float relZoom{1}; ///< relative zoom contents of this group are displayed at
    float computeDisplayZoom();
    void computeRelZoom();
    float localZoom() const {
      return (displayZoom>0 && displayContents())
        ? zoomFactor()/displayZoom: 1;
    }
    /// sets the zoom factor to \a factor. Recursively set the
    /// zoomfactor on contained objects to the computed localzoom
    void setZoom(float factor);
    void zoom(float xOrigin, float yOrigin,float factor);

    /// scale used to render io variables. Smoothly interpolates
    /// between the scale at which internal items are displayed, and
    /// the outside zoom factor
    float edgeScale() const {
      float z=zoomFactor();
      return z*relZoom>1? z*relZoom: z>1? 1: z;
    }

    /// for TCL debugging
    ecolab::array<double> cBounds() const {
      ecolab::array<double> r(4);
      contentBounds(r[0],r[1],r[2],r[3]);
      return r;
    }

    /// scaling factor to allow a rotated icon to fit on the bitmap
    float rotFactor() const;
    
    void positionEdgeVariables() const;

    /// returns the variable if point (x,y) is within a
    /// I/O variable icon, null otherwise, indicating that the Group
    /// has been selected.
    ItemPtr select(float x, float y) const override;

    /// splits any wires that cross group boundaries
    void splitBoundaryCrossingWires();

    /// fix up subgroup pointers
    /// \a self is a shared pointer ref to this
    void normaliseGroupRefs(const std::shared_ptr<Group>& self);

    /// rotate all conatined items by 180 degrees
    void flipContents();

    /// return a list of existing variables a variable in this group
    /// could be connected to
    std::vector<std::string> accessibleVars() const;

    /// returns list of bookmark names for populating menu 
    std::vector<std::string> bookmarkList() {
      std::vector<std::string> r;
      for (auto& b: bookmarks) r.push_back(b.name);
      return r;
    }
    void addBookmarkXY(float dx, float dy, const std::string& name) {
      bookmarks.erase(Bookmark{0,0,0,name});
      bookmarks.emplace(x()-dx, y()-dy, relZoom*zoomFactor(), name);
    }
    void addBookmark(const std::string& name) {addBookmarkXY(0,0,name);}
    void deleteBookmark(std::size_t i) {
      if (i<bookmarks.size())
        {
          auto j=bookmarks.begin();
          std::advance(j, i);
          bookmarks.erase(j);
        }
    }
    void gotoBookmark_b(const Bookmark& b);
    void gotoBookmark(std::size_t i) {
      if (i<bookmarks.size()) {
          auto j=bookmarks.begin();
          std::advance(j, i);
          gotoBookmark_b(*j);
      }
    }
    
    /// return default extension for this group - .mky if no ravels in group, .rvl otherwise
    std::string defaultExtension() const;

    /// automatically lay out items in this group using a graph layout algorithm
    void autoLayout();
    /// randomly lay out items in this group
    void randomLayout();

    /// produce a summary of godley table variables
    std::vector<Summary> summariseGodleys() const;

    /// rename all instances of a variable matching \a valueId to \a newName
    void renameAllInstances(const std::string& valueId, const std::string& newName);
    
  };

  template <class M, class C>
  const typename M::value_type GroupItems::findAny(M GroupItems::*map, C c) const
  {
    for (auto& i: this->*map)
      if (c(i))
        return i;
    for (auto& g: groups)
      if (auto& r=g->findAny(map, c))
        return r;
    return typename M::value_type();
  }

  template <class R, class M, class C, class X>
  std::vector<R> GroupItems::findAll(C c, M (GroupItems::*m), X xfm) const {
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
  
}

#ifdef CLASSDESC
// omit these, because weak/shared pointers cause problems, and its
// not needed anyway
#pragma omit pack minsky::Group
#pragma omit unpack minsky::Group
#endif
namespace classdesc_access
{
  template <> struct access_pack<minsky::Group>: 
    public classdesc::NullDescriptor<classdesc::pack_t> {};
  template <> struct access_unpack<minsky::Group>: 
    public classdesc::NullDescriptor<classdesc::unpack_t> {};
}
#include "group.cd"
#include "group.xcd"
#endif
