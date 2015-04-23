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

#include <Wt/WAbstractArea>
#include "abstractCanvasItem.h"
#include "canvasView.h"
#include "displaySettings.h"
#include "canvasPainter.h"
#include <ecolab_epilogue.h>
#include <Wt/WPolygonArea>
#include <Wt/WPopupMenu>
#include <cstdio>

namespace minsky
{
namespace gui
{

using namespace Wt;

AbstractCanvasItem::AbstractCanvasItem(MinskyDoc::ObjectId _id,
    CanvasView* parent) :
    _base_class(parent), id(_id), selected(notSelected), origin(0, 0)
{
  setPopup(true);
  setPositionScheme(Absolute);
  setZIndex(DisplaySettings::widgetZIndex);
}

AbstractCanvasItem::~AbstractCanvasItem()
{
}

CanvasView* AbstractCanvasItem::parent()
{
  return dynamic_cast<CanvasView*>(_base_class::parent());
}

const CanvasView* AbstractCanvasItem::parent() const
{
  return dynamic_cast<const CanvasView*>(_base_class::parent());
}

void AbstractCanvasItem::initInteractive()
{
  //  setup an area to catch mouse events.

  if (area(0))
  {
    delete area(0);
  }

  Polygon outline = shapeOutline;
  Wt::WPointF widgetOrigin = Wt::WPointF(offset(Left).value(), offset(Top).value());

  std::for_each(outline.begin(), outline.end(),
      [this, &widgetOrigin](Wt::WPointF& pnt)
      {
        pnt = parent()->modelToScreen(pnt) - widgetOrigin;
      });

  WPolygonArea* area = new WPolygonArea(outline);

  area->setCursor(PointingHandCursor);

  // Move on mouse down.  We need to transmit movement information
  // to the view, so that links to/from the op can be redrawn as well,
  // at least while movement is going on.

  area->mouseWentDown().connect([this](const WMouseEvent& event) -> void
      {
        if (event.button() & WMouseEvent::LeftButton)
        {
          beginMoveWithMouse(event);
        }
        else if (event.button() & WMouseEvent::RightButton)
        {
          showContextMenu(event);
        }
      });

  area->mouseWentUp().connect([this](const WMouseEvent& event) -> void
      {
        if (movingWithMouse())
        {
          endMoveWithMouse(event);
        }
      });

  area->mouseMoved().connect([this](const WMouseEvent& event) -> void
      {
        if (movingWithMouse())
        {
          updateMoveWithMouse(event);
        }
      });

  area->clicked().connect([this](const WMouseEvent& event) -> void
      {
        if (event.button() & WMouseEvent::RightButton)
        {
          showContextMenu(event);
        }
      });

  area->escapePressed().connect([this](Wt::NoClass) -> void
      {
        if (movingWithMouse())
        {
          cancelMoveWithMouse();
        }
      });

  area->doubleClicked().connect([this](const WMouseEvent&) ->void
      {
        editProperties();
      });

  setAttributeValue("oncontextmenu", "event.cancelBubble = true; event.returnValue = false; return false;");

  addArea(area);
}

WRectF AbstractCanvasItem::getModelRect() const
{
  WPointF o = parent()->screenToModel(
      WPointF(offset(Left).value(), offset(Top).value()));
  WPointF wh = parent()->screenToModel(
      WPointF(width().value(), height().value()), false);
  return WRectF(o.x(), o.y(), wh.x(), wh.y());
}

void AbstractCanvasItem::getShapeOutline(Polygon& poly) const
{
  poly = shapeOutline;
}

void AbstractCanvasItem::showContextMenu(const Wt::WMouseEvent& event)
{
  Wt::WPopupMenu menu;

  // important so that delete works.
  CanvasView* view = parent();

  if (initPopupMenu(menu))
  {
    view->disableItems();

    menu.exec(event);

    // no 'this' access beyond this point.

    view->enableItems();
  }
}

bool AbstractCanvasItem::initPopupMenu(Wt::WPopupMenu& menu)
{
  menu.addItem(Wt::WString::tr("contextmenu.delete"))
    ->clicked().connect(
      [this](const Wt::WMouseEvent& event)
      {
        parent()->deleteItem(this);
      });

  menu.addSeparator();

  menu.addItem(Wt::WString::tr("contextmenu.properties"))
    ->clicked().connect(
      [this](const Wt::WMouseEvent& event)
      {
        editProperties();
      });

  return true;
}

void AbstractCanvasItem::resize(const WLength& width, const WLength& height)
{
  _base_class::resize(width, height);
}

void AbstractCanvasItem::moveTo(const double& x, const double& y)
{
  WPointF pnt = parent()->modelToScreen(WPointF(x, y) - origin);
  setOffsets(pnt.x(), Left);
  setOffsets(pnt.y(), Top);
  parent()->addRectToModel(getModelRect());
}

void AbstractCanvasItem::adjustWidgetPos()
{
  if (!movingWithMouse())
  {
    double x, y;
    getModelPos(x, y);
    moveTo(x, y);
  }
}

void AbstractCanvasItem::setSelected(SelectionStatus sel,
    bool repaintNow /*= true*/)
{
  if (sel != selected)
  {
    selected = sel;
    if (repaintNow)
    {
      update();
    }
  }
}

void AbstractCanvasItem::beginMoveWithMouse(const WMouseEvent& event)
{
  // store the object and mouse position n model coordinates.

  moving = true;

  savedMousePos = parent()->deviceToModel(
      WPointF(event.screen().x, event.screen().y));

  double x, y;
  getModelPos(x, y);
  savedPos = WPointF(x, y);

  parent()->moveItem(*this, moveBegin, x, y);
}

void AbstractCanvasItem::updateMoveWithMouse(const WMouseEvent& event)
{
  if (moving)
  {
    if (event.button() & WMouseEvent::LeftButton)
    {
      WPointF pnt = parent()->deviceToModel(WPointF(event.screen().x, event.screen().y));
      pnt = savedPos + (pnt - savedMousePos);

      double x = pnt.x(), y = pnt.y();
      if (!parent()->moveItem(*this, moveInProgress, x, y))
      {
        cancelMoveWithMouse();
      }
    }
    else
    {
      endMoveWithMouse(event);
    }
  }
}

void AbstractCanvasItem::endMoveWithMouse(const WMouseEvent& event)
{
  if (moving)
  {
    moving = false;

    WPointF pnt = parent()->deviceToModel(WPointF(event.screen().x, event.screen().y));
    pnt = savedPos + (pnt - savedMousePos);

    double x = pnt.x(), y = pnt.y();

    if (parent()->moveItem(*this, moveConfirm, x, y))
    {
      getModelPos(x, y);
      WPointF delta = WPointF(x, y) - savedPos;
      gui::translate(shapeOutline, delta);
    }
  }
}

void AbstractCanvasItem::cancelMoveWithMouse()
{
  if (moving)
  {
    moving = false;

    double x = savedPos.x();
    double y = savedPos.y();

    parent()->moveItem(*this, moveCancelled, x, y);
  }
}

void AbstractCanvasItem::paintEvent (WPaintDevice *paintDevice)
{
  CanvasPainter painter(paintDevice);
  draw(painter, shapeOutline);

  // Apply view offset, painter already sized the widget for us.
  adjustWidgetPos();

  // when the sprite is large, the interactive area does not cover the
  //  whole size of the widget, so rebuild it.
  initInteractive();
}

}} // namespace minsky::gui
