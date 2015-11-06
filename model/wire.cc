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
          for (size_t i=0; i<m_coords.size()-1; i+=2)
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
              assert(f->x()==coords[0] && f->y()==coords[1]);
              assert(t->x()==coords[coords.size()-1] && t->y()==coords[coords.size()-2]);
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
    assert(coords==this->coords());
    return coords;
  }

  Wire::~Wire()
  {
    if (auto toPort=to())
      toPort->eraseWire(this);
    if (auto fromPort=from())
      fromPort->eraseWire(this);
  }

  void WirePtr::moveGroup(Group& src, Group& dest)
  {
    if (&src==&dest) return;
    auto oldWit=src.wires.find(id());
    auto& newWp=dest.wires[id()]=*oldWit;
    // we need to ensure that the port references are updated too
    assert(newWp->from() && newWp->to());
    newWp->from()->eraseWire(oldWit->get());
    newWp->from()->wires.push_back(&newWp);
    newWp->to()->eraseWire(oldWit->get());
    newWp->to()->wires.push_back(&newWp);

    src.wires.erase(oldWit);
    // TODO
    // assert(w->from.lock().group()==dest);
    // assert(w->to.lock().group()==dest);
  }
  void WirePtr::addPorts(const shared_ptr<Port>& from, const shared_ptr<Port>& to)
  {
    get()->m_from=from; 
    get()->m_to=to;
    if (from) 
      from->wires.push_back(this);
    if (to) 
      to->wires.push_back(this);
  }

}
