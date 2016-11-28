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

#include "canvasPainter.h"
#include "abstractCanvasItem.h"
#include "linkCanvasItem.h"
#include "plotCanvasItem.h"
#include "operationCanvasItem.h"
#include "godleyCanvasItem.h"
#include "linkCanvasItem.h"
#include "variableCanvasItem.h"
#include "groupCanvasItem.h"
#include "displaySettings.h"
#include "linkCanvasOverlay.h"
#include "plotDlg.h"
#include "canvasView.h"
#include <Wt/WPen>
#include <Wt/WColor>
#include <Wt/WPainterPath>
#include <Wt/WImage>
#include <cairo.h>
#include <boost/filesystem.hpp>
#include <sstream>
#include "cairoItems.h"
#include "godleyIcon.h"
#include "plotWidget.h"
#include "wtGeometry.h"
#include <cairo_base.h>
#include <algorithm>
#include <megabitz/xgl.h>
#include <megabitz/cairorenderer.h>
#include <ecolab_epilogue.h>

namespace minsky { namespace gui {

using namespace Wt;
using namespace boost::filesystem;
using namespace ecolab::cairo;

CanvasPainter::CanvasPainter()
{ }

CanvasPainter::CanvasPainter(WPaintDevice *device)
  : _base_class(device)
{ }

CanvasPainter::~CanvasPainter()
{ }

void CanvasPainter::drawLine(const PointsList& line)
{
  for (size_t i = 0; i < line.size() - 1; ++i)
  {
    _base_class::drawLine(line[i], line[i + 1]);
  }
}

void CanvasPainter::draw(OperationCanvasItem& item, gui::Polygon& extentsPolygon)
{
  OperationPtr op = item.getOp();
  CanvasView* view = item.parent();

  double h;
  double w;

  //  make a dry run to get operation extents.
  {
    Surface cairoSurface(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1));
    RenderOperation renderer(*op, cairoSurface.cairo());
    h = renderer.height() * 2;
    w = renderer.width() * 2;
  }

  WPointF pntCenter;
  if (op->type() == OperationType::constant)
  {
    //  additional space not reported by RenderOperation.
    //  2 + pen width + allow for some rounding errors.
    w += 8;
    h += 4;
    pntCenter = WPointF(w / 2, h / 2);
    w += 4;
  }
  else if (op->type() == OperationType::integrate)
  {
    IntOp* intOp = dynamic_cast<IntOp*>(op.get());

    double left = op->l - (w / 2); //-0.5 * (intOp->intVarOffset + w + 2 + intOp->r);
//    w = intOp->intVarOffset + w + 2 + intOp->r;
    h += 4;
    //TODO: there is something strange with width reported. Investigate
    pntCenter = WPointF((w / 2) + 2, h / 2);
  }

  WRectF extentsRect;

  if (op->type() != OperationType::integrate
      && op->type() != OperationType::constant)
  {
    //  triangular shape
    extentsPolygon.clear();
    extentsPolygon.push_back(WPointF(op->l - 2, -(op->h + 2)));
    extentsPolygon.push_back(WPointF(op->l - 2,  op->h + 2));
    extentsPolygon.push_back(WPointF(op->r + 2,  0));
    gui::rotate(extentsPolygon, op->rotation);
  }
  else
  {
    //  calculate extents in model space, with offset set to zero.
    extentsRect = WRectF(-pntCenter.x(), -pntCenter.y(), w, h);
    gui::rotate(extentsPolygon, extentsRect, op->rotation);
  }

  gui::extents(extentsRect, extentsPolygon);

  pntCenter = WPointF(-extentsRect.left(), -extentsRect.top());

  //  set local origin in model space and center the extents polygon.
  item.setLocalOrigin(pntCenter.x(), pntCenter.y());


  //  convert to screen space
  extentsRect = view->modelToScreen(extentsRect);
  item.resize(extentsRect.width(), extentsRect.height());;

  Surface cairoSurface(cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                       extentsRect.width(), extentsRect.height()));
  cairo_t* ctx = cairoSurface.cairo();

  cairo_scale(ctx, view->zoom(), view->zoom());
  cairo_translate(ctx, item.getLocalOrigin().x(), item.getLocalOrigin().y());

  RenderOperation renderer(*op, ctx);
  renderer.draw();

  item.setImage(new OperationCanvasItem::Image(cairoSurface.surface()));
  drawImage(0., 0., item.painterImage());

  //  create extents polygon for interactive area,
  //  must be in model coordinates
  gui::translate(extentsPolygon, WPointF(op->x(), op->y()));
}

void CanvasPainter::draw(VariableCanvasItem& item, gui::Polygon& extentsPolygon)
{
  VariablePtr var = item.getVar();
  CanvasView* view = item.parent();

  WRectF extentsRect;

  //  make a dry run to get operation extents.
  {
    Surface cairoSurface(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1));
    RenderVariable renderer(*var, cairoSurface.cairo());

    //  add any additional space not reported by RenderVariable.

    extentsRect = WRectF(-(renderer.width() + 2), -renderer.height(), \
                         (2 * renderer.width()) + 6, (2 * renderer.height()) + 2);
  }

  gui::rotate(extentsPolygon, extentsRect, var->rotation);
  gui::extents(extentsRect, extentsPolygon);

  WPointF pntCenter(-extentsRect.left(), -extentsRect.top());

  item.setLocalOrigin(pntCenter.x(), pntCenter.y());

  extentsRect = view->modelToScreen(extentsRect);
  item.resize(extentsRect.width(), extentsRect.height());

  Surface cairoSurface(cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                       extentsRect.width(), extentsRect.height()));
  cairo_t* ctx = cairoSurface.cairo();

  cairo_scale(ctx, view->zoom(), view->zoom());
  cairo_translate(ctx, item.getLocalOrigin().x(), item.getLocalOrigin().y());

  RenderVariable renderer(*var, ctx);
  renderer.draw();

  item.setImage(new VariableCanvasItem::Image(cairoSurface.surface()));
  drawImage(0., 0., item.painterImage());

  //  create extents polygon for interactive area
  gui::translate(extentsPolygon, WPointF(var->x(), var->y()));
}

void CanvasPainter::draw(PlotCanvasItem& item, gui::Polygon& extentsPolygon)
{
  minsky::PlotWidget& plotWidget =  item.getPlot();

  CanvasView* view = item.parent();

  WPointF savedPos(plotWidget.x(), plotWidget.y());
  item.setLocalOrigin(plotWidget.width / 2, plotWidget.height / 2);

  plotWidget.moveTo(0, 0);
  plotWidget.zoomFactor = view->zoom();

  double w = plotWidget.width * view->zoom();
  double h = plotWidget.height * view->zoom();

  Surface cairoSurface(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h));

  cairo_translate(cairoSurface.cairo(), item.getLocalOrigin().x(), item.getLocalOrigin().y());

  plotWidget.draw(cairoSurface);

  item.resize(w, h);

  item.setImage(new OperationCanvasItem::Image(cairoSurface.surface()));
  drawImage(0., 0., item.painterImage());

  plotWidget.zoomFactor = 1;
  plotWidget.moveTo(savedPos.x(), savedPos.y());
  plotWidget.updatePortLocation();

  WRectF extentsRect(-plotWidget.width / 2, -plotWidget.height / 2, plotWidget.width, plotWidget.height);
  gui::rectToPolygon(extentsPolygon, extentsRect);
  gui::translate(extentsPolygon, WPointF(plotWidget.x(), plotWidget.y()));
}

void CanvasPainter::draw(LargePlotWidget& plot, minsky::PlotWidget& plotWidget)
{
  double w = plot.width().value();
  double h = plot.height().value();

  Surface cairoSurface(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h));
  static_cast<ecolab::Plot&>(plotWidget).draw(cairoSurface.cairo(),w,h);

  plot.setImage(std::make_shared<TempCairoImageFile>(cairoSurface.surface()));
  drawImage(0, 0, *(plot.getImage()->getImage()));
}


void CanvasPainter::draw(GodleyCanvasItem& item, gui::Polygon& extentsPolygon)
{
  GodleyIcon& icon = item.getIcon();
  CanvasView* view = item.parent();


  WPointF savedPos(icon.x(), icon.y());
  item.setLocalOrigin(icon.width() / 2, icon.height() / 2);

  icon.moveTo(0, 0);
  icon.zoomFactor = view->zoom();
  icon.update();

  item.resize(icon.width(), icon.height());

  Surface cairoSurface(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, icon.width(), icon.height()));
//  CairoRenderer renderer(cairoSurface.surface());

////  cairo_translate(renderer.cairo(), icon.leftMargin(), 0);
////  cairo_scale(renderer.cairo(), (icon.width() - icon.leftMargin()) / icon.width(),
////                                (icon.height() - icon.bottomMargin() / icon.height()));
//  xgl bank(renderer);
//  bank.load("app-res/bank.xgl");
//  bank.render();

  cairo_translate(cairoSurface.cairo(), item.getLocalOrigin().x(), item.getLocalOrigin().y());
  icon.wtDraw(cairoSurface);

  item.setImage(new OperationCanvasItem::Image(cairoSurface.surface()));
  drawImage(0., 0., item.painterImage());

  icon.zoomFactor = 1;
  icon.moveTo(savedPos.x(), savedPos.y());
  icon.updatePortLocation();

  WRectF extentsRect(-icon.width() / 2, -icon.height() / 2, icon.width(), icon.height());
  gui::rectToPolygon(extentsPolygon, extentsRect);
  gui::translate(extentsPolygon, WPointF(icon.x(), icon.y()));
}

void CanvasPainter::draw(LinkCanvasItem& item, PointsList& lineSegments)
{
  minsky::Wire& wire =  item.getWire();
  CanvasView* view = item.parent();

  PointsList points;
  item.getPoints(points);
  if (points.size() < 2)
  {
    return;
  }

  // create curve in model space.
  gui::quadBezierLine(lineSegments, bezierResolution, points);

  //  get extents
  double top, left, right, bottom;
  gui::extents(left, top, right, bottom, points);

  //  add some room for the arrow head.
  left -= wireMargin;
  top  -= wireMargin;
  right += wireMargin;
  bottom += wireMargin;

  //  compute widget size and local insertion point,
  //  resize and place.
  WPointF origin = points[0] - WPointF(left, top);
  item.setLocalOrigin(origin.x(), origin.y());
  item.moveTo(points[0].x(), points[0].y());
  item.resize((right - left) * view->zoom(), (bottom -top) * view->zoom());

  //  transform to widget space and draw.
  scale(view->zoom(), view->zoom());
  translate(-1 * WPointF(left, top));
  setPen(DisplaySettings::wireColor);
  drawLine(lineSegments);
}

void CanvasPainter::draw(GroupCanvasItem& item, gui::Polygon& extentsPolygon)
{
  GroupIcon& icon = item.getIcon();
  CanvasView* view = item.parent();

  double w = icon.width * view->zoom();
  double h = icon.height * view->zoom();

  WPointF savedPos(icon.x(), icon.y());
  item.setLocalOrigin(icon.width / 2, icon.height / 2);

  icon.moveTo(0, 0);
  icon.zoomFactor = view->zoom();
  //icon.update();

  item.resize(w, h);

  Surface cairoSurface(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h));

  cairo_translate(cairoSurface.cairo(), w / 2, h / 2);
  icon.wtDraw(cairoSurface);

//  WPainter::Image bank("/resources/bank.png", "resources/bank.png");
//
//  WRectF srcRect(0, 0, bank.width(), bank.height());
//  WRectF dstRect(icon.leftMargin(), 0, icon.width() -icon.leftMargin(), icon.height() - icon.bottomMargin());
//
//  drawImage(dstRect, bank, srcRect);

  item.setImage(new OperationCanvasItem::Image(cairoSurface.surface()));
  drawImage(0., 0., item.painterImage());

  icon.zoomFactor = 1;
  icon.moveTo(savedPos.x(), savedPos.y());
  icon.updatePortLocation();

  WRectF extentsRect(-icon.width / 2, -icon.height / 2, icon.width, icon.height);
  gui::rectToPolygon(extentsPolygon, extentsRect);
  gui::translate(extentsPolygon, WPointF(icon.x(), icon.y()));
}

void CanvasPainter::draw(BezierHandleWidget& widget)
{
  //  draws a filled circle with a border.  Color depends on the type.
  //  This kind of object is not affected by zooming

  if (widget.pointType() == BezierHandleWidget::bezierSummit)
  {
    setBrush(DisplaySettings::bezierSummitColor);
  }
  else
  {
    setBrush(DisplaySettings::bezierPassThroughColor);
  }

  drawEllipse(0, 0, widget.width().value(), widget.height().value());
}

void CanvasPainter::draw(SliderWidget::SliderThumb& widget)
{
  //  slider thumb, a beveled rectangle  with a blue tick mark.
  setBrush(DisplaySettings::sliderThumbColor);

  drawRect(0, 0, widget.width, widget.height);

  setPen(DisplaySettings::bevelOutNorthColor);

  _base_class::drawLine(WPointF(0, 0),
                        WPointF(widget.width, 0));

  setPen(DisplaySettings::bevelOutWestColor);

  _base_class::drawLine(WPointF(0, 1),
                        WPointF(0, widget.height));

  setPen(DisplaySettings::bevelOutWestColor);

  _base_class::drawLine(WPointF(widget.width, 1),
                        WPointF(widget.width, widget.height));

  setPen(DisplaySettings::bevelOutSouthColor);

  _base_class::drawLine(WPointF(1, widget.height),
                        WPointF(widget.width, widget.height));

  setPen(DisplaySettings::sliderTickMarkColor);

  _base_class::drawLine(WPointF(widget.width / 2., 0),
                        WPointF(widget.width / 2., widget.height));
}

void CanvasPainter::draw(SliderWidget::SliderBar& widget)
{
  //  slider bar, a beveled rectangle
  setBrush(DisplaySettings::sliderTrackColor);

  drawRect(0, 0, widget.width, widget.height);

  setPen(DisplaySettings::bevelInNorthColor);

  _base_class::drawLine(WPointF(0, 0),
                        WPointF(widget.width, 0));

  setPen(DisplaySettings::bevelInWestColor);

  _base_class::drawLine(WPointF(0, 1),
                        WPointF(0, widget.height));

  setPen(DisplaySettings::bevelInEastColor);

  _base_class::drawLine(WPointF(widget.width, 1),
                        WPointF(widget.width, widget.height));

  setPen(DisplaySettings::bevelInSouthColor);

  _base_class::drawLine(WPointF(1, widget.height),
                        WPointF(widget.width, widget.height));
}

void CanvasPainter::draw(SliderWidget::SliderLabel& widget)
{
  WFont font;
  font.setFamily(DisplaySettings::sliderLabelFontFamily);
  font.setWeight(DisplaySettings::sliderLabelFontWeight);
  font.setSize(DisplaySettings::sliderLabelFontSize);

  setFont(font);

  std::stringstream str;
  str.setf(ios_base::showpoint | ios_base::fixed);
  str.precision(widget.precision);
  str << widget.value();

  drawText(0, 0, widget.width, widget.height, AlignCenter | AlignMiddle, str.str());
}


}}  // namespace minsky::gui
