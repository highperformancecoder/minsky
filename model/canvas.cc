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
  void Canvas::mouseDown(float x, float y)
  {
    // firstly, see if the user is selecting an item
    itemFocus=model->findAny(&Group::items,
                       [&](const ItemPtr& i){return i->contains(x,y);});
    if (!itemFocus)
      {
        wireFocus=model->findAny(&Group::wires,
                       [&](const WirePtr& i){return i->near(x,y);});
        // TODO - decorate wire with handles
      }
    if (!wireFocus)
      {
        // TODO - lasso mode
      }
  }

  
  void Canvas::mouseUp(float x, float y)
  {
    mouseMove(x,y);
    itemFocus.reset();
    wireFocus.reset();
    selection.clear();
  }
  
  void Canvas::mouseMove(float x, float y)
  {
    if (itemFocus)
      {
        updateRegion=LassoBox(itemFocus->x(),itemFocus->y(),x,y);
        itemFocus->moveTo(x,y);
        surface->requestRedraw();
      }
    else
      {
        // set mouse focus to display ports etc.
        model->recursiveDo(&Group::items, [&](Items&,Items::iterator& i)
                           {
                             bool mf=(*i)->contains(x,y);
                             if (mf!=(*i)->mouseFocus)
                               {
                                 (*i)->mouseFocus=mf;
                                 surface->requestRedraw();
                               }        
                             return false;
                           });
      }
    // TODO - wire editing and lasso
  }
  
  void Canvas::getItemAt(float x, float y)
  {
    item=model->findAny(&Group::items,
                       [&](const ItemPtr& i){return i->contains(x,y);});
  }
  
  void Canvas::getWireAt(float x, float y)
  {
    wire=model->findAny(&Group::wires,
                       [&](const WirePtr& i){return i->near(x,y);});
  }

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

  int createCanvasImage(Tcl_Interp *interp,	Tk_Canvas canvas, Tk_Item *itemPtr, 
                        int objc,Tcl_Obj *CONST objv[])
  {
    int rc=createImage<CanvasDisplayItem>(interp,canvas,itemPtr,objc,objv);
    // adjust image bounding box to the entire canvas, not just what's there
    itemPtr->x1=-10000;
    itemPtr->x2=10000;
    itemPtr->y1=-10000;
    itemPtr->y2=10000;
    return rc;
  }
    
  // register OperatorItem with Tk for use in canvases.
  int registerItems()
  {
    static Tk_ItemType canvasDisplayItemType = cairoItemType();
    canvasDisplayItemType.name=const_cast<char*>("canvas");
    canvasDisplayItemType.createProc=createCanvasImage;
    Tk_CreateItemType(&canvasDisplayItemType);
    return 0;
  }

}

static int dum=(initVec().push_back(registerItems), 0);
