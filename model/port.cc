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
#include <ecolab_epilogue.h>
using namespace std;

namespace minsky
{

  float Port::x() const 
  {
    if (auto v=dynamic_cast<VariableBase*>(&item))
      if (input()&& v->name()=="0")
        cout << v->name() << " @ "<<m_x+item.x()<<"this="<<this<<" item="<<&item<<" ports={"<<
          item.ports[0].get()<<","<<item.ports[1].get()<<"}"<<endl;
    // check this is in items list of ports
    assert(find_if(item.ports.begin(),item.ports.end(),
                   [this](const shared_ptr<Port>& i)
                   {return i.get()==this;}) != item.ports.end());
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
    if (auto v=dynamic_cast<VariableBase*>(&item))
      if (input()&& v->name()=="0")
        cout << v->name() << "= "<<x<<endl;
  }

  GroupPtr Port::group() const
  {
    return item.group.lock();
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
    if (auto g=item.group.lock())
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
      return item.value();
  }

  const VariableValue& Port::getVariableValue() const {
    if (input() && !m_wires.empty())
      return m_wires[0]->from()->getVariableValue();
    return variableValue;
  }

}
