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

namespace
{

  struct TestFixture: public Minsky
  {
    GroupPtr group0;
    ItemPtr a,b,c;
    WirePtr ab,bc; 
    LocalMinsky lm;

    TestFixture(): lm(*this)
    {
     a=model->addItem(new Variable<VariableType::flow>);
     b=model->addItem(new Variable<VariableType::flow>);
     c=model->addItem(new Variable<VariableType::flow>);
     // create 3 variables, wire them and add first two to a group,
     // leaving 3rd external
     a->moveTo(100,100);
     b->moveTo(200,100);
     c->moveTo(300,100);
     CHECK_EQUAL(2,a->ports.size());
     CHECK_EQUAL(2,b->ports.size());
     CHECK_EQUAL(2,c->ports.size());

     ab=model->addWire(new Wire(a->ports[0], b->ports[1]));
     bc=model->addWire(new Wire(b->ports[0], c->ports[1]));
     checkWiresConsistent();

     group0=model->addGroup(new Group);
     checkWiresConsistent();
     group0->addItem(a);
     checkWiresConsistent();
     group0->addItem(b);

     CHECK_EQUAL(2,group0->items.size());
     CHECK_EQUAL(1,group0->wires.size());
     CHECK_EQUAL(1,model->items.size());
     CHECK_EQUAL(1,group0->wires.size());
     checkWiresConsistent();

      // add a couple of time operators, to ensure the group has finite size
     model->addItem(OperationPtr(OperationType::time))->moveTo(100,75);
     model->addItem(OperationPtr(OperationType::time))->moveTo(200,125);
     select(50,50,250,150);
     save("TestGroupFixture.mky");
     CHECK(model->uniqueItems());
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
  TEST_FIXTURE(TestFixture, SelectGroup)
  {
    Group& g=*model->addGroup(new Group);
    g.addItem(a);
    g.addItem(b);
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
    CHECK(find(g.wires.begin(), g.wires.end(), ab) != g.wires.end()); 
    CHECK(model->uniqueItems());
  }

  TEST_FIXTURE(TestFixture, AddVariable)
  {
    group0->addItem(c);
    CHECK(model->uniqueItems());
    CHECK_EQUAL(3,group0->items.size());
    CHECK_EQUAL(2,model->items.size());
    CHECK_EQUAL(2,group0->wires.size());
    CHECK_EQUAL(0,model->wires.size());
    
    checkWiresConsistent();

    // now check removal
    group0->group.lock()->addItem(c);

    CHECK_EQUAL(2,group0->items.size());
    CHECK_EQUAL(1,group0->wires.size());
    CHECK_EQUAL(3,model->items.size());
    CHECK_EQUAL(1,model->wires.size());
  }
}

SUITE(Canvas)
{
  TEST_FIXTURE(TestFixture, getItemAt)
    {
      canvas.getItemAt(a->x()+2,a->y()+2);
      CHECK(a==canvas.item);
      canvas.getItemAt(b->x()-2,b->y()-2);
      CHECK(b==canvas.item);
      canvas.getItemAt(c->x()-2,c->y()+2);
      CHECK(c==canvas.item);
    }
  
  TEST_FIXTURE(TestFixture, getWireAt)
    {
      auto from=a->ports[0], to=b->ports[1];
      float x=0.5f*(from->x()+to->x())+1;
      float y=0.5f*(from->y()+to->y())+1;
      canvas.getWireAt(x,y);
      CHECK(canvas.wire==ab);
    }
}
