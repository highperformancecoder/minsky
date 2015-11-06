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

#include "port.h"
#include "item.h"
#include "wire.h"
#include "group.h"
#include <ecolab_epilogue.h>
using namespace std;

namespace minsky
{

  float Port::x() const 
  {
    if (auto i=item.lock())
      return m_x+i->x();
    else
      return m_x;
  }

  float Port::y() const 
  {
    if (auto i=item.lock())
      return m_y+i->y();
    else
      return m_y;
  }

  GroupPtr Port::group() const
  {
    if (auto i=item.lock())
      return i->group.lock();
    else
      return GroupPtr();
  }

  void Port::eraseWire(Wire* w) 
  {
    for (auto i=wires.begin(); i!=wires.end(); ++i)
      if ((*i)->get()==w) 
        {
          wires.erase(i);
          break;
        }
  }

  Port::~Port()
  {
    // destruction of this port must also destroy all attached wires

    // copy out list of wire ids to prevent a snarl up from ~wires()
    vector<int> wiresToDelete;
    for (auto& w: wires)
      wiresToDelete.push_back(w->id());
    wires.clear();

    if (auto i=item.lock())
      if (auto g=i->group.lock())
        for (int w: wiresToDelete)
          g->wires.erase(w);
  }

}
