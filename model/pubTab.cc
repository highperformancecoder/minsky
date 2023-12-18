/*
  @copyright Steve Keen 2023
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

#include "lasso.h"
#include "minsky.h"
#include "pubTab.h"
#include "pubTab.xcd"
#include "pubTab.rcd"
#include "pannableTab.rcd"
#include "pannableTab.xcd"
#include "minsky_epilogue.h"
using namespace ecolab::cairo;

namespace minsky
{
  Point PubItem::itemCoords(float x, float y) const
  {
    if (!itemRef) return {0,0};
    float scale=itemRef->zoomFactor()/zoomFactor;
    return {scale*(x-this->x)+itemRef->x(), scale*(y-this->y)+itemRef->y()};
  }

  void PubTab::removeSelf()
  {
    auto& publicationTabs=minsky::minsky().publicationTabs;
    for (auto i=publicationTabs.begin(); i!=publicationTabs.end(); ++i)
      if (this==&*i) {
        publicationTabs.erase(i);
        return;
      }
  }

  void PubTab::removeItemAt(float x, float y)
  {
    if (auto item=m_getItemAt(x-offsx,y-offsy))
        for (auto i=items.begin(); i!=items.end(); ++i)
          if (&*i==item)
            {
              items.erase(i);
              requestRedraw();
              return;
            }
  }

  void PubTab::rotateItemAt(float x, float y)
  {
    item=m_getItemAt(x-offsx,y-offsy);
    if (item) {
      rx=x;
      ry=y;
      rotating=true;
    }
  }
 
  bool PubTab::redraw(int x0, int y0, int width, int height)
  {
    if (!surface.get()) {
      return false;
    }
    auto cairo=surface->cairo();
    CairoSave cs(cairo);
    cairo_translate(cairo, offsx, offsy);
    cairo_scale(cairo, m_zoomFactor, m_zoomFactor);
    for (auto& i: items)
      {
        CairoSave cs(cairo);
        cairo_translate(cairo, i.x, i.y);
        cairo_scale(cairo, i.zoomFactor, i.zoomFactor);
        cairo_rotate(cairo,(M_PI/180)*i.rotation-i.itemRef->rotation());
        try
          {
            i.itemRef->draw(cairo);
          }
        catch (...) {}
      }
    if (resizing)
      {
        cairo_rectangle(cairo,std::min(lasso.x0,lasso.x1), std::min(lasso.y0,lasso.y1),
                        abs(lasso.x0-lasso.x1), abs(lasso.y0-lasso.y1));
        cairo_stroke(cairo);
      }
    return !items.empty() || resizing;
  }

  PubItem* PubTab::m_getItemAt(float x, float y) 
  {
    for (auto& i: items)
      if (i.itemRef->contains(i.itemCoords(x,y)))
          return &i;
    return nullptr;
  }

  void PubTab::mouseDown(float x, float y)
  {
    x-=offsx; y-=offsy;
    item=m_getItemAt(x,y);
    if (item)
      if (auto p=item->itemCoords(x,y);
          item->itemRef->clickType(p.x(),p.y())==ClickType::onResize)
        {
          resizing=true;
          auto scale=item->zoomFactor/item->itemRef->zoomFactor();
          lasso.x0=x>item->x? x-item->itemRef->width()*scale: x+item->itemRef->width()*scale;
          lasso.y0=y>item->y? y-item->itemRef->height()*scale: y+item->itemRef->height()*scale;
          lasso.x1=x;
          lasso.y1=y;
        }
  }
  
  void PubTab::mouseUp(float x, float y)
  {
    if (panning)
      {
        PannableTab<PubTabBase>::mouseUp(x,y);
        panning=false;
        return;
      }
    mouseMove(x,y);
    if (item && resizing)
      {
        item->zoomFactor=std::min(
                                  abs(lasso.x1-lasso.x0)/item->itemRef->width(),
                                  abs(lasso.y1-lasso.y0)/item->itemRef->height());
        item->x=0.5*(lasso.x0+lasso.x1);
        item->y=0.5*(lasso.y0+lasso.y1);
      }
    item=nullptr;
    resizing=false;
    rotating=false;
  }
  
  void PubTab::mouseMove(float x, float y)
  {
    x-=offsx; y-=offsy;
    if (panning)
      {
        PannableTab<PubTabBase>::mouseMove(x,y);
        return;
      }
    for (auto& i: items) i.itemRef->mouseFocus=false;
    if (item)
      {
        if (rotating)
          {
            item->rotation=(180/M_PI)*atan2(x-rx, y-ry);
          }
        else if (resizing)
          {
            lasso.x1=x;
            lasso.y1=y;
          }
        else
          {
            item->x=x;
            item->y=y;
          }
      }
    else
      // indicate mouse focus
      if (auto i=m_getItemAt(x,y))
        i->itemRef->mouseFocus=true;
    requestRedraw();
  }
}
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::PubTab);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::PubItem);
