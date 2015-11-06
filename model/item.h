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

  class Item: public NoteBase
  {
  public:
    float m_x, m_y; ///< position in canvas, or within group
    float zoomFactor;
    double rotation; ///< rotation of icon, in degrees
    std::weak_ptr<Group> group;
  
    std::vector<std::shared_ptr<Port> > ports;
    float x() const; 
    float y() const; 

    void moveTo(float x, float y);

    /// draw this item into a cairo context (circle is just some prelimary scaffolding)
    virtual void draw(cairo_t* cairo) {cairo_arc(cairo,0,0,3,0,2*M_PI);}
    virtual ~Item() {}
  };

  class ItemPtr: public std::shared_ptr<Item>
  {
  public:
    virtual int id() const {return -1;}

    template <class... A> ItemPtr(A... x):
      std::shared_ptr<Item>(std::forward<A>(x)...) {}
  };

  typedef IntrusiveMap<int, ItemPtr> Items;

}

#include "item.cd"
#endif

