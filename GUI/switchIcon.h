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

#include "opVarBaseAttributes.h"
#include <cairo.h>

namespace minsky
{
  class SwitchIcon: public OpVarBaseAttributes
  {
    CLASSDESC_ACCESS(SwitchIcon);
    friend class SchemaHelper;
  public:
    SwitchIcon();
    ~SwitchIcon();

    /** @{
        number of cases switched between.
        If input <1, case 0 is chosen, if input >=numCases-1, case numCases-1 is chosen
    */
    unsigned numCases() const {return m_ports.size()-2;}
    void setNumCases(unsigned);
    /// @}

    /// value of switch according to current inputs
    unsigned value() const;

    bool flipped=false;

    /// draw icon to \a context
    void draw(cairo_t* context) const;

    ClickType::Type clickType(float x, float y) const override {
      return minsky::clickType(*this,x,y);
    }
  };

  struct SwitchIconPtr: public std::shared_ptr<SwitchIcon>
  {
    virtual int id() const {return -1;}
  };

  typedef TrackedIntrusiveMap<int, SwitchIconPtr> SwitchIcons;
}

#include "switchIcon.cd"
#endif
