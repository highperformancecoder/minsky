/*
  @copyright Steve Keen 2017
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

#include "canvas.h"
#include "minsky.h"
#include "init.h"
#include <cairo_base.h>
#include <ecolab_epilogue.h>
using namespace std;
using namespace ecolab::cairo;
using namespace minsky;

namespace minsky
{
  void Canvas::mouseDown(float x, float y) {}
  void Canvas::mouseUp(float x, float y) {}
  void Canvas::mouseMove(float x, float y) {}
  void Canvas::getItemAt(float x, float y) {}
  void Canvas::getWireAt(float x, float y) {}
  void Canvas::redraw()
  {
    updateRegion.x0=updateRegion.y0=-numeric_limits<float>::max();
    updateRegion.x1=updateRegion.y1=numeric_limits<float>::max();
    redrawUpdateRegion();
    updateRegion.x0=updateRegion.y0=updateRegion.x1=updateRegion.y1=0;
  }
  void Canvas::redrawUpdateRegion()
  {
    if (!surface.get()) return;
    auto cairo=surface->cairo();
    cairo_set_line_width(cairo, 1);
    // items
    model->recursiveDo
      (&GroupItems::items, [&](const Items&, Items::const_iterator i)
       {
         auto& it=**i;
         if (it.visible() && updateRegion.intersects(it))
           {
             cairo_save(cairo);
             cairo_identity_matrix(cairo);
             cairo_translate(cairo,it.x(), it.y());
             it.draw(cairo);
             cairo_restore(cairo);
           }
         return false;
       });

    // groups
    model->recursiveDo
      (&GroupItems::groups, [&](const Groups&, Groups::const_iterator i)
       {
         auto& it=**i;
         if (it.visible() && updateRegion.intersects(it))
           {
             cairo_save(cairo);
             cairo_identity_matrix(cairo);
             cairo_translate(cairo,it.x(), it.y());
             it.draw(cairo);
             cairo_restore(cairo);
           }
         return false;
       });

    // draw all wires - wires will go over the top of any icons. TODO
    // introduce an ordering concept if needed
    model->recursiveDo
      (&GroupItems::wires, [&](const Wires&, Wires::const_iterator i)
       {
         const Wire& w=**i;
         if (w.visible() && updateRegion.intersects(w))
           w.draw(cairo);
         return false;
       });
    surface->blit();
  }
}

namespace
{
  struct CanvasDisplayItem: public CairoImage
  {
    void draw()
    {
      if (cairoSurface)
        {
          minsky::minsky().canvas.surface=cairoSurface;
          minsky::minsky().canvas.redraw();
        }
    }
  };

  // register OperatorItem with Tk for use in canvases.
  int registerItems()
  {
    static Tk_ItemType canvasDisplayItemType = cairoItemType();
    canvasDisplayItemType.name=const_cast<char*>("canvas");
    canvasDisplayItemType.createProc=createImage<CanvasDisplayItem>;
    Tk_CreateItemType(&canvasDisplayItemType);
    return 0;
  }

}

static int dum=(initVec().push_back(registerItems), 0);
