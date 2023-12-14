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
#include "pubTab.h"
#include "pubTab.xcd"
#include "pubTab.rcd"
#include "pannableTab.rcd"
#include "pannableTab.xcd"
#include "minsky_epilogue.h"
using namespace ecolab::cairo;

namespace minsky
{
  bool PubTab::redraw(int x0, int y0, int width, int height)
  {
    if (!surface.get()) {
      return false;
    }
    auto cairo=surface->cairo();
    CairoSave cs(cairo);
    cairo_translate(cairo, offsx, offsy);
    for (auto& i: items)
      {
        CairoSave cs(cairo);
        cairo_translate(cairo, i.x, i.y);
        try
          {
            i.itemRef->draw(cairo);
          }
        catch (...) {}
      }
    return !items.empty();
  }

  void PubTab::mouseDown(float x, float y)
  {
    x-=offsx; y-=offsy;
    for (auto& i: items)
      if (i.itemRef->contains(x-i.x+i.itemRef->x(), y-i.y+i.itemRef->y()))
        item=&i;
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
    item=nullptr;
  }
  
  void PubTab::mouseMove(float x, float y)
  {
    x-=offsx; y-=offsy;
    if (panning)
      {
        PannableTab<PubTabBase>::mouseMove(x,y);
        return;
      }
    if (item)
      {
        item->x=x;
        item->y=y;
        requestRedraw();
      }
  }
  void PubTab::zoom(double x, double y, double z) {}
  double PubTab::zoomFactor() const {return m_zoomFactor;}
}
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::PubTab);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::PubItem);
