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

#include "cairoItems.h"
#include "equationDisplay.h"
#include "equationDisplay.rcd"
#include "equationDisplay.xcd"

#include "equations.h"
#include "minsky.h"
#include "progress.h"

#include "item.rcd"
#include "noteBase.rcd"
#include "port.rcd"
#include "IMansoura.h"
#include "IMansoura.cd"
#include "minsky_epilogue.h"

namespace minsky
{
  bool EquationDisplay::redraw(const mansoura::IMansoura& shim, int x0, int y0, int width, int height)
  {
    const BusyCursor busy(m);
    const MathDAG::SystemOfEquations system(m);
    shim.rectangle(0,0,width,height);
    shim.clip();
    shim.moveTo(offsx,offsy);
    shim.scale(m_zoomFactor,m_zoomFactor);
    // TODO: remove this bridge once SystemOfEquations::renderEquations
    // is migrated to IMansoura. It draws on a second cairo context
    // sharing the target surface and the state set up above.
    {
      auto cr=static_cast<cairo_t*>(shim.context());
      double x,y;
      shim.getCurrentPoint(x,y);
      ecolab::cairo::Surface surf(cairo_surface_reference(cairo_get_target(cr)));
      cairo_rectangle(surf.cairo(),0,0,width,height);
      cairo_clip(surf.cairo());
      cairo_matrix_t ctm;
      cairo_get_matrix(cr,&ctm);
      cairo_set_matrix(surf.cairo(),&ctm);
      cairo_move_to(surf.cairo(),x,y);
      system.renderEquations(surf,height);
    }
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
}

CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::EquationDisplay);
