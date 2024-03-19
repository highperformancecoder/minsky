/*
  @copyright Steve Keen 2021
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

#ifndef LOCK_H
#define LOCK_H

#include "item.h"
#include "ravelState.h"
#include "ravelWrap.h"
#include "SVGItem.h"

namespace minsky
{
  class Lock: public ItemT<Lock>
  {
  public:
    Lock();
    Lock(const Lock& x): ItemT<Lock>(x) {addPorts();}
    Lock& operator=(const Lock& x)=default;
    void addPorts();
    ravel::RavelState lockedState;

    bool locked() const {return !lockedState.empty();}
    void toggleLocked();

    static SVGRenderer lockedIcon;
    static SVGRenderer unlockedIcon;
    void draw(cairo_t* cairo) const override;
    Units units(bool) const override;
    /// Ravel this is connected to. nullptr if not connected to a Ravel
    Ravel* ravelInput() const;
    void applyLockedStateToRavel() const;
  };
}

#include "lock.cd"
#include "lock.rcd"
#include "lock.xcd"
#endif
