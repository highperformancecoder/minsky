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

#include "wire.h"

#include <cairo_base.h>

#include <vector>

namespace minsky
{
  /// represents items that have been selected
  struct Selection
  {
    std::vector<int> wires, operations, variables, groups, godleys, plots;

    /// group within which selection refers (-1 == no group)
    int group;

    void clear() {
      wires.clear(); operations.clear(); variables.clear();
      groups.clear(); godleys.clear(); plots.clear();
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
      ecolab::cairo::Surface dummySurf
        (cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,nullptr));
      cairo_rectangle(dummySurf.cairo(), x0-item.x(), y0-item.y(), x1-x0, y1-y0);
      cairo_clip(dummySurf.cairo());
      item.draw(dummySurf.cairo());
      double x,y,w,h;
      cairo_recording_surface_ink_extents(dummySurf.surface(), &x, &y, &w, &h);
      return w>0 && h>0;
    }

    

    /// return true if both endpoints of the wire lie
    /// within the lasso
    bool contains(const Wire& wire) const {
      ecolab::array<float> c=wire.coords();
      return c[0]>=x0 && c[0]<=x1 && c[1]>=y0 && c[1]<=y1 &&
              c[c.size()-2]>=x0 && c[c.size()-2]<=x1 &&
              c[c.size()-1]>=y0 && c[c.size()-1]<=y1;
    }
  };
}

#include "selection.cd"
#endif
