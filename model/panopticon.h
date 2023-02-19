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
#include "renderNativeWindow.h"
#include <canvas.h>

namespace minsky
{
  class Panopticon: public RenderNativeWindow
  {
    bool redraw(int, int, int width, int height) override;
    CLASSDESC_ACCESS(Panopticon);
  public:
    double cleft=0, ctop=0, cwidth=0, cheight=0;
    Exclude<Canvas::Timestamp> lastBoundsCheck;
    double width=0,height=0;
    Canvas& canvas;
    Exclude<cairo::SurfacePtr> cachedImage;
    Panopticon(Canvas& canvas): canvas(canvas)  {}
    void requestRedraw() {if (surface.get()) surface->requestRedraw();}

    Panopticon(Panopticon&)=default;
    Panopticon& operator=(const Panopticon&) {return *this;}
  };
}
#include "panopticon.cd"
#include "panopticon.xcd"
#endif
