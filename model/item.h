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
#include "geometry.h"
//#include "RESTProcess_base.h"
#include <accessor.h>
#include <TCL_obj_base.h>

#include <cairo.h>
#include <vector>
#include <cairo_base.h>

namespace minsky 
{
  struct LassoBox;
  struct Selection;
  class Group;
  class VariablePtr;
  class VariableBase;
  class OperationBase;
  class SwitchIcon;

  class Item;
  typedef std::shared_ptr<Item> ItemPtr;

  /// represents whether a mouse click is on the item, on an output
  /// port (for wiring, or is actually outside the items boundary, and
  /// a lasso is intended
  struct ClickType
  {
    enum Type {onItem, onPort, outside, onSlider, inItem, onResize, legendMove, legendResize};
  };

  /// radius of circle marking ports at zoom=1
  constexpr float portRadius=6;
  constexpr float portRadiusMult=2.0f*portRadius;  

  // ports are owned by their items, so it is not appropriate to
  // default copy the port references
  struct ItemPortVector: public std::vector<std::shared_ptr<Port> >
  {
    ItemPortVector() {}
    ItemPortVector(const ItemPortVector&) {}
    ItemPortVector& operator=(const ItemPortVector&) {return *this;}
  };

  /// bounding box information (at zoom=1 scale)
  class BoundingBox
  {
    float m_left=0, m_right=0, m_top, m_bottom;  	  
  public:
    void update(const Item& x);
    bool contains(float x, float y) const {
      // extend each item by a portradius to solve ticket #903
      return m_left-portRadius<=x && m_right+portRadius>=x && m_top-portRadius<=y && m_bottom+portRadius>=y;
    }
    bool valid() const {return m_left!=m_right;}
    float width() const {return m_right-m_left;}
    float height() const {return m_bottom-m_top;}
    float left() const {return m_left;}
    float right() const {return m_right;}
    float top() const {return m_top;}
    float bottom() const {return m_bottom;}
  };

  class Item: virtual public NoteBase, public ecolab::TCLAccessor<Item,double>
  {
    double m_rotation=0; ///< rotation of icon, in degrees
  protected:
    // these need to be protected, not private to allow the setting of these in constructors.
    double m_width=0, m_height=0;
  public:

    Item(): TCLAccessor<Item,double>("rotation",(Getter)&Item::rotation,(Setter)&Item::rotation) {}
    float m_x=0, m_y=0; ///< position in canvas, or within group
    float m_sf=1; ///< scale factor of item on canvas, or within group
    mutable bool onResizeHandles=false; ///< set to true to indicate mouse is over resize handles
    bool onBorder=false; ///< true to indicate mouse hovering over border
    std::string deleteCallback; /// callback to be run when item deleted from group
    /// owning group of this item.
    classdesc::Exclude<std::weak_ptr<Group>> group; 
    /// canvas bounding box.
    mutable BoundingBox bb;
    bool contains(float xx, float yy) {
      if (!bb.valid()) bb.update(*this);
      float invZ=1/zoomFactor();
      return bb.contains((xx-x())*invZ, (yy-y())*invZ);
    }
    void updateBoundingBox() {bb.update(*this);}
    
    /// mark item on canvas, then throw
    [[noreturn]] void throw_error(const std::string&) const;

    /// indicates this is a group I/O variable
    virtual bool ioVar() const {return false;}
    /// current value of output port
    virtual double value() const {return 0;}

    double rotation() const {return m_rotation;}
    double rotation(const double& r) {
      m_rotation=r;
      bb.update(*this);
      return m_rotation;
    }
    
    float iWidth() const {return m_width;}
    float iWidth(const float& w) {
      m_width=w;
      bb.update(*this);
      return m_width;
    }
    
    float iHeight() const {return m_height;}
    float iHeight(const float& h) {
      m_height=h;
      bb.update(*this);
      return m_height;
    }         
    
    /// rotate icon though 180∘
    void flip() {rotation(rotation()+180);}

    virtual std::string classType() const {return "Item";}

    /// @{ a more efficient replacement for dynamic_cast<VariableBase*>(this)
    virtual const VariableBase* variableCast() const {return nullptr;}
    virtual VariableBase* variableCast() {return nullptr;}
    /// @}
    /// @{ a more efficient replacement for dynamic_cast<OperationBase*>(this)
    virtual const OperationBase* operationCast() const {return nullptr;}
    virtual OperationBase* operationCast() {return nullptr;}
    /// @}
    /// @{ a more efficient replacement for dynamic_cast<SwitchIcon*>(this)
    virtual const SwitchIcon* switchIconCast() const {return nullptr;}
    virtual SwitchIcon* switchIconCast() {return nullptr;}
    /// @}      

    ItemPortVector ports;
    virtual float x() const; 
    virtual float y() const;
    virtual float zoomFactor() const;
    void ensureBBValid() const {if (!bb.valid()) bb.update(*this);}
    float width()  const {ensureBBValid(); return bb.width()*zoomFactor();}
    float height() const {ensureBBValid(); return bb.height()*zoomFactor();}
    float left()   const {ensureBBValid(); return x()+bb.left()*zoomFactor();}
    float right()  const {ensureBBValid(); return x()+bb.right()*zoomFactor();}
    float top()    const {ensureBBValid(); return y()+bb.top()*zoomFactor();}
    float bottom() const {ensureBBValid(); return y()+bb.bottom()*zoomFactor();}

    /// resize handles should be at least a percentage if the icon size (#1025)
    float resizeHandleSize() const {return std::max(portRadius*zoomFactor(), std::max(0.02f*width(), 0.02f*height()));}
    /// @return true is (x,y) is located on a resize handle
    virtual bool onResizeHandle(float x, float y) const;
    /// @return true if item internally responds to the mouse, and (x,y) is within editable area
    virtual bool inItem(float x, float y) const {return false;}
    /// respond to mouse down events
    virtual void onMouseDown(float x, float y) {}
    /// respond to mouse up events
    virtual void onMouseUp(float x, float y) {}
    /// respond to mouse motion events with button pressed
    virtual bool onMouseMotion(float x, float y) {return false;}
    /// respond to mouse motion events (hover) without button pressed
    virtual bool onMouseOver(float x, float y) {return false;}
    /// respond to mouse leave events (when mouse leaves item)
    virtual void onMouseLeave() {}
    /// respond to key press events
    virtual bool onKeyPress(int keySym, const std::string& utf8, int state)
    {return false;}

    /// delete all attached wires
    virtual void deleteAttachedWires();
    
    virtual Item* clone() const {
      auto r=new Item(*this);
      r->group.reset();
      return r;
    }

    /// whether this item is visible on the canvas. 
    virtual bool visible() const;
    
    /// whether this item is attached to a defining variable that is hidden
    virtual bool attachedToDefiningVar() const;    

    void moveTo(float x, float y);

    /// draw this item into a cairo context
    virtual void draw(cairo_t* cairo) const;
    /// resize this item on the canvas
    virtual void resize(const LassoBox& b);
    /// factor by which item has been resized
    virtual float scaleFactor() const;
    virtual float scaleFactor(const float& sf);
    
    /// draw into a dummy cairo context, for purposes of calculating
    /// port positions
    void dummyDraw() const;

    /// display tooltip text, eg on mouseover
    void displayTooltip(cairo_t*, const std::string&) const;
    
    /// update display after a step()
    virtual void updateIcon(double t) {}
    virtual ~Item() {}

    void drawPorts(cairo_t* cairo) const;
    void drawSelected(cairo_t* cairo) const;
    virtual void drawResizeHandles(cairo_t* cairo) const;
    virtual std::pair<double,Point> rotatedPoints() const;    
    
    /// returns the clicktype given a mouse click at \a x, \a y.
    virtual ClickType::Type clickType(float x, float y);

    /// returns closest output port to \a x,y
    virtual std::shared_ptr<Port> closestOutPort(float x, float y) const; 
    virtual std::shared_ptr<Port> closestInPort(float x, float y) const;

    /// returns the variable if point (x,y) is within a
    /// visible variable icon, null otherwise.
    virtual std::shared_ptr<Item> select(float x, float y) const {return {};}
    virtual void TCL_obj(classdesc::TCL_obj_t& t, const classdesc::string& d)
    {::TCL_obj(t,d,*this);}
    /// returns a RESTProcessor appropriate for this item type
//    virtual std::unique_ptr<classdesc::RESTProcessBase> restProcess()
//    {
//      return std::unique_ptr<classdesc::RESTProcessBase>
//        (new classdesc::RESTProcessObject<Item>(*this));
//    }

    /// enable extended tooltip help message appropriate for mouse at (x,y)
    virtual void displayDelayedTooltip(float x, float y) {}
    virtual void disableDelayedTooltip() {}
    /// compute the dimensional units
    /// @param check - if true, then perform consistency checks
    /// @throw if check=true and dimensions inconsistent
    // all items feeding into other items must implement this
    virtual Units units(bool check=false) const {
      if (check) throw_error("units not implemented");
      return {};
    }
    /// perform units consistency checks
    Units checkUnits() const {return units(true);}
    /// insert this items controlled or controller items are inserted
    /// correctly into \a selection.
    virtual void insertControlled(Selection& selection) {}
    /// remove all controlled items from a group
    virtual void removeControlledItems(Group&) const {}
    /// remove all controlled items their owning group
    void removeControlledItems() const;

    /// return a shared_ptr to this
    ItemPtr itemPtrFromThis() const;
  };

  typedef std::vector<ItemPtr> Items;
  
  /** curiously recursive template pattern for generating overrides */
  template <class T, class Base=Item>
  struct ItemT: virtual public Base
  {
    std::string classType() const override {
      auto s=classdesc::typeName<T>();
      // remove minsky namespace
      static const char* ns="::minsky::";
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
//    std::unique_ptr<classdesc::RESTProcessBase> restProcess() override
//    {
//      return std::unique_ptr<classdesc::RESTProcessBase>
//        (new classdesc::RESTProcessObject<T>(dynamic_cast<T&>(*this)));
//    }
  };

  struct BottomRightResizerItem: public Item
  {
    bool onResizeHandle(float x, float y) const override; 
    void drawResizeHandles(cairo_t* cairo) const override;
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
#include "item.xcd"

#endif

