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
#ifndef PORT_H
#define PORT_H
#include "classdesc_access.h"
#include "variableValue.h"
#include <error.h>
#include <vector>
#include <memory>

namespace minsky
{
  class Item;
  class Wire;
  class Group;
  typedef std::shared_ptr<Group> GroupPtr;

  class Port
  {
  public:
    enum Flags {noFlags=0, multiWire=1, inputPort=2};
  private:
    float m_x{0}, m_y{0};
    int flags{0};
//    bool m_multiWireAllowed;
//    bool m_input; ///<true if input port
    CLASSDESC_ACCESS(Port);
    friend struct SchemaHelper;
    Port(const Port&)=delete;
    void operator=(const Port&)=delete;
    VariableValue variableValue; //refers to variable value representing this port
    std::vector<Wire*> m_wires;
    friend class Wire;
    Item& m_item;
  public:
    /// @{ owner of this port
    // this is an accessor to prevent serialisation infinite loops
    Item& item() {return m_item;};
    const Item& item() const {return m_item;}
    /// @}
    GroupPtr group() const;

    /// returns a vector of weak references to the wires attached to this port
    const std::vector<Wire*>& wires() const {return m_wires;}
    size_t numWires() const {return m_wires.size();}
    
    /// remove wire from wires. No ownership passed.
    void eraseWire(Wire*);
    /// delete all attached wires
    void deleteWires();
    
    bool input() const {return flags&inputPort;}

    /// true if multiple wires are allowed to connect to an input
    /// port, such as an input port of an add operation. Irrelevant,
    /// otherwise
    bool multiWireAllowed() const {return flags&multiWire;}
    float x() const;
    float y() const;
    void moveTo(float x, float y);
    //Port() {}
    Port(Item& item, int f=noFlags): flags(f), m_item(item) {}

    // destruction of this port must also destroy all attached wires
    ~Port() {deleteWires();}

    /// sets the VariableValue associated with this port. Only for output ports
    void setVariableValue(const VariableValue& v);
    const VariableValue& getVariableValue() const;
    /// value associated with this port
    double value() const;
    Units units(bool) const;
    /// dimensional analysis with consistency check
    Units checkUnits() const {return units(true);}
  };
}

#include "port.cd"
#include "port.xcd"
#endif
