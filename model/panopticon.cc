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
#include "minsky_epilogue.h"
using namespace minsky;

bool Panopticon::redraw(int, int, int w, int h)
{
  // cache results of canvas image size
  if (canvas.model.timestamp>lastBoundsCheck || !cachedImage.get())
    {
      lastBoundsCheck=Canvas::Timestamp::clock::now();
      cachedImage.reset(new cairo::Surface
                            (cairo_recording_surface_create(CAIRO_CONTENT_COLOR,nullptr)));
      cachedImage.swap(canvas.surface());
      // render at zoom=1, 0,0
      double zf=canvas.model->zoomFactor(), x=canvas.model->x(), y=canvas.model->y();
      canvas.model->zoom(x,y,1/zf);
      canvas.model->moveTo(0,0);
      canvas.redraw();
      canvas.model->moveTo(x,y);
      canvas.model->zoom(x,y,zf);
      cachedImage.swap(canvas.surface());
    }
  
  double xscale=w/cachedImage->width(), yscale=h/cachedImage->height();
  double scale=min(xscale,yscale);
  cairo_scale(surface->cairo(),scale,scale);
  // Heuristic to not render when scale is too small for things to be visible
  if (scale>0.03)
    {
      cairo_set_source_surface(surface->cairo(), cachedImage->surface(), -cachedImage->left(),-cachedImage->top());
      cairo_paint(surface->cairo());
    }
  
  // draw indicator rectangle
  double zf=1/canvas.model->zoomFactor();
  cairo_rectangle(surface->cairo(),-cachedImage->left()-canvas.model->x(),-cachedImage->top()-canvas.model->y(),zf*width,zf*height);
  cairo_set_source_rgba(surface->cairo(),0,0,0,0.5);
  cairo_fill(surface->cairo());
  surface->blit();
  return true;
}
