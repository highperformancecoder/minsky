/*
  @copyright Steve Keen 2021
  @author Russell Standish
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

/* We have created a struct `WindowInformation` that stores the `childWindowId` along with other details like display and window attributes. This information is reused across multiple calls to `renderFrame`. 

The flow for code will be -- when minsky starts, a call to /minsky/canvas/initializeNativeWindow will be made, with parentWindowId (and offsets) as the parameters (creating child window in electron did not work as expected, so we need to work with offsets). Subsequent repaints can be requested with /minsky/canvas/renderFrame

As of now, we create the cairo surface with each call to `renderFrame`, though I think the surface can also be reused. I have a placeholder for pointer to cairo::SurfacePtr (not sure we should have pointer to pointer) but it didn't work as expected, so for now I am recreating the surface in `renderFrame`

Please especially review the lifecycle (constructors, desctructors and copy constructors) that I have defined in `renderNativeWindow.cc `. I think the WindowInformation object that is destroyed in the destructor for RenderNativeWindow can be reused (perhaps it can be made a static object?). Also - am not sure how to distinguish between destructor for RenderNativeWindow that will be called with each call to load model (or undo/redo as you mentioned), and the final call when minsky is closed.
 */

#include "renderNativeWindow.h"
#include "windowInformation.h"
#include "minsky_epilogue.h"

#include <stdexcept>
#include <string>

using namespace std;
using namespace ecolab;

namespace minsky
{
  void RenderNativeWindow::renderFrame(unsigned long parentWindowId, int offsetLeft, int offsetTop, int childWidth, int childHeight)
  {
    // bool isInitial = false;
    if (!winInfoPtr) {
      isInitial = true;
      winInfoPtr = std::make_shared<WindowInformation>(parentWindowId, offsetLeft, offsetTop, childWidth, childHeight);
    }

    winInfoPtr->clear();
    auto tmp = winInfoPtr->getSurface();

    //TODO:: Review if this paint with each frame (below 3 lines) are needed on other systems (Windows/MacOS). If not, delete the code
    // if (isInitial || true)
    // {
    //   cairo_move_to(tmp->cairo(), 0, 0);
    //   cairo_set_source_rgb(tmp->cairo(), 1, 1, 1);
    //   cairo_paint(tmp->cairo());
    // }

    tmp.swap(surface);
    redraw(0, 0, surface->width(), surface->height());
    tmp.swap(surface);
  }

  void RenderNativeWindow::resizeWindow(int offsetLeft, int offsetTop, int childWidth, int childHeight)
  {
    // TODO:: To be implemented... need to recreate child window
  }
} // namespace minsky
