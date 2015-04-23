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

#include "linkCanvasOverlay.h"
#include "canvasView.h"
#include "linkCanvasItem.h"
#include "canvasView.h"
#include "canvasPainter.h"
#include <ecolab_epilogue.h>
#include <Wt/WCircleArea>
#include <Wt/WEvent>
#include <algorithm>

namespace minsky { namespace gui {

using namespace Wt;

Wt::WPointF BezierHandleWidget::savedMousePos;
Wt::WPointF BezierHandleWidget::savedPos;

BezierHandleWidget::BezierHandleWidget (PointType type,
                                        size_t _pointIndex,
                                        const Wt::WPointF& point,
                                        LinkCanvasOverlay& _overlay,
                                        CanvasView* parent)
    : _base_class(parent),
      overlay(_overlay),
      pntIndex(_pointIndex),
      pntType(type)
{
  setPositionScheme(Absolute);
  resize(widgetSize, widgetSize);
  setZIndex((type == bezierSummit) ? DisplaySettings::bezierSummitZIndex
                                   : DisplaySettings::bezierPassThroughZIndex);
}

BezierHandleWidget::~BezierHandleWidget()
{ }


CanvasView* BezierHandleWidget::parent()
{
  //  dynamioc_cast OK because other type for parent is not valid.
  return dynamic_cast<CanvasView*>(_base_class::parent());
}

const CanvasView* BezierHandleWidget::parent() const
{
  return dynamic_cast<const CanvasView*>(_base_class::parent());
}

Wt::WPointF BezierHandleWidget::point() const
{
  return overlay.point(pntIndex, pntType);
}

void BezierHandleWidget::initInteractiveArea()
{
  if (area(0))
  {
    return;
  }

  WCircleArea* area = new WCircleArea(width().value() / 2,
                                      height().value() / 2,
                                      widgetSize / 2);
  addArea(area);
  area->setCursor(CrossCursor);

  area->doubleClicked().connect(
      [this](const Wt::WMouseEvent& event) -> void
      {
        if (pointType() == bezierSummit)
        {
          overlay.removePoint(this);
        }
      });

  area->mouseWentDown().connect(
      [this](const WMouseEvent& event) -> void
      {
        if (event.button() & WMouseEvent::LeftButton)
        {
          if (pointType() != bezierSummit)
          {
            overlay.upgradeHandleType(this);
          }
          savedMousePos = parent()->deviceToModel(WPointF(event.screen().x, event.screen().y));
          savedPos = point();
          moving = true;
          overlay.setInUseFlag(true);
        }
      });

  area->mouseMoved().connect(
      [this](const WMouseEvent& event) -> void
      {
        if (moving && (event.button() & WMouseEvent::LeftButton))
        {
          WPointF newPos = parent()->deviceToModel(WPointF(event.screen().x, event.screen().y));
          newPos = savedPos + (newPos - savedMousePos);
          adjustWidgetPos();
          overlay.movePoint(this, newPos);
        }
      });

  area->mouseWentUp().connect(
      [this](const WMouseEvent& event) -> void
      {
        if (moving && (event.button() & WMouseEvent::LeftButton))
        {
          moving = false;
          overlay.setInUseFlag(false);
          WPointF newPos = parent()->deviceToModel(WPointF(event.screen().x, event.screen().y));
          newPos = savedPos + (newPos - savedMousePos);
          adjustWidgetPos();
          overlay.movePoint(this, newPos );
        }
      });

}

void BezierHandleWidget::adjustWidgetPos()
{
  WPointF newPos = parent()->modelToScreen(point()) - localOrigin();
  setOffsets(newPos.x(), Left);
  setOffsets(newPos.y(), Top);
}

void BezierHandleWidget::paintEvent(Wt::WPaintDevice *paintDevice)
{
  CanvasPainter painter(paintDevice);
  painter.draw(*this);

  initInteractiveArea();
  adjustWidgetPos();
}

LinkCanvasOverlay::LinkCanvasOverlay(LinkCanvasItem* _linkItem, CanvasView* _view)
  : id(_linkItem->getId()),
    view(_view),
    linkItem(_linkItem)
{
  PointsList pointsList;
  linkItem->getPoints(pointsList);

  for (size_t index = 0; index < pointsList.size() - 1; ++index)
  {
    if (index > 0)
    {
      handles.push_back(new BezierHandleWidget \
                              (BezierHandleWidget::bezierSummit,
                               index, pointsList[index], *this, view));
    }

    WPointF newPoint = gui::midPoint(pointsList[index], pointsList[index + 1]);

    handles.push_back(new BezierHandleWidget \
                            (BezierHandleWidget::bezierPassThrough,
                             index, newPoint, *this, view));
  }
}

LinkCanvasOverlay::~LinkCanvasOverlay()
{
  //  remove handles from canvas
  //
  //  Note that these pointers belong to CanvasView,
  //  This works because this object itself also belongs
  //  to canvasView.
  std::for_each(handles.begin(), handles.end(),
      [](BezierHandleWidget* widget)
      {
        delete widget;
      });
}

Wire& LinkCanvasOverlay::getWire()
{
  return linkItem->getWire();
}

const Wire& LinkCanvasOverlay::getWire() const
{
  return linkItem->getWire();
}

Wt::WPointF LinkCanvasOverlay::point(size_t pointIndex, BezierHandleWidget::PointType pointType)
{
  PointsList pointsList;
  linkItem->getPoints(pointsList);

  WPointF pnt = pointsList[pointIndex];
  if (pointIndex < (pointsList.size() - 1)
      && pointType == BezierHandleWidget::bezierPassThrough)
  {
    return midPoint(pnt, pointsList[pointIndex + 1]);
  }
  return pnt;
}

void LinkCanvasOverlay::removePoint(BezierHandleWidget* widget)
{
  size_t index  = widget->pointIndex();
  linkItem->removePoint(index);

  //  remove the point handle and one of the virtual intermediary points
  //  it generated, the rule is that this point will have the same index.
  //  Fix higher indices to stay in sync while we're scanning the array.
  handles.remove_if(
      [index](BezierHandleWidget* handle) -> bool
      {
        if (handle->pointIndex() == index)
        {
          delete handle;
          return true;
        }
        else if (handle->pointIndex() > index)
        {
          handle->setPointIndex(handle->pointIndex() - 1);
        }
        return false;
      });

  update();
}

void LinkCanvasOverlay::upgradeHandleType(BezierHandleWidget* widget)
{
  if (widget->pointType() == BezierHandleWidget::bezierPassThrough)
  {
    PointsList wirePoints;
    linkItem->getPoints(wirePoints);

    WPointF point = widget->point();

    widget->setPointType(BezierHandleWidget::bezierSummit);

    //  fix indices above newly inserted point.
    size_t index = widget->pointIndex() + 1;
    std::for_each(handles.begin(), handles.end(),
        [this, index](BezierHandleWidget* handle) -> void
        {
          if (handle->pointIndex() >= index)
          {
            handle->setPointIndex(handle->pointIndex() + 1);
          }
        });

    //  Add new midpoint between this point and the preceding one.
    WPointF newPoint = gui::midPoint(wirePoints[index - 1], point);

    handles.push_back(new BezierHandleWidget(BezierHandleWidget::bezierPassThrough,
                                                        index - 1, newPoint, *this, view));
    //  increment point index for newly created control point.
    widget->setPointIndex(index);

    //  add new mid point between this point and the next
    newPoint = gui::midPoint(point, wirePoints[index]);

    handles.push_back(new BezierHandleWidget(BezierHandleWidget::bezierPassThrough,
                                                        index, newPoint, *this, view));

    //  propagate changes to model this will cause a redraw
    linkItem->addPoint(index, point);
  }
}

void LinkCanvasOverlay::movePoint(BezierHandleWidget* widget, const Wt::WPointF& newPos)
{
  linkItem->setPoint(widget->pointIndex(), newPos);
  update();
}

void LinkCanvasOverlay::update()
{
  std::for_each(handles.begin(), handles.end(),
    [](BezierHandleWidget* handle) -> void
    {
      handle->update();
    });
}

void LinkCanvasOverlay::adjustWidgetPos()
{
  std::for_each(handles.begin(), handles.end(),
      [](BezierHandleWidget* handle) -> void
      {
        handle->adjustWidgetPos();
      });
}

}}  // namespace minsky::gui
