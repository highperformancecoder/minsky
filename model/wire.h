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

#include <error.h>
#include <arrays.h>
#include <TCL_obj_base.h>
#include "classdesc_access.h"

namespace minsky
{
  class Port;
  class Group;
  using ecolab::error;

  class Wire: public NoteBase
  {
    CLASSDESC_ACCESS(Wire);
    friend struct SchemaHelper;

    // intermediate control point coords, relative to ports
    std::vector<float> m_coords;
    /// ports this wire connects
    std::weak_ptr<Port> m_from, m_to;
  public:

    Wire() {}
    Wire(const std::shared_ptr<Port>& from, const std::shared_ptr<Port>& to, 
         const std::vector<float>& a_coords=std::vector<float>()); 

   ~Wire();

    std::shared_ptr<Port> from() const {return m_from.lock();}
    std::shared_ptr<Port> to() const {return m_to.lock();}

    /// display coordinates 
    std::vector<float> coords() const;
    std::vector<float> coords(const std::vector<float>& coords);

    void straighten() {m_coords.clear();}

    /// whether this wire is visible or not
    bool visible() const;
    /// move this from its group into dest
    void moveIntoGroup(Group& dest);
    /// splits wires crossing group boundaries
    void split();
  };

  typedef std::shared_ptr<Wire> WirePtr;
  typedef std::vector<WirePtr> Wires;

}

#ifdef CLASSDESC
// omit these, because weak/shared pointers cause problems, and its
// not needed anyway
#pragma omit pack minsky::Wire
#pragma omit unpack minsky::Wire
#endif
namespace classdesc_access
{
template <> struct access_pack<minsky::Wire>: 
  public classdesc::NullDescriptor<classdesc::pack_t> {};
template <> struct access_unpack<minsky::Wire>: 
  public classdesc::NullDescriptor<classdesc::unpack_t> {};
}
#include "wire.cd"
#endif
