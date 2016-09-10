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

#include "wireCreateOverlay.h"
#include "canvasView.h"
#include "GUI/portManager.h"
#include <ecolab_epilogue.h>
#include <Wt/WPainter>
#include <Wt/WRectArea>
#include <Wt/WPaintDevice>

namespace minsky { namespace gui {

WireCreateOverlay::WireCreateOverlay(CanvasView* _parent)
  : _base_class(_parent)
{
  setId("wire-create");
  setPositionScheme(Absolute);
  setZIndex(DisplaySettings::wireCreateZIndex);
  resize(parent()->width(), parent()->height());

  Wt::WRectArea* area = new Wt::WRectArea(0., 0., width().value(), height().value());
  addArea(area);

  area->setCursor(CrossCursor);

  area->mouseWentDown().connect(
      [this](const Wt::WMouseEvent& event)
      {
        mousePos = parent()->screenToModel(WPointF(event.widget().x, event.widget().y));
        fromPort = parent()->getDoc().getModel().closestOutPort(mousePos.x(), mousePos.y());
        isActive = (fromPort != invalidPort);
      });

  area->mouseMoved().connect(
      [this](const Wt::WMouseEvent& event)
      {
        if (active())
        {
          mousePos = parent()->screenToModel(WPointF(event.widget().x, event.widget().y));
          update();
        }
      });

  area->mouseWentUp().connect(
      [this](const Wt::WMouseEvent& event)
      {
        if (active())
        {
          mousePos = parent()->screenToModel(WPointF(event.widget().x, event.widget().y));
          toPort = parent()->getDoc().getModel().closestInPort(mousePos.x(), mousePos.y());
          isActive = false;
          update();
          parent()->addWire(fromPort, toPort);
        }

      });
}

WireCreateOverlay::~WireCreateOverlay()
{ }

CanvasView* WireCreateOverlay::parent()
{
  return dynamic_cast<CanvasView*>(_base_class::parent());
}

int WireCreateOverlay::from ()
{
  return fromPort;
}

int WireCreateOverlay::to ()
{
  return toPort;
}

void WireCreateOverlay::paintEvent(Wt::WPaintDevice* paintDevice)
{
  Wt::WPainter painter(paintDevice);

  if (active())
  {
    if (parent()->getDoc().getModel().ports.count(fromPort))
    {
      Port portA = parent()->getDoc().getModel().ports[fromPort];
      Wt::WPointF p0(portA.x(), portA.y());
      p0 = parent()->modelToScreen(p0);
      Wt::WPointF p1 = parent()->modelToScreen(mousePos);
      painter.drawLine(p0, p1);
    }
    else
    {
      isActive = false;
    }
  }
}

}} // namespace minsky::gui
