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

#ifndef LINKCANVASITEM_H_
#define LINKCANVASITEM_H_

#include "abstractCanvasItem.h"
#include "wtGeometry.h"
#include "GUI/wire.h"
#undef None
#include <Wt/WAbstractArea>
#include <memory>

namespace minsky { namespace gui {

class CanvasView;

/**
*  A widget that represents a link between two other canvas items.
 */
class LinkCanvasItem: public AbstractCanvasItem
{
private:
  typedef std::shared_ptr<Wt::WAbstractArea> AreaPtr;

private:
  /**
   *  link to the wire in the model.
   */
  minsky::Wire& wire;

  /**
   * The actual line drawn in model coordinates.
   */
  PointsList lineSegments;

  /**
   * The event area to trigger the display of editing handles.
   */
  AreaPtr area;

public:
  /**
   * Constructor.
   */
  LinkCanvasItem(MinskyDoc::ObjectId _id, minsky::Wire& _wire, CanvasView* parent);

  /**
   * Destructor.
   */
  virtual ~LinkCanvasItem();

  /**
   * Create the user interaction zones.  Differs from other items because
   * the interactive area belongs to a global WireEventArea object.
   */
  void initInteractive();

  /**
   * Accesses the associated model wire.
   */
  inline minsky::Wire& getWire()
  {
    return wire;
  }

  /**
   * Accesses the points for this wire.
   */
  void getPoints(PointsList& points);

  /**
   * Sets the points for this wire.
   */
  void setPoints(const PointsList& points);

  /**
   * Returns a list of ports that provide connection to this item
   */
  virtual void getPortsList(PortsList& ports) const;

  /**
   *  Removes the control point at pointIndex.
   */
  void removePoint(size_t pointIndex);

  /**
   *  Inserts a control point at pointIndex.
   */
  void addPoint(size_t insertAt, const Wt::WPointF& pnt);

  /**
   *  Sets the point at poinIndex.
   */
  void setPoint(size_t pointIndex, const Wt::WPointF& pnt);

protected:
  /**
   * Draw the widget using the painter.
   * @param painter Painter to use for drawing.
   * @param outline On return, contains the shape outline of the widget in model space.
   */
  virtual void draw(CanvasPainter& painter, Polygon& outline);

  /**
   * This function is called by the framework when the widget needs redraw.
   */
  virtual void paintEvent (WPaintDevice *paintDevice);

private:
  /**
   *  returns the position of the object in model coordinates.  Non-applicable
   *  to wires.
   *  @param x  Always 0.
   *  @param y  Always 0.
   */
  virtual void getModelPos(double& x, double& y) const;

  /**
   *  Opens the properties dialog so the user can edit object.
   *  @returns true if object properties were modified.
   */
  virtual bool editProperties();
};

/**
 * Wires share this widget for their events. This fixes ovelapping
 * issues.
 */
class WireEventArea: public Wt::WPaintedWidget
{
private:
  typedef Wt::WPaintedWidget _base_class;

public:
  /**
   * Constructor.
   * @param parent  parent widget, must be a CanvasView.
   */
  WireEventArea(CanvasView* parent);

  /**
   * Destructor
   */
  virtual ~WireEventArea();

protected:
  /**
   * Needed to instantiate object, but does nothing, as there
   * is nothing to draw.
   * @param paintDevice
   */
  virtual void paintEvent(Wt::WPaintDevice*);
};

typedef std::shared_ptr<WireEventArea> WireEventAreaPtr;

}} //  namespace minsky::gui

#endif /* LINKCANVASITEM_H_ */
