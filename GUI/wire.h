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
#ifndef WIRE_H
#define WIRE_H

#include "note.h"

#include <arrays.h>
#include <TCL_obj_base.h>
#include "classdesc_access.h"

namespace minsky
{
  class Wire: public Note
  {
    ecolab::array<float> m_coords;
    CLASSDESC_ACCESS(Wire);
    friend struct SchemaHelper;
  public:
    virtual int id() const {return -1;}

    /// ports this wire connects
    int from, to;
    // for use in sets, etc
    bool operator<(const Wire& x) const {
      return from < x.from || (from==x.from && to<x.to);
    }
    bool visible; ///<whether wire is visible on Canvas 
    int group;

    /// display coordinates 
    ecolab::array<float> coords() const;
    ecolab::array<float> coords(const ecolab::array<float>& coords);

    Wire(int from=0, int to=0, 
         const ecolab::array<float>& coords=ecolab::array<float>(), 
         bool visible=true): 
      m_coords(coords), from(from), to(to), visible(visible), group(-1) {}

    /// zoom by \a factor, scaling all widget's coordinates, using (\a
    /// xOrigin, \a yOrigin) as the origin of the zoom transformation
    void zoom(float xOrigin, float yOrigin, float factor);
    /// move wire by \a dx, \a dy. End ports are moved also
    void move(float dx, float dy);
  };
}
#include "wire.cd"
#endif
