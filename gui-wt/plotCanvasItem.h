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

#ifndef PLOTCANVASITEM_H
#define PLOTCANVASITEM_H

#include "guiDefs.h"
#include "abstractCanvasItem.h"
#include "GUI/plotWidget.h"

namespace minsky { namespace gui {

class PlotCanvasItem : public AbstractCanvasItem
{
private:
  ::minsky::PlotWidget& plot;
  int name;

public:
  /**
   *  Constructor
   */
  PlotCanvasItem(int _name,
                 ::minsky::PlotWidget& _plot,
                 CanvasView* parent);

  /**
   *  Destructor
   */
  virtual ~PlotCanvasItem();

  //
  //  Accessors.
  //

  const int getName() const
  {
    return name;
  }

  /**
   *  returns the position of the object in model coordinates.
   */
  virtual void getModelPos(double& x, double& y) const;

  /**
   * Returns the underlying model plot object.
   */
  inline ::minsky::PlotWidget& getPlot()
  {
    return plot;
  }

  /**
   * Returns the bounding box port orientation for port \a idx.
   */
  static double boundingBoxPortOrientation(size_t idx);

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
   * Resets the plot by destroying the bqckbuffer.
   */
  void reset();

protected:
  /**
   * Draw the widget using the painter.
   * @param painter Painter to use for drawing.
   * @param outline On return, contains the shape outline of the widget in model space.
   */
  virtual void draw(CanvasPainter& painter, Polygon& outline);
};

}}  // namespace minsky::gui

#endif // PLOTCANVASITEM_H
