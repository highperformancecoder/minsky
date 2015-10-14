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

#include "group.h"
#include <ecolab_epilogue.h>

#include <UnitTest++/UnitTest++.h>
using namespace minsky;
using namespace std;

namespace
{
  int getNextId() 
  {
    static int id=0;
    return id++;
  }

  struct TestFixture: public GroupPtr
  {
    Group &self;
    int gid;
    int aid,bid,cid;
    int abid,bcid; 
    ItemPtr& addItem(int id, Item* it)
    {
      it->group=dynamic_pointer_cast<Group>(*this);
      return self.addItem(id,it);
    }
    GroupPtr& addGroup(int id, Group* it)
    {
      it->group=dynamic_pointer_cast<Group>(*this);
      return self.addGroup(id,it);
    }

    TestFixture(): 
      GroupPtr(new Group),
      self(**this),
      gid(getNextId()),
      aid(getNextId()), bid(getNextId()), cid(getNextId()),
      abid(getNextId()), bcid(getNextId())
   {
     ItemPtr a=addItem(aid, new Item);
     ItemPtr b=addItem(bid, new Item);
     ItemPtr c=addItem(cid, new Item);
     // create 3 variables, wire them and add first two to a group,
     // leaving 3rd external
     a->moveTo(100,100);
     b->moveTo(200,100);
     c->moveTo(300,100);
     a->ports.emplace_back(new Port(a));
     a->ports.emplace_back(new Port(a,Port::inputPort));
     b->ports.emplace_back(new Port(b));
     b->ports.emplace_back(new Port(b,Port::inputPort));
     c->ports.emplace_back(new Port(c));
     c->ports.emplace_back(new Port(c,Port::inputPort));

     WirePtr& ab=self.addWire(abid, new Wire);
     WirePtr& bc=self.addWire(bcid, new Wire);
     ab.addPorts(a->ports[0], b->ports[1]);
     bc.addPorts(b->ports[0], c->ports[1]);
     checkWiresConsistent();

     GroupPtr group0=addGroup(gid,new Group);
     checkWiresConsistent();
     group0->addItem(aid,a);
     checkWiresConsistent();
     group0->addItem(bid,b);

     CHECK_EQUAL(2,group0->items.size());
     CHECK_EQUAL(1,group0->wires.size());
     CHECK_EQUAL(1,self.items.size());
     CHECK_EQUAL(1,group0->wires.size());
     checkWiresConsistent();

//      // add a couple of time operators, to ensure the group has finite size
//      operations[addOperation("time")]->moveTo(100,75);
//      operations[addOperation("time")]->moveTo(200,125);
//      select(50,50,250,150);
//      GroupIcon& g=*groupItems[gid=createGroup()];      
//      save("TestGroupFixture.mky");
//      CHECK_EQUAL(2, g.variables.size());
//      CHECK_EQUAL(1, g.wires().size());
//      CHECK_EQUAL(g.id(), a->group);
//      CHECK(!a->visible);
//      CHECK_EQUAL(g.id(), b->group);
//      CHECK(!b->visible); 
//      CHECK_EQUAL(-1, c->group);
//      CHECK(c->visible);
//      CHECK(!wires[ab].visible);
//      CHECK(wires[bc].visible);
//      CHECK_EQUAL(ab, g.wires()[0]); 
//      CHECK(uniqueGroupMembership());
    }

    void checkWiresConsistent() {
      for (auto& i: self.items)
        for (auto& p: i->ports)
          for (auto& w: p->wires)
            {
              CHECK(w);
              CHECK(p->input() || p==w->from());
              CHECK(!p->input() || p==w->to());
            }
      for (auto& g: self.groups)
        for (auto& i: g->items)
          for (auto& p: i->ports)
            for (auto& w: p->wires)
              {
                assert(w);
                assert(p->input() || p==w->from());
                assert(!p->input() || p==w->to());
              }
    }
  };
}


SUITE(Group)
{
  TEST_FIXTURE(TestFixture, AddVariable)
  {
    GroupPtr group0=self.groups[gid];
    ItemPtr c=self.findItem(cid);
    group0->addItem(cid,c);
    CHECK_EQUAL(3,group0->items.size());
    CHECK_EQUAL(0,self.items.size());
    CHECK_EQUAL(2,group0->wires.size());
    CHECK_EQUAL(0,self.wires.size());
    
    checkWiresConsistent();

    // now check removal
    group0->group.lock()->addItem(cid,c);

    CHECK_EQUAL(2,group0->items.size());
    CHECK_EQUAL(1,group0->wires.size());
    CHECK_EQUAL(1,self.items.size());
    CHECK_EQUAL(1,self.wires.size());
  }
}
