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

#include "item.h"
#include "group.h"
#include "zoom.h"
#include <cairo_base.h>
#include <ecolab_epilogue.h>

namespace minsky
{

  float Item::x() const 
  {
    if (auto g=group.lock())
      return m_x+g->x();
    else
      return m_x;
  }

  float Item::y() const 
  {
    if (auto g=group.lock())
      return m_y+g->y();
    else
      return m_y;
  }

  bool Item::visible() const 
  {
    if (auto g=group.lock())
      return m_visible && g->displayContents();
    else
      return m_visible;
  }
  

  void Item::moveTo(float x, float y)
  {
    if (auto g=group.lock())
      {
        m_x=x-g->x();
        m_y=y-g->y();
      }
    else
      {
        m_x=x;
        m_y=y;
      }
    assert(x==this->x() && y==this->y());
  }

  ClickType::Type Item::clickType(float x, float y)
  {
    // firstly, check whether a port has been selected
    for (auto& p: ports)
      {
        if (hypot(x-p->x(), y-p->y()) < portRadius*zoomFactor)
          return ClickType::onPort;
      }
    ecolab::cairo::Surface dummySurf
      (cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,nullptr));
    draw(dummySurf.cairo());
    if (cairo_in_clip(dummySurf.cairo(), (x-this->x()), (y-this->y())))
      return ClickType::onItem;
    else
      return ClickType::outside;
  }

  void Item::zoom(float xOrigin, float yOrigin,float factor)
  {
    if (visible())
      {
        auto g=group.lock();
        if (!g)
          {
            minsky::zoom(m_x,xOrigin,factor);
            minsky::zoom(m_y,yOrigin,factor);
          }
        else if (g->displayContents())
          {
            m_x*=factor;
            m_y*=factor;
          }
        zoomFactor*=factor;
      }
  }


}
