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

#ifndef DELETEITEMSOVERLAY_H_
#define DELETEITEMSOVERLAY_H_

#include <Wt/WPaintedWidget>

namespace minsky { namespace gui {

class CanvasView;
class AbstractCanvasItem;

/**
 * Overlay that lets user delete items from the canvas.
 */
class DeleteItemsOverlay: public Wt::WPaintedWidget
{
private:
  typedef Wt::WPaintedWidget _base_class;

public:
  /**
   * Constructor
   */
  DeleteItemsOverlay(CanvasView* parent);

  /**
   * Destructor.
   */
  virtual ~DeleteItemsOverlay();

  /**
   * Initializes areas for user interactivity.
   */
  void initInteractive();

  /**
   * Returns the parent cast as CanvasView.
   * @return
   */
  CanvasView* parent();

  /**
   * Creates interactive area for an individual item.
   * @param item Item of interest.
   */
  void createItemDeleteArea(AbstractCanvasItem* item);

protected:
  /**
   * Drawing function.  This overlay is invisible.
   */
  virtual void paintEvent(Wt::WPaintDevice* paintDevice);
};

typedef std::shared_ptr<DeleteItemsOverlay> DeleteItemsOverlayPtr;

}} // namespace minsky::gui

#endif /* DELETEITEMSOVERLAY_H_ */
