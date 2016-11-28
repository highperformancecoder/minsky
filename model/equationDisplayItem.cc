/*
  @copyright Steve Keen 2014
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

// defines a single canvas item, onto which Minsky equations are written
#include "minsky.h"
#include "init.h"

#include <cairo_base.h>
#include <ecolab_epilogue.h>

using namespace minsky;
using namespace ecolab::cairo;

namespace
{
  struct EquationDisplayItem: public CairoImage
  {
    void draw()
    {
      if (cairoSurface)
        try
          {
            MathDAG::SystemOfEquations system(cminsky());
            cairo_move_to(cairoSurface->cairo(),0,0);
            system.renderEquations(*cairoSurface);
          }
        catch (std::exception& ex) {} // ignore errors due to ill-formed models
    }
  };

  // register OperatorItem with Tk for use in canvases.
  int registerItems()
  {
    static Tk_ItemType equationDisplayItemType = cairoItemType();
    equationDisplayItemType.name=const_cast<char*>("equations");
    equationDisplayItemType.createProc=createImage<EquationDisplayItem>;
    Tk_CreateItemType(&equationDisplayItemType);
    return 0;
  }

}

static int dum=(initVec().push_back(registerItems), 0);
