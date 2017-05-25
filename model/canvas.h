/*
  @copyright Steve Keen 2017
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

#ifndef CANVAS_H
#define CANVAS_H

#include "group.h"
#include "selection.h"
#include <cairo_base.h>

namespace minsky
{
  class Canvas
  {
  public:
    GroupPtr model;
    Selection selection;
    Exclude<ecolab::cairo::SurfacePtr> surface;
    ItemPtr itemFocus; ///< item selected by clicking
    WirePtr wireFocus; ///< wire that mouse is hovering over
    classdesc::Exclude<std::shared_ptr<Port>> fromPort; ///< from port when creating a new wire
    double termX,termY; ///< terminal of wire when extending

    Canvas() {}
    Canvas(const GroupPtr& m): model(m) {}
    void addImage(const char* imageName) {
      auto photo=Tk_FindPhoto(interp(),imageName);
      if (!photo) throw ecolab::error("photo %s not found",imageName);
      surface.reset(new ecolab::cairo::TkPhotoSurface(photo));
    }
    
    /// event handling for the canvas
    void mouseDown(float x, float y);
    void mouseUp(float x, float y);
    void mouseMove(float x, float y);

    /// @{ item or wire obtained by get*At() calls
    ItemPtr item;
    WirePtr wire;
    void getItemAt(float x, float y);
    void getWireAt(float x, float y);

    /// redraw whole model
    void redraw();
    /// region to be updated
    LassoBox updateRegion{0,0,0,0};
    /// update region given by updateRegion
    void redrawUpdateRegion();
  };
}

#include "canvas.cd"
#endif
