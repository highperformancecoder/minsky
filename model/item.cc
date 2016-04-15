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
#include "latexMarkup.h"
#include <pango.h>
#include <cairo_base.h>
#include <ecolab_epilogue.h>

using ecolab::Pango;

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

  void Item::drawPorts(cairo_t* cairo) const
  {
    cairo_save(cairo);
    for (auto& p: ports)
      {
        cairo_new_sub_path(cairo);
        cairo_arc(cairo, p->x()-x(), p->y()-y(), portRadius*zoomFactor, 0, 2*M_PI);
      }
    cairo_set_source_rgb(cairo, 0,0,0);
    cairo_set_line_width(cairo,1);
    cairo_stroke(cairo);
    cairo_restore(cairo);
  }

  // default is just to display the detailed text (ie a "note")
  void Item::draw(cairo_t* cairo) const
  {
    Pango pango(cairo);
    pango.setMarkup(latexToPango(detailedText)); 
    pango.show();
  }

  namespace
  {
    inline float sqr(float x) {return x*x;}
  }

  shared_ptr<Port> Item::closestInPort(float x, float y) const
  {
    shared_ptr<Port> r;
    for (size_t i=1; i<ports.size(); ++i)
      if (!r || sqr(ports[i].x()-x)+sqr(ports[i].y()-y) <
          sqr(r->x()-x)+sqr(r->y()-y))
        r=ports[i];
    return r;
  }

}
