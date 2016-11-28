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

#include "rectSelectionOverlay.h"
#include "displaySettings.h"
#include "canvasView.h"
#include <ecolab_epilogue.h>
#include <Wt/WPainter>
#include <Wt/WRectF>
#include <Wt/WRectArea>

namespace minsky { namespace gui {

RectSelectionOverlay::RectSelectionOverlay(CanvasView* _parent)
  : _base_class(_parent)
{
  setId("rect-sel");
  setPositionScheme(Absolute);
  setZIndex(DisplaySettings::selectionOverlayZIndex);
  resize(parent()->width(), parent()->height());

  Wt::WRectArea* area = new Wt::WRectArea(0., 0., width().value(), height().value());
  addArea(area);

  area->setCursor(CrossCursor);

  area->mouseWentDown().connect(
      [this](Wt::WMouseEvent& event)
      {
        pointA = parent()->screenToModel(Wt::WPointF(event.widget().x, event.widget().y));
        isActive = true;
      });

  area->mouseMoved().connect(
      [this](Wt::WMouseEvent& event)
      {
        if (active())
        {
          pointB = parent()->screenToModel(Wt::WPointF(event.widget().x, event.widget().y));
          update();
        }
      });

  area->mouseWentUp().connect(
      [this](Wt::WMouseEvent& event)
      {
        if (active())
        {
          pointB = parent()->screenToModel(Wt::WPointF(event.widget().x, event.widget().y));
          isActive = false;
          parent()->groupRect(rectangle());
          update();
        }
      });
}

RectSelectionOverlay::~RectSelectionOverlay()
{ }

CanvasView* RectSelectionOverlay::parent()
{
  return dynamic_cast<CanvasView*>(_base_class::parent());
}

Wt::WRectF RectSelectionOverlay::rectangle() const
{
  Wt::WPointF delta = pointB - pointA;
  Wt::WRectF rect(pointA.x(), pointA.y(), delta.x(), delta.y());
  return rect.normalized();
}

void RectSelectionOverlay::paintEvent(Wt::WPaintDevice* paintDevice)
{
  Wt::WPainter painter(paintDevice);

  if (active())
  {
    painter.setBrush(Wt::WColor(0, 0, 0, 0));
    painter.drawRect(parent()->modelToScreen(rectangle()));
  }
}

}}  // namespace minsky::gui
