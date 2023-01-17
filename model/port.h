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
#include "units.h"
#include <error.h>
#include <vector>
#include <memory>

namespace minsky
{
  class Item;
  class Wire;
  class Group;
  class VariableValue;
  typedef std::shared_ptr<Group> GroupPtr;

  /// components of Port excluded from reflection
  struct PortExclude
  {
    enum Flags {noFlags=0, multiWire=1, inputPort=2};
    // const_cast hack required to work around classdesc::Exclude API
    PortExclude(const Item& item): m_item(const_cast<Item&>(item)) {}
    virtual ~PortExclude()=default;
    /// sets the VariableValue associated with this port. Only for output ports
    void setVariableValue(const std::shared_ptr<VariableValue>& v);
    /// returns the variableValue associated with this port. May be null if not applicable
    std::shared_ptr<VariableValue> getVariableValue() const;
    virtual bool input() const=0;
    GroupPtr group() const;
  protected:
    Item& m_item;
    int flags{0};
    std::weak_ptr<VariableValue> variableValue; //refers to variable value representing this port
    std::vector<Wire*> m_wires;
  };
  
  class Port: public classdesc::Exclude<PortExclude>
  {
  public:
  private:
    float m_x{0}, m_y{0};
    CLASSDESC_ACCESS(Port);
    friend struct SchemaHelper;
    Port(const Port&)=delete;
    void operator=(const Port&)=delete;
    friend class Wire;
  public:
    /// @{ owner of this port
    // this is an accessor to prevent serialisation infinite loops
    Item& item() {return m_item;};
    const Item& item() const {return m_item;}
    /// @}

    /// returns a vector of weak references to the wires attached to this port
    const std::vector<Wire*>& wires() const {return m_wires;}
    std::size_t numWires() const {return m_wires.size();}
    
    /// remove wire from wires. No ownership passed.
    void eraseWire(Wire*);
    /// delete all attached wires
    void deleteWires();

    /// true if input port
    virtual bool input() const {return false;}

    /// true if multiple wires are allowed to connect to an input
    /// port, such as an input port of an add operation. Irrelevant,
    /// otherwise
    virtual bool multiWireAllowed() const {return false;}
    /// combine two input wires
    /// @param x input to be updated
    /// @param y input to be combined with x
    virtual void combineInput(double& x, double y) const {x=y;}
    /// input port value if no wire attached
    virtual double identity() const {return 0;}

    float x() const;
    float y() const;
    void moveTo(float x, float y);
    Port(Item& item, int f=noFlags): classdesc::Exclude<PortExclude>(item) {flags=f;}

    // destruction of this port must also destroy all attached wires
    ~Port() {deleteWires();}

    /// value associated with this port
    double value() const;
    Units units(bool) const;
    /// dimensional analysis with consistency check
    Units checkUnits() const {return units(true);}
  };

  struct InputPort: public Port
  {
    bool input() const override {return true;}
    InputPort(Item& item): Port(item) {}
  };
  
  struct MultiWireInputPort: public InputPort
  {
    bool multiWireAllowed() const override {return true;}
    MultiWireInputPort(Item& item): InputPort(item) {}
  };
}

#ifdef CLASSDESC
#pragma omit RESTProcess minsky::PortExclude
#endif

#include "port.cd"
#include "port.xcd"
#endif
