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

#ifndef ITEM_H
#define ITEM_H

#include "noteBase.h"
#include "port.h"
#include "intrusiveMap.h"
#include <TCL_obj_base.h>

#include <cairo.h>
#include <vector>
#include <cairo_base.h>

namespace minsky 
{
  class Group;
  class LassoBox;

  /// represents whether a mouse click is on the item, on an output
  /// port (for wiring, or is actually outside the items boundary, and
  /// a lasso is intended
  struct ClickType
  {
    enum Type {onItem, onPort, outside, onSlider};
  };

  /// radius of circle marking ports at zoom=1
  constexpr float portRadius=6;

  // ports are owned by their items, so it is not appropriate to
  // default copy the port references
  struct ItemPortVector: public std::vector<std::shared_ptr<Port> >
  {
    ItemPortVector() {}
    ItemPortVector(const ItemPortVector&) {}
    ItemPortVector& operator=(const ItemPortVector&) {return *this;}
  };

  class VariablePtr;

  class Item;
  /// bounding box information (at zoom=1 scale)
  class BoundingBox
  {
    float left=0, right=0, top, bottom;
  public:
    void update(const Item& x);
    bool contains(float x, float y) const {
      return left<=x && right>=x && bottom>=y && top<=y;
    }
    bool valid() const {return left!=right;}
  };
  
  class Item: public NoteBase
  {
  public:
    float m_x=0, m_y=0; ///< position in canvas, or within group
    float zoomFactor=1;
    double rotation=0; ///< rotation of icon, in degrees
    bool m_visible=true; ///< if false, then this item is invisible
    std::weak_ptr<Group> group;
    /// canvas bounding box.
    mutable BoundingBox bb;
    bool contains(float xx, float yy) {
      if (!bb.valid()) bb.update(*this);
      return bb.contains((xx-x())/zoomFactor, (yy-y())/zoomFactor);
    }
    
    /// indicates this is a group I/O variable
    virtual bool ioVar() const {return false;}

    /// rotate icon though 180∘
    void flip() {rotation+=180;}

    virtual std::string classType() const {return "Item";}
    /// sets the cairo surface, allow redraws to be requested
    virtual void setCairoSurface(const ecolab::cairo::SurfacePtr&) {}

    ItemPortVector ports;
    float x() const; 
    float y() const; 

    virtual void resize(const LassoBox&) {}

    /// delete all attached wires
    virtual void deleteAttachedWires();
    
    virtual Item* clone() const {return new Item(*this);}

    /// whether this item is visible on the canvas. 
    bool visible() const;

    void moveTo(float x, float y);
    /// zoom by \a factor, scaling all widget's coordinates, using (\a
    /// xOrigin, \a yOrigin) as the origin of the zoom transformation
    virtual void zoom(float xOrigin, float yOrigin,float factor);

    /// draw this item into a cairo context
    virtual void draw(cairo_t* cairo) const;
    /// draw into a dummy cairo context, for purposes of calculating
    /// port positions
    void dummyDraw() const;

    /// display tooltip text, eg on mouseover
    void displayTooltip(cairo_t*) const;
    
    /// update display after a step()
    virtual void updateIcon(double t) {}
    virtual ~Item() {}

    void drawPorts(cairo_t* cairo) const;
    void drawSelected(cairo_t* cairo) const;
    
    /// returns the clicktype given a mouse click at \a x, \a y.
    virtual ClickType::Type clickType(float x, float y);

    /// returns closest output port to \a x,y
    virtual std::shared_ptr<Port> closestOutPort(float x, float y) const; 
    virtual std::shared_ptr<Port> closestInPort(float x, float y) const;

    /// respond to arrow keys.
    /// @param dir = -1/1 if left/down, right/up pressed
    /// @return true if state changed, and item needs to be redrawn
    virtual bool handleArrows(int dir) {return false;}
    
    /// returns the variable if point (x,y) is within a
    /// visible variable icon, null otherwise.
    virtual std::shared_ptr<Item> select(float x, float y) const;
    virtual void TCL_obj(classdesc::TCL_obj_t& t, const classdesc::string& d)
    {::TCL_obj(t,d,*this);}

  };

  typedef std::shared_ptr<Item> ItemPtr;
  typedef std::vector<ItemPtr> Items;

  /** curiously recursive template pattern for generating overrides */
  template <class T, class Base=Item>
  struct ItemT: virtual public Base
  {
    std::string classType() const override {
      auto s=classdesc::typeName<T>();
      // remove minsky namespace
      static const char* ns="minsky::";
      static const int eop=strlen(ns);
      if (s.substr(0,eop)==ns)
        s=s.substr(eop);
      return s;
    }
    ItemT* clone() const override {
      auto r=new T(*dynamic_cast<const T*>(this));
      r->group.reset();
      return r;
    }
    void TCL_obj(classdesc::TCL_obj_t& t, const classdesc::string& d) override
    {::TCL_obj(t,d,*dynamic_cast<T*>(this));}
  };
  
}

#ifdef CLASSDESC
// omit these, because weak/shared pointers cause problems, and its
// not needed anyway
#pragma omit pack minsky::Item
#pragma omit unpack minsky::Item
#endif
namespace classdesc_access
{
  template <> struct access_pack<minsky::Item>: 
    public classdesc::NullDescriptor<classdesc::pack_t> {};
  template <> struct access_unpack<minsky::Item>: 
    public classdesc::NullDescriptor<classdesc::unpack_t> {};

  // this implements polymorphic TCL_obj drilldown
  template <>
  struct access_TCL_obj<minsky::ItemPtr>
  {
    template <class U>
    void operator()(cd::TCL_obj_t& t, const cd::string& d, U& a)
    {
      if (a) a->TCL_obj(t,d);
    }
  };
}
#include "item.cd"

#endif

