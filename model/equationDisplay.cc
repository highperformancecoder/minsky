/*
  @copyright Steve Keen 2023
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

#include "equationDisplay.h"
#include "equationDisplay.rcd"
#include "equationDisplay.xcd"

#include "equations.h"
#include "minsky.h"
#include "progress.h"

#include "item.rcd"
#include "minsky_epilogue.h"

namespace minsky
{
  bool EquationDisplay::redraw(int x0, int y0, int width, int height)
  {
    if (surface.get()) {
      const BusyCursor busy(m);
      const MathDAG::SystemOfEquations system(m);
      cairo_rectangle(surface->cairo(),0,0,width,height);
      cairo_clip(surface->cairo());
      cairo_move_to(surface->cairo(),offsx,offsy);
      cairo_scale(surface->cairo(),m_zoomFactor,m_zoomFactor);
      system.renderEquations(*surface,height);
      if (m.flags & Minsky::fullEqnDisplay_needed)
        {
          ecolab::cairo::Surface surf
            (cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,NULL));
          system.renderEquations(surf,std::numeric_limits<double>::max());
          m_width=surf.width();
          m_height=surf.height();
          m.flags &= ~Minsky::fullEqnDisplay_needed;
        }
      return true;
    }
    return surface.get();
  }
}

CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::EquationDisplay);
