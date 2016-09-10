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

#ifndef ABSTRACTCANVASITEM_H_
#define ABSTRACTCANVASITEM_H_

#include <Wt/WPaintedWidget>
#include <Wt/WPointF>
#include "minskyDoc.h"
#include "tempCairoImageFile.h"
#include "wtGeometry.h"
#include <memory>

namespace minsky { namespace gui {

using namespace Wt;

class CanvasView;
class CanvasPainter;

/**
 *  Base class for items that belong on the canvas.
 *
 *  The base class is protected to enforce that all coordinates are
 *  expressed in model space units.
 */
class AbstractCanvasItem : protected WPaintedWidget
{
  friend CanvasPainter; ///< The painter needs access to base class.

private:
  typedef WPaintedWidget _base_class;  ///< The base class.
  typedef TempCairoImageFile Image;
  typedef std::shared_ptr<Image> ImagePtr;

public:
  /**
   * Exception thrown when reference checking against model object
   * dictionaries fail.
   */
  struct mapReferenceException
      : virtual boost::exception,
        virtual std::exception { };

  /**
   * States for moving the object around on the canvas.
   * @see CanvasView::moveItem()
   */
  enum MoveState
  {
    moveBegin,        ///< User initiated moving by pressing the
                      ///  left mouse button.
    moveInProgress,   ///< User is dragging the object on the canvas.
    moveCancelled,    ///< Move operation is cancelled.
    moveConfirm,      ///< User ended the operation by releasing the
                      ///  mouse button or pressing the escape key.
  };

  /**
   *  The different possible selection states.
   */
  enum SelectionStatus
  {
    notSelected,  ///<  Item is not selected.
    singleSelect, ///<  Item is the only selected item in the view.
    multiSelect,  ///<  Item is selected, along with other items in the view.
  };

  /**
   * A list of Canvas items.
   */
  typedef std::vector<AbstractCanvasItem*> ItemsList;

  /**
   * A list of ports.
   */
  typedef std::vector<int> PortsList;

private:
  MinskyDoc::ObjectId id;      ///< TCL object id.
  SelectionStatus  selected;   ///< Indicates the current selection status.

  bool moving = false;  ///<  Flag indicating the usert is moving the object
                        ///   with the mouse.
  WPointF savedMousePos;///<  saved mouse position in model coordinates. Used to
                        ///   move object with mouse.
  WPointF savedPos;     ///<  saved object position in model coordinates. Used to
                        ///   move object with mouse.
  WPointF origin;       ///<  Local origin for drawing and insertion.
  ImagePtr sprite;      ///<  Graphic representation of the object.

protected:
  Polygon shapeOutline; ///<  Outline of object in model space.

protected:
  /**
   *  Constructor.  The specialized classes constructors are responsible for
   *  creating an area for the widget to respond to events.
   */
  AbstractCanvasItem(MinskyDoc::ObjectId id, CanvasView* parent);

public:
  /**
   *  Destructor.
   */
  virtual ~AbstractCanvasItem();

  /**
   * Returns the parent cast as a CanvasView object.
   */
  /** @{ */
  CanvasView* parent();
  const CanvasView* parent() const;
  /** @} */

  /**
   * Accesses the sprite.
   */
  inline const Wt::WPainter::Image& painterImage()
  {
    return *(sprite->getImage());
  }

  /**
   * Sets a new sprite, the old one is deleted along with its file.
   */
  inline void setImage(Image* image)
  {
    sprite = ImagePtr(image);
  }

  /**
   * Returns the insertion point within the widget rectangle. In model units.
   */
  inline const WPointF& getLocalOrigin() const
  {
    return origin;
  }

  /**
   * Adds an interactive area for user mouse action.
   */
  inline void addArea(WAbstractArea* area)
  {
    _base_class::addArea(area);
  }

  /**
   * Accesses the object TCL id.
   */
  inline MinskyDoc::ObjectId getId() const
  {
    return id;
  }

  /**
   *  returns the position of the object in model coordinates.
   */
  virtual void getModelPos(double& x, double& y) const = 0;

  /**
   * Returns the model space rectangle.
   */
  virtual WRectF getModelRect() const;

  /**
   *  Sometimes the view needs to translate the origin of its coordinate
   *  system.  It then tells all its children to adjust their position
   *  through this method.
   *
   */
  virtual void adjustWidgetPos();

  /**
   *  Moves the widget to a new location in model space.
   *  The widget is moved to a new position.  The coordinates are referenced
   *  to the top left corner of the canvas.
   *  @param x the abscissa of the new position in model space.
   *  @param y the ordinate of the new position in model space.
   */
  virtual void moveTo(const double& x, const double& y);

  /**
   *  Sets the selection status used for painting.
   *  @param  sel the new selection state
   *  @param repaintNow if true (the default) the item is redrawn immediately
   */
  virtual void setSelected(SelectionStatus sel, bool repaintNow = true);

  /**
   *  Returns the selection status
   */
  inline SelectionStatus getSelected() const
  {
    return selected;
  }

  /**
   * Forces a redraw of the widget.
   */
  inline void update()
  {
    _base_class::update();
  }

  /**
   *  Indicates whether the user is moving the object with the mouse.
   */
  bool movingWithMouse() const
  {
    return moving;
  }

  /**
   * Provides access to the base class object.
   */
  /** @{ */
  inline operator WPaintedWidget& ()
  {
    return *this;
  }

  inline operator const WPaintedWidget& () const
  {
    return *this;
  }
  /** @} */

  /**
   *  Opens the properties dialog so the user can edit object.
   *  @returns true if object properties were modified.
   */
  virtual bool editProperties() = 0;

  /**
   * Returns a list of ports that provide connection to this item
   */
  virtual void getPortsList(PortsList& ports) const = 0;

  /**
   *  Returns the visible outline of the object.
   *  @param poly On return, contains the shape outline in model space.
   */
  virtual void getShapeOutline(Polygon& poly) const;

  /**
   * Convenience for access to z-index
   */
  inline int zIndex() const
  {
    return _base_class::zIndex();
  }

  /**
   * Convenience for access to z-index
   */
  inline void setZIndex(int index)
  {
    return _base_class::setZIndex(index);
  }

  /**
   * Sets up interactive behaviour.
   */
  virtual void initInteractive();

  /**
   * Activates the context menu associated with this item.
   */
  virtual void showContextMenu(const Wt::WMouseEvent& event);

  /**
   * Initializes the popup menu.  Specializing this method allows for
   * cutomizaqtion of the popup menu.
   */
  virtual bool initPopupMenu(Wt::WPopupMenu& menu);

protected:
  /**
   *  Resizes the widget, and its corresponding event-catching area.
   *  @param w new width of the widget.
   *  @param h new height for the widget.
   */
  virtual void resize(const WLength& width, const WLength& height);

  /**
   * Called by derived objects to initiate moving with mouse.
   */
  void beginMoveWithMouse(const WMouseEvent& event);

  /**
   * Called by derived objects while moving with mouse.
   */
  void updateMoveWithMouse(const WMouseEvent& event);

  /**
   * Called by derived objects to indicate moving with mouse is complete.
   */
  void endMoveWithMouse(const WMouseEvent& event);

  /**
   * Cancels moving the object.
   */
  void cancelMoveWithMouse();

 /**
  * Sets the local origin, which corresponds to the insertion point
  * within the model rectangle.  In model space coordinates.
  */
  inline void setLocalOrigin(const double& x, const double& y)
  {
    origin.setX(x);
    origin.setY(y);
  }

  /**
   * Draw the widget using the painter.
   * @param painter Painter to use for drawing.
   * @param outline On return, contains the shape outline of the widget in model space.
   */
  virtual void draw(CanvasPainter& painter, Polygon& outline) = 0;

  /**
   * Called by Wt  when the widget needs redrawn.
   */
  virtual void  paintEvent(WPaintDevice *paintDevice);
};

}} // namespace minsky::gui

#endif /* ABSTRACTCANVASITEM_H_ */
