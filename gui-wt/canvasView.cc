/*
 @copyright Steve Keen 2012
 @author Michael Roy
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

#include "canvasView.h"
#include "minskyApp.h"
#include "cairoWidget.h"
#include "operationCanvasItem.h"
#include "linkCanvasItem.h"
#include "plotCanvasItem.h"
#include "godleyCanvasItem.h"
#include "variableCanvasItem.h"
#include "groupCanvasItem.h"
#include "scrollBarArea.h"
#include "constantPropertiesDlg.h"
#include "linkCanvasOverlay.h"
#include <Wt/WPaintedWidget>
#include <Wt/WVBoxLayout>
#include <Wt/Ext/Panel>
#include <Wt/WRectArea>
#include <memory>
#include <algorithm>
#include "GUI/minsky.h"
#include <ecolab_epilogue.h>

namespace minsky { namespace gui {

using namespace Wt;

CanvasView::CanvasView(MinskyDoc& doc, WContainerWidget* parent) :
    AbstractView(doc),
    _base_class(parent),
    document(doc),
    mouseTool(toolDefault),
    activity(actDefault),
    keyModifier(keyNoKey),
    mousePos0(0, 0),
    mousePos(0, 0)
{
  //  do not create any children here, as the widget is not yet fully
  //  part of the widget tree, do it in initialize() below..
}

CanvasView::~CanvasView()
{
  //  disable the toolbar so it won't signal us anymore.  Menus have a
  //  different dynamic mechanism for handling this.
  //
  //  Normally, it should be destroyed very soon, as we are closing...

  WWidget* widget = parent()->find("minsky-main-toolbar");
  if (widget)
  {
    widget->disable();
  }
}

void CanvasView::initialize()
{
  setId("minsky-canvas-view");
  setActivity(actDefault);
  setLayoutSizeAware(true);
  setSelectable(false);
  _base_class::initialize();
}

void CanvasView::clear()
{
  //  this deletes the current activity overlay.
  setActivity(actDefault);

  if (linkOverlay)
  {
    linkOverlay.reset();
  }
  if (wireEventArea)
  {
    wireEventArea.reset();
  }

  _base_class::clear();
  widgets.clear();
  plots.clear();
  resetModelRect();
  setZoom(DisplaySettings::defaultZoom, 0., 0.);
}

bool CanvasView::moveItem(AbstractCanvasItem& item,
    AbstractCanvasItem::MoveState state, double& x, double& y)
{
  switch (state)
  {
  case AbstractCanvasItem::moveBegin:
    getAttachedWires(affectedWires, item);
    break;

  case AbstractCanvasItem::moveInProgress:
    {
      item.moveTo(x, y);
      if (dynamic_cast<OperationCanvasItem*>(&item))
      {
        dynamic_cast<OperationCanvasItem*>(&item)->getOp()->moveTo(x, y);
      }
      else if (dynamic_cast<GodleyCanvasItem*>(&item))
      {
        dynamic_cast<GodleyCanvasItem*>(&item)->getIcon().moveTo(x, y);
      }
      else if (dynamic_cast<PlotCanvasItem*>(&item))
      {
        dynamic_cast<PlotCanvasItem*>(&item)->getPlot().moveTo(x, y);
      }
      else if (dynamic_cast<VariableCanvasItem*>(&item))
      {
        dynamic_cast<VariableCanvasItem*>(&item)->getVar()->moveTo(x, y);
      }
      else if (dynamic_cast<GroupCanvasItem*>(&item))
      {
        dynamic_cast<GroupCanvasItem*>(&item)->getIcon().moveTo(x, y);
      }

      std::for_each(affectedWires.begin(), affectedWires.end(),
          [](LinkCanvasItem* wire)
          {
            wire->update();
          });
      scrollIntoView(item.getModelRect());
    }
    break;

  case AbstractCanvasItem::moveCancelled:
    {
      item.moveTo(x, y);
      if (dynamic_cast<OperationCanvasItem*>(&item))
      {
        dynamic_cast<OperationCanvasItem*>(&item)->getOp()->moveTo(x, y);
      }
      else if (dynamic_cast<GodleyCanvasItem*>(&item))
      {
        dynamic_cast<GodleyCanvasItem*>(&item)->getIcon().moveTo(x, y);
      }
      else if (dynamic_cast<PlotCanvasItem*>(&item))
      {
        dynamic_cast<PlotCanvasItem*>(&item)->getPlot().moveTo(x, y);
      }
      else if (dynamic_cast<VariableCanvasItem*>(&item))
      {
        dynamic_cast<VariableCanvasItem*>(&item)->getVar()->moveTo(x, y);
      }
      else if (dynamic_cast<GroupCanvasItem*>(&item))
      {
        dynamic_cast<GroupCanvasItem*>(&item)->getIcon().moveTo(x, y);
      }
      scrollIntoView(item.getModelRect());
      std::for_each(affectedWires.begin(), affectedWires.end(),
          [](LinkCanvasItem* wire)
          {
            wire->update();
          });
    }
    break;

  case AbstractCanvasItem::moveConfirm:
    {
      if (dynamic_cast<OperationCanvasItem*>(&item))
      {
        document.moveOperation(this, item.getId(), x, y);
      }
      else if (dynamic_cast<GodleyCanvasItem*>(&item))
      {
        document.moveGodleyTable(this, item.getId(), x, y);
      }
      else if (dynamic_cast<PlotCanvasItem*>(&item))
      {
        document.movePlot(this, dynamic_cast<PlotCanvasItem&>(item).getName(), x, y);
      }
      else if (dynamic_cast<VariableCanvasItem*>(&item))
      {
        document.moveVariable(this, item.getId(), x, y);
      }
      else if (dynamic_cast<GroupCanvasItem*>(&item))
      {
        document.moveGroup(this, item.getId(), x, y);
      }

      //TODO: collision check.
      item.moveTo(x, y);
      scrollIntoView(item.getModelRect());
      std::for_each(affectedWires.begin(), affectedWires.end(),
          [](LinkCanvasItem* wire)
          {
            wire->update();
          });
    }
    break;
  }
  return true;
}

WireEventAreaPtr CanvasView::getWireEventArea()
{
  if (!wireEventArea)
  {
    if (width() == 0 || height() == 0)
    {
      setModelRect(0, 0, 10, 10);
    }
    wireEventArea = std::make_shared<WireEventArea>(this);
  }
  return wireEventArea;
}

void CanvasView::groupRect(const Wt::WRectF rect)
{
  getDoc().groupRect(this, rect);
  auditModel();
}

void CanvasView::enableItems()
{
  disablingOverlay.reset();
}

/**
 * Disables mouse access to items on the canvas.
 */
void CanvasView::disableItems()
{
  if (!disablingOverlay)
  {
    disablingOverlay = std::make_shared<DisablingOverlay>(this);
  }
}

void CanvasView::auditSelection()
{
}

void CanvasView::copySelectionToClipBoard()
{
  //TODO: selection
  //MinskyApp::getClipBoard().insert((void*)&selection);
}

void CanvasView::editCopyImpl()
{
  copySelectionToClipBoard();
}

void CanvasView::editCutImpl()
{
  copySelectionToClipBoard();
  //deleteSelection()
}

void CanvasView::editPasteImpl()
{
  //TODO:
}

void CanvasView::setActivity(Activity act)
{
  //
  //  Activity is contyrolled by the use of overlays.
  //
  if (act != activity)
  {
    selectionOverlay.reset();
    addWireOverlay.reset();
    deleteItemsOverlay.reset();

    switch (act)
    {
    default:
    case actDefault:
      activity = actDefault;
      break;

    case actAddWire:
      activity = actAddWire;
      addWireOverlay = std::make_shared<WireCreateOverlay>(this);
      break;

    case actSelectGroup:
      activity = actSelectGroup;
      selectionOverlay = std::make_shared<RectSelectionOverlay>(this);
      break;

    case actDelete:
      activity = actDelete;
      deleteItemsOverlay = std::make_shared<DeleteItemsOverlay>(this);
      break;
    }
  }
}

void CanvasView::deleteItem(AbstractCanvasItem* item)
{
  if (dynamic_cast<OperationCanvasItem*>(item))
  {
    getDoc().removeOperation(this, item->getId());
  }
  else if (dynamic_cast<VariableCanvasItem*>(item))
  {
    getDoc().removeVariable(this, item->getId());
  }
  else if (dynamic_cast<LinkCanvasItem*>(item))
  {
    getDoc().removeWire(this, item->getId());
  }
  else if (dynamic_cast<GroupCanvasItem*>(item))
  {
//    getDoc().removeGroup(this, item->getId());
  }
  else if (dynamic_cast<GodleyCanvasItem*>(item))
  {
    getDoc().removeGodleyTable(this, item->getId());
  }
  else if (dynamic_cast<PlotCanvasItem*>(item))
  {
    getDoc().removePlot(this, dynamic_cast<PlotCanvasItem*>(item)->getName());
  }

  //  if all tests above failed, there is something amiss,
  //  so an audit of the model is a good idea anyways.

  auditModel();
  if (deleteItemsOverlay)
  {
    deleteItemsOverlay = std::make_shared<DeleteItemsOverlay>(this);
  }
}

void CanvasView::addOperation(minsky::OperationType::Type opType)
{
  setActivity(actDefault);
  WPointF pntCenter = getScreenCenter();
  selection.clear();
  ObjectId id = getDoc().addOperation(this, opType, pntCenter);
  loadOperation(id, document.getModel().operations[id]);

  if (opType == OperationType::constant)
  {
    if (!widgets[id]->editProperties())
    {
      //  cancel operation
      //  TODO: should use Undo
      delete widgets[id];
      widgets.erase(widgets.find(id));
      getDoc().removeOperation(this, id);
    }
  }
//  selection.push_back(id);
//  setActivity(actMoveSelection);
}

void CanvasView::addGodleyTable()
{
  setActivity(actDefault);
  WPointF pntCenter = getScreenCenter();
  ObjectId id = getDoc().addGodleyTable(this, pntCenter);
  GodleyIcon& icon = document.getModel().godleyItems[id];
  icon.table.nameUnique();
  loadGodleyIcon(id, icon);
//  selection.push_back(id);
//  setActivity(actMoveSelection);
}

void CanvasView::addPlotWidget()
{
  setActivity(actDefault);
  WPointF pntCenter = getScreenCenter();
  int id = getDoc().addPlot(this, pntCenter);
  PlotWidget& plot = document.getModel().plots[id];
  loadPlot(id, plot);
}

void CanvasView::addVariable()
{
  setActivity(actDefault);
  std::stringstream str;
  str << "variable";
  std::string name = str.str();
  WPointF pntCenter = getScreenCenter();
  ObjectId id = getDoc().addVariable(this, name, pntCenter);
  loadVariable(id, document.getModel().variables[id]);

  if (!widgets[id]->editProperties())
  {
    //  cancel operation
    //  TODO: should use Undo
    delete widgets[id];
    widgets.erase(widgets.find(id));
    getDoc().removeVariable(this, id);
  }
}

void CanvasView::addWire(int from, int to)
{
  MinskyDoc::ObjectId id = getDoc().addWire(this, from, to);
  if (id != MinskyDoc::invalidId)
  {
    if (getDoc().getModel().wires.count(id))
    {
      loadWire(id, getDoc().getModel().wires[id]);
      if (widgets.count(id))
      {
        widgets[id]->update();
        widgets[id]->initInteractive();
        getWireEventArea()->update();
      }
    }
  }
}

void CanvasView::docHasChanged(AbstractView* source, MinskyDoc::Hint hint,
    const AnyParams& data)
{
  if (source != this)
  {
    //
    //  There is only one editing source at the moment,
    //  so not all hints need handled.
    //
    switch (hint)
    {
    case MinskyDoc::hintDocumentLoad:
      scanAndLoadDocument();
      break;

    case MinskyDoc::hintDocumentClose:
      clear();
      break;

    case MinskyDoc::hintSimulationStep:
      std::for_each(plots.begin(), plots.end(),
        [](ModelPlotWidgets::value_type& val)
        {
          val.second->update();
        });
      break;

    case MinskyDoc::hintSimulationReset:
      std::for_each(plots.begin(), plots.end(),
        [this](ModelPlotWidgets::value_type& val)
        {
          val.second->reset();
        });
        break;

    case MinskyDoc::hintEditGodleyTable:
      if (data[0].type() == typeid(ObjectId))
      {
        ObjectId tableId = boost::any_cast<ObjectId>(data[0]);
        ModelWidgets::iterator it = widgets.find(tableId);
        if(it != widgets.end())
        {
          it->second->update();
        }
      }
      break;

      // Example for undo/redo operation
    case MinskyDoc::hintAddOperation:
      if (0)  // not implemented
      {
        ObjectId id = boost::any_cast<ObjectId>(data[0]);
        OperationPtr& op = *document.getModel().operations.find(id);
        loadOperation(id, op);
      }
      break;

    default:
      // not implemented
      break;
    }
  }
}

void CanvasView::scanAndLoadDocument()
{
  minsky::Minsky& model = document.getModel();

  //  reset zoom and offsets
  setZoom(DisplaySettings::defaultZoom, 0, 0);
  std::for_each(model.operations.begin(), model.operations.end(),
      [this](minsky::Operations::value_type& val)
      {
        if (isVisible(val))
        {
          loadOperation(val.id(), val);
        }
      });

  std::for_each(model.wires.begin(), model.wires.end(),
      [this](minsky::PortManager::Wires::value_type& val)
      {
        if (isVisible(val))
        {
          loadWire(val.id(), val);
        }
      });

  for(minsky::Plots::value_type& p: model.plots)
      {
        if (isVisible(p))
        {
          loadPlot(p.id(), p);
        }
      }

  std::for_each(model.godleyItems.begin(), model.godleyItems.end(),
      [this](minsky::Minsky::GodleyItems::value_type& val)
      {
        if (isVisible(val))
        {
          loadGodleyIcon(val.id(), val);
        }
      });

  std::for_each(model.variables.begin(), model.variables.end(),
      [this](VariableManager::value_type& val)
      {
        //  skip godley table variables
        if (isVisible(val))
        {
          loadVariable(val.id(), val);
        }
      });

  std::for_each(model.groupItems.begin(), model.groupItems.end(),
      [this](GroupIcons::value_type& val)
      {
        if (isVisible(val))
        {
          loadGroup(val.id(), val);
        }
      });

  computeModelExtents();
  zoomAll();
}

void CanvasView::loadOperation(ObjectId id, OperationPtr op)
{
  OperationCanvasItem* opItem = new OperationCanvasItem(id, op, this);
  widgets[id] = opItem;
}

void CanvasView::loadWire(ObjectId id, Wire& wire)
{
  LinkCanvasItem* linkItem = new LinkCanvasItem(id, wire, this);
  widgets[id] = linkItem;
}

void CanvasView::loadPlot(int name, PlotWidget& plot)
{
  PlotCanvasItem* plotItem = new PlotCanvasItem(name, plot, this);
  plots[name] = plotItem;
}

void CanvasView::loadGodleyIcon(ObjectId id, minsky::GodleyIcon& table)
{
  GodleyCanvasItem* godleyItem = new GodleyCanvasItem(id, table, this);
  widgets[id] = godleyItem;
}

void CanvasView::loadVariable(ObjectId id, minsky::VariablePtr var)
{
  VariableCanvasItem* item = new VariableCanvasItem(id, var, this);
  widgets[id] = item;
}

void CanvasView::loadGroup(ObjectId id, GroupIcon& icon)
{
  GroupCanvasItem* item = new GroupCanvasItem(id, icon, this);
  widgets[id] = item;
}

void CanvasView::invalidate()
{
  forAllViewObjects([](AbstractCanvasItem* item) -> void
  {
    item->update();
  });

  if (linkOverlay)
  {
    linkOverlay->update();
  }

  //  refresh the wires overlay
  getWireEventArea()->resize(width().value(), height().value());

  std::for_each(widgets.begin(), widgets.end(),
      [](ModelWidgets::value_type& pair)
      {
        LinkCanvasItem* linkItem = dynamic_cast<LinkCanvasItem*>(pair.second);
        if (linkItem)
        {
          linkItem->initInteractive();
        }
      });

  //  debug trace.
//  getWireEventArea()->update();
  if (deleteItemsOverlay)
  {
    deleteItemsOverlay = std::make_shared<DeleteItemsOverlay>(this);
  }
}

void CanvasView::clickedToolDefault()
{
  setActivity(actDefault);
}

void CanvasView::clickedToolLink()
{
  setActivity(actAddWire);
}

void CanvasView::clickedToolLasso()
{
  setActivity(actSelectGroup);
}

void CanvasView::clickedToolPan()
{
  setActivity(actDelete);
}

void CanvasView::clickedZoomIn()
{
  zoomIn(DisplaySettings::zoomFactorIncrement);
}

void CanvasView::clickedZoomOut()
{
  zoomOut(DisplaySettings::zoomFactorIncrement);
}

void CanvasView::clickedZoomReset()
{
  WPointF pntCenter = getScreenCenter();
  setZoom(DisplaySettings::defaultZoom, pntCenter.x(), pntCenter.y());
}

void CanvasView::clickedPlot()
{
}


void CanvasView::layoutSizeChanged(int width, int height)
{
  _base_class::layoutSizeChanged(width, height);

  getWireEventArea()->resize(width, height);
}

void CanvasView::computeModelExtents()
{
  WRectF rectModel(0, 0, 0, 0);

  forAllViewObjects([&rectModel](const AbstractCanvasItem* item) -> void
  {
    rectModel = rectModel.united(item->getModelRect());
  });

  setModelRect(rectModel.left(), rectModel.top(), rectModel.right(),
      rectModel.bottom());
}

void CanvasView::adjustViewPos()
{
  //
  //  no scaling is taking place and there is no need to
  //  redraw the whole thing, as a little nudge to the side
  //  will have the desired effect.
  //

  forAllViewObjects([](AbstractCanvasItem* item) -> void
  {
    item->adjustWidgetPos();
  });

  if (linkOverlay)
  {
    linkOverlay->adjustWidgetPos();
  }

  //  resize and refresh the wires overlay
  getWireEventArea()->resize(width().value(), height().value());

  std::for_each(widgets.begin(), widgets.end(),
      [](ModelWidgets::value_type& pair)
      {
        LinkCanvasItem* linkItem = dynamic_cast<LinkCanvasItem*>(pair.second);
        if (linkItem)
        {
          linkItem->initInteractive();
        }
      });
}

void CanvasView::forAllViewObjects(
    boost::function<void(AbstractCanvasItem*)> fn)
{
  //  always do the wires last.

  std::for_each(widgets.begin(), widgets.end(),
      [&fn](ModelWidgets::value_type& val) -> void
      {
        if (!dynamic_cast<LinkCanvasItem*>(val.second))
        {
          fn(val.second);
        }
      });

  std::for_each(plots.begin(), plots.end(),
      [&fn](ModelPlotWidgets::value_type& val) -> void
      {
        fn(val.second);
      });

  std::for_each(widgets.begin(), widgets.end(),
      [&fn](ModelWidgets::value_type& val) -> void
      {
        if (dynamic_cast<LinkCanvasItem*>(val.second))
        {
          fn(val.second);
        }
      });
}

void CanvasView::forAllViewObjects(
    boost::function<void(const AbstractCanvasItem*)> fn) const
{
  //  always do the wires last.

  std::for_each(widgets.begin(), widgets.end(),
      [&fn](const ModelWidgets::value_type& val) -> void
      {
        if (!dynamic_cast<const LinkCanvasItem*>(val.second))
        {
          fn(val.second);
        }
      });

  std::for_each(plots.begin(), plots.end(),
      [&fn](const ModelPlotWidgets::value_type& val) -> void
      {
        fn(val.second);
      });

  std::for_each(widgets.begin(), widgets.end(),
      [&fn](const ModelWidgets::value_type& val) -> void
      {
        if (dynamic_cast<const LinkCanvasItem*>(val.second))
        {
          fn(val.second);
        }
      });
}

void CanvasView::getAttachedWires(WiresList& wiresList, const AbstractCanvasItem& item)
{
  AbstractCanvasItem::PortsList portsList;
  item.getPortsList(portsList);

  wiresList.clear();
  std::for_each(widgets.begin(), widgets.end(),
      [this, &portsList, &wiresList](ModelWidgets::value_type& val) -> void
      {
        LinkCanvasItem* item = dynamic_cast<LinkCanvasItem*>(val.second);
        if (item)
        {
          Wire& wire = item->getWire();
          if (std::find(portsList.begin(), portsList.end(), wire.from) != portsList.end()
             || std::find(portsList.begin(), portsList.end(), wire.to) != portsList.end())
          {
            wiresList.push_back(item);
          }
        }
      });
}

void CanvasView::auditModel()
{
  deleteItemsNotInModel();
  createItemsNotRendered();
}

void CanvasView::deleteItemsNotInModel()
{
  std::vector<ObjectId> deleteList;
  std::for_each(widgets.begin(), widgets.end(),
      [this, &deleteList](const ModelWidgets::value_type& val) -> void
      {
        ObjectId id = val.second->getId();
        if (dynamic_cast<const GodleyCanvasItem*>(val.second))
        {
          if (!getDoc().getModel().godleyItems.count(id)
            || !isVisible(dynamic_cast<const GodleyCanvasItem*>(val.second)->getIcon()))
          {
            deleteList.push_back(id);
          }
        }
        else if (dynamic_cast<const GroupCanvasItem*>(val.second))
        {
          if (!getDoc().getModel().groupItems.count(id)
              || !isVisible(dynamic_cast<const GroupCanvasItem*>(val.second)->getIcon()))
          {
            deleteList.push_back(id);
          }
        }
        else if (dynamic_cast<const OperationCanvasItem*>(val.second))
        {
          if (!getDoc().getModel().operations.count(id)
              || !isVisible(dynamic_cast<const OperationCanvasItem*>(val.second)->getOp()))
          {
            deleteList.push_back(id);
          }
        }
        else if (dynamic_cast<const VariableCanvasItem*>(val.second))
        {
          if (!getDoc().getModel().variables.count(id)
              || !isVisible(dynamic_cast<const VariableCanvasItem*>(val.second)->getVar()))
          {
            deleteList.push_back(id);
          }
        }
      });

  std::for_each(deleteList.begin(), deleteList.end(),
      [this](ObjectId id)
      {
        delete widgets[id];
        widgets.erase(id);
      });

  std::vector<int> deleteNames;

  std::for_each(plots.begin(), plots.end(),
      [this, &deleteNames](ModelPlotWidgets::value_type& val)
      {
        if (!getDoc().getModel().plots.count(val.first)
            || !isVisible(val.second->getPlot()))
        {
            deleteNames.push_back(val.first);
        }
      });

  for (int str: deleteNames)
    {
      delete plots[str];
      plots.erase(str);
    };

  //
  //  do wires last, as releasing objects could have had some effect.
  //

  deleteList.clear();
  std::for_each(widgets.begin(), widgets.end(),
      [this, &deleteList](ModelWidgets::value_type& val) -> void
      {
        ObjectId id = val.second->getId();
        if (dynamic_cast<LinkCanvasItem*>(val.second))
        {
          if (!getDoc().getModel().wires.count(id)
              || !isVisible(dynamic_cast<LinkCanvasItem*>(val.second)->getWire()))
          {
            deleteList.push_back(id);
          }
        }
      });

  std::for_each(deleteList.begin(), deleteList.end(),
      [this](ObjectId id)
      {
        delete widgets[id];
        widgets.erase(id);
      });

  if (linkOverlay && std::find(deleteList.begin(),
                               deleteList.end(),
                               linkOverlay->getId()) != deleteList.end())
  {
    linkOverlay.reset();
  }
}

void CanvasView::createItemsNotRendered()
{
  std::for_each(getDoc().getModel().godleyItems.begin(), getDoc().getModel().godleyItems.end(),
      [this](GodleyIcons::value_type& val)
      {
        if (isVisible(val) && !widgets.count(val.id()))
        {
          loadGodleyIcon(val.id(), val);
        }
      });

  std::for_each(getDoc().getModel().groupItems.begin(), getDoc().getModel().groupItems.end(),
      [this](GroupIcons::value_type& val)
      {
        if (isVisible(val) && !widgets.count(val.id()))
        {
          loadGroup(val.id(), val);
        }
      });

  std::for_each(getDoc().getModel().operations.begin(), getDoc().getModel().operations.end(),
      [this](Operations::value_type& val)
      {
        if (isVisible(val) && !widgets.count(val.id()))
        {
          loadOperation(val.id(), val);
        }
      });

  std::for_each(getDoc().getModel().variables.begin(), getDoc().getModel().variables.end(),
      [this](VariableManager::value_type& val)
      {
        if (isVisible(val) && !widgets.count(val.id()))
        {
          loadVariable(val.id(), val);
        }
      });

  for (Plots::value_type& val: getDoc().getModel().plots)
    {
      if (isVisible(val) && !plots.count(val.id()))
        {
          loadPlot(val.id(), val);
        }
    };

  //  wires last
  std::for_each(getDoc().getModel().wires.begin(), getDoc().getModel().wires.end(),
      [this](Minsky::Wires::value_type& val)
      {
        if (isVisible(val) && !widgets.count(val.id()))
        {
          loadWire(val.id(), val);
        }
      });
}

bool CanvasView::isVisible(const GroupIcon& group)
{
  return (group.groupId() != MinskyDoc::invalidId
          && group.parent() == MinskyDoc::invalidId);
}

bool CanvasView::isVisible(const PlotWidget& plot)
{
  return true;
}

bool CanvasView::isVisible(const OperationPtr op)
{
  return (op->visible && op->group == MinskyDoc::invalidId);
}

bool CanvasView::isVisible(const VariablePtr var)
{
  return (var->visible && var->group == MinskyDoc::invalidId);
}

bool CanvasView::isVisible(const GodleyIcon& icon)
{
  return true;
}

bool CanvasView::isVisible(const Wire& wire)
{
  return (wire.visible
          && wire.group == MinskyDoc::invalidId);
}

}}  // namespace minsky::gui

