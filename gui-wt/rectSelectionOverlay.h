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

#ifndef RECTSELECTIONOVERLAY_H_
#define RECTSELECTIONOVERLAY_H_

#include <Wt/WPaintedWidget>
#include <Wt/WPointF>
#include <memory>

namespace minsky { namespace gui {

class CanvasView;

/**
 * Overlay that displays and controls a draggable selection rectangle
 */
class RectSelectionOverlay : public Wt::WPaintedWidget
{
private:
  typedef Wt::WPaintedWidget _base_class;

private:
  Wt::WPointF pointA;     ///<  first point clicked, this point is fixed.
  Wt::WPointF pointB;     ///<  second point dragged to, this point is tracked
                          ///   with the mouse of finger.
  bool isActive = false;  ///<  true while the user is dragging the rectangle.

public:
  /**
   *  Constuctor.
   */
  RectSelectionOverlay(CanvasView* parent);

  /**
   * Destructor.
   */
  virtual ~RectSelectionOverlay();

  /**
   * Returns the parent widget, cast as a CanvasView*
   */
  CanvasView* parent();

  /**
   * Indicates the user is dragging the rectangle.
   * @return Member isActive value.
   */
  inline bool active() const
  {
    return isActive;
  }

  /**
   * Returns the selection rectangle in model space.
   */
  Wt::WRectF rectangle() const;

protected:
  /**
   * Drawing function.
   * @param paintDevice
   */
  virtual void paintEvent(Wt::WPaintDevice* paintDevice);
};

/**
 * Managed pointer type for RectSelectionOverlay.  There are restrictions
 * on where a managed pointer to a Wt object can be stored.  They cannot
 * be stored outside one of its parents in the widget tree.
 */
typedef std::shared_ptr<RectSelectionOverlay> RectSelectionOverlayPtr;

}}  // namespace minsky::gui

#endif /* RECTSELECTIONOVERLAY_H_ */

