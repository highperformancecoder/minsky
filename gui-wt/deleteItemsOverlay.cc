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

#include "deleteItemsOverlay.h"
#include "canvasView.h"
#include "displaySettings.h"
#include "abstractCanvasItem.h"
#include "wtGeometry.h"
#include <ecolab_epilogue.h>
#include <Wt/WPolygonArea>

//  for debug trace
#include <Wt/WPainter>
#include <Wt/WBrush>
#include <Wt/WPen>

namespace minsky { namespace gui {

DeleteItemsOverlay::DeleteItemsOverlay(CanvasView* _parent)
    : _base_class(_parent)
{
  setId("canvas-delete");
  setZIndex(DisplaySettings::deleteItemsOverlayZIndex);
  resize(parent()->width(), parent()->height());
  initInteractive();
}

DeleteItemsOverlay::~DeleteItemsOverlay()
{ }

void DeleteItemsOverlay::initInteractive()
{
  parent()->forAllViewObjects(
      [this](AbstractCanvasItem* item)
      {
        createItemDeleteArea(item);
      });

  //  debug trace
  update();
}

void DeleteItemsOverlay::createItemDeleteArea(AbstractCanvasItem* item)
{
  Polygon poly;
  item->getShapeOutline(poly);

  std::for_each(poly.begin(), poly.end(),
      [this](Wt::WPointF& pnt)
      {
        pnt = parent()->modelToScreen(pnt);
      });

  Wt::WPolygonArea* area = new Wt::WPolygonArea(poly);
  addArea(area);

  area->setCursor(Wt::PointingHandCursor);

  area->clicked().connect(
      [this, item, area](const Wt::WMouseEvent&) -> void
      {
        //  the following line will also cause a delete of the overlay
        parent()->deleteItem(item);
        //  this, area, item are no longer valid pointers.
      });
}

CanvasView* DeleteItemsOverlay::parent()
{
  return dynamic_cast<CanvasView*>(_base_class::parent());
}

void DeleteItemsOverlay::paintEvent(Wt::WPaintDevice* paintDevice)
{
  //  invisible.

  //  debug trace.
  Wt::WPainter painter(paintDevice);
  painter.setPen(Wt::WColor(255, 0, 0));
  painter.setBrush(Wt::WColor(255, 0, 0, 127));

  int i = 0;
  for (WAbstractArea* a = area(i);  a != NULL; a = area(++i))
  {
    WPolygonArea* pa = dynamic_cast<WPolygonArea*>(a);
    if (pa && pa->points().size() >= 3)
    {
      Polygon poly;
      for (size_t i = 0; i < pa->points().size(); ++i)
      {
        poly.push_back(Wt::WPointF(pa->points().at(i).x(), pa->points().at(i).y()));
      }
      painter.drawPolygon(&(poly[0]), poly.size());
    }
  }
}

}} // namespace minsky::gui
