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
#include "str.h"
#include "polyRESTProcessBase.h"

#include <json_pack_base.h>

#include <cairo.h>
#include <vector>
#include <cairo_base.h>

#include <iostream>
#include <assert.h>

#include <RESTProcess_base.h>

namespace minsky 
{
  struct LassoBox;
  struct Selection;
  class GroupItems;
  class Group; 
  class VariablePtr;
  class VariableBase;
  class OperationBase;
  class SwitchIcon;
  class PlotWidget;
  class GodleyIcon;
  class Ravel;

  class Item;
  typedef std::shared_ptr<Item> ItemPtr;

  /// represents whether a mouse click is on the item, on an output
  /// port (for wiring, or is actually outside the items boundary, and
  /// a lasso is intended
  struct ClickType
  {
    enum Type {onItem, onPort, outside, inItem, onResize, legendMove, legendResize};
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
    ItemPortVector(ItemPortVector&&) {}
    ItemPortVector& operator=(const ItemPortVector&) {return *this;}
    ItemPortVector& operator=(ItemPortVector&&) {return *this;}
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

  /// Item members excluded from reflection
  struct ItemExclude
  {
    /// owning group of this item.
    classdesc::Exclude<std::weak_ptr<Group>> group;

    virtual ~ItemExclude() {}
    
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
    /// @{ a more efficient replacement for dynamic_cast<PlotWidget*>(this)
    virtual const PlotWidget* plotWidgetCast() const {return nullptr;}
    virtual PlotWidget* plotWidgetCast() {return nullptr;}
    /// @}            
    /// @{ a more efficient replacement for dynamic_cast<GodleyIcon*>(this)
    virtual const GodleyIcon* godleyIconCast() const {return nullptr;}
    virtual GodleyIcon* godleyIconCast() {return nullptr;}
    /// @}            
    /// @{ a more efficient replacement for dynamic_cast<Ravel*>(this)
    virtual const Ravel* ravelCast() const {return nullptr;}
    virtual Ravel* ravelCast() {return nullptr;}
    /// @}            

    /// insert this items controlled or controller items are inserted
    /// correctly into \a selection.
    virtual void insertControlled(Selection& selection) {}
    /// remove all controlled items from a group
    virtual void removeControlledItems(GroupItems&) {}
    /// remove all controlled items from their owning group
    void removeControlledItems();
    
    double m_rotation=0; ///< rotation of icon, in degrees
    /// rotate item based on vector from \a orig to \a mouse
    void rotate(const Point& mouse, const Point& orig);
  };

  class Item: public NoteBase,
              public classdesc::PolyRESTProcessBase,
              public classdesc::Exclude<ItemExclude>
  {

  protected:
    // these need to be protected, not private to allow the setting of these in constructors.
    double m_width=20, m_height=20;
    ItemPortVector m_ports;
    
    mutable struct MemoisedRotator: public Rotate
    {
      MemoisedRotator(): Rotate(0,0,0) {}
      void update(float a,float x, float y) {
        if (!initialisedFrom(a,x,y))
          Rotate::operator=(Rotate(a,x,y));
      }
    } memoisedRotator;

    static void drawResizeHandle(cairo_t* cairo, double x, double y, double sf, double angle);
    

  public:
    Item()=default;
    float m_x=0, m_y=0; ///< position in canvas, or within group
    float m_sf=1; ///< scale factor of item on canvas, or within group
    mutable bool onResizeHandles=false; ///< set to true to indicate mouse is ovcaler resize handles
    bool onBorder=false; ///< true to indicate mouse hovering over border
    std::string deleteCallback; /// callback to be run when item deleted from group
    
    /// return a weak reference to the ith port
    virtual std::weak_ptr<Port> ports(std::size_t i) const {
      return i<m_ports.size()? m_ports[i]: nullptr;
    }
    /// number of ports
    std::size_t portsSize() const {return m_ports.size();}
    float portX(std::size_t i) {
      if (auto p=ports(i).lock()) return p->x();
      return 0;
    }
   
    float portY(std::size_t i) {
      if (auto p=ports(i).lock()) return p->y();
      return 0;
    }
    /// canvas bounding box.
    mutable BoundingBox bb;
    virtual bool contains(float xx, float yy) const {
      auto hz=resizeHandleSize(); // extend by resize handle size (which is also portRadius)
      return left()-hz<=xx && right()+hz>=xx && top()-hz<=yy && bottom()+hz>=yy; 
    }
    bool contains(const Point& p) const {return contains(p.x(),p.y());}
    void updateBoundingBox() override {bb.update(*this);}
    
    /// mark item on canvas, then throw
    [[noreturn]] void throw_error(const std::string&) const;

    /// indicates this is a group I/O variable
    virtual bool ioVar() const {return false;}
    /// current value of output port
    virtual double value() const {return 0;}

    double rotation() const {return m_rotation;}
    double rotation(const double& r) {return m_rotation=r;}

    /// return the rotation as radians, and whether rotation should have additional straight angle added for text that stays upright.
    std::pair<double,bool> rotationAsRadians() const;
    
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
    virtual void flip() {rotation(rotation()+180);}

    virtual std::string classType() const {return "Item";}
    /// return an id uniquely identifying this item
    std::string id() const {return str(size_t(this));}

    virtual float x() const; 
    virtual float y() const;
    virtual float zoomFactor() const;
    void ensureBBValid() const {if (!bb.valid()) bb.update(*this);}
    float width()  const {return right()-left();}
    float height() const {return bottom()-top();}
    virtual std::vector<Point> corners() const; // 4 corners of item
    float left()   const;
    float right()  const;
    float top()    const;
    float bottom() const;

    /// Id of bookmark associated with this
    std::string bookmarkId() const {return tooltip().empty()? std::to_string(size_t(this)): tooltip();}
    void adjustBookmark() const override;
    
    /// resize handles should be at least a percentage if the icon size (#1025)
    float resizeHandleSize() const {return std::max(portRadius*zoomFactor(), std::max(0.02f*width(), 0.02f*height()));}
    /// @return true is (x,y) is located on a resize handle
    virtual bool onResizeHandle(float x, float y) const;
    /// @return true if item internally responds to the mouse, and (x,y) is within editable area
    virtual bool inItem(float x, float y) const {return false;}
    /// returns true if (x,y) is on the icon
    virtual bool onItem(float x, float y) const;
    /// respond to mouse down events
    virtual void onMouseDown(float x, float y) {}
    /// respond to mouse up events
    virtual void onMouseUp(float x, float y) {}
    /// respond to mouse motion events with button pressed
    /// @return true if it needs to be rerendered
    virtual bool onMouseMotion(float x, float y) {return false;}
    /// respond to mouse motion events (hover) without button pressed
    /// @return true if it needs to be rerendered
    virtual bool onMouseOver(float x, float y) {return false;}
    /// respond to mouse leave events (when mouse leaves item)
    virtual void onMouseLeave() {}
    /// respond to key press events
    /// @return true if it needs to be rerendered
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
    virtual void displayTooltip(cairo_t*, const std::string&) const;
    
    /// update display after a step()
    virtual void updateIcon(double t) {}

    Item(const Item&)=default;
    Item& operator=(const Item&)=default;
    virtual ~Item() {}

    void drawPorts(cairo_t* cairo) const;
    static void drawSelected(cairo_t* cairo);
    virtual void drawResizeHandles(cairo_t* cairo) const;
    
    /// returns the clicktype given a mouse click at \a x, \a y.
    virtual ClickType::Type clickType(float x, float y) const;

    /// returns closest output port to \a x,y
    virtual std::shared_ptr<Port> closestOutPort(float x, float y) const; 
    virtual std::shared_ptr<Port> closestInPort(float x, float y) const;

    /// returns the variable if point (x,y) is within a
    /// visible variable icon, null otherwise.
    virtual std::shared_ptr<Item> select(float x, float y) const {return {};}
    /// runs the RESTProcess descriptor suitable for this type
    void RESTProcess(classdesc::RESTProcess_t& rp,const std::string& d) override
    {::RESTProcess(rp,d,*this);}
    void RESTProcess(classdesc::RESTProcess_t& rp,const std::string& d) const override
    {::RESTProcess(rp,d,*this);}
    virtual void json_pack(classdesc::json_pack_t& j) const
    {::json_pack(j,"",*this);}

    /// enable extended tooltip help message appropriate for mouse at (x,y)
    virtual void displayDelayedTooltip(float x, float y) {}
    virtual void disableDelayedTooltip() {}

    /// some items have an editor mode attribute
    virtual bool editorMode() const {return false;}
    virtual void toggleEditorMode() {}
    
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

    /// return a shared_ptr to this
    ItemPtr itemPtrFromThis() const;

    /// destroy any popup windows associated with this
    virtual void destroyFrame() {}
  };

  typedef std::vector<ItemPtr> Items;
  
  struct BottomRightResizerItem: public Item
  {
    bool onResizeHandle(float x, float y) const override; 
    void drawResizeHandles(cairo_t* cairo) const override;
    /// returns coordinates of the resizer handle
    virtual Point resizeHandleCoords() const;
  };

}

#ifdef CLASSDESC
// omit these, because weak/shared pointers cause problems, and its
// not needed anyway
#pragma omit pack minsky::Item
#pragma omit unpack minsky::Item

// omit ItemExclude to reduce the amount of boilerplate code needing to be compiled
#pragma omit pack minsky::ItemExclude
#pragma omit unpack minsky::ItemExclude
#pragma omit json_pack minsky::ItemExclude
#pragma omit json_unpack minsky::ItemExclude
#pragma omit xml_pack minsky::ItemExclude
#pragma omit xml_unpack minsky::ItemExclude
#pragma omit TCL_obj minsky::ItemExclude
#pragma omit RESTProcess minsky::ItemExclude

#endif
namespace classdesc_access
{
  template <> struct access_pack<minsky::Item>: 
    public classdesc::NullDescriptor<classdesc::pack_t> {};
  template <> struct access_unpack<minsky::Item>: 
    public classdesc::NullDescriptor<classdesc::unpack_t> {};
}
#include "item.cd"
#include "item.xcd"
#endif

