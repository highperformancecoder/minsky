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
#include "portManager.h"
#include "minsky.h"
#include <tcl++.h>
#include <ecolab_epilogue.h>

using ecolab::array;
using namespace std;
using namespace minsky;

PortManager& minsky::portManager() {return minsky::minsky();}
const PortManager& minsky::cportManager() {return minsky::minsky();}

int PortManager::addPort(const Port& p) 
{
  int nextId=minsky().getNewId();
  ports.insert(Ports::value_type(nextId, p));
  return nextId;
}

int PortManager::addWire(Wire w) 
{
  // adjust end points to be aligned with ports
  array<float> coords=w.coords();
  if (coords.size()<4)
    coords.resize(4);
  const Port& to=ports[w.to];
  const Port& from=ports[w.from];
  coords[0]=from.x();
  coords[1]=from.y();
  coords[coords.size()-2]=to.x();
  coords[coords.size()-1]=to.y();
  w.coords(coords);


  int nextId=minsky().getNewId();
  wires.insert(Wires::value_type(nextId, w));
  
  assert(minsky().variables.noMultipleWiredInputs());
  return nextId;
}


void PortManager::movePortTo(int port, float x, float y)
{
  if (ports.count(port))
    {
      Port& p=ports[port];
      if (abs(p.m_x-x)<1 && fabs(p.m_y-y)<1) return;
      p.m_x=x; p.m_y=y;
      array<int> attachedWires=wiresAttachedToPort(port);
      for (array<int>::iterator i=attachedWires.begin(); 
           i!=attachedWires.end(); ++i)
        {
          assert(wires.count(*i));
          Wire& w=wires[*i];
          array<float> coords=w.coords();
          assert(coords.size()>=4);
          if (w.from==port)
            {
              coords[0]=p.x();
              coords[1]=p.y();
            }
          else if (w.to==port)
            {
              coords[coords.size()-2]=p.x();
              coords[coords.size()-1]=p.y();
            }
          w.coords(coords);
        }
    }
}

template <class C>
int PortManager::closestPortImpl(float x, float y)
{
  int port=-1;
  float minr=numeric_limits<float>::max();
  for (Ports::const_iterator i=ports.begin(); i!=ports.end(); ++i)
    if (C::eval(*i))
    {
      const Port& p=*i;
      float r=(x-p.x())*(x-p.x()) + (y-p.y())*(y-p.y());
      if (r<minr)
        {
          minr=r;
          port=i->id();
        }
    }
  return port;
}

namespace
{
  struct AlwaysTrue
  {
    inline static bool eval(const Port& p) {return true;}
  };
  struct InputTrue
  {
    inline static bool eval(const Port& p) {return p.input();}
  };
  struct OutputTrue
  {
    inline static bool eval(const Port& p) {return !p.input();}
  };
}


int PortManager::closestPort(float x, float y)
{return closestPortImpl<AlwaysTrue>(x,y);}

int PortManager::closestOutPort(float x, float y)
{return closestPortImpl<OutputTrue>(x,y);}

int PortManager::closestInPort(float x, float y)
{return closestPortImpl<InputTrue>(x,y);}

ecolab::array<int> PortManager::wiresAttachedToPort(int port) const
{
  ecolab::array<int> ret;
  for (Wires::const_iterator w=wires.begin(); w!=wires.end(); ++w)
    if (w->to == port || w->from == port)
      ret <<= w->id();
  return ret;
}

void PortManager::delPort(int port)
{
  if (port>=0)
    {
      array<int> wires=wiresAttachedToPort(port);
      for (size_t i=0; i<wires.size(); ++i)
        deleteWire(wires[i]);
      ports.erase(port);
    }
}


ecolab::array<int> PortManager::visibleWires() const
{
  ecolab::array<int> ret;
  for (Wires::const_iterator w=wires.begin(); w!=wires.end(); ++w)
    if (w->visible)
      ret<<=w->id();
  return ret;
}

void PortManager::deleteWire(int id)
{
  Wires::iterator w=wires.find(id);
  if (w!=wires.end())
    {
      if (w->group>=0)
        minsky().groupItems[w->group].delWire(id);
      // urgh!
      minsky().variables.deleteWire(w->from);
      minsky().variables.deleteWire(w->to);
      wires.erase(w);
    }
}

namespace
{
  // comparison operation used for removing duplicate wires
  struct LessWire
  {
    bool operator()(const Minsky::Wires::value_type& x, 
                   const Minsky::Wires::value_type& y) const
    {
      return x.from<y.from || (x.from==y.from && x.to<y.to);
    }
  };
}

void PortManager::removeDuplicateWires()
{
    // remove multiply connected wires (ticket 171)
    set<Wires::value_type, LessWire> wireSet(wires.begin(), wires.end());
    wires.clear();
    wires.insert(wireSet.begin(), wireSet.end());
}

void PortManager::removeUnusedPorts(const std::set<int>& keep)
{
  auto keys=ports.keys();
  vector<int> toRemove;
  set_difference(keys.begin(), keys.end(), keep.begin(), keep.end(), back_inserter(toRemove));
  for (int p: toRemove)
    delPort(p); // consistently remove wires
}
