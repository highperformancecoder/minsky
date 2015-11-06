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
#ifndef WIRE_H
#define WIRE_H

#include "noteBase.h"
#include "intrusiveMap.h"

#include <arrays.h>
#include <TCL_obj_base.h>
#include "classdesc_access.h"

namespace minsky
{
  class Port;
  class Group;

  class Wire: public NoteBase
  {
    CLASSDESC_ACCESS(Wire);
    friend struct SchemaHelper;

    // intermediate control point coords, relative to ports
    std::vector<float> m_coords;
    /// ports this wire connects
    std::weak_ptr<Port> m_from, m_to;
    friend class WirePtr;
  public:

    Wire() {}
    Wire(const std::shared_ptr<Port>& from, const std::shared_ptr<Port>& to, 
         const std::vector<float>& a_coords=std::vector<float>()): 
      m_from(from), m_to(to) {coords(a_coords);}

   ~Wire();

    std::shared_ptr<Port> from() const {return m_from.lock();}
    std::shared_ptr<Port> to() const {return m_to.lock();}

    /// display coordinates 
    std::vector<float> coords() const;
    std::vector<float> coords(const std::vector<float>& coords);
  };

  class WirePtr: public std::shared_ptr<Wire>
  {
  public:
    virtual int id() const {return -1;}
   template <class... A> WirePtr(A... x):
      std::shared_ptr<Wire>(std::forward<A>(x)...) {}
    virtual ~WirePtr() {}

    /// move this wire from \a src to \a dest
    void moveGroup(Group& src, Group& dest);
    void addPorts(const std::shared_ptr<Port>& from, const std::shared_ptr<Port>& to);
    std::shared_ptr<Port> from() const {return get()->from();}
    std::shared_ptr<Port> to() const {return get()->to();}
  };

  typedef IntrusiveMap<int, WirePtr> Wires;

}
#include "wire.cd"
#endif
