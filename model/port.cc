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
#include "selection.h"
#include "operation.h"
#include "switchIcon.h"
#include "minsky_epilogue.h"
using namespace std;

namespace minsky
{

  float Port::x() const 
  {
    return m_x+item().x();
  }

  float Port::y() const 
  {
    return m_y+item().y();
  }

  void Port::moveTo(float x, float y)
  {
    m_x=x-item().x();
    m_y=y-item().y();
  }

  GroupPtr Port::group() const
  {
    return item().group.lock();
  }

  void Port::eraseWire(Wire* w) 
  {
    for (auto i=m_wires.begin(); i!=m_wires.end(); ++i)
      if (*i==w) 
        {
          m_wires.erase(i);
          break;
        }
  }

  void Port::deleteWires() 
  {
    /// wires could be anywhere, so we need to walk the whole heirachy
    if (auto g=item().group.lock())
      {
        auto& gg=g->globalGroup();
        auto wires=m_wires; // save copy, as Group::removeWire mutates it
        for (auto& w: wires)
          gg.removeWire(*w);
      }
    m_wires.clear();
  }
  
  /// sets the VariableValue associated with this port
  void Port::setVariableValue(const VariableValue& v) {
    if (!input())
      variableValue=v;
  }

  /// value associated with this port
  double Port::value() const {
    auto& vv=getVariableValue();
    if (vv.type()!=VariableType::undefined)
      return vv.value();
    if (input())
      {
        if (!m_wires.empty())
          return m_wires[0]->from()->value();
        return 0;
      }
    else
      return item().value();
  }

  
  Units Port::units(bool check) const
  {
    if (!wires().empty())
      return wires()[0]->units(check);
    else
      return {};
  }

  const VariableValue& Port::getVariableValue() const {
    if (input() && !m_wires.empty())
      return m_wires[0]->from()->getVariableValue();
    return variableValue;
  }

}
