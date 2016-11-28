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

#include "sliderWidget.h"
#include "operationCanvasItem.h"
#include "canvasView.h"
#include "canvasPainter.h"
#include <ecolab_epilogue.h>
#include <Wt/WRectArea>
#include <math.h>
#include <algorithm>

namespace minsky { namespace gui {

SliderWidget::SliderBar::SliderBar(SliderWidget* parent)
  : _base_class(parent)
{
  setPositionScheme(Absolute);
  setOffsets(left, Left);
  setOffsets(top, Top);
  resize(width, height);
  setSelectable(false);
  setZIndex(DisplaySettings::sliderZIndex + 1);
}

SliderWidget::SliderBar::~SliderBar()
{ }

void SliderWidget::SliderBar::paintEvent(Wt::WPaintDevice *paintDevice)
{
  CanvasPainter painter(paintDevice);
  painter.draw(*this);
}

SliderWidget::SliderThumb::SliderThumb(SliderWidget* _parent)
  : _base_class(_parent)
{
  setPositionScheme(Absolute);
  setOffsets(left, Left);
  setOffsets(top, Top);
  resize(width, height);
  setZIndex(DisplaySettings::sliderZIndex + 2);
  setSelectable(false);

  Wt::WRectArea* area = new Wt::WRectArea(0, 0, width, height);
  addArea(area);

  area->mouseWentDown().connect(
      [this](const Wt::WMouseEvent& event)
      {
        if (event.button() & WMouseEvent::LeftButton)
        {
          moving = true;
          mouseX = event.screen().x;
          savedPos = offset(Left).value();
        }
      });

  area->mouseMoved().connect(
      [this](const Wt::WMouseEvent& event)
      {
        if (moving)
        {
          moving = !!(event.button() & WMouseEvent::LeftButton);
          double delta = event.screen().x - mouseX;
          setPos(savedPos + delta);
        }
      });

  area->mouseWentUp().connect(
      [this](const Wt::WMouseEvent& event)
      {
        if (moving)
        {
          moving = false;
          double delta = event.screen().x - mouseX;
          setPos(savedPos + delta);
        }
      });
}

SliderWidget::SliderThumb::~SliderThumb()
{ }

void SliderWidget::SliderThumb::setPos(const double& newPos)
{
  double pos = std::max(double(leftLimit), std::min(newPos, double(rightLimit)));
  setOffsets(pos, Left);
  parent()->getOp().sliderSetValue(posToValue(pos));
  parent()->label()->update();
}

void SliderWidget::SliderThumb::setRangeAndStepSize(const double& _stepSize,
                                                    const double& _min,
                                                    const double& _max)
{
  double value = posToValue(offset(Left).value());
  stepSize = _stepSize;
  min = _min;
  max = _max;
  setPos(valueToPos(value));
}

double SliderWidget::SliderThumb::value() const
{
  return posToValue(offset(Left).value());
}

double SliderWidget::SliderThumb::posToValue(const double& pos) const
{
  double value = min + (pos * ((max - min) / double(rightLimit - leftLimit)));
  value = std::max(min, std::min(value, max));
  value = int(value / stepSize) * stepSize;
  return value;
}

double SliderWidget::SliderThumb::valueToPos(const double& value) const
{
  double val = int((value  - min) / stepSize) * stepSize;
  double pos = val * (double(rightLimit - leftLimit) / (max - min));
  pos = std::max(double(leftLimit), std::min(pos, double(rightLimit)));
  return pos;
}

void SliderWidget::SliderThumb::paintEvent(Wt::WPaintDevice *paintDevice)
{
  CanvasPainter painter(paintDevice);
  painter.draw(*this);
}

SliderWidget::SliderLabel::SliderLabel(SliderWidget* _parent)
    : _base_class(_parent)
{
  setPositionScheme(Absolute);
  resize(width, height);
  setZIndex(DisplaySettings::sliderZIndex + 1);
  setSelectable(false);
}

SliderWidget::SliderLabel::~SliderLabel()
{ }

double SliderWidget::SliderLabel::value()
{
  return parent()->value();
}

void SliderWidget::SliderLabel::paintEvent(Wt::WPaintDevice *paintDevice)
{
  CanvasPainter painter(paintDevice);
  painter.draw(*this);
}

SliderWidget::SliderWidget(OperationCanvasItem& item, CanvasView* _parent)
  : _base_class(_parent),
    opItem(item)
{
  setPositionScheme(Absolute);
  setZIndex(DisplaySettings::sliderZIndex);
  resize(width, height);
  setSelectable(false);
  //  position is set by opItem

  new SliderBar(this);
  sliderThumb = new SliderThumb(this);
  sliderLabel = new SliderLabel(this);
}

SliderWidget::~SliderWidget()
{
  //  children are destroyed by Wt.
  //  tell the op this slider is attached to that the widget was destroyed.
  opItem.sliderDestroyed();
}

CanvasView* SliderWidget::parent()
{
  return dynamic_cast<CanvasView*>(_base_class::parent());
}

void SliderWidget::update()
{
  for (int i = 0; i < count(); ++i)
  {
    Wt::WPaintedWidget* child = dynamic_cast<Wt::WPaintedWidget*>(widget(i));
    if (child)
    {
      child->update();
    }
  }
}

double SliderWidget::value()
{
  return sliderThumb->value();
}

void SliderWidget::thumbSetValue(const double& value)
{
  Constant* op = dynamic_cast<Constant*>(getOp().getOp().get());

  if (op)
  {
    //TODO: formula for "relative value" ?
    op->value = value;
  }

}

}}  // namespace minsky::gui
