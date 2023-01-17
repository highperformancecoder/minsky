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
  class SwitchIcon: public ItemT<SwitchIcon, BottomRightResizerItem>
  {
    using Super=ItemT<SwitchIcon, BottomRightResizerItem>;
    CLASSDESC_ACCESS(SwitchIcon);
    friend struct SchemaHelper;
  public:
    SwitchIcon() {setNumCases(2);} ///<default to if/then
    SwitchIcon(const SwitchIcon& x): Super(x), flipped(x.flipped) {setNumCases(x.numCases());}
    SwitchIcon(SwitchIcon&& x): Super(x), flipped(x.flipped) {setNumCases(x.numCases());}
    SwitchIcon& operator=(const SwitchIcon& x) {
      Super::operator=(x);
      flipped=x.flipped;
      setNumCases(x.numCases());
      return *this;
    }
    SwitchIcon& operator=(SwitchIcon&& x) {
      Super::operator=(x);
      flipped=x.flipped;
      setNumCases(x.numCases());
      return *this;
    }
    
    /** @{
        number of cases switched between.
        If input <1, case 0 is chosen, if input >=numCases-1, case numCases-1 is chosen
    */
    unsigned numCases() const {return m_ports.size()-2;}
    void setNumCases(unsigned);
    /// @}

    /// value of switch according to current inputs
    unsigned switchValue() const;
    double value() const override {return m_ports[switchValue()+2]->value();}
    
    const SwitchIcon* switchIconCast() const override {return this;}
    SwitchIcon* switchIconCast() override {return this;}    
    
    Units units(bool) const override;

    /// draw icon to \a context
    void draw(cairo_t* context) const override;

    /// whether icon is oriented so input ports are on the rhs, and output on the lhs
    bool flipped=false;
    void flip() override {flipped=!flipped;}
  };
}

#include "switchIcon.cd"
#include "switchIcon.xcd"
#endif
