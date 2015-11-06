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
#include <error.h>
#include <vector>
#include <memory>

namespace minsky
{
  class Item;
  class Wire;
  class WirePtr;
  class Group;
  class GroupPtr;

  class Port
  {
  public:
    enum Flags {noFlags=0, multiWire=1, inputPort=2};
    private:
    float m_x{0}, m_y{0};
    int flags{0};
    bool m_multiWireAllowed;
    bool m_input; ///<true if input port
    CLASSDESC_ACCESS(Port);
    friend class SchemaHelper;
    Port(const Port&)=delete;
    void operator=(const Port&)=delete;
  public:
    std::weak_ptr<Item> item;
    std::vector<WirePtr*> wires;
    GroupPtr group() const;

    /// remove wire from wires. No ownership passed.
    void eraseWire(Wire*);

    bool input() const {return flags&inputPort;}

    /// true if multiple wires are allowed to connect to an input
    /// port, such as an input port of an add operation. Irrelevant,
    /// otherwise
    bool multiWireAllowed() const {return flags&multiWire;}
    float x() const;
    float y() const;
    Port() {}
    Port(const std::shared_ptr<Item>& a_item, Flags f=noFlags): flags(f), item(a_item) {}

    ~Port();

  };
}

#include "port.cd"
#endif
