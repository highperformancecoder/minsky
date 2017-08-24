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
#include <cairo_base.h>

namespace minsky
{
  class Canvas
  {
    // for drawing error indicators on the canvas
    bool itemIndicator=false;
    CLASSDESC_ACCESS(Canvas);
  public:
    GroupPtr model;
    Selection selection;
    Exclude<ecolab::cairo::SurfacePtr> surface;
    ItemPtr itemFocus; ///< item selected by clicking
    WirePtr wireFocus; ///< wire that mouse is hovering over
    unsigned handleSelected;
    Exclude<shared_ptr<Port>> fromPort; ///< from port when creating a new wire
    double termX,termY; ///< terminal of wire when extending
    float moveOffsX, moveOffsY;
    
    /// lasso mode support
    enum class LassoMode {none, lasso, itemResize};
    LassoMode lassoMode=LassoMode::none;
    LassoBox lasso{0,0,0,0};
    
    Canvas() {}
    Canvas(const GroupPtr& m): model(m) {}
    void resizeWindow(int width, int height);
    
    /// event handling for the canvas
    void mouseDown(float x, float y);
    void mouseUp(float x, float y);
    void mouseMove(float x, float y);

    /// select all items in a given region
    void select(float x0, float y0, float x1, float y1);

    /// sets itemFocus, and resets mouse offset for placement
    void setItemFocus(const ItemPtr& x) {
      itemFocus=x;
      moveOffsX=moveOffsY=0;
    }

    
    /// @{ item or wire obtained by get*At() calls
    ItemPtr item;
    WirePtr wire;
    void getItemAt(float x, float y);
    void getWireAt(float x, float y);

    double defaultRotation=0;
    void addOperation(OperationType::Type op) {
      setItemFocus(model->addItem(OperationBase::create(op)));
      itemFocus->rotation=defaultRotation;
    }
    void addVariable(const std::string& name, VariableType::Type type) {
      setItemFocus(model->addItem(VariablePtr(type,name)));
      itemFocus->rotation=defaultRotation;
    }
    void addNote(const std::string& text) {
      setItemFocus(model->addItem(new Item));
      itemFocus->detailedText=text;
    }
    void addPlot() {setItemFocus(model->addItem(new PlotWidget));}
    void addGodley() {setItemFocus(model->addItem(new GodleyIcon));}
    void addGroup() {setItemFocus(model->addItem(new Group));}
    void addSwitch() {setItemFocus(model->addItem(new SwitchIcon));}
    
    /// delete item referenced by item
    void deleteItem() {if (item) model->deleteItem(*item);}
    /// delete wire referenced by wire
    void deleteWire() {if (wire) model->removeWire(*wire); requestRedraw();}
    /// remove item from its group, leaving in itemFocus for later placement
    void removeItemFromItsGroup();

    /// if item is a Group, move its contents to its parent and delete the group
    void ungroupItem();
    
    /// create a copy of item, and leave it focused
    void copyItem();

    /// reinitialises canvas to the group located in item
    void openGroupInCanvas(const ItemPtr& item);

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
    void indicateItem() {itemIndicator=true;}

    /// redraw whole model
    void redraw();
    /// region to be updated
    LassoBox updateRegion{0,0,0,0};
    /// update region given by updateRegion
    void redrawUpdateRegion();

    /// adjust canvas so that -ve coordinates appear on canvas
    void recentre();
    
    /// request a redraw on the screen
    void requestRedraw() {if (surface.get()) surface->requestRedraw();}
  };
}

#include "canvas.cd"
#endif
