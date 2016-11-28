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

#ifndef WIRECREATEOVERLAY_H_
#define WIRECREATEOVERLAY_H_

#include <Wt/WPaintedWidget>
#include <Wt/WPointF>
#include <memory>

namespace minsky { namespace gui {

class CanvasView;

/**
 * Overlay that allows the user to create wire links between items on the canvas.
 */
class WireCreateOverlay : public Wt::WPaintedWidget
{
private:
  typedef Wt::WPaintedWidget _base_class;

  typedef int PortId;

  enum
  {
    invalidPort = -1,
  };

private:
  bool isActive = false;          ///<  true while the user is dragging the wire.
  PortId fromPort = invalidPort;  ///<  source port.
  PortId toPort   = invalidPort;  ///<  destination port.
  Wt::WPointF mousePos;           ///<  current mouse position, if active.

public:
  /**
   * Constructor.
   * @param parent  Parent view.
   */
  WireCreateOverlay(CanvasView* parent);

  /**
   * Destructor.
   */
  virtual ~WireCreateOverlay();

  /**
   * Accesses the parent cast as a CanvasView.
   */
  CanvasView* parent();

  inline bool active() const
  {
    return isActive;
  }

  /**
   * Source port.
   */
  int from ();

  /**
   * Destination port.
   */
  int to ();

protected:
  /**
   * Drawing function.
   */
  virtual void paintEvent(Wt::WPaintDevice* paintDevice);
};

/**
 * Managed pointer to WireCreateOverlay object
 */
typedef std::shared_ptr<WireCreateOverlay> WireCreateOverlayPtr;

}} // namespace minsky::gui

#endif /* WIRECREATEOVERLAY_H_ */
