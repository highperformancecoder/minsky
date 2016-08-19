/*
  @copyright Steve Keen 2015
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
#ifndef SWITCHICON_H
#define SWITCHICON_H

#include "item.h"
#include <cairo.h>

namespace minsky
{
  class SwitchIcon: public Item
  {
    CLASSDESC_ACCESS(SwitchIcon);
    friend class SchemaHelper;
    ecolab::cairo::SurfacePtr cairoSurface;
  public:
    SwitchIcon();

    /** @{
        number of cases switched between.
        If input <1, case 0 is chosen, if input >=numCases-1, case numCases-1 is chosen
    */
    unsigned numCases() const {return ports.size()-2;}
    void setNumCases(unsigned);
    /// @}

    void updateIcon(double t) override 
    {if (cairoSurface) cairoSurface->requestRedraw();}
    void setCairoSurface(const ecolab::cairo::SurfacePtr& s) override 
    {cairoSurface=s;}

    /// value of switch according to current inputs
    unsigned value() const;
    std::string classType() const override {return "SwitchIcon";}

    bool flipped=false;

    /// draw icon to \a context
    void draw(cairo_t* context) const override;
  };
}

#include "switchIcon.cd"
#endif
