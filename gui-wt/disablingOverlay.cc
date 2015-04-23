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

#include "disablingOverlay.h"
#include "canvasView.h"
#include <ecolab_epilogue.h>
#include <Wt/WRectArea>

namespace minsky { namespace gui {

DisablingOverlay::DisablingOverlay(CanvasView* _parent)
  : _base_class(_parent)
{
  setId("disable-items");
  setPositionScheme(Absolute);
  setZIndex(DisplaySettings::disableItemsOverlay);
  resize(parent()->width(), parent()->height());

  setAttributeValue("oncontextmenu", "event.cancelBubble = true; event.returnValue = false; return false;");

  addArea(new Wt::WRectArea(0., 0., width().value(), height().value()));
}

DisablingOverlay::~DisablingOverlay()
{ }

void DisablingOverlay::paintEvent(Wt::WPaintDevice* paintDevice)
{ }

}} // namespace minsky::gui
