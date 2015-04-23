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

#ifndef SCROLLBARAREA_H_
#define SCROLLBARAREA_H_

#include <Wt/WContainerWidget>
#include <Wt/WRectF>
#include <Wt/WPointF>
#include "wtGeometry.h"

namespace minsky { namespace gui {

/**
 *  A scrollable widget with zoom support.
 *
 *  TODO:  Scroll notifications arrive with a delay.  This
 *         creates jumps when repeatedly changing zoom really fast.
 */
class ScrollBarArea: public Wt::WContainerWidget
{
private:
  typedef Wt::WContainerWidget _base_class;

  class Container : public Wt::WContainerWidget
  {
  public:
    Container (Wt::WContainerWidget* parent)
      : Wt::WContainerWidget(parent) { }

    void initialize()
    {
      setOverflow(OverflowAuto);
      setPositionScheme(Wt::Absolute);
      setLayoutSizeAware(true);
    }

    virtual void layoutSizeChanged(int width, int height)
    {
      if (count())
      {
        dynamic_cast<ScrollBarArea*>(widget(0))->setViewportSize(width, height);
      }
    }
  };

private:
  Wt::WPointF screenScrollPoint;  ///< scroll point in screen space
  double zoomFactor = 1.;         ///< Current zoom factor.
  Wt::WPointF scrollPoint;        ///< Current scroll position, in model space.
  Wt::WRectF modelRect;           ///<  The model's work area in model
                                  //    coordinates.
  Wt::WPointF clientSize;         ///<  size of client window in pixels, device space.

public:
  /**
   * Constructor.
   */
  ScrollBarArea(Wt::WContainerWidget *parent = 0);

  virtual ~ScrollBarArea();

  void initialize();

  virtual void adjustViewPos() = 0;

  virtual void invalidate() = 0;


  /**
   *  Zooms in by the given increment.
   *  @returns true if the zoom  operation was successful.
   */
  inline bool zoomIn (double increment)
  {
    Wt::WPointF pnt = getScreenCenter();
    return setZoom (zoomFactor + increment, pnt.x(), pnt.y());
  }

  /**
   *  Zooms out by the given increment.
   *  @returns true if the zoom  operation was successful.
   */
  inline bool zoomOut (double increment)
  {
    Wt::WPointF pnt = getScreenCenter();
    return setZoom (zoomFactor - increment, pnt.x(), pnt.y());
  }

  /**
   *  Zooms, keeping the given point stable on screen. Causes a
   *  refresh of all items on the web client.
   *  @param xCenter Absciss of point of interest.
   *  @param yCenter Ordinate of the point of interest.
   */
  bool setZoom (double zoomFactor, double xCenter, double yCenter);

  /**
   * Zooms the view so the entire model is visible on screen.
   */
  void zoomAll();

  /**
   * Indicates whether a zoom in operation is possible
   * at the moment.
   */
  bool canZoomIn() const;

  /**
   * Indicates whether a zoom out operation is possible
   * at the moment.
   */
  bool canZoomOut() const;

  /**
   * Returns the current zoom factor.
   */
  inline double zoom() const
  {
    return zoomFactor;
  }

  /**
   * Returns the model width in model units.
   */
  double modelWidth() const
  {
    return modelRect.width();
  }

  /**
   * Returns the model height in model units.
   */
  double modelHeight() const
  {
    return modelRect.height();
  }

  inline void getScrollPos(double& x, double& y) const
  {
    x = scrollPoint.x();
    y = scrollPoint.y();
  }

  /**
   * Moves the viewpoint on screen.
   */
  void scrollTo(const Wt::WPointF& newPos, bool force = false)
  {
    scrollTo (newPos.x(), newPos.y(), force);
  }

  /**
   * Moves the viewpoint on screen.
   */
  void scrollTo(double x, double y, bool force = false);

  /**
   * Scrolls to make a the area defined by \p rect visible.
   */
  void scrollIntoView(const Wt::WRectF rect);

  /**
   * Transforms a point in model space to device space. Device
   * space is pixels on screen, including scrolling.
   */
  Wt::WPointF modelToDevice(const Wt::WPointF& modelCoords) const;

  /**
   * Transforms a point in device space to model space. Device
   * space is pixels on screen, including scrolling.
   */
  Wt::WPointF deviceToModel(const Wt::WPointF& deviceCoords) const;

  /**
   * Transforms a point in model space to screen space. Screen
   * space is pixels on screen, not including scrolling, so it
   * may extend beyond the physical limits of the screen.
   */
  Wt::WPointF modelToScreen(const Wt::WPointF& modelCoords, bool applyOffset = true) const;

  /**
   * Transforms a point in screen space to model space. Screen
   * space is pixels on screen, not including scrolling, so it
   * may extend beyond the physical limits of the screen.
   */
  Wt::WPointF screenToModel(const Wt::WPointF& screenCoords, bool applyOffset = true) const;

  /**
   * Transforms a rectangle in model space to screen space. Screen
   * space is pixels on screen, not including scrolling, so it
   * may extend beyond the physical limits of the screen.
   */
  Wt::WRectF modelToScreen(const Wt::WRectF& rect) const;

  /**
   * Transforms a rectangle in screen space to model space. Screen
   * space is pixels on screen, not including scrolling, so it
   * may extend beyond the physical limits of the screen.
   */
  Wt::WRectF screenToModel(const Wt::WRectF& rect) const;

  /**
   * Returns the viewport. the viewport is the part of model space
   * visible on screen.
   */
  Wt::WRectF getViewport() const;

  inline void addViewportToModelRect()
  {
    addRectToModel(getViewport());
  }

  /**
   *  Sets the size of the work area, in model units.
   */
  void setModelRect (double left, double top, double right, double bottom, bool keepScrollPos = true);

  void resetModelRect()
  {
    scrollTo(0, 0);
    setModelRect(0, 0, 0, 0);
  }

  void addRectToModel(const Wt::WRectF& rect, bool keepScrollPos = true);

  /**
   * Returns the center of the viewport in model space.
   */
  Wt::WPointF getScreenCenter() const;

  /**
   * Indicates the viewport size, called from Container::layoutSizeChanged()
   */
  void setViewportSize(int width, int height);

private:
  /**
   * Resizes the view, showing scroll bars as needed.  Coordinates in screen space.
   */
  void resize (const double& cx, const double& cy);

  /**
   * Sets scroll position.
   */
  void setDeviceScrollPos(const Wt::WPointF& newPos, bool force = false);
};

}}  // namespace minsky::gui

#endif /* SCROLLBARAREA_H_ */
