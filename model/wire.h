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
#include "ICairoShim.h"

#include <error.h>
#include <arrays.h>
#include "classdesc_access.h"
#include <cairo.h>

namespace minsky
{
  class Wire;
  class Port;
  class Group;
  class Item;
  struct Units;
  using ecolab::error;
  
  class Wire: public NoteBase
  {
    CLASSDESC_ACCESS(Wire);
    friend struct SchemaHelper;

    // intermediate control point coords, relative to ports
    std::vector<float> m_coords;
    /// ports this wire connects
    std::weak_ptr<Port> m_from, m_to;

    constexpr static float handleRadius=3;
    mutable int unitsCtr=0; ///< for detecting wiring loops in units()
    mutable std::vector<std::pair<float,float>> cairoCoords; ///< contains all the internal cairo coordinates used to draw a wire
  public:

    Wire() {}
    Wire(const std::weak_ptr<Port>& from, const std::weak_ptr<Port>& to, 
         const std::vector<float>& a_coords=std::vector<float>()); 
   ~Wire();

    std::shared_ptr<Port> from() const {return m_from.lock();}
    std::shared_ptr<Port> to() const {return m_to.lock();}

    /// switch ports this wire links to
    void moveToPorts(const std::shared_ptr<Port>& from, const std::shared_ptr<Port>& to);
    /// stash all the internal cairo coordinates along a wire 
    void storeCairoCoords(cairo_t* cairo) const;
    
    /// draw this item into a cairo context
    void draw(cairo_t* cairo, bool reverseArrow=false) const;
    void draw(const ICairoShim& cairoShim, bool reverseArrow=false) const;
    
    /// display coordinates 
    std::vector<float> coords() const;
    std::vector<float> coords(const std::vector<float>& coords);

#undef near
    /// returns true if coordinates are near this wire
    bool near(float x, float y) const;
    /// returns the index into the coordinate list if x,y is close to
    /// it. Otherwise inserts midpoints and returns that. Wire
    /// endpoints are not returned
    unsigned nearestHandle(float x, float y);
    void insertHandle(unsigned position, float x, float y);
    // For ticket 1092. Reinstate delete handle user interaction
    void deleteHandle(float x, float y);    
    void editHandle(unsigned position, float x, float y);
    
    void straighten() {m_coords.clear();}

    /// whether this wire is visible or not
    bool visible() const;
    /// move this from its group into dest
    void moveIntoGroup(Group& dest);
    /// splits wires crossing group boundaries
    void split();
    /// units (dimensional analysis) of data flowing across wire
    Units units(bool) const;
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
#include "wire.xcd"
#endif
