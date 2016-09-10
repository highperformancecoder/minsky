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

#include "scrollBarArea.h"
#include "canvasView.h"
#include "minskyApp.h"
#include <ecolab_epilogue.h>
#include <Wt/WText>

namespace minsky { namespace gui {

ScrollBarArea::ScrollBarArea(WContainerWidget *_parent)
  : _base_class(NULL)
{
  Container* container = NULL;

  try
  {
    // setParent() doesn't work here.
    container = new Container(_parent);
    container->addWidget(this);
  }
  catch(...)
  {
    if (container)
    {
      if (container->widget(0))
      {
        container->removeWidget(this);
      }
      delete container;
    }
    throw;
  }
}

ScrollBarArea::~ScrollBarArea()
{
  //  delete the inserted parent in case something
  //  went wrong and we are not yet connected to
  //  the widget tree.

  //  if widget(0) == NULL, then we are called from the parent's
  //  destructor..

  Container* myParent = dynamic_cast<Container*>(parent());

  if (myParent && !(myParent->parent() && myParent->widget(0)))
  {
    //  We are called from an exception in MainWindow::MainWindow
    //  detach from parent, so delete this will
    //  not be called a second time
    myParent->removeWidget(widget(0));
    delete myParent;
  }
}

void ScrollBarArea::initialize()
{
  setOverflow(OverflowHidden);
  setPositionScheme(Absolute);
  scrollTo(0, 0, true);

  Container* container = dynamic_cast<Container*>(parent());

  if (container)
  {
    container->initialize();

    zoomFactor = DisplaySettings::defaultZoom;

    //  keep track of user scrolling.

    container->scrolled().connect ( \
        [this](const Wt::WScrollEvent& event)
        {
          //std::cout << "onscroll: " << event.scrollX() << ", " << event.scrollY() << endl;
          screenScrollPoint.setX(event.scrollX());
          screenScrollPoint.setY(event.scrollY());
          scrollPoint = screenToModel(screenScrollPoint);
        });
  }
  else
  {
    //  test MUST always be true.
    throw;
  }
}

WPointF ScrollBarArea::modelToDevice(const WPointF& modelCoords) const
{
  return modelToScreen(modelCoords) - screenScrollPoint;
}

WPointF ScrollBarArea::deviceToModel(const WPointF& deviceCoords) const
{
  return screenToModel(deviceCoords + screenScrollPoint);
}

WPointF ScrollBarArea::modelToScreen(const WPointF& modelCoords, bool applyOffset) const
{
  if (applyOffset)
  {
    WPointF offset(modelRect.x(), modelRect.y());
    return (modelCoords - offset) * zoomFactor;
  }
  return modelCoords * zoomFactor;
}

WPointF ScrollBarArea::screenToModel(const WPointF& screenCoords, bool applyOffset) const
{
  if (applyOffset)
  {
    WPointF offset(modelRect.x(), modelRect.y());
    return (screenCoords / zoomFactor) + offset;
  }
  return screenCoords / zoomFactor;
}

WRectF ScrollBarArea::modelToScreen(const WRectF& rect) const
{
  WPointF o = modelToScreen(WPointF(rect.x(), rect.y()));
  WPointF wh = modelToScreen(WPointF(rect.width(), rect.height()), false);

  return WRectF(o.x(), o.y(), wh.x(), wh.y());
}

WRectF ScrollBarArea::screenToModel(const WRectF& rect) const
{
  WPointF o = screenToModel(WPointF(rect.x(), rect.y()));
  WPointF wh = screenToModel(WPointF(rect.width(), rect.height()), false);

  return WRectF(o.x(), o.y(), wh.x(), wh.y());
}

WRectF ScrollBarArea::getViewport() const
{
  WPointF wh = screenToModel(clientSize, false);
  return WRectF (scrollPoint.x(), scrollPoint.y(), wh.x(), wh.y());
}

bool ScrollBarArea::setZoom (double _zoomFactor, double xCenter, double yCenter)
{
  if (DisplaySettings::minZoomFactor <= _zoomFactor
      && _zoomFactor <= DisplaySettings::maxZoomFactor)
  {
    // get new scroll position.

    WPointF center = WPointF(xCenter, yCenter);
    WPointF delta = scrollPoint - center;

    scrollPoint = center + (delta * (zoomFactor / _zoomFactor));
    zoomFactor = _zoomFactor;

    //  calculate new viewport.
    WRectF screenRect = modelToScreen(getViewport().united(modelRect));
    modelRect = screenToModel(screenRect);

    resize(screenRect.width(), screenRect.height());
    scrollTo(scrollPoint, true);

    // redraw contents
    invalidate();

    return true;
  }
  return false;
}

bool ScrollBarArea::canZoomIn() const
{
  return (zoomFactor < DisplaySettings::maxZoomFactor);
}

/**
 * Indicates whether a zoom out operation is possible
 * at the moment.
 */
bool ScrollBarArea::canZoomOut() const
{
  return (zoomFactor > DisplaySettings::minZoomFactor);
}

void ScrollBarArea::zoomAll()
{
  // TODO:
}

void ScrollBarArea::scrollTo(double x, double y, bool force)
{
  std::cout << "scrollTo " << x << ", " << y << ")" << std::endl;
  if (force || x != scrollPoint.x() || y != scrollPoint.y())
  {
    scrollPoint = WPointF(x, y);
    addViewportToModelRect();
    setDeviceScrollPos(modelToScreen(scrollPoint), force);
  }
}

void ScrollBarArea::scrollIntoView(const Wt::WRectF rect)
{
  WRectF viewport = getViewport();
  WPointF newScrollPoint = scrollPoint;

  if (viewport.left() > rect.left())
  {
    newScrollPoint.setX(rect.left());
  }

  if (viewport.right() < rect.right())
  {
    newScrollPoint.setX(rect.right() - viewport.width());
  }

  if (viewport.top() > rect.top())
  {
    newScrollPoint.setY(rect.top());
  }

  if (viewport.bottom() < rect.bottom())
  {
    newScrollPoint.setY(rect.bottom() - viewport.height());
  }

  if (newScrollPoint != scrollPoint)
  {
    scrollTo(newScrollPoint);
  }
}

void ScrollBarArea::setModelRect (double left, double top, double right, double bottom, bool keepScrollPos)
{
  std::cout << "SetModelRect(" << left << "," << top << "," << right << "," << bottom << ")" << std::endl;
  std::cout << "offset (" << modelRect.left() << "," <<   modelRect.top() << ")" << endl;
  std::cout << "scroll (" << scrollPoint.x() << "," <<   scrollPoint.y() << ")" << endl;

  std::cout << "Old Rect(" << modelRect.x() << "," << modelRect.y()<< ","
                           << modelRect.width() << "," << modelRect.height() << ")" << std::endl;

  WRectF oldRect = modelRect;
  modelRect = WRectF(left, top, right - left, bottom - top).united(getViewport());

  std::cout << "New Rect(" << modelRect.x() << "," << modelRect.y()<< ","
                           << modelRect.width() << "," << modelRect.height() << ")" << std::endl;
  std::cout << "offset (" << modelRect.left() << "," <<   modelRect.top() << ")" << endl;
  std::cout << "scroll (" << scrollPoint.x() << "," <<   scrollPoint.y() << ")" << endl;

  if (oldRect.width() != modelRect.width() || oldRect.height() != modelRect.height())
  {
    WRectF screenRect = modelToScreen(modelRect);
    resize(screenRect.width(), screenRect.height());
  }

  //  keep same scroll pos
  if (oldRect.left() != modelRect.left() || oldRect.top() != modelRect.top())
  {
    std::cout << "adjustViewPos" << endl;
    adjustViewPos();
    if (keepScrollPos)
    {
      scrollTo(scrollPoint, true);
    }
  }
}

void ScrollBarArea::addRectToModel(const Wt::WRectF& rect, bool keepScrollPos)
{
  Wt::WRectF newRect = modelRect.united(rect);
  if (newRect != modelRect)
  {
    setModelRect(newRect.left(), newRect.top(),
                 newRect.right(), newRect.bottom(), keepScrollPos);
  }
}

WPointF ScrollBarArea::getScreenCenter() const
{
  WRectF viewport = getViewport();

  return WPointF(viewport.x() + (viewport.width() / 2), viewport.y() + (viewport.height() / 2));
}

void ScrollBarArea::setViewportSize(int width, int height)
{
  if (width > this->width().value() || height > this->height().value())
  {
    resize(width, height);
  }
  clientSize.setX(width);
  clientSize.setY(height);
  addViewportToModelRect();
}

void ScrollBarArea::resize (const double& cx, const double& cy)
{
  std::cout << "resize " << cx << ", " << cy << std::endl;
  _base_class::resize(int(cx), int(cy));
}

void ScrollBarArea::setDeviceScrollPos(const Wt::WPointF& newPos, bool force)
{
  if (force ||  newPos != screenScrollPoint)
  {
    std::stringstream str;
    str << parent()->jsRef() << ".scrollLeft = " << int(newPos.x()) << ";"
        << parent()->jsRef() << ".scrollTop = "  << int(newPos.y()) << ";";

    MinskyApp::getApp()->doJavaScript(str.str());
  }
}

}}  // namespace minsky::gui
