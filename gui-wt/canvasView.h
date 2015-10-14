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

#ifndef CANVASVIEW_H
#define CANVASVIEW_H

#include "guiDefs.h"
#include "GUI/port.h"
#undef None
#include <Wt/WContainerWidget>
#include <Wt/WPaintedWidget>
#include <Wt/WPainter>
#include "abstractView.h"
#include "abstractCanvasItem.h"
#include "minskyDoc.h"
#include "scrollBarArea.h"
#include "plot.h"       //  from ecolab
#undef None
#include "linkCanvasOverlay.h"
#include "linkCanvasItem.h"
#include "rectSelectionOverlay.h"
#include "wireCreateOverlay.h"
#include "deleteItemsOverlay.h"
#include "disablingOverlay.h"

namespace minsky { namespace gui {

using namespace Wt;
using ecolab::Plot;
class MainToolbar;
class PlotCanvasItem;

/**
 *  CanvasView controls the main canvas where the model is displayed.
 *
 *  As the main editing view of the application, children are held in
 *  CanvasView::widgets and CanvasViewPlots, as these use different
 *  referencing schemes in the model.
 *
 *  All the modifications to the model are delegated to the document
 *  class MinskyDoc.  This insures that changes are propagated to
 *  other views.  There are no other visible view at the moment, but
 *  the undo/redo mechanism uses the document/view system to catch
 *  modifications and restore state.
 *
 *  All painting operations for this class is handled by CanvasPainter,
 *  so there is no drawing taking place here.  Also, all coordinates
 *  are in model space.
 *
 *  TODO:  Selecting, grouping, and ungrouping model elements. Cut/paste/copy
 *
 */
class CanvasView: public AbstractView, public ScrollBarArea
{
  /**
   * MainToolbar has the right to signal event directly to the view.
   */
  friend MainToolbar;

  /**
   *  DeleteItemsOverlay has the right to access individual canvas items.
   */
  friend DeleteItemsOverlay;

private:
  /**
   * Base class.
   */
  typedef ScrollBarArea _base_class;

public:
  /**
   *  Id used as handle to refer to objects.
   */
  typedef MinskyDoc::ObjectId ObjectId;

  /**
   *  The selection
   */
  typedef std::vector<AbstractCanvasItem*> Selection;

  /**
   * Plots in the plot store need a server counterpart for display.  This type
   * corresponds 1 to 1 to plots in the minsky:Plots database, this will have
   * to be optimized and streamlined at some point.
   */
  typedef ::classdesc::shared_ptr<WPainter::Image> ImagePtr;

  /**
   * A repository of sprites the browser will use for display.
   */
  typedef std::map<std::string, ImagePtr> SpriteStore;

  /**
   * Plot from ecolab.
   */
  typedef classdesc::shared_ptr<Plot> PlotPtr;

  /**
   *  Collection of widget, the pointers belong to the Wt framework.
   */
  typedef std::map<MinskyDoc::ObjectId, AbstractCanvasItem*> ModelWidgets;

  /**
   *  Collection of plot widget, the pointers belong to the Wt framework.
   *  Plots have no Object ID, and need a different indexing scheme.
   */
  typedef std::map<int, PlotCanvasItem*> ModelPlotWidgets;

  /**
   * Available mouse tools
   */
  enum MouseTool
  {
    toolDefault,  ///<  The default tools, allows for selection of objects,
                  ///   rectangle selection and moving.
    toolLink,     ///<  The link tool lets the user create links between objects.
    toolPan,      ///<  The pan tool allows the user to move to move the viewport.
    toolLasso,    ///<  The lasso tool allows for advanced selection via a ploygon.
  };

  /**
   *  Key modifiers, these indicate which key the user is currently holding.
   */
  enum KeyModifier
  {
    keyNoKey = 0x00, ///<  No keys are being pressed.
    keyShift = 0x01, ///<  The shift key is pressed.
    keyControl = 0x02, ///<  The control key is pressed.
    keyAlt = 0x04, ///<  The Alt key is pressed.
  };

  /**
   *  Activities.  These define possible activities for the user interaction.
   *  Further refinement is defined using key modifiers.
   */
  enum Activity
  {
    actDefault,         ///<  Default: Add Items and move them around.
    actAddWire,         ///<  The user is adding wires.
    actSelectGroup,     ///<  The user is selecting items to create a group.
    actDelete,          ///<  The user is removing items.
  };

  /**
   * A list of wires.
   */
  typedef std::vector<LinkCanvasItem*> WiresList;

private:
  MinskyDoc& document;   ///<  The document.
  MouseTool mouseTool;        ///<  The currently selected mouse tool.
  Activity activity;
  Selection selection;        ///<  The current selection.
  KeyModifier keyModifier;    ///<  The keys currently pressed by the user.
  Wt::Coordinates mousePos0;  ///<  Position of the mouse at the beginning
                              ///   of the current operation.
  Wt::Coordinates mousePos;   ///<  Latest known mouse position.

  WiresList affectedWires;    ///<  List of wires affected by a move operation.

  //
  //  Links to model data
  //

  ModelWidgets widgets;       ///<  Items to draw, including wires.
  ModelPlotWidgets plots;     ///<  Plots have no ID, but a name and need a different
                              ///   addressing scheme.
  //
  //  Overlays
  //

  LinkCanvasOverlayPtr linkOverlay;         ///< overlay to let the user position control points.
  WireEventAreaPtr     wireEventArea;       ///< Area to catch wire events.  Wires share this area
                                            ///  for their events, as Their individual widgets
                                            ///  overlap and do otherwise mask each other.

  //
  //  Activity overlays
  //

  RectSelectionOverlayPtr selectionOverlay; ///< this overlay allows selection within a rectangle.
  WireCreateOverlayPtr    addWireOverlay;   ///< this overlay allows user to add wire links between objects.
  DeleteItemsOverlayPtr   deleteItemsOverlay; ///<  this overlay allows user to remove items from the model.
  DisablingOverlayPtr     disablingOverlay; ///< Disables mouse acces to canvas items.

public:
  /**
   *  Constructor
   */
  CanvasView(MinskyDoc& doc, WContainerWidget* parent = NULL);

  /**
   *  Destructor
   */
  ~CanvasView();

  /**
   *  Initializes the canvas window.  This is also where the user
   *  interaction events are defined.
   */
  void initialize();

  /**
   * Clears all models links
   */
  virtual void clear();

  //
  //  Contents notifications
  //

  /**
   * Called by canvas items to indicate the user is moving them around.
   * This method is called by all items except wires, which have no
   * independent moving mechanism.
   * @param item Item being moved.
   * @param state State of the move.
   * @returns true if the current position is good for dropping the object.
   */
  bool moveItem(AbstractCanvasItem& item, AbstractCanvasItem::MoveState state,
      double& x, double& y);

  /**
   * Redraws the entire model.
   */
  virtual void invalidate();

  /**
   * Recalculate offsets for child widgets, usually after the model rectangle
   * has changed.
   */
  virtual void adjustViewPos();

  /**
   * Checks for any model-generated changes in the data.  Some
   * changes like renaming variables can generate a detroy/create
   * operation on the variable and its connected wires, for example.
   */
  void auditModel();

  /**
   * Generates the list of wires connected to an item.
   */
  void getAttachedWires(WiresList& wiresList, const AbstractCanvasItem& item);

  /**
   * Display a link overlay to edit the specified wire item.
   */
  inline LinkCanvasOverlayPtr setLinkOverlay(LinkCanvasItem* item)
  {
    linkOverlay = std::make_shared<LinkCanvasOverlay>(item, this);
    return linkOverlay;
  }

  /**
   * Accesses the link overlay.
   */
  inline LinkCanvasOverlayPtr getLinkOverlay()
  {
    return linkOverlay;
  }

  /**
   * Creates the selection overlay, which inhibits all other
   * operations.
   * @return A shared pointer to the selection overlay.
   */
  inline RectSelectionOverlayPtr createSelectionOverlay()
  {
    if (!selectionOverlay)
    {
      selectionOverlay = make_shared<RectSelectionOverlay>(this);
    }
    return selectionOverlay;
  }

  /**
   * destroys the selection overlay, effectively exiting the selection
   * mode
   */
  inline void destroySelectionOverlay()
  {
    selectionOverlay.reset();
  }

  /**
   * Accesses the wires shared event area.
   * @return
   */
  WireEventAreaPtr getWireEventArea();

  /**
   * Sets the current user activity.  Activity is controlled with the use of overlays
   * that mask individual canvas items's event areas.
   */
  void setActivity(Activity act);

  /**
   * removes item from canvas and model.
   */
  void deleteItem(AbstractCanvasItem* item);

  /**
   * Groups items within a rectangle.
   */
  void groupRect(const Wt::WRectF rect);

  /**
   * Adds a new wire to the model.
   * @param from  Source port
   * @param to    Destination  port.
   */
  void addWire(int from, int to);

  /**
   * Enables mouse access to items on the canvas.
   */
  void enableItems();

  /**
   * Disables mouse access to items on the canvas.
   */
  void disableItems();

  /**
   * retrives a pointer to one of the canvas items, plots execpted.
   * @param id ID of the widget
   * @return Pointer to the widget, or NULL if not found.
   */
  inline AbstractCanvasItem* getItem(MinskyDoc::ObjectId id)
  {
    ModelWidgets::iterator it = widgets.find(id);
    if (it != widgets.end())
    {
      return it->second;
    }
    return NULL;
  }

protected:
  /**
   *  Checks that links between object in the selection are included as well.
   *  Links to/from object outside the selection are rejected and removed.
   */
  void auditSelection();

  /**
   *  Places a copy of the selection on the clipboard.
   */
  void copySelectionToClipBoard();

  /**
   *  Handler for edit/copy.
   */
  virtual void editCopyImpl();

  /**
   * Handler for edit/cut.
   */
  virtual void editCutImpl();

  /**
   * Handler for edit/paste
   */
  virtual void editPasteImpl();

  /**
   * Inserts an operation in the model.
   */
  void addOperation(minsky::OperationType::Type opType);

  /**
   * Inserts a godley table in the model.
   */
  void addGodleyTable();

  /**
   * Inserts a plot in the model.
   */
  void addPlotWidget();

  /**
   * Inserts a variable in the model
   */
  void addVariable();

  /**
   *  Called when the document has changed and needs refresh
   *  @param The View that caused the modification, or  NULL if not applicable.
   */
  void docHasChanged(AbstractView* source, MinskyDoc::Hint hint,
      const AnyParams& data);

  /**
   * Loads items from the models
   */
  void scanAndLoadDocument();

  /**
   * Loads an operation from the model
   */
  void loadOperation(ObjectId id, OperationPtr op);

  /**
   * Loads a wire (link)from the model
   */
  void loadWire(ObjectId id, Wire& wire);

  /**
   * Loads a plot from the model
   */
  void loadPlot(int, PlotWidget& plot);

  /**
   * Loads a godley table from the model
   */
  void loadGodleyIcon(ObjectId id, minsky::GodleyIcon& table);

  /**
   * Loads a variable from the model
   */
  void loadVariable(ObjectId id, minsky::VariablePtr var);

  /**
   * Loads a group from the model.
   */
  void loadGroup(ObjectId id, GroupIcon& icon);

  /**
   *  Handler for 'default tool' button on the main toolbar.
   */
  void clickedToolDefault();

  /**
   *  Handler for 'link tool' button on the main toolbar.
   */
  void clickedToolLink();

  /**
   *  Handler for 'lasso tool' button on the main toolbar.
   */
  void clickedToolLasso();

  /**
   *  Handler for 'pan tool' button on the main toolbar.
   */
  void clickedToolPan();

  /**
   *  Handler for 'zoom in' button on the main toolbar.
   */
  void clickedZoomIn();

  /**
   *  Handler for 'zoom out' button on the main toolbar.
   */
  void clickedZoomOut();

  /**
   *  Handler for 'zoom reset' button on the main toolbar.
   */
  void clickedZoomReset();

  /**
   *  Handler for 'plot' button on the main toolbar.
   */
  void clickedPlot();

  //
  //  Zooming and panning implementation
  //

  /**
   * Called by the framework when the size of the underlying page changes
   * browser-side.
   */
  virtual void layoutSizeChanged(int width, int height);

  /**
   * Computes the size of the model in model units.  This information is
   * kept and used by the zoom helper.
   */
  void computeModelExtents();

  /**
   * Executes an operation for all model objects represented.
   */
  /** @{ */
  void forAllViewObjects(boost::function<void(AbstractCanvasItem*)> fn);
  void forAllViewObjects(boost::function<void(const AbstractCanvasItem*)> fn) const;
  /** @} */

  /**
   * Deletes CanvasItems that are not found in model.  Called by auditModel.
   */
  void deleteItemsNotInModel();

  /**
   * Creates CanvasItems that are not yet rendered.  Called by auditModel.
   */
  void createItemsNotRendered();

  /**
   *  indicates whether an item should be shown on the canvas.
   */
  /** @{ */
  static bool isVisible(const GroupIcon& group);
  static bool isVisible(const PlotWidget& plot);
  static bool isVisible(const OperationPtr op);
  static bool isVisible(const VariablePtr var);
  static bool isVisible(const GodleyIcon& icon);
  static bool isVisible(const Wire& wire);
  /** @} */
};

}}  // namespace minsky::gui

#endif
