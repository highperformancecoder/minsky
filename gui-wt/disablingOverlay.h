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

#ifndef DISABLINGOVERLAY_H_
#define DISABLINGOVERLAY_H_

#include <Wt/WPaintedWidget>
#include <memory>

namespace minsky { namespace gui {

class CanvasView;

/**
 * This overlay disables mouse access to canvas items.
 */
class DisablingOverlay : public Wt::WPaintedWidget
{
private:
  typedef Wt::WPaintedWidget _base_class;

public:
  /**
   * Constructor.
   * @param parent A pointer to the canvas
   */
  DisablingOverlay(CanvasView* parent);

  /**
   * Destructor.
   */
  virtual ~DisablingOverlay();

protected:
  /**
   * Drawing function.
   */
  virtual void paintEvent(Wt::WPaintDevice* paintDevice);
};

/**
 * Managed pointer to DisablingOverlay.
 */
typedef std::shared_ptr<DisablingOverlay> DisablingOverlayPtr;

}} // namespace minsky::gui

#endif /* DISABLINGOVERLAY_H_ */
