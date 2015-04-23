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

#ifndef OPERATIONCANVASITEM_H_
#define OPERATIONCANVASITEM_H_

#include "abstractCanvasItem.h"
#include "operationType.h"
#include "wtGeometry.h"
#include <Wt/WEvent>
#include <Wt/WPainter>
#include <classdesc.h>

namespace minsky { namespace gui {

  class SliderWidget;

/**
 * A canvas item representing one of the basic operations.
 *
 * An operation is basically represented as a triangle with hooks for links
 * and a symbol.
 */
class OperationCanvasItem: public AbstractCanvasItem
{
public:
  typedef AbstractCanvasItem _base_class;

private:
  OperationPtr operation;           ///<  The model object this widget represents.
  SliderWidget* sliderWidget = NULL;///<  Slider widget for constant ops.

public:
  /**
   * Constructor.
   */
  OperationCanvasItem(MinskyDoc::ObjectId id, OperationPtr op, CanvasView* parent);

  /**
   * Destructor
   */
  virtual ~OperationCanvasItem();

  /**
   *  Moves the widget to a new location in model space.
   *  The widget is moved to a new position.  The coordinates are referenced
   *  to the top left corner of the canvas.
   *  @param x the abscissa of the new position in model space.
   *  @param y the ordinate of the new position in model space.
   */
  virtual void moveTo(const double& x, const double& y);

  /**
   *  Returns a the type of the operation
   *  @returns one of the enum values defined by enum OperationType::Type
   */

  inline minsky::OperationType::Type type() const
  {
    return operation->type();
  }

  /**
   * Accesses the underlying model operation.
   */
  inline OperationPtr getOp() const
  {
    return operation;
  }

  /**
   *  returns the position of the object in model coordinates.
   */
  virtual void getModelPos(double& x, double& y) const;

  /**
   *  Opens the properties dialog so the user can edit object.
   *  @returns true if object properties were modified.
   */
  virtual bool editProperties();

  /**
   * Returns a list of ports that provide connection to this item
   */
  virtual void getPortsList(PortsList& ports) const;

  /**
   * Called by attached slider to indicate user moved slider thumb.
   */
  void sliderSetValue(const double& value);

  /**
   * This tells us Wt has independently destroyed the slider.
   */
  inline void sliderDestroyed()
  {
    sliderWidget = NULL;
  }

protected:
  /**
   * Sets the position of the slider widget, if visible.
   */
  void setSliderWidgetPos();

  /**
   * Draw the widget using the painter.
   * @param painter Painter to use for drawing.
   * @param outline On return, contains the shape outline of the widget in model space.
   */
  virtual void draw(CanvasPainter& painter, Polygon& outline);

  /**
   * This function is called by the framework when the widget needs redrawn.
   */
  virtual void  paintEvent (Wt::WPaintDevice *paintDevice);
};

}}  // namespace minsky::gui

#endif /* OPERATIONCANVASITEM_H_ */
