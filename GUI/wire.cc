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

#include "wire.h"
#include "zoom.h"
#include "minsky.h"
#include <ecolab_epilogue.h>
using namespace minsky;
using namespace ecolab;

void Wire::zoom(float xOrigin, float yOrigin, float factor)
{
  if (group>-1)
    {
      GroupIcon& g=minsky().groupItems[group];
      xOrigin-=g.x();
      yOrigin-=g.y();
    }
  if (visible)
    for (size_t i=0; i<m_coords.size(); ++i)
      minsky::zoom(m_coords[i], (i&1)? yOrigin: xOrigin, factor);
}

void Wire::move(float dx, float dy)
{
  m_coords[pcoord(m_coords.size()/2)*2]+=dx;
  m_coords[pcoord(m_coords.size()/2)*2+1]+=dy;
  assert(m_coords.size()>=4);
  array<float> c=coords();
  minsky().movePortTo(from, c[0], c[1]);
  minsky().movePortTo(to, c[c.size()-2], c[c.size()-1]);
}

ecolab::array<float> Wire::coords() const
{
  array<float> coords=m_coords;
  if (group>-1)
    {
      GroupIcon& g=minsky().groupItems[group];
      coords[pcoord(coords.size()/2)*2]+=g.x();
      coords[pcoord(coords.size()/2)*2+1]+=g.y();
    }
  return coords;
}

ecolab::array<float> Wire::coords(const ecolab::array<float>& coords)
{
  array<float> offs(coords.size(), 0);
  if (group>-1)
    {
      GroupIcon& g=minsky().groupItems[group];
      offs[pcoord(coords.size()/2)*2]=g.x();
      offs[pcoord(coords.size()/2)*2+1]=g.y();
    }
  m_coords=coords-offs;
  return coords;
}

