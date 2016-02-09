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
#include "minsky.h"
#include <ecolab_epilogue.h>

#include <UnitTest++/UnitTest++.h>
using namespace minsky;
using namespace std;

namespace minsky
{
  namespace
  {
    Minsky* l_minsky=NULL;
  }

  Minsky& minsky()
  {
    static Minsky s_minsky;
    if (l_minsky)
      return *l_minsky;
    else
      return s_minsky;
  }
}

namespace
{

  struct TestFixture: public Minsky
  {
   int gid;
    int aid,bid,cid;
    int abid,bcid; 
//    ItemPtr addItem(int id, Item* it)
//    {
//      it->group=dynamic_pointer_cast<Group>(*this);
//      return self.addItem(id,it);
//    }
//    GroupPtr& addGroup(int id, Group* it)
//    {
//      it->group=dynamic_pointer_cast<Group>(*this);
//      return self.addGroup(id,it);
//    }

    TestFixture(): 
      gid(getNewId()),
      aid(getNewId()), bid(getNewId()), cid(getNewId()),
      abid(getNewId()), bcid(getNewId())
   {
     ItemPtr a=model->addItem(aid, new Variable<VariableType::flow>);
     ItemPtr b=model->addItem(bid, new Variable<VariableType::flow>);
     ItemPtr c=model->addItem(cid, new Variable<VariableType::flow>);
     // create 3 variables, wire them and add first two to a group,
     // leaving 3rd external
     a->moveTo(100,100);
     b->moveTo(200,100);
     c->moveTo(300,100);
     CHECK_EQUAL(2,a->ports.size());
     CHECK_EQUAL(2,b->ports.size());
     CHECK_EQUAL(2,c->ports.size());

     WirePtr& ab=model->addWire(abid, new Wire(a->ports[0], b->ports[1]));
     WirePtr& bc=model->addWire(bcid, new Wire(b->ports[0], c->ports[1]));
     checkWiresConsistent();

     GroupPtr group0=model->addGroup(gid,new Group);
     checkWiresConsistent();
     group0->addItem(aid,a);
     checkWiresConsistent();
     group0->addItem(bid,b);

     CHECK_EQUAL(2,group0->items.size());
     CHECK_EQUAL(1,group0->wires.size());
     CHECK_EQUAL(1,model->items.size());
     CHECK_EQUAL(1,group0->wires.size());
     checkWiresConsistent();

      // add a couple of time operators, to ensure the group has finite size
     model->findItem(addOperation("time"))->moveTo(100,75);
     model->findItem(addOperation("time"))->moveTo(200,125);
      select(50,50,250,150);
      Group& g=*model->groups[gid=createGroup()];      
      save("TestGroupFixture.mky");
      // TODO: selection not yet working
      g.addItem(aid,a);
      g.addItem(bid,b);
      CHECK_EQUAL(2, g.items.size());
      CHECK_EQUAL(1, g.wires.size());
      CHECK(&g==a->group.lock().get());
      CHECK(!a->visible());
      CHECK(&g==b->group.lock().get());
      CHECK(!b->visible()); 
      CHECK(c->group.lock()==model);
      CHECK(c->visible());
      CHECK(!model->findWire(*ab)->visible());
      CHECK(model->findWire(*bc)->visible());
      CHECK(g.wires.find(abid) != g.wires.end()); 
      CHECK(model->uniqueKeys());
    }

    void checkWiresConsistent() {
      for (auto& i: model->items)
        for (auto& p: i->ports)
          for (auto& w: p->wires)
            {
              CHECK(w);
              CHECK(p->input() || p==w->from());
              CHECK(!p->input() || p==w->to());
            }
      for (auto& g: model->groups)
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
    GroupPtr group0=model->findGroup(gid);
    ItemPtr c=model->findItem(cid);
    group0->addItem(cid,c);
    CHECK(model->uniqueKeys());
    CHECK_EQUAL(3,group0->items.size());
    CHECK_EQUAL(2,model->items.size());
    CHECK_EQUAL(2,group0->wires.size());
    CHECK_EQUAL(0,model->wires.size());
    
    checkWiresConsistent();

    // now check removal
    group0->group.lock()->addItem(cid,c);

    CHECK_EQUAL(2,group0->items.size());
    CHECK_EQUAL(1,group0->wires.size());
    CHECK_EQUAL(3,model->items.size());
    CHECK_EQUAL(1,model->wires.size());
  }
}
