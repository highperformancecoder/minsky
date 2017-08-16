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
#include <cairo/cairo-xlib.h>
#include <cairo/cairo-xlib.h>
#include <ecolab_epilogue.h>
using namespace std;
using namespace ecolab::cairo;
using namespace minsky;

namespace minsky
{
  void Canvas::mouseDown(float x, float y)
  {
    if (ignoreButtonEvent) return;
    // firstly, see if the user is selecting an item
    itemFocus=model->findAny(&Group::items,
                       [&](const ItemPtr& i){return i->contains(x,y);});
    if (itemFocus)
      {
        auto clickType=itemFocus->clickType(x,y);
        switch (clickType)
          {
          case ClickType::onPort:
            // items all have their output port first, if they have an output port at all.
            if ((fromPort=itemFocus->closestOutPort(x,y)))
              {
                termX=x;
                termY=y;
                itemFocus.reset();
              }
            break;
          case ClickType::onItem:
            break;
          case ClickType::outside:
            itemFocus.reset();
            lassoMode=true;
            break;
          }
      }
    else
      {
        wireFocus=model->findAny(&Group::wires,
                       [&](const WirePtr& i){return i->near(x,y);});
        if (wireFocus)
          handleSelected=wireFocus->nearestHandle(x,y);
        else
          lassoMode=true;
      }

    if (lassoMode)
      {
        lasso.x0=x;
        lasso.y0=y;
      }
  }

  
  void Canvas::mouseUp(float x, float y)
  {
    if (ignoreButtonEvent)
      {
        ignoreButtonEvent=false;
        return;
      }
    mouseMove(x,y);
    if (fromPort.get())
      {
        if (auto dest=model->findAny(&Group::items,
                                     [&](const ItemPtr& i){return i->contains(x,y);}))
          if (auto to=dest->closestInPort(x,y))
            model->addWire(fromPort,to);
        fromPort.reset();
      }
    
    if (wireFocus)
      wireFocus->editHandle(handleSelected,x,y);
    
    if (lassoMode)
      {
        select(lasso.x0,lasso.y0,x,y); //TODO move this into a method of selection
        lassoMode=false;
        requestRedraw();
      }

    
    itemIndicator=false;
    itemFocus.reset();
    wireFocus.reset();
  }
  
  void Canvas::mouseMove(float x, float y)
  {
    if (ignoreButtonEvent) return;
    if (itemFocus)
      {
        updateRegion=LassoBox(itemFocus->x(),itemFocus->y(),x,y);
        itemFocus->moveTo(x,y);
        requestRedraw();
      }
    else if (fromPort.get())
      {
        termX=x;
        termY=y;
        requestRedraw();
      }
    else if (wireFocus)
      {
        wireFocus->editHandle(handleSelected,x,y);
        requestRedraw();
      }
    else if (lassoMode)
      {
        lasso.x1=x;
        lasso.y1=y;
        requestRedraw();
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
                                 requestRedraw();
                               }        
                             return false;
                           });
        model->recursiveDo(&Group::wires, [&](Wires&,Wires::iterator& i)
                           {
                             bool mf=(*i)->near(x,y);
                             if (mf!=(*i)->mouseFocus)
                               {
                                 (*i)->mouseFocus=mf;
                                 requestRedraw();
                               }        
                             return false;
                           });
      }
  }

  void Canvas::select(float x0, float y0, float x1, float y1)
  {
    LassoBox lasso(x0,y0,x1,y1);
    selection.clear();

    auto topLevel = model->minimalEnclosingGroup(x0,y0,x1,y1);

    if (!topLevel) topLevel=&*model;

    for (auto& i: topLevel->items)
      if (i->visible() && lasso.intersects(*i))
        {
          selection.items.push_back(i);
          i->selected=true;
        }

    for (auto& i: topLevel->groups)
      if (i->visible() && lasso.intersects(*i))
        {
          selection.groups.push_back(i);
          i->selected=true;
        }

    for (auto& i: topLevel->wires)
      if (i->visible() && lasso.contains(*i))
        selection.wires.push_back(i);

    minsky().copy();
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

  void Canvas::copyItem()
  {
    if (item)
      {
        auto newItem=item->clone();
        newItem->group.reset();
        itemFocus=model->addItem(newItem);
      }
  }

  bool Canvas::findVariableDefinition()
  {
    if (auto iv=dynamic_cast<VariableBase*>(item.get()))
      {
        if (iv->type()==VariableType::constant ||
            iv->type()==VariableType::parameter || iv->inputWired())
          return true;
        
        auto def=model->findAny
          (&GroupItems::items, [&](const ItemPtr& i) {
            if (auto v=dynamic_cast<VariableBase*>(i.get()))
              return v->inputWired() && v->valueId()==iv->valueId();
            else if (auto g=dynamic_cast<GodleyIcon*>(i.get()))
              for (auto& v: g->stockVars)
                {
                  if (v->valueId()==iv->valueId())
                    return true;
                }
            else if (auto o=dynamic_cast<IntOp*>(i.get()))
              return o->intVar->valueId()==iv->valueId();
            return false;
          });
        if (def)
          item=def;
        return def.get();
      }
    return false;
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
         it.setCairoSurface(surface);
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

    if (fromPort.get()) // we're in process of creating a wire
      {
        cairo_move_to(cairo,fromPort->x(),fromPort->y());
        cairo_line_to(cairo,termX,termY);
        cairo_stroke(cairo);
        // draw arrow
        cairo_save(cairo);
        cairo_translate(cairo, termX,termY);
        cairo_rotate(cairo,atan2(termY-fromPort->y(), termX-fromPort->x()));
        cairo_move_to(cairo,0,0);
        cairo_line_to(cairo,-5,-3); 
        cairo_line_to(cairo,-3,0); 
        cairo_line_to(cairo,-5,3);
        cairo_close_path(cairo);
        cairo_fill(cairo);
        cairo_restore(cairo);
      }

    if (lassoMode)
      {
        cairo_rectangle(cairo,lasso.x0,lasso.y0,lasso.x1-lasso.x0,lasso.y1-lasso.y0);
        cairo_stroke(cairo);
      }

    if (itemIndicator) // draw a red circle to indicate an error or other marker
      {
        cairo_save(surface->cairo());
        cairo_set_source_rgb(surface->cairo(),1,0,0);
        cairo_arc(surface->cairo(),item->x(),item->y(),15,0,2*M_PI);
        cairo_stroke(surface->cairo());
        cairo_restore(surface->cairo());
      }
    
    surface->blit();
  }

  void Canvas::resizeWindow(int width, int height)
  {
    if (surface.get() && cairo_surface_get_type
        (surface->surface())==CAIRO_SURFACE_TYPE_QUARTZ)
      cairo_xlib_surface_set_size(surface->surface(),width,height);
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
