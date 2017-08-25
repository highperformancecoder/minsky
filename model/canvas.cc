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

#include "geometry.h"
#include "canvas.h"
#include "minsky.h"
#include "init.h"
#include <cairo_base.h>

#ifdef CAIRO_HAS_XLIB_SURFACE
#include <cairo/cairo-xlib.h>
#endif
// CYGWIN has problems with WIN32_SURFACE
#define USEWIN32_SURFACE defined(CAIRO_HAS_WIN32_SURFACE) && !defined(__CYGWIN__)
#if USEWIN32_SURFACE
#undef Realloc
#include <cairo/cairo-win32.h>
// undocumented internal function for extracting the HDC from a Drawable
extern "C" HDC TkWinGetDrawableDC(Display*, Drawable, void*);
extern "C" HDC TkWinReleaseDrawableDC(Drawable, HDC, void*);
#endif

#undef near

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
                       [&](const ItemPtr& i){return i->visible() && i->contains(x,y);});
    if (!itemFocus)
      // check for groups
      itemFocus=model->findAny(&Group::groups,
                               [&](const GroupPtr& i){return i->contains(x,y);});

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
            moveOffsX=x-itemFocus->x();
            moveOffsY=y-itemFocus->y();
            break;
          case ClickType::outside:
            itemFocus.reset();
            if (lassoMode==LassoMode::none)
              lassoMode=LassoMode::lasso;
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
          if (lassoMode==LassoMode::none)
            lassoMode=LassoMode::lasso;
     }

    if (lassoMode==LassoMode::lasso)
      {
        lasso.x0=x;
        lasso.y0=y;
      }
  }

  
  void Canvas::mouseUp(float x, float y)
  {
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
    
    switch (lassoMode)
      {
      case LassoMode::lasso:
        select(lasso.x0,lasso.y0,x,y); //TODO move this into a method of selection
        requestRedraw();
        break;
      case LassoMode::itemResize:
        if (item)
          {
            item->resize(lasso);
            requestRedraw();
          }
        break;
      default: break;
      }
    lassoMode=LassoMode::none;

    
    itemIndicator=false;
    itemFocus.reset();
    wireFocus.reset();
  }
  
  void Canvas::mouseMove(float x, float y)
  {
    if (itemFocus)
      {
        updateRegion=LassoBox(itemFocus->x(),itemFocus->y(),x,y);
        // move item relatively to avoid accidental moves on double click
        itemFocus->moveTo(x-moveOffsX, y-moveOffsY);
        // check if the move has moved outside or into a group
        if (auto g=itemFocus->group.lock())
          if (g==model || !g->contains(itemFocus->x(),itemFocus->y()))
            if (auto toGroup=model->minimalEnclosingGroup
                (itemFocus->x(),itemFocus->y(),itemFocus->x(),itemFocus->y(),itemFocus.get()))
              {
                toGroup->addItem(itemFocus);
                toGroup->splitBoundaryCrossingWires();
              }
            else
              {
                model->addItem(itemFocus);
                model->splitBoundaryCrossingWires();
              }
        if (auto g=itemFocus->group.lock())
          g->checkAddIORegion(itemFocus);
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
    else if (lassoMode==LassoMode::lasso)
      {
        lasso.x1=x;
        lasso.y1=y;
        requestRedraw();
      }
    else if (lassoMode==LassoMode::itemResize && item.get())
      {
        lasso.x1=x;
        lasso.y1=y;
        // make lasso symmetric around item's (x,y)
        lasso.x0=2*item->x() - x;
        lasso.y0=2*item->y() - y;
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
                        [&](const ItemPtr& i){return i->visible() && i->contains(x,y);});
    if (!item)
      item=model->findAny(&Group::groups,
                       [&](const ItemPtr& i){return i->visible() && i->contains(x,y);});
  }
  
  void Canvas::getWireAt(float x, float y)
  {
    wire=model->findAny(&Group::wires,
                        [&](const WirePtr& i){return i->near(x,y);});
  }

  void Canvas::removeItemFromItsGroup()
  {
    if (item)
      if (auto g=item->group.lock())
        {
          if (auto parent=g->group.lock())
            itemFocus=parent->addItem(item);
          else
            itemFocus=model->addItem(item);
          itemFocus->m_visible=true;
          g->splitBoundaryCrossingWires();
        }
  }
  
  void Canvas::ungroupItem()
  {
    if (auto g=dynamic_cast<Group*>(item.get()))
      {
        if (auto p=g->group.lock())
          p->moveContents(*g);
        else
          model->moveContents(*g);
          deleteItem();
      }
  }


  void Canvas::copyItem()
  {
    if (item)
      {
        ItemPtr newItem;
        // cannot duplicate an integral, just its variable
        if (auto intop=dynamic_cast<IntOp*>(item.get()))
          newItem.reset(intop->intVar->clone()); 
        else
          newItem.reset(item->clone());
        setItemFocus(model->addItem(newItem));
        model->normaliseGroupRefs(model);
        newItem->m_visible=true;
      }
  }

  void Canvas::openGroupInCanvas(const ItemPtr& item)
  {
    if (auto g=dynamic_cast<Group*>(item.get()))
      {
        if (auto parent=model->group.lock())
          model->setZoom(parent->zoomFactor);
        model=item;
        float zoomFactor=1.1*model->displayZoom;
        if (!model->displayContents())
          {
            // we need to move the io variables
            for (auto& v: model->inVariables)
              {
                float x=v->x(), y=v->y();
                zoom(x,model->x(),zoomFactor);
                zoom(y,model->y(),zoomFactor);
                v->moveTo(x,y);
              }
            for (auto& v: model->outVariables)
              {
                float x=v->x(), y=v->y();
                zoom(x,model->x(),zoomFactor);
                zoom(y,model->y(),zoomFactor);
                v->moveTo(x,y);
              }
          }
        model->zoom(model->x(),model->y(),zoomFactor);
        requestRedraw();
      }
  }

  void Canvas::zoomToDisplay()
  {
    if (auto g=dynamic_cast<Group*>(item.get()))
      model->zoom(g->x(),g->y(),1.1*g->displayZoom);
  }

  bool Canvas::selectVar(float x, float y) 
  {
    if (item)
      {
        if (auto v=item->select(x,y))
          {
            item=v;
            return true;
          }
      }
    else
      return false;
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

    if (lassoMode!=LassoMode::none)
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

  void Canvas::recentre()
  {
    SurfacePtr tmp(surface);
    surface.reset(new Surface(cairo_recording_surface_create(CAIRO_CONTENT_COLOR,nullptr)));
    redraw();
    model->moveTo(model->x()-surface->left(), model->y()-surface->top());
    surface=tmp;
    requestRedraw();
  }

  
//  void Canvas::resizeWindow(int width, int height)
//  {
//    if (surface.get() && cairo_surface_get_type
//        (surface->surface())==CAIRO_SURFACE_TYPE_QUARTZ)
//      cairo_xlib_surface_set_size(surface->surface(),width,height);
//  }

}

