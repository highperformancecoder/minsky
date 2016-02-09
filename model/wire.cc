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

#include "wire.h"
#include "port.h"
#include "group.h"
#include <ecolab_epilogue.h>

using namespace std;

namespace minsky
{
  vector<float> Wire::coords() const
  {
    vector<float> c;
    assert(from() && to());
    assert(m_coords.size() % 2 == 0);
    if (auto f=from())
      if (auto t=to())
        {
          c.push_back(f->x());
          c.push_back(f->y());
          for (size_t i=0; m_coords.size()>1 && i<m_coords.size()-1; i+=2)
            {
              c.push_back(f->x() + (t->x()-f->x())*m_coords[i]);
              c.push_back(f->y() + (t->y()-f->y())*m_coords[i+1]);
            }
          c.push_back(t->x());
          c.push_back(t->y());
        }
    return c;
  }

  vector<float> Wire::coords(const vector<float>& coords)
  {
    if (coords.size()<6) 
      m_coords.clear();
    else
      {
        assert(from() && to());
        assert(coords.size() % 2 == 0);
        
        if (auto f=from())
          if (auto t=to())
            {
              float dx=coords[coords.size()-2]-coords[0];
              float dy=coords[coords.size()-1]-coords[1];
              m_coords.resize(coords.size()-4);
              for (size_t i=2; i<coords.size()-3; i+=2)
                {
                  m_coords[i-2] = (coords[i]-coords[0])/dx;
                  m_coords[i-1] = (coords[i+1]-coords[1])/dy;
                }
            }
      }
    return coords;
  }


  Wire::Wire(const shared_ptr<Port>& from, const shared_ptr<Port>& to, 
         const vector<float>& a_coords): 
      m_from(from), m_to(to) 
  {
    if (!from || !to) throw error("wiring defunct ports");
    coords(a_coords);
    m_from.lock()->wires.push_back(this);
    m_to.lock()->wires.push_back(this);
  }

  Wire::~Wire()
  {
    if (auto toPort=to())
      toPort->eraseWire(this);
    if (auto fromPort=from())
      fromPort->eraseWire(this);
  }

  bool Wire::visible() const
  {
    auto f=from(), t=to();
    return f && t && (f->item.visible() || (t->item.visible())); 
  }

  void Wire::moveIntoGroup(Group& dest)
  {
    WirePtr wp;
    int id=-1;
    // one hit find and remove wire from its map, saving the wire and
    // its id
    dest.globalGroup().recursiveDo
      (&Group::wires, 
       [&](Wires& wires, Wires::iterator i) {
        if (i->get()==this) 
          {
            wp=*i;
            id=i->id();
            wires.erase(i);
            return true;
          }
        else
          return false;
      }); 
    if (wp)
      dest.addWire(id,wp);
  }

}
