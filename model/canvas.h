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

#ifndef CANVAS_H
#define CANVAS_H

#include "group.h"
#include "godleyIcon.h"
#include "operation.h"
#include "selection.h"
#include "switchIcon.h"
#include "wire.h"
#include "ravelWrap.h"
#include "sheet.h"
#include <cairoSurfaceImage.h>

#include <chrono>

namespace minsky
{
  template <class T>
  struct NoAssign: public T
  {
    // disable assignment for objects wrapped in this class, but allow
    // assignment otherwise
    NoAssign& operator=(const NoAssign&) {return *this;}
    template <class U>
    NoAssign& operator=(const U& x) {T::operator=(x); return *this;}
  };
  
  class Canvas: public ecolab::CairoSurface
  {
    CLASSDESC_ACCESS(Canvas);
    void copyVars(const std::vector<VariablePtr>&);
    void reportDrawTime(double) override;
    void mouseDownCommon(float x, float y);

  public:
    typedef std::chrono::time_point<std::chrono::high_resolution_clock> Timestamp;
    struct Model: public GroupPtr
    {
      Exclude<Timestamp> timestamp{Timestamp::clock::now()};
      void updateTimestamp() {timestamp=Timestamp::clock::now();}
      GroupPtr parent; // stash a ref to this groups parent for later restore
      float px=0,py=0,pz=1;
      Model() {}
      Model(const GroupPtr& g) {operator=(g);}
      Model& operator=(const GroupPtr& model) {
        updateTimestamp();
        if (this->get())
          {
            // restore previous stuff
            (*this)->group=parent;
            (*this)->m_x=px;
            (*this)->m_y=py;
            (*this)->relZoom=pz;
          }
        parent=model->group.lock();
        model->group.reset();
        px=model->m_x;
        py=model->m_y;
        pz=model->relZoom;
        model->group.reset(); // disassociate model from it's parent
        GroupPtr::operator=(model);
        return *this;
      }
      void zoom(double x, double y, double z) {
        if (fabs(x-(*this)->x())>1e-5 || fabs(y-(*this)->y())>1e-5)
          updateTimestamp(); // Why is this needed??
        (*this)->zoom(x,y,z);
      }
    };

    Model model;
    
    Selection selection;
    // we don't want surface to be reassigned every time a new model is loaded.
    //NoAssign<Exclude<ecolab::cairo::SurfacePtr>> surface;
    ItemPtr itemFocus; ///< item selected by clicking
    WirePtr wireFocus; ///< wire that mouse is hovering over
    int handleSelected; ///< selected handle for handle moves, -ve = invalid 
    Exclude<shared_ptr<Port>> fromPort; ///< from port when creating a new wire
    double termX,termY; ///< terminal of wire when extending
    float moveOffsX, moveOffsY;
    ClickType::Type clickType;
    /// for drawing error indicators on the canvas
    bool itemIndicator=false;
    
    /// lasso mode support
    struct LassoMode {enum type  {none, lasso, itemResize};};
    LassoMode::type lassoMode=LassoMode::none;
    LassoBox lasso{0,0,0,0};

    bool redrawAll=true; ///< if false, then only redraw graphs
    
    Canvas() {}
    Canvas(const GroupPtr& m): model(m) {}

    ecolab::cairo::SurfacePtr& surface() {return ecolab::CairoSurface::surface;}
    
    /// event handling for the canvas
    void mouseDown(float x, float y);
    void controlMouseDown(float x, float y);
    void mouseUp(float x, float y);
    void mouseMove(float x, float y);
    void displayDelayedTooltip(float x, float y);
    
    /// return closest visible port to (x,y). nullptr is nothing suitable
    std::shared_ptr<Port> closestInPort(float x, float y) const;

    /// select all items in a given region
    void select(float x0, float y0, float x1, float y1) {
      select(LassoBox(x0,y0,x1,y1));
    }
    void select(const LassoBox&);
    
    /// sets itemFocus, and resets mouse offset for placement
    void setItemFocus(const ItemPtr& x) {
      itemFocus=x;
      moveOffsX=moveOffsY=0;
      clickType=ClickType::onItem;
    }

    
    /// @{ item or wire obtained by get*At() calls
    ItemPtr item;
    WirePtr wire;
    ItemPtr itemAt(float x, float y);
    void getItemAt(float x, float y) {item=itemAt(x,y);}
    void getWireAt(float x, float y);

    double defaultRotation=0;
    void addOperation(OperationType::Type op) {
      setItemFocus(model->addItem(OperationBase::create(op)));
      itemFocus->rotation(defaultRotation);
    }
    void addVariable(const std::string& name, VariableType::Type type) {
      setItemFocus(model->addItem(VariablePtr(type,name)));
      itemFocus->rotation(defaultRotation);
    }
    void addNote(const std::string& text) {
      setItemFocus(model->addItem(new Item));
      itemFocus->detailedText=text;
    }
    void addPlot() {setItemFocus(model->addItem(new PlotWidget));}
    void addGodley() {setItemFocus(model->addItem(new GodleyIcon));}
    void addGroup() {setItemFocus(model->addItem(new Group));}
    void addSwitch() {setItemFocus(model->addItem(new SwitchIcon));}
    void addRavel() {setItemFocus(model->addItem(new Ravel));}
    void addSheet() {setItemFocus(model->addItem(new Sheet));}
    
    /// create a group from items found in the current selection
    void groupSelection();
    /// lock all ravels in the selection together. If ravels already
    /// belong to more than one group, then a completely new group is
    /// set up.
    void lockRavelsInSelection();
    void unlockRavelsInSelection();

    /// delete item referenced by item
    void deleteItem();
    /// delete wire referenced by wire
    void deleteWire();
    /// delete wire handle referenced by wire
    void delHandle(float x, float y);    
    /// remove item from its group, leaving in itemFocus for later placement
    void removeItemFromItsGroup();

    /// select all variables referring to same variableValue as item
    void selectAllVariables();
    /// rename all instances of variable as item to \a newName
    void renameAllInstances(const std::string newName);

    /// if item is a Group, move its contents to its parent and delete the group
    void ungroupItem();
    
    /// create a copy of item, and leave it focused
    void copyItem();

    /// reinitialises canvas to the group located in item
    void openGroupInCanvas(const ItemPtr& item);

    /// copy all flowVars of a GodleyIcon in \a item
    void copyAllFlowVars() {
      if (auto g=dynamic_cast<GodleyIcon*>(item.get()))
        copyVars(g->flowVars());
    }
    /// copy all stockVars of a GodleyIcon in \a item
    void copyAllStockVars(){
      if (auto g=dynamic_cast<GodleyIcon*>(item.get()))
        copyVars(g->stockVars());
    }

    /// handle arrow keys
    /// @param int - direction (-1=left/up, +1=right/down)
    /// @param x, y coordinates of mouse in canvas coordinates
    /// @param reset - if true, then call Minsky::reset()
    void handleArrows(int dir, float x, float y, bool reset);
    
    /// zooms canvas such that group indexed by item is displayed
    void zoomToDisplay();

    /// populates item with a variable if (x,y) is over a variable part
    /// @return true if item is updated
    bool selectVar(float x, float y);
    
    /// populates item with the defining variable if there is
    /// one. Leave item unchanged if it is not a variable, or variable
    /// is undefined. NB may be a Godley table or integral
    /// @return true if definition found.
    bool findVariableDefinition();

    /// draw a red circle around item
    //    void indicateItem() {itemIndicator=true;}

    /// redraw whole model
    void redraw(int x0, int y0, int width, int height) override;
    void redraw();
    /// region to be updated
    LassoBox updateRegion{0,0,0,0};
    /// update region given by updateRegion
    void redrawUpdateRegion();

    /// adjust canvas so that -ve coordinates appear on canvas
    void recentre();
    
    /// request a redraw on the screen
    void requestRedraw() {if (surface().get()) surface()->requestRedraw();}
  };
}

#include "canvas.cd"
#include "canvas.xcd"
#endif
