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

#ifndef PANOPTICON_H
#define PANOPTICON_H
#include <cairoSurfaceImage.h>  // for CairoSurface
#include <canvas.h>             // for Canvas::Timestamp, Canvas
#include <memory>               // for __shared_ptr_access
#include "cairo_base.h"         // for SurfacePtr, Surface
#include "classdesc.h"          // for Exclude

namespace minsky
{
  struct Panopticon: public ecolab::CairoSurface
  {
    double cleft=0, ctop=0, cwidth=0, cheight=0;
    Exclude<Canvas::Timestamp> lastBoundsCheck;
    double width=0,height=0;
    Canvas& canvas;
    Exclude<ecolab::cairo::SurfacePtr> cachedImage;
    Panopticon(Canvas& canvas): canvas(canvas)  {}
    void redraw(int, int, int width, int height) override;
    void requestRedraw() {if (surface.get()) surface->requestRedraw();}

    Panopticon& operator=(const Panopticon&) {return *this;}
  };
}
#include "panopticon.cd"
#include "panopticon.xcd"
#endif
