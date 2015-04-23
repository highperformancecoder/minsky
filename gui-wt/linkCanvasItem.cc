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

#include "linkCanvasItem.h"
#include "canvasPainter.h"
#include "canvasView.h"
#include "linkCanvasOverlay.h"
#include <ecolab_epilogue.h>
#include <Wt/WPaintDevice>
#include <Wt/WPolygonArea>

namespace minsky { namespace gui {

using namespace Wt;

LinkCanvasItem::LinkCanvasItem(MinskyDoc::ObjectId _id, minsky::Wire& _wire, CanvasView* parent)
  : AbstractCanvasItem(_id, parent),
    wire(_wire)
{
  setPositionScheme(Absolute);
  adjustWidgetPos();
  resize(10, 10);                 //  must give a (dummy) size for Wt.

  std::stringstream str;
  str << "wire" << _id;
  setId(str.str());
  setZIndex(DisplaySettings::wireZIndex);
}

LinkCanvasItem::~LinkCanvasItem()
{ }

void LinkCanvasItem::initInteractive()
{
  Polygon poly = shapeOutline;

  gui::scale(poly, parent()->zoom());
  gui::translate(poly, parent()->modelToScreen(WPointF(0, 0)));

  area = std::make_shared<WPolygonArea>(poly);
  parent()->getWireEventArea()->addArea(area.get());

  area->mouseWentOver().connect(
      [this](const WMouseEvent& event) -> void
      {
        //  shift allows the user to mask this event, also
        //  mask event when the user is using the Bezier handles
        //  as this can get quite aggravating.
        if (!event.shiftKey()
            && !(parent()->getLinkOverlay() && parent()->getLinkOverlay()->inUse()))
        {
          parent()->setLinkOverlay(this);
        }
      });
}

void LinkCanvasItem::getModelPos(double& x, double& y) const
{
  ecolab::array<float> coords = const_cast<LinkCanvasItem*>(this)->getWire().coords();

  x = coords[0];
  y = coords[1];
}

void LinkCanvasItem::getPoints(PointsList& points)
{
  ecolab::array<float> coords = getWire().coords();
  points.clear();

  for (size_t i = 0; i < coords.size(); i += 2)
  {
    WPointF p(coords[i], coords[i + 1]);
    points.push_back(p);
  }
}

void LinkCanvasItem::setPoints(const PointsList& points)
{
  ecolab::array<float> coords(points.size() * 2);

  int i = 0;
  std::for_each(points.begin(), points.end(),
      [&coords, &i](const WPointF& pnt)
      {
        coords[i++] = float(pnt.x());
        coords[i++] = float(pnt.y());
      });

  wire.coords(coords);
}

bool LinkCanvasItem::editProperties()
{
  return false;
}

void LinkCanvasItem::getPortsList(PortsList& ports) const
{
  ports.clear();
  ports.push_back(wire.from);
  ports.push_back(wire.to);
}

void LinkCanvasItem::removePoint(size_t pointIndex)
{
  parent()->getDoc().removeWirePoint(parent(), getId(), pointIndex);
  update();
}

void LinkCanvasItem::addPoint(size_t insertAt, const WPointF& pnt)
{
  parent()->getDoc().addWirePoint(parent(), getId(), insertAt, float(pnt.x()), float(pnt.y()));
  update();
}

void LinkCanvasItem::setPoint(size_t pointIndex, const WPointF& pnt)
{
  parent()->getDoc().setWirePoint(parent(), getId(), pointIndex, float(pnt.x()), float(pnt.y()));
  update();
}

void LinkCanvasItem::draw(CanvasPainter& painter, Polygon& outline)
{
  painter.draw(*this, lineSegments);
  gui::createPolygonArounPolyline(outline, 3., lineSegments);
}

void LinkCanvasItem::paintEvent (WPaintDevice *paintDevice)
{
  AbstractCanvasItem::paintEvent(paintDevice);

  // update link overlay if it is editing this wire.
  if (parent()->getLinkOverlay() && parent()->getLinkOverlay()->getId() == getId())
  {
    parent()->getLinkOverlay()->update();
  }
}

WireEventArea::WireEventArea(CanvasView* _parent)
  : _base_class(_parent)
{
  setId("wire-events");
  setPositionScheme(Wt::Absolute);
  resize(parent()->width(), parent()->height());   //  needed so Wt behaves.
  setZIndex(DisplaySettings::wireEventAreaZIndex);
}

WireEventArea::~WireEventArea()
{
  //  release the area and leave them dangling, since they
  //  are held in shared_ptrs
  while (area(0))
  {
    removeArea(area(0));
  }
}

void WireEventArea::paintEvent(Wt::WPaintDevice* paintDevice)
{
//  debug trace.
//  WPainter painter(paintDevice);
//  painter.setPen(WColor(0, 0, 255));
//
//  int i = 0;
//  for (WAbstractArea* a = area(i);  a != NULL; a = area(++i))
//  {
//    WPolygonArea* pa = dynamic_cast<WPolygonArea*>(a);
//    if (pa && pa->points().size() >= 2)
//    {
//      WPoint p1 = pa->points().at(0);
//      for (size_t i = 1; i < pa->points().size(); ++i)
//      {
//        WPoint p0 = p1;
//        p1 = pa->points().at(i);
//        painter.drawLine(p0.x(), p0.y(), p1.x(), p1.y());
//      }
//    }
//  }
}

}} //  namespace minsky::gui
