/*
  @copyright Steve Keen 2021
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

#ifndef LASSO_H
#define LASSO_H

namespace minsky
{
  class Wire;

  /// represents rectangular region of a lasso operation
  struct LassoBox
  {
    float x0=0, y0=0, x1=0, y1=0, angle=0;
    LassoBox() {}
    LassoBox(float x0, float y0, float x1, float y1); 
    
    /// returns whether item's icon overlaps the lasso
    template <class Item>
    bool intersects(const Item& item) const {
      return item.right() >= x0 && item.left() <= x1 &&
        item.bottom() >= y0 && item.top() <= y1;
    }

    /// return true if both endpoints of the wire lie
    /// within the lasso
    bool contains(const Wire& wire) const;
  };
}

#include "lasso.cd"
#include "lasso.xcd"
#include "lasso.rcd"
#endif
