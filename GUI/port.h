/*
  @copyright Steve Keen 2012
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

namespace minsky
{
  class Port
  {
    float m_x, m_y;
    bool m_multiWireAllowed;
    bool m_input; ///<true if input port
    CLASSDESC_ACCESS(Port);
    friend class PortManager;
    friend class SchemaHelper;
  public:
    bool input() const {return m_input;}

    /// true if multiple wires are allowed to connect to an input
    /// port, such as an input port of an add operation. Irrelevant,
    /// otherwise
    bool multiWireAllowed() const {return m_multiWireAllowed;}
    float x() const {return m_x;}
    float y() const {return m_y;}
    Port(float x=0, float y=0, bool input=false, bool multiWireAllowed=false): 
      m_x(x), m_y(y), m_multiWireAllowed(multiWireAllowed), m_input(input) {}

    /// move port by a relative distance
    void move(float dx, float dy) {m_x+=dx; m_y+=dy;}
  };
}

#include "port.cd"
#endif
