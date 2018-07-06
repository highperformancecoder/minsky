/*
  @copyright Steve Keen 2013
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

#ifndef SELECTION_H
#define SELECTION_H

#include "group.h"

#include <cairo_base.h>

#include <vector>

namespace minsky
{
  /// represents items that have been selected
  struct Selection: public Group
  {
    void clear() {
      for (auto& i: items) i->selected=false;
      for (auto& i: groups) i->selected=false;
      Group::clear();
    }
  };

  /// represents rectangular region of a lasso operation
  struct LassoBox
  {
    float x0, y0, x1, y1;
    LassoBox(float x0, float y0, float x1, float y1): 
      x0(x0), y0(y0), x1(x1), y1(y1) {
      if (x0>x1) std::swap(this->x0,this->x1);
      if (y0>y1) std::swap(this->y0,this->y1);
    }
    /// returns whether item's icon overlaps the lasso
    template <class Item>
    bool intersects(const Item& item) const {
      return item.right() >= x0 && item.left() <= x1 &&
        item.top() >= y0 && item.bottom() <= y1;
    }

    /// return true if both endpoints of the wire lie
    /// within the lasso
    bool contains(const Wire& wire) const {
      auto c=wire.coords();
      return c[0]>=x0 && c[0]<=x1 && c[1]>=y0 && c[1]<=y1 &&
              c[c.size()-2]>=x0 && c[c.size()-2]<=x1 &&
              c[c.size()-1]>=y0 && c[c.size()-1]<=y1;
    }

  };
}

#include "selection.cd"
#endif
