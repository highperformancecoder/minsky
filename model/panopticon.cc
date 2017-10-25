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

#include "panopticon.h"
#include "ecolab_epilogue.h"
using namespace minsky;
void Panopticon::redraw(int, int, int width, int height)
{
  double x0, y0, x1, y1;
  canvas.model->contentBounds(x0,y0,x1,y1);
  double xscale=width/abs(x1-x0), yscale=height/abs(y1-y0);
  double scale=min(xscale,yscale);
  cairo_surface_set_device_scale(surface->surface(),scale,scale);
 
  cairo::SurfacePtr tmp(canvas.surface);
  canvas.surface=surface;
  canvas.redraw();
  canvas.surface=tmp;
}
