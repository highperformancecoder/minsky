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
#ifndef PORTMANAGER_H
#define PORTMANAGER_H
#include "classdesc_access.h"
#include "port.h"
#include "wire.h"
#include "intrusiveMap.h"
#include <vector>
#include <map>

#include <TCL_obj_base.h>
#include <assert.h>

namespace minsky
{
  // encapsulate insertion and deletion operations to be only
  // accessible from PortManager
  template <class T>
  struct PMMap: private IntrusiveMap<int,T>
  {
    friend class PortManager;
    friend struct SchemaHelper;
    CLASSDESC_ACCESS(PMMap);
    
    typedef IntrusiveMap<int,T> Super;
    friend struct classdesc::base_cast<Super>;
    using Super::iterator;
    using Super::const_iterator;
    using Super::mapped_type;
    using Super::value_type;
    using Super::key_type;
    using Super::size;
    using Super::begin;
    using Super::end;
    using Super::rbegin;
    using Super::rend;
    using Super::find;
    using Super::count;
    using Super::empty;
    // redefine [] to prevent insertions
    T& operator[](int x) {
      auto i=Super::find(x);
      if (i!=end()) return *i;
      else throw ecolab::error("invalid item id %d\n",x);
    }
    const T& operator[](int x) const {
      auto i=Super::find(x);
      if (i!=end()) return *i;
      else throw ecolab::error("invalid item id %d\n",x);
    }
    void TCL_obj(classdesc::TCL_obj_t& t, const classdesc::string& d) {
      ::TCL_obj(t,d,static_cast<Super&>(*this));
    }
  };

  class PortManager
  {
  public:

    typedef PMMap<Port> Ports;
    typedef PMMap<Wire> Wires;
    Ports ports;
    Wires wires;

    // add a port to the port map
    int addPort(const Port& p);
    int addOutputPort() {return addPort(Port(0,0,false));}
    int addInputPort() {return addPort(Port(0,0,true));}
    int addMultipleInputPort() {return addPort(Port(0,0,true,true));}

    int addWire(Wire w); 
  
    void delPort(int port);
    /// move port to an absolute location
    void movePortTo(int port, float x, float y);
    /// move port by an increment
    void movePort(int port, float dx, float dy) {
      if (ports.count(port)) movePortTo(port, ports[port].x()+dx, ports[port].y()+dy);
    } 

    /// return ID of the closest port
    int closestPort(float x, float y);
    /// return ID of the closest output port
    int closestOutPort(float x, float y);
    /// return ID of the closest output port
    int closestInPort(float x, float y);

    void deleteWire(int id);
    /// return a list of wires attached to a \a port
    ecolab::array<int> wiresAttachedToPort(int) const;


    ecolab::array<int> visibleWires() const;

    /// removes all ports, except for those contained in \a keep
    void removeUnusedPorts(const std::set<int>& keep);

  protected:
    // called from Minsky::clearAllMaps()
    void clear() {wires.clear(); ports.clear();}
    // called from Minsky::load()
    void removeDuplicateWires();

  private:
    // the common closestPort implementation code
    template <class C> int closestPortImpl(float, float);

  };


  /// global portmanager - identical to minsky(), but invokable from here
  /// without yanking in all of Minsky's definition
  PortManager& portManager();
  const PortManager& cportManager();
}

// for TCL debugging and drill down
#ifdef _CLASSDESC
#pragma omit TCL_obj minsky::PMMap
#endif
template <class T>
void TCL_obj(classdesc::TCL_obj_t& t, const classdesc::string& d, 
             minsky::PMMap<T>& a)
{a.TCL_obj(t,d);}

  
#include "portManager.cd"
#endif
