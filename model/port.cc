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
    return m_x+item.x();
  }

  float Port::y() const 
  {
    return m_y+item.y();
  }

  void Port::moveTo(float x, float y)
  {
    m_x=x-item.x();
    m_y=y-item.y();
  }

  GroupPtr Port::group() const
  {
    return item.group.lock();
  }

  void Port::eraseWire(Wire* w) 
  {
    for (auto i=wires.begin(); i!=wires.end(); ++i)
      if (*i==w) 
        {
          wires.erase(i);
          break;
        }
  }

  Port::~Port()
  {
    // destruction of this port must also destroy all attached wires

    /// wires could be anywhere, so we need to walk the whole heirachy
    if (auto g=item.group.lock())
      {
        auto& gg=g->globalGroup();
        for (auto& w: wires)
          gg.removeWire(*w);
      }
    wires.clear();
  }

  /// sets the VariableValue associated with this port
  void Port::setVariableValue(const VariableValue& v) {
    if (!input())
      variableValue=v;
  }

  /// value associated with this port
  double Port::value() const {
    if (input() && wires.size()==1)
      return wires[0]->from()->value();
    if (!input())
      return variableValue.value();
    return 0;
  }

}
