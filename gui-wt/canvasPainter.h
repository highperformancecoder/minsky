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

#ifndef CANVASPAINTER_H_
#define CANVASPAINTER_H_

#include "guiDefs.h"
#include "minskyApp.h"
#include "wtGeometry.h"
#include "sliderWidget.h"
#include <Wt/WPainter>

namespace minsky { namespace gui {

class PlotCanvasItem;
class AbstractCanvasItem;
class OperationCanvasItem;
class GodleyCanvasItem;
class LinkCanvasItem;
class VariableCanvasItem;
class GroupCanvasItem;
class BezierHandleWidget;
class LargePlotWidget;

/**
 *  The Minsky artist.  Draws all items on the canvas.
 */
class CanvasPainter : public Wt::WPainter
{
private:
  typedef Wt::WPainter _base_class;

  /**
   * Bezier line detail level, or the number of line segments
   * per control points.
   */
  static const int bezierResolution = 20;

  /**
   * Margin to add to wire widget area, to allow for drawing
   * the arrow head and ontrol points.
   */
  const double wireMargin = 10.;

public:
  /**
   * Constructor.
   */
  /** @{ */
  CanvasPainter();
  CanvasPainter(Wt::WPaintDevice *device);
  /** @} */

  /**
   * Destructor.
   * @note WPainter's destructor is not virtual.
   */
  ~CanvasPainter();

  inline const DisplaySettings& displaySettings() const
  {
    return MinskyApp::displaySettings();
  }

  /**
   *  Overrides Wt::WPainter::begin() to set our options.
   */
  inline bool begin(Wt::WPaintDevice *device)
  {
    return WPainter::begin(device);
  }

  /**
   * Draws a line made of multiple segments using the current pen.
   */
  void drawLine(const PointsList& line);

  /**
   * Drawing function for operations.
   *
   * @param item Operation object to render.
   * @param extentsPolygon On return, contains a polygon that
   *        matches the object's image extents.
   */
  void draw(OperationCanvasItem& item, gui::Polygon& extentsPolygon);

  /**
   * Draws a variable.
   * @param item Variable to render.
   * @param extentsPolygon On return, contains a polygon that
   *        matches the object's image extents.
   */
  void draw(VariableCanvasItem& item, gui::Polygon& extentsPolygon);

  /**
   * Draw a plot.
   */
  /** @{ */
  void draw(PlotCanvasItem& item, gui::Polygon& extentsPolygon);
  void draw(LargePlotWidget& plot, minsky::PlotWidget& plotWidget);
  /** @} */

  /**
   * Draws a godley icon.
   */
  void draw(GodleyCanvasItem& item, gui::Polygon& extentsPolygon);

  /**
   * Draws a wire.
   * @param item line to render.
   * @param lineSegments The bezier curve actually drawn.
   */
  void draw(LinkCanvasItem& item, PointsList& lineSegments);

  /**
   * Draws a group icon.
   */
  void draw(GroupCanvasItem& item, gui::Polygon& extentsPolygon);

  /**
   * Draws a bezier control point handle
   */
  void draw(BezierHandleWidget& widget);

  /**
   * Draws a slider thumb
   */
  void draw(SliderWidget::SliderThumb& widget);

  /**
   * Draws a slider bar.
   */
  void draw(SliderWidget::SliderBar& widget);

  /**
   * Draws a slider label.
   */
  void draw(SliderWidget::SliderLabel& widget);
};

}}  // namespace minsky::gui

#endif /* CANVASPAINTER_H_ */
