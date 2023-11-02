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

#undef CLASSDESC_ARITIES
#define CLASSDESC_ARITIES 0x3F 
#include "minsky.h"
#include "geometry.h"
#include "canvas.h"
#include "cairoItems.h"
#include "ravelWrap.h"
#include <cairo_base.h>

#include "canvas.rcd"
#include "canvas.xcd"
#include "eventInterface.rcd"
#include "polyRESTProcessBase.xcd"
#include "nobble.h"
#include "minsky_epilogue.h"
using namespace std;
using namespace ecolab::cairo;
using namespace minsky;

namespace minsky
{
  void Canvas::controlMouseDown(float x, float y)
  {
    mouseDownCommon(x,y);
    if (itemFocus && itemFocus->group.lock() == model)
      selection.toggleItemMembership(itemFocus);
  }

  void Canvas::mouseDown(float x, float y)
  {
    mouseDownCommon(x,y);
    if (!itemFocus || !selection.contains(itemFocus))
      selection.clear(); // clicking outside a selection clears it
  }
  
  
  void Canvas::mouseDownCommon(float x, float y)
  {
    wireFocus.reset();
    // firstly, see if the user is selecting an item
    if ((itemFocus=itemAt(x,y)))
      {
        clickType=itemFocus->clickType(x,y);
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
          case ClickType::onSlider:
          case ClickType::onItem:
            moveOffsX=x-itemFocus->x();
            moveOffsY=y-itemFocus->y();
            break;
          case ClickType::outside:
            itemFocus.reset();
            if (lassoMode==LassoMode::none)
              lassoMode=LassoMode::lasso;
            break;
          case ClickType::inItem:
            itemFocus->onMouseDown(x,y);
            break;
          case ClickType::onResize:
            lassoMode=LassoMode::itemResize;
            // set x0,y0 to the opposite corner of (x,y)
            lasso.x0 = x>itemFocus->x()? itemFocus->left(): itemFocus->right();
            lasso.y0 = y>itemFocus->y()? itemFocus->top(): itemFocus->bottom();
            lasso.x1=x;
            lasso.y1=y;
            item=itemFocus;
            break;
          case ClickType::legendMove: case ClickType::legendResize:
            if (auto p=itemFocus->plotWidgetCast())
              p->mouseDown(x,y);
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

  shared_ptr<Port> Canvas::closestInPort(float x, float y) const
  {
    shared_ptr<Port> closestPort;
    auto minD=numeric_limits<float>::max();
    model->recursiveDo(&GroupItems::items,
                       [&](const Items&, Items::const_iterator i)
                       {
                         if ((*i)->group.lock()->displayContents())
                           for (size_t pi=0; pi<(*i)->portsSize(); ++pi)
                             {
                               auto p=(*i)->ports(pi).lock();
                               float d=sqr(p->x()-x)+sqr(p->y()-y);
                               if (d<minD)
                                 {
                                   minD=d;
                                   closestPort=p;
                                 }
                             }
                         return false;
                       });
    return closestPort;
  }
  
  void Canvas::mouseUp(float x, float y)
  {
    mouseMove(x,y);
    
    if (itemFocus && clickType==ClickType::inItem)
      {
        itemFocus->onMouseUp(x,y);
        itemFocus.reset(); // prevent spurious mousemove events being processed
        minsky().requestReset();
      }
    if (fromPort.get())
      {
          if (auto to=closestInPort(x,y)) {
            model->addWire(static_cast<shared_ptr<Port>&>(fromPort),to);

            // populate the destination tooltip if a Ravel
            if (to->item().tooltip.empty() && dynamic_cast<Ravel*>(&to->item()))
              if (auto v=fromPort->item().variableCast())
                to->item().tooltip=v->name();
            
            fromPort.reset();
            minsky().requestReset(); 
          } else {
            fromPort.reset();
          }
      }
    
    if (wireFocus)
      wireFocus->editHandle(handleSelected,x,y);
    
    switch (lassoMode)
      {
      case LassoMode::lasso:
        select({lasso.x0,lasso.y0,x,y});
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

  void Canvas::mouseMoveOnItem(float x, float y)
  {
    updateRegion=LassoBox(itemFocus->x(),itemFocus->y(),x,y);
    // move item relatively to avoid accidental moves on double click
    if (selection.empty() || !selection.contains(itemFocus))
      itemFocus->moveTo(x-moveOffsX, y-moveOffsY);
    else
      {
        // move the whole selection
        auto deltaX=x-moveOffsX-itemFocus->x(), deltaY=y-moveOffsY-itemFocus->y();
        for (auto& i: selection.items)
          i->moveTo(i->x()+deltaX, i->y()+deltaY);
        for (auto& i: selection.groups)
          i->moveTo(i->x()+deltaX, i->y()+deltaY);
      }
    requestRedraw();
    // check if the move has moved outside or into a group
    if (auto g=itemFocus->group.lock())
      if (g==model || !g->contains(itemFocus->x(),itemFocus->y()))
        {
          if (auto toGroup=model->minimalEnclosingGroup
              (itemFocus->x(),itemFocus->y(),itemFocus->x(),itemFocus->y(),itemFocus.get()))
            {
              if (g.get()==toGroup) return;
              // prevent moving a group inside itself
              if (auto g=dynamic_cast<Group*>(itemFocus.get()))
                if (g->higher(*toGroup))
                  return;
              selection.clear(); // prevent old wires from being held onto
              toGroup->addItem(itemFocus);
              toGroup->splitBoundaryCrossingWires();
              g->splitBoundaryCrossingWires();
            }
          else if (g!=model)
            {
              selection.clear(); // prevent old wires from being held onto
              model->addItem(itemFocus);
              model->splitBoundaryCrossingWires();
              g->splitBoundaryCrossingWires();
            }
        }
    if (auto g=itemFocus->group.lock())
      if (g!=model)
        g->checkAddIORegion(itemFocus);
  }
  
  void Canvas::mouseMove(float x, float y)
    try
      {
        if (itemFocus)
          {
            switch (clickType)
              {
              case ClickType::onItem:
                mouseMoveOnItem(x,y);
                return;
              case ClickType::onSlider:
                if (auto v=itemFocus->variableCast())
                  {
                    RenderVariable rv(*v);
                    double rw=fabs(v->zoomFactor()*(rv.width()<v->iWidth()? 0.5*v->iWidth() : rv.width())*cos(v->rotation()*M_PI/180));
                    double sliderPos=(x-v->x())* (v->sliderMax-v->sliderMin)/rw+0.5*(v->sliderMin+v->sliderMax);
                    double sliderHatch=sliderPos-fmod(sliderPos,v->sliderStep);   // matches slider's hatch marks to sliderStep value. for ticket 1258
                    v->sliderSet(sliderHatch);
                    // push History to prevent an unnecessary reset when
                    // adjusting the slider whilst paused. See ticket #812
                    minsky().pushHistory();
                    if (minsky().reset_flag())
                      minsky().requestReset();
                    minsky().evalEquations();
                    requestRedraw();
                  }
                return;
              case ClickType::inItem:
                if (itemFocus->onMouseMotion(x,y))
                  requestRedraw();
                return;
              case ClickType::legendMove: case ClickType::legendResize:
                if (auto p=itemFocus->plotWidgetCast())
                  {
                    p->mouseMove(x,y);
                    requestRedraw();
                  }
                return;
              case ClickType::onResize:
                lasso.x1=x;
                lasso.y1=y;
                requestRedraw();
                break;
              default:
                break;
              }
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
        else if (lassoMode==LassoMode::lasso || (lassoMode==LassoMode::itemResize && item.get()))
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
                                                (*i)->disableDelayedTooltip();
                                                // with coupled integration variables, we
                                                // do not want to set mousefocus, as this
                                                // draws unnecessary port circles on the
                                                // variable
                                                if (!(*i)->visible() && 
                                                    dynamic_cast<Variable<VariableBase::integral>*>(i->get()))
                                                  (*i)->mouseFocus=false;
                                                else
                                                  {
                                                    auto ct=(*i)->clickType(x,y);
                                                    if (ct==ClickType::inItem)
                                                      {
                                                        (*i)->mouseFocus=true;
                                                        (*i)->onBorder = false;
                                                        if ((*i)->onMouseOver(x,y))
                                                          requestRedraw();
                                                      }
                                                    else
                                                      {
                                                        auto mf = ct!=ClickType::outside;
                                                        if ((*i)->mouseFocus!=mf)
                                                          {
                                                            requestRedraw();
                                                            (*i)->mouseFocus=mf;
                                                          }
                                                        (*i)->onResizeHandles=ct==ClickType::onResize;
                                                        (*i)->onBorder = ct==ClickType::onItem;
                                                        (*i)->onMouseLeave();
                                                      }
                                                  }
                                                return false;
                                              });
            model->recursiveDo(&Group::groups, [&](Groups&,Groups::iterator& i)
                                               {
                                                 auto ct=(*i)->clickType(x,y);
                                                 bool mf=ct!=ClickType::outside;
                                                 if (mf!=(*i)->mouseFocus)
                                                   {
                                                     (*i)->mouseFocus=mf;
                                                     requestRedraw();
                                                   }
                                                 bool onResize = ct==ClickType::onResize;
                                                 if (onResize!=(*i)->onResizeHandles)
                                                   {
                                                     (*i)->onResizeHandles=onResize;
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
        if (minsky().reset_flag())
          minsky().requestReset(); // postpone reset whilst mousing
      }
    catch (...) {/* absorb any exceptions, as they're not useful here */}

  bool Canvas::keyPress(const KeyPressArgs& args)
  {
    if (auto item=itemAt(args.x,args.y))
      if (item->onKeyPress(args.keySym, args.utf8, args.state))
        {
          minsky().markEdited(); // keyPresses don't set the dirty flag by default
          requestRedraw();
          return true;
        }
    return false;
  }

  
  void Canvas::displayDelayedTooltip(float x, float y)
  {
    if (auto item=itemAt(x,y))
      {
        item->displayDelayedTooltip(x,y);
        requestRedraw();
      }
  }
  
  void Canvas::select(const LassoBox& lasso)
  {
    selection.clear();

    auto topLevel = model->minimalEnclosingGroup(lasso.x0,lasso.y0,lasso.x1,lasso.y1);

    if (!topLevel) topLevel=&*model;

    for (auto& i: topLevel->items)
      if (i->visible() && lasso.intersects(*i))
        selection.ensureItemInserted(i);

    for (auto& i: topLevel->groups)
      if (i->visible() && lasso.intersects(*i))
        selection.ensureGroupInserted(i);

    if (focusFollowsMouse)
      minsky().copy();
  }

  int Canvas::ravelsSelected()
  {
    int ravelsSelected = 0;
    for (auto& i: selection.items) {
      if (dynamic_pointer_cast<Ravel>(i))
      {
        ravelsSelected++;
      }
    }
    return ravelsSelected;
  }
  
  ItemPtr Canvas::itemAt(float x, float y)
  {
    // Fix for library dependency problem with items during Travis build     
    ItemPtr item;                    
    auto minD=numeric_limits<float>::max();
    model->recursiveDo(&GroupItems::items,
                       [&](const Items&, Items::const_iterator i)
                       {
                          float d=sqr((*i)->x()-x)+sqr((*i)->y()-y);
                          if (d<minD && (*i)->visible() && (*i)->contains(x,y))
                            {
                              minD=d;
                              item=*i;
                            }
                          return false;  
                       });
    if (!item)
      item=model->findAny
        (&Group::groups, [&](const GroupPtr& i)
                         {return i->visible() && i->clickType(x,y)!=ClickType::outside;});
    return item;
  }
  
  bool Canvas::getWireAt(float x, float y)
  {
    wire=model->findAny(&Group::wires,
                        [&](const WirePtr& i){return i->near(x,y);});
    return wire.get();
  }

  void Canvas::groupSelection()
  {
    GroupPtr r=model->addGroup(new Group);
    for (auto& i: selection.items)
      r->addItem(i);
    for (auto& i: selection.groups)
      r->addItem(i);
    r->splitBoundaryCrossingWires();
    r->resizeOnContents();
  }

  void Canvas::lockRavelsInSelection()
  {
    vector<shared_ptr<Ravel> > ravelsToLock;
    shared_ptr<RavelLockGroup> lockGroup;
    bool conflictingLockGroups=false;
    for (auto& i: selection.items)
      if (auto r=dynamic_pointer_cast<Ravel>(i))
        {
          ravelsToLock.push_back(r);
          if (!lockGroup)
            lockGroup=r->lockGroup;
          if (lockGroup!=r->lockGroup)
            conflictingLockGroups=true;
        }
    if (ravelsToLock.size()<2)
      return;
    if (!lockGroup || conflictingLockGroups)
      lockGroup.reset(new RavelLockGroup);
    for (auto& r: ravelsToLock)
      {
        lockGroup->addRavel(r);
        r->leaveLockGroup();
        r->lockGroup=lockGroup;
      }
    if (lockGroup) lockGroup->initialBroadcast();
  }

  void Canvas::unlockRavelsInSelection()
  {
    for (auto& i: selection.items)
      if (auto r=dynamic_cast<Ravel*>(i.get()))
        r->leaveLockGroup();
  }
  
  void Canvas::pushDefiningVarsToTab()
  {
    for (auto& i: selection.items)
      {
        auto v=i->variableCast();
        if (v && v->defined() && !v->varTabDisplay) {
          itemVector.push_back(i);
          v->toggleVarTabDisplay();	  
        }
      }
    requestRedraw();
  }
  
  void Canvas::showDefiningVarsOnCanvas()
  {
    for (auto& i: itemVector)
      {
        auto v=(*i).variableCast();
        if (v && v->defined() && v->varTabDisplay)
          v->toggleVarTabDisplay();	  
      }
    // ensure individual hidden defining vars can be made visible once more. for ticket 145.
    if (itemVector.empty())
	{
        model->recursiveDo
          (&GroupItems::items, [&](const Items&,Items::const_iterator i)
           {
             if (auto v=(*i)->variableCast())
               if (v->defined() && v->varTabDisplay)
				 v->toggleVarTabDisplay();	 
             return false;
           });
	}      
    itemVector.clear();
    requestRedraw();
  }
  
  void Canvas::showPlotsOnTab()
  {
     model->recursiveDo
       (&GroupItems::items, [&](const Items&,Items::const_iterator i)
        {
          if (auto p=(*i)->plotWidgetCast())
            if (!p->plotTabDisplay) p->togglePlotTabDisplay();	 
          return false;
        });
  }    
  
  void Canvas::deleteItem()
  {
    if (item)
    {
      model->deleteItem(*item);
      minsky().requestReset();
    }
  }
  
  void Canvas::deleteWire()
  {
    if (wire)
    {
      model->removeWire(*wire);
      wire.reset();
      minsky().requestReset();
    }
  }
  
  // For ticket 1092. Reinstate delete handle user interaction
  void Canvas::delHandle(float x, float y)
  {
    wireFocus=model->findAny(&Group::wires,
                   [&](const WirePtr& i){return i->near(x,y);});
    if (wireFocus)
      {
        wireFocus->deleteHandle(x,y);
        wireFocus.reset(); // prevent accidental moves of handle
      }
    requestRedraw();
  }  

  void Canvas::removeItemFromItsGroup()
  {
    if (item)
      if (auto g=item->group.lock())
        {
          if (auto parent=g->group.lock())
            {
              itemFocus=parent->addItem(item);
              if (auto v=itemFocus->variableCast())
                v->controller.reset();
              g->splitBoundaryCrossingWires();
            }
          // else item already at toplevel
        }
  }

  void Canvas::selectAllVariables()
  {
    selection.clear();
    auto var=item->variableCast();
    if (!var)
      if (auto i=dynamic_cast<IntOp*>(item.get()))
        var=i->intVar.get();
    if (var)
      {
        model->recursiveDo
          (&GroupItems::items, [&](const Items&,Items::const_iterator i)
           {
             if (auto v=(*i)->variableCast())
               if (v->valueId()==var->valueId())
                 {
                   selection.items.push_back(*i);
                   v->selected=true;
                 }
             return false;
           });
       }
  }

  void Canvas::renameAllInstances(const string newName)
  {
    auto var=item->variableCast();
    if (!var)
      if (auto i=dynamic_cast<IntOp*>(item.get()))
        var=i->intVar.get();
    if (var)
      {
        // cache name and valueId for later use as var gets invalidated in the recursiveDo
        auto valueId=var->valueId();
        model->renameAllInstances(valueId, newName);
      }
   }
  
  void Canvas::ungroupItem()
  {
    if (auto g=dynamic_cast<Group*>(item.get()))
      {		  
        if (auto p=g->group.lock())
          {
            if (!g->empty()) // minskly crashes if group empty and ungrouped subsequently. for ticket 1243
              {  	    				 
                // stash values of parameters in copied group, as they are reset for some unknown reason later on. for tickets 1243/1258
                map<string,string> existingParms; 
                for (auto& i: g->items) {
                  auto v=i->variableCast(); 
                  if (v && v->type()==VariableType::parameter) 
                    existingParms.emplace(v->valueId(),v->init());
                }

                // blow up containe items so they appear in same relative locationsat parent scalefactor
                auto scaleFactor=1/g->relZoom;
                for (auto& i: g->items)
                  i->moveTo((i->x()-g->x())*scaleFactor+g->x(), (i->y()-g->y())*scaleFactor+g->y());
                for (auto& i: g->groups)
                  i->moveTo((i->x()-g->x())*scaleFactor+g->x(), (i->y()-g->y())*scaleFactor+g->y());
                p->moveContents(*g);
                deleteItem();
               	    
		existingParms.clear();    
	         
              } else deleteItem();
          }       
        
        // else item is toplevel which can't be ungrouped
      }
  }

  void Canvas::renameItem(const std::string& newName)
  {
    if (auto var=item->variableCast())
      {
        var->name(newName);
        // check stock variables are defined, convert to flow if not
        if (var->isStock() && !cminsky().definingVar(var->valueId()))
          var->retype(VariableType::flow);
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
        else if (auto group=dynamic_cast<Group*>(item.get()))
          newItem=group->copy();
        else
          {    			    
            if (item->ioVar())
              if (auto v=item->variableCast())
                if (v->rawName()[0]!=':')
                  try
                    {
                      minsky().pushHistory(); // save current state in case of failure
                      // attempt to make variable outer scoped. For #1100
                      minsky().canvas.renameAllInstances(":"+v->rawName());
                    }
                  catch (...)
                    {
                      minsky().undo(); // back out of change
                      throw;
                    }
            
            newItem.reset(item->clone());
            // if copied from a Godley table or I/O var, set orientation to default
            if (auto v=item->variableCast())
              if (v->controller.lock())
                newItem->rotation(defaultRotation);
                    
          }
        setItemFocus(model->addItem(newItem));
        model->normaliseGroupRefs(model);
      }
  }

  void Canvas::openGroupInCanvas(const ItemPtr& item)
  {
    if (auto g=dynamic_pointer_cast<Group>(item))
      {
        if (auto parent=model->group.lock())
          model->setZoom(parent->zoomFactor());
        model=g;
        minsky().bookmarkRefresh();
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
        this->item.reset();
        itemFocus.reset();
        wire.reset();
        selection.clear();
        fromPort.reset();
        requestRedraw();
      }
  }

  void Canvas::copyVars(const std::vector<VariablePtr>& v)
  {
    float maxWidth=0, totalHeight=0;
    vector<float> widths, heights;
    // Throw error if no stock/flow vars on Godley icon. For ticket 1039 
    if (!v.empty()) {    
	  selection.clear();	
      for (auto& i: v)
        {
          RenderVariable rv(*i);
          widths.push_back(rv.width());
          heights.push_back(rv.height());
          maxWidth=max(maxWidth, widths.back());
          totalHeight+=heights.back();
        }
      float y=v[0]->y() - totalHeight;
      for (size_t i=0; i<v.size(); ++i)
        {
		  // Stock and flow variables on Godley icons should not be copied as groups. For ticket 1039	
          ItemPtr ni(v[i]->clone());
          (ni->variableCast())->rotation(0);
          ni->moveTo(v[0]->x()+maxWidth-v[i]->zoomFactor()*widths[i],
                     y+heights[i]);
          // variables need to refer to outer scope
          if ((ni->variableCast())->name()[0]!=':')
            (ni->variableCast())->name(':'+(ni->variableCast())->name());
          y+=2*v[i]->zoomFactor()*heights[i];
          selection.insertItem(model->addItem(ni));		 
        }
        // Item focus put on one of the copied vars that are still in selection. For ticket 1039
        if (!selection.empty()) setItemFocus(selection.items[0]);
        else setItemFocus(nullptr);  
    } else throw error("no flow or stock variables to copy");    
  }

  void Canvas::zoomToDisplay()
  {
    if (auto g=dynamic_cast<Group*>(item.get()))
      model->zoom(g->x(),g->y(),1.1/(g->relZoom*g->zoomFactor()));
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
    return false;
  }
    
  bool Canvas::findVariableDefinition()
  {
    if (auto iv=item->variableCast())
      {
        if (iv->type()==VariableType::constant ||
            iv->type()==VariableType::parameter || iv->inputWired())
          return true;
        
        auto def=model->findAny
          (&GroupItems::items, [&](const ItemPtr& i) {
            if (auto v=i->variableCast())
              return v->inputWired() && v->valueId()==iv->valueId();
            if (auto g=dynamic_cast<GodleyIcon*>(i.get()))
              for (auto& v: g->stockVars())
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

  bool Canvas::redraw(int x0, int y0, int width, int height)
  {
    updateRegion.x0=x0;
    updateRegion.y0=y0;
    updateRegion.x1=x0+width;
    updateRegion.y1=y0+height;
    // redraw of canvas may throw if called during a reset operation
    try
      {
        return redrawUpdateRegion();
      }
#ifndef NDEBUG
    catch (std::exception& ex)
      {
        cerr << ex.what() << endl;
      }
#endif
    catch (...)
      {
        // consume exception and try redrawing
        // this leads to an endless loop...
        //requestRedraw();
      }
    return false;
  }
  
  bool Canvas::redraw()
  {
    // nb using maxint here doesn't seem to work
    return redraw(-1e9,-1e9,2e9,2e9);
  }

  bool Canvas::redrawUpdateRegion()
  {
    bool didDrawSomething = false;
    if (!surface().get()) {
      return didDrawSomething;
    }
    m_redrawRequested=false;
    auto cairo=surface()->cairo();
    CairoSave cs(cairo);
    cairo_rectangle(cairo,updateRegion.x0,updateRegion.y0,updateRegion.x1-updateRegion.x0,updateRegion.y1-updateRegion.y0);
    cairo_clip(cairo);
    cairo_set_line_width(cairo, 1);
    // items
    model->recursiveDo
      (&GroupItems::items, [&](const Items&, Items::const_iterator i)
       {
         auto& it=**i;
         if (it.visible() && updateRegion.intersects(it))
           {
             didDrawSomething = true;
             CairoSave cs(cairo);
             cairo_identity_matrix(cairo);
             cairo_translate(cairo,it.x(), it.y());
             it.draw(cairo);
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
             didDrawSomething = true;
             CairoSave cs(cairo);
             cairo_identity_matrix(cairo);
             cairo_translate(cairo,it.x(), it.y());
             it.draw(cairo);
           }
         return false;
       });

    // draw all wires - wires will go over the top of any icons. TODO
    // introduce an ordering concept if needed
    model->recursiveDo
      (&GroupItems::wires, [&](const Wires&, Wires::const_iterator i)
       {
         const Wire& w=**i;
         if (w.visible()) {
           w.draw(cairo);
         }
         return false;
       });

    if (fromPort.get()) // we're in process of creating a wire
      {
        didDrawSomething = true;
        cairo_move_to(cairo,fromPort->x(),fromPort->y());
        cairo_line_to(cairo,termX,termY);
        cairo_stroke(cairo);
        // draw arrow
        CairoSave cs(cairo);
        cairo_translate(cairo, termX,termY);
        cairo_rotate(cairo,atan2(termY-fromPort->y(), termX-fromPort->x()));
        cairo_move_to(cairo,0,0);
        cairo_line_to(cairo,-5,-3); 
        cairo_line_to(cairo,-3,0); 
        cairo_line_to(cairo,-5,3);
        cairo_close_path(cairo);
        cairo_fill(cairo);
      }

    if (lassoMode!=LassoMode::none)
      {
        didDrawSomething = true;
        cairo_rectangle(cairo,lasso.x0,lasso.y0,lasso.x1-lasso.x0,lasso.y1-lasso.y0);
        cairo_stroke(cairo);
      }

    if (itemIndicator && item) // draw a red circle to indicate an error or other marker
      {
        CairoSave cs(surface()->cairo());
        cairo_set_source_rgb(surface()->cairo(),1,0,0);
        cairo_arc(surface()->cairo(),item->x(),item->y(),15,0,2*M_PI);
        cairo_stroke(surface()->cairo());
      }

    surface()->blit();
    return didDrawSomething;
  }

  void Canvas::recentre()
  {
    SurfacePtr tmp(surface());
    surface().reset(new Surface(cairo_recording_surface_create(CAIRO_CONTENT_COLOR,nullptr)));
    redraw();
    model->moveTo(model->x()-surface()->left(), model->y()-surface()->top());
    surface()=tmp;
    requestRedraw();
  }

  void Canvas::reportDrawTime(double t) 
  {
    // ensure screen refresh time is less than a third
    minsky().maxWaitMS=(t>0.03)? 3000*t: 100.0;
  }

  void Canvas::applyDefaultPlotOptions() {
      if (auto p=item->plotWidgetCast()) {
        // stash titles to restore later
        string title(p->title), xlabel(p->xlabel()),
          ylabel(p->ylabel()), y1label(p->y1label());
        defaultPlotOptions.applyPlotOptions(*p);
        p->title=title, p->xlabel(xlabel),
          p->ylabel(ylabel), p->y1label(y1label);
      }
    }


}

CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::EventInterface);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::Canvas);
