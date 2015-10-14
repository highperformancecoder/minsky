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

#ifndef CLICKTYPE_H
#define CLICKTYPE_H

#include "portManager.h"
#include "cairo_base.h"
#include <math.h>

namespace minsky
{
  /// represents whether a mouse click is on the item, on an output
  /// port (for wiring, or is actually outside the items boundary, and
  /// a lasso is intended
  struct ClickType
  {
    enum Type {onItem, onPort, outside};
  };

  /// radius of circle marking ports at zoom=1
  constexpr float portRadius=6;

  /// returns the clicktype for \a item, given a mouse click at \a x, \a y.
  /// assumes that the icon is bounded by it's clip path
  template <class Item>
  ClickType::Type clickType(Item& item, float x, float y)
  {
    // firstly, check whether a port has been selected
    for (int p: item.ports())
      {
        const Port& port=cportManager().ports[p];
        if (hypot(x-port.x(), y-port.y()) < portRadius*item.zoomFactor)
          return ClickType::onPort;
      }
    ecolab::cairo::Surface dummySurf
      (cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,nullptr));
    item.draw(dummySurf.cairo());
    if (cairo_in_clip(dummySurf.cairo(), (x-item.x()), (y-item.y())))
      return ClickType::onItem;
    else
      return ClickType::outside;
  }

}

#include "clickType.cd"
#endif
