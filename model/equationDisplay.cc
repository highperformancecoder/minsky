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
    system.renderEquations(shim,height);
    if (m.flags & Minsky::fullEqnDisplay_needed)
      {
        const auto ink=mansoura::inkExtents([&](const mansoura::IMansoura& rec)
        {
          rec.moveTo(0,0);
          system.renderEquations(rec,std::numeric_limits<double>::max());
        });
        m_width=ink.width;
        m_height=ink.height;
        m.flags &= ~Minsky::fullEqnDisplay_needed;
      }
    return true;
  }
}

CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::EquationDisplay);
