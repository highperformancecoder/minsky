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
void Panopticon::redraw(int, int, int w, int h)
{
  cairo::SurfacePtr tmp(new cairo::Surface
                        (cairo_recording_surface_create(CAIRO_CONTENT_COLOR,nullptr)));
  tmp.swap(canvas.surface);
  canvas.redraw();

  double xscale=w/canvas.surface->width(), yscale=h/canvas.surface->height();
  double scale=min(xscale,yscale);
  cairo_surface_set_device_scale(surface->surface(),scale,scale);
  cairo_surface_set_device_offset(surface->surface(),-scale*canvas.surface->left(),
                                  -scale*canvas.surface->top());
  
  canvas.surface=surface;
  canvas.redraw();
  canvas.surface.swap(tmp);

  // draw indicator rectangle
  cairo_rectangle(surface->cairo(),0,0,width,height);
  cairo_set_source_rgba(surface->cairo(),0,0,0,0.5);
  cairo_fill(surface->cairo());
  surface->blit();
}
