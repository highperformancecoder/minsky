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

#ifndef LINKCANVASOVERLAY_H_
#define LINKCANVASOVERLAY_H_

#include "minskyDoc.h"
#include "wtGeometry.h"
#include "displaySettings.h"
#include "GUI/wire.h"
#include <Wt/WPaintedWidget>
#include <list>
#include <vector>

namespace minsky { namespace gui {

class CanvasView;
class LinkCanvasItem;
class LinkCanvasOverlay;

/**
 * Widget representing a handle with which the user can control Bezier line
 * bending.
 *
 * @see LinkCanvasOverlay
 */
class BezierHandleWidget
    : public Wt::WPaintedWidget
{
private:
  typedef Wt::WPaintedWidget _base_class;

public:
  static const int widgetSize = 10;       ///<  size of widget in pixels.

public:
  /**
   * Function of the point in drawing Bezier lines.
   */
  enum PointType
  {
    bezierSummit,       ///<  Control point, or summit.
    bezierPassThrough,  ///<  Virtual pass-through point.  These virtual
                        ///   points are not represented in the minsky::Wire
                        ///   structure, and are added exactly at mid-point
                        ///   between control points.  When the user can move
                        ///   selects one and moves it on the canvas, it then
                        ///   becomes a control point and new pass-through
                        ///   points will be created.
  };

private:
  LinkCanvasOverlay&   overlay; ///< Overlay that handles user interaction.
  //Wt::WPointF pnt;              ///<  Location of the control point in model space.
  size_t      pntIndex;         ///<  index of the point in the Wire structure.
  PointType   pntType;          ///<  Type of Bezier point represented by the widget.
  bool        moving = false;   ///<  indicates we are moving the object with the mouse
  static Wt::WPointF savedMousePos; ///<  saved mouse position in model coordinates. Used to
                                    ///   move object with mouse.
  static Wt::WPointF savedPos;      ///<  saved object position in model coordinates. Used to
                                    ///   move object with mouse.

public:
  /**
   * Constructor.
   *
   * @param type        Point function in drawing Bezier lines.
   * @param point       Location of point in model space.
   * @param pointIndex  Index of point in the wire control points list.  If
   *                    type is bezierPassThrough, this is the lowest index of
   *                    the two points this virtual point is inserted between.
   * @param overlay     Overlay that handles user events.
   * @param parent      Canvas where the widget will be drawn.
   */
  BezierHandleWidget(PointType type,
                                size_t pointIndex,
                                const Wt::WPointF& point,
                                LinkCanvasOverlay& overlay,
                                CanvasView* parent);

  /**
   * Destructor
   */
  virtual ~BezierHandleWidget();

  /**
   * Returns the parent cast as a CanvasView object.
   */
  /** @{ */
  CanvasView* parent();
  const CanvasView* parent() const;
  /** @} */

  /**
   * Accesses the point type.
   */
  inline PointType pointType() const
  {
    return pntType;
  }

  /**
   * Changes the point type.  Does not force a redraw.
   */
  inline void setPointType(PointType type)
  {
    pntType = type;
    setZIndex((type == bezierSummit) ? DisplaySettings::bezierSummitZIndex
                                     : DisplaySettings::bezierPassThroughZIndex);
  }

  /**
   * Accesses the point index.
   */
  inline size_t pointIndex() const
  {
    return pntIndex;
  }

  /**
   * Sets the point index.
   */
  void setPointIndex(size_t index)
  {
    pntIndex = index;
    initInteractiveArea();
  }

  /**
   * Returns the location of this handle in model coordinates
   */
  Wt::WPointF point() const;

  /**
   * Creates the interactive area for user interaction.
   */
  void initInteractiveArea();

  /**
   * Returns the offset of the insertion point within the widget.
   */
  inline Wt::WPointF localOrigin() const
  {
    return Wt::WPointF(widgetSize / 2., widgetSize / 2.);
  }

  /**
   * Adjusts the widget position when the canvas expands to the
   * left or upwards.
   */
  void adjustWidgetPos();

protected:
  /**
   * Called by the Wt framework to redraw the widget.
   * Call Wt::WPaintedWidget::update() to redraw.
   */
  virtual void paintEvent(Wt::WPaintDevice *paintDevice);
};

/**
 * This class controls small widgets representing handles that represent
 * wire control points.  The handles are drawn directly onto the canvas view.
 *
 * This class is free standing and does not belong to the Wt tree.
 *
 * See important destructor notes.
 *
 * @see LinkCanvasOverlayHandle.
 */
class LinkCanvasOverlay
{
private:
    /**
     * A list of handle widgets.  Note that the widgets pointers belong to Wt.
     * this list is not sorted.
     */
    typedef std::list<BezierHandleWidget*> HandlesList;

public:
    typedef std::shared_ptr<LinkCanvasOverlay> Ptr;

private:
    MinskyDoc::ObjectId id = MinskyDoc::invalidId;  ///< ID of the wire this overlay controls.
    CanvasView*     view;       ///<  The canvas view on width items are drawn.
    LinkCanvasItem* linkItem;   ///<  Weak pointer to wire item, pointer belongs to Wt.
    HandlesList     handles;    ///<  List of "handles" that represent wire
                                ///   Bezier control points. See destructor notes.
    bool moving = false;        ///   true when the user is using the handles to edit.

public:
  /**
   * Constructor
   */
  LinkCanvasOverlay(LinkCanvasItem* linkItem, CanvasView* view);

  /**
   * Destructor.  Note that destroy() should be called first if and only if
   * this destructor is not itself called from a Wt object destructor.
   * leak memory, but will leave opened signal handlers pointing to a deleted
   * object.
   */
  virtual ~LinkCanvasOverlay();

  /**
   * returns the ID of the wire this object controls.
   */
  inline MinskyDoc::ObjectId getId() const
  {
    return id;
  }

  /**
   * Accesses the wire data.
   */
  /** @{ */
  Wire& getWire();
  const Wire& getWire() const;
  /** @} */

  /**
   * Initializes the interactive areas.
   */
  void initInteractive();

  /**
   * Returns the coordinates of a point.
   * @param pointIndex  index of the point.
   * @param pointType   type , bezierSummit or bezierPassThrough.
   * @return Coordinates of the point.
   */
  Wt::WPointF point(size_t pointIndex, BezierHandleWidget::PointType pointType);

  /**
   * Removes a control point from the wire.
   */
  void removePoint(BezierHandleWidget* widget);

  /**
   * Upgrades a handle type to LinkCanvasOverlayHandleWidget::bezierSummit,
   * thus creating a new control point.
   */
  void upgradeHandleType(BezierHandleWidget* widget);

  /**
   * Moves a wire control point.
   */
  void movePoint(BezierHandleWidget* widget, const Wt::WPointF& newPos);

  /**
   * Forces a redraw of the points handles
   */
  void update();

  /**
   * Adjusts the point handles positions when the canvas expands to the
   * left or upwards.
   */
  void adjustWidgetPos();

  /**
   * Sets/reset the in use marker.
   */
  inline void setInUseFlag(bool b)
  {
    moving = b;
  }

  /**
   * True while user is using the Bezier handles.
   * @return  the value of moving flag.
   */
  inline bool inUse() const
  {
    return moving;
  }
};

/**
 * Link canvas overlay shared pointer type.
 */
typedef std::shared_ptr<LinkCanvasOverlay> LinkCanvasOverlayPtr;

}}  // namespace minsky::gui

#endif /* LINKCANVASOVERLAY_H_ */
