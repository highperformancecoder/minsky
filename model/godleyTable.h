/*
  @copyright Steve Keen 2018
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

/**
   Provide support for the Godley Table spreadsheet window
 */

#ifndef GODLEYTABLE_H
#define GODLEYTABLE_H
#include "godleyIcon.h"
#include <cairoSurfaceImage.h>
#include <memory>
#include <vector>

namespace minsky
{
  struct GodleyTableWindow: public ecolab::CairoSurface
  {
    std::shared_ptr<GodleyIcon> godleyIcon;
    unsigned scrollRowStart=1, scrollColStart=1;
    int selectedRow=-1, selectedCol=-1;
    std::vector<double> colLeftMargin;
    void redraw(int, int, int width, int height) override;
    void requestRedraw() {if (surface.get()) surface->requestRedraw();}

  };
}

#include "godleyTable.cd"
#endif
