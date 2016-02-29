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

#include <cairo.h>
#include <vector>

namespace minsky 
{
  class Group;

  /// represents whether a mouse click is on the item, on an output
  /// port (for wiring, or is actually outside the items boundary, and
  /// a lasso is intended
  struct ClickType
  {
    enum Type {onItem, onPort, outside};
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

  class Item: public NoteBase
  {
  public:
    float m_x, m_y; ///< position in canvas, or within group
    float zoomFactor;
    double rotation; ///< rotation of icon, in degrees
    bool m_visible=true; ///< if false, then this item is invisible
    std::weak_ptr<Group> group;
  
    ItemPortVector ports;
    float x() const; 
    float y() const; 

    virtual Item* clone() const {return new Item(*this);}

    /// whether this item is visible on the canvas. 
    bool visible() const;

    void moveTo(float x, float y);
    void zoom(float xOrigin, float yOrigin,float factor);

    /// draw this item into a cairo context
    virtual void draw(cairo_t* cairo) const {}
    virtual ~Item() {}

    void drawPorts(cairo_t* cairo) const;

    /// returns the clicktype given a mouse click at \a x, \a y.
    ClickType::Type clickType(float x, float y);
  };

  class ItemPtr: public std::shared_ptr<Item>
  {
  public:
    virtual int id() const {return -1;}
    virtual ~ItemPtr() {}

    template <class... A> ItemPtr(A... x):
      std::shared_ptr<Item>(std::forward<A>(x)...) {}
  };

  typedef IntrusiveMap<int, ItemPtr> Items;

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
}
#include "item.cd"
#endif

