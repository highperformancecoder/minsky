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
#include "minsky.h"
#include "inGroupTest.h"
#include <ecolab_epilogue.h>
#include <UnitTest++/UnitTest++.h>
using namespace minsky;

namespace
{
  void pWireCoords()
  {
    cout << "------------------------------------"<<endl;
    for (PortManager::Wires::const_iterator w=minsky::minsky().wires.begin();
           w!=minsky::minsky().wires.end(); ++w)
      cout <<"Wire "<<w->id()<<": "<<w->coords()<<endl;
  }

  struct TestFixture: public Minsky
  {
    LocalMinsky lm;
    VariablePtr a,b,c;
    int aid, bid, cid;
    int ab,bc; // wire ids
    int gid;
    TestFixture(): lm(*this)
    {
      a=variables[aid=variables.newVariable("a", VariableType::flow)];
      b=variables[bid=variables.newVariable("b", VariableType::flow)];
      c=variables[cid=variables.newVariable("c", VariableType::flow)];
      // create 3 variables, wire them and add first two to a group,
      // leaving 3rd external
      a->moveTo(100,100);
      b->moveTo(200,100);
      c->moveTo(300,100);
      ab=PortManager::addWire(Wire(a->outPort(),b->inPort()));
      bc=PortManager::addWire(Wire(b->outPort(),c->inPort()));

      // add a couple of time operators, to ensure the group has finite size
      operations[addOperation("time")]->moveTo(100,75);
      operations[addOperation("time")]->moveTo(200,125);
      select(50,50,250,150);
      GroupIcon& g=groupItems[gid=createGroup()];      
      save("TestGroupFixture.mky");
      CHECK_EQUAL(2, g.variables().size());
      CHECK_EQUAL(1, g.wires().size());
      CHECK_EQUAL(g.id(), a->group);
      CHECK(!a->visible);
      CHECK_EQUAL(g.id(), b->group);
      CHECK(!b->visible); 
      CHECK_EQUAL(-1, c->group);
      CHECK(c->visible);
      CHECK(!wires[ab].visible);
      CHECK(wires[bc].visible);
      CHECK_EQUAL(ab, g.wires()[0]); 
      CHECK(uniqueGroupMembership());
    }
    bool uniqueGroupMembership() const
    {
      return groupItems.uniqueGroupMembership();
    }
  };
}

SUITE(Group)
{
  TEST_FIXTURE(TestFixture, AddVariable)
  {
    GroupIcon& g=groupItems[gid];
    addVariableToGroup(gid, variables.getIDFromVariable(c));
    CHECK_EQUAL(3, g.variables().size());
    CHECK_EQUAL(gid, c->group);
    CHECK_EQUAL(variables.getIDFromVariable(c), g.variables().back());
    CHECK_EQUAL(2, g.wires().size());
    CHECK_EQUAL(bc, g.wires().back());
    CHECK(!wires[ab].visible);
    CHECK(!wires[bc].visible);
    
    CHECK(uniqueGroupMembership());

    // now check removal
    removeVariableFromGroup(gid, variables.getIDFromVariable(a));
    CHECK_EQUAL(2, g.variables().size());
    CHECK_EQUAL(-1, a->group);
    CHECK_EQUAL(1, g.wires().size());
    CHECK_EQUAL(bc, g.wires().back());
    CHECK(wires[ab].visible);
    CHECK(!wires[bc].visible);
    CHECK(uniqueGroupMembership());
  }

  TEST_FIXTURE(TestFixture, AddOperation)
  {
    int addOpId=addOperation("add");
    int timeOpId=addOperation("time");
    CHECK(addOpId >= 0);
    CHECK(timeOpId >= 0);
    OperationPtr addOp=operations[addOpId];
    OperationPtr timeOp=operations[timeOpId];
    addOp->moveTo(150,100);
    timeOp->moveTo(250,100);


    int timeA=PortManager::addWire(Wire(timeOp->ports()[0], a->inPort()));
    int bAdd=PortManager::addWire(Wire(b->outPort(), addOp->ports()[1]));

    GroupIcon& g=groupItems[gid];

    CHECK_EQUAL(1, g.wires().size());
    addOperationToGroup(gid, addOpId);
    CHECK_EQUAL(3, g.operations().size());
    CHECK_EQUAL(addOpId, g.operations().back());
    CHECK_EQUAL(2, g.wires().size());
    CHECK_EQUAL(bAdd,  g.wires().back());
    CHECK_EQUAL(2, g.variables().size());
    CHECK(!addOp->visible);
    CHECK(timeOp->visible);
    CHECK_EQUAL(gid,addOp->group);
    CHECK_EQUAL(-1,timeOp->group);
    CHECK(!wires[bAdd].visible);
    CHECK(wires[timeA].visible);

    CHECK(uniqueGroupMembership());

    addOperationToGroup(gid, timeOpId);
    CHECK_EQUAL(4, g.operations().size());
    CHECK_EQUAL(timeOpId, g.operations().back());
    CHECK_EQUAL(3, g.wires().size());
    CHECK_EQUAL(timeA,  g.wires().back());
    CHECK_EQUAL(2, g.variables().size());
    CHECK(!addOp->visible);
    CHECK(!timeOp->visible);
    CHECK_EQUAL(gid,addOp->group);
    CHECK_EQUAL(gid,timeOp->group);
    CHECK(!wires[bAdd].visible);
    CHECK(!wires[timeA].visible);
    CHECK(uniqueGroupMembership());
   
    removeOperationFromGroup(gid, addOpId);
    CHECK_EQUAL(3, g.operations().size());
    CHECK_EQUAL(timeOpId, g.operations().back());
    CHECK_EQUAL(2, g.wires().size());
    CHECK_EQUAL(timeA,  g.wires().back());
    CHECK_EQUAL(2, g.variables().size());
    CHECK(addOp->visible);
    CHECK(!timeOp->visible);
    CHECK_EQUAL(-1,addOp->group);
    CHECK_EQUAL(gid,timeOp->group);
    CHECK(wires[bAdd].visible);
    CHECK(!wires[timeA].visible);
    CHECK(uniqueGroupMembership());
  }

  TEST_FIXTURE(TestFixture, AddGroup)
  {
    GroupIcon& g=groupItems[gid];
    select(250,50,350,150);
    GroupIcon& g1=groupItems[createGroup()];
    addGroupToGroup(gid,g1.id());
    CHECK(g1.visible==g.displayContents());
    CHECK_EQUAL(1,g.groups().size());
    CHECK_EQUAL(g1.id(),g.groups()[0]);
    CHECK_EQUAL(g.localZoom(), g1.zoomFactor);
    CHECK(uniqueGroupMembership());

    removeGroupFromGroup(gid,g1.id());
    CHECK(g1.visible);
    CHECK_EQUAL(0,g.groups().size());
    CHECK_EQUAL(zoomFactor(), g1.zoomFactor);
    CHECK(uniqueGroupMembership());
  }


  TEST_FIXTURE(TestFixture, NestedGroupUngroup)
  {
    GroupIcon& g=groupItems[gid];
    GroupIcon& g1=groupItems[copyGroup(gid)];
    CHECK(variables.noMultipleWiredInputs());
    g1.moveTo(300,300);
    select(300,0,450,400);
    save("B4createGroup.mky");
    GroupIcon& g2=groupItems[createGroup()];
    CHECK_EQUAL(1,g2.variables().size());
    CHECK_EQUAL(1,g2.numPorts());
    CHECK_EQUAL(0,g2.operations().size());
    CHECK_EQUAL(1,g2.groups().size());
    CHECK_EQUAL(g1.id(),g2.groups()[0]);
    CHECK_EQUAL(0,g2.wires().size()); 
    CHECK(uniqueGroupMembership());
    save("B4NestedGroupUngroup.mky");

    
    // now ungroup g1
    zoom(g2.x(),g2.y(),1.1*g2.computeDisplayZoom()/g2.zoomFactor);
    ungroup(g1.id());
    CHECK_EQUAL(0,g2.groups().size());
    CHECK_EQUAL(3, g2.variables().size());
    CHECK_EQUAL(2,g2.operations().size());
    CHECK_EQUAL(0,g2.wires().size());
    CHECK(uniqueGroupMembership());

    save("B4NestedGroup.mky");

    // now try to regroup within g2
    float b0,b1,b2,b3;
    g2.contentBounds(b0,b1,b2,b3);

    clearSelection();
    g2.select(currentSelection, b0, b1, b2, b3);
    GroupIcon& g3=groupItems[createGroup()]; 
    addGroupToGroup(g2.id(), g3.id());
    save("NestedGroupUngroup.mky");
    //TODO: add some wires connecting variables to make the test a bit more demanding
    CHECK_EQUAL(2,g3.variables().size());
    CHECK_EQUAL(2,g3.operations().size());
    CHECK_EQUAL(0,g3.wires().size());
    CHECK_EQUAL(1,g2.variables().size());
    CHECK_EQUAL(0,g2.operations().size());
    CHECK_EQUAL(0,g2.wires().size());
    CHECK_EQUAL(1,g2.groups().size());
    CHECK(uniqueGroupMembership());
  }

  // test that the new IO variables created do not introduce
  // extraneous references
  TEST_FIXTURE(Minsky, NewIOVariables)
  {
    LocalMinsky lm(*this);
    OperationPtr& a=operations[addOperation("exp")]; a->moveTo(100,100);
    OperationPtr& b=operations[addOperation("exp")]; b->moveTo(200,100);
    OperationPtr& c=operations[addOperation("exp")]; c->moveTo(300,100);
    OperationPtr& d=operations[addOperation("exp")]; d->moveTo(100,200);
    OperationPtr& e=operations[addOperation("exp")]; e->moveTo(200,200);
    OperationPtr& f=operations[addOperation("exp")]; f->moveTo(300,200);
    // following assume ports are allocated in triplets to each operation:
    // [out, in, opid]
    PortManager::addWire(Wire(a->ports()[0],b->ports()[1]));
    PortManager::addWire(Wire(b->ports()[0],c->ports()[1]));
    PortManager::addWire(Wire(d->ports()[0],e->ports()[1]));
    PortManager::addWire(Wire(e->ports()[0],f->ports()[1]));
    select(150,50,250,150);
    createGroup();
    select(150,150,250,250);
    createGroup();
    garbageCollect();
    save("NewIOVariables.mky");
    // +2 to allow for the extra constant:zero and constant:one values
    CHECK_EQUAL(variables.size()+2, variables.values.size());
  }

  TEST_FIXTURE(TestFixture, scoping)
  {
    LocalMinsky lm(*this);
    // check that fully qualified names are correct
    string prefix="["+str(gid)+"]:", vpref=str(gid)+":";
    a->setScope(gid);
    b->setScope(gid);
    CHECK_EQUAL(prefix+"a",a->fqName());
    CHECK_EQUAL(prefix+"b",b->fqName());
    CHECK_EQUAL(":c",c->fqName());
    // variables are all local, so Name() should not be qualified
    CHECK_EQUAL("a",a->name());
    CHECK_EQUAL("b",b->name());
    CHECK_EQUAL("c",c->name());
    // and check valueIds
    CHECK_EQUAL(vpref+"a",a->valueId());
    CHECK_EQUAL(vpref+"b",b->valueId());
    CHECK_EQUAL(":c",c->valueId());

    addVariableToGroup(gid,cid);
    c->setScope(gid);
    removeVariableFromGroup(gid,aid);
    a->setScope(-1);

    CHECK_EQUAL(":a",a->fqName());
    CHECK_EQUAL(prefix+"b",b->fqName());
    CHECK_EQUAL(prefix+"c",c->fqName());
    // all variables are now local
    CHECK_EQUAL("a",a->name());
    CHECK_EQUAL("b",b->name());
    CHECK_EQUAL("c",c->name());
    // and check valueIds
    CHECK_EQUAL(":a",a->valueId());
    CHECK_EQUAL(vpref+"b",b->valueId());
    CHECK_EQUAL(vpref+"c",c->valueId());

    // now name group
    groupItems[gid].setName("foo");
    CHECK_EQUAL(":a",a->fqName());
    CHECK_EQUAL("foo"+prefix+"b",b->fqName());
    CHECK_EQUAL("foo"+prefix+"c",c->fqName());
    // variables are all local, so Name() should not be qualified
    CHECK_EQUAL("a",a->name());
    CHECK_EQUAL("b",b->name());
    CHECK_EQUAL("c",c->name());
    // and check valueIds
    CHECK_EQUAL(":a",a->valueId());
    CHECK_EQUAL(vpref+"b",b->valueId());
    CHECK_EQUAL(vpref+"c",c->valueId());
    
    // now rename a variable to refer something different
    c->name(":c");
    a->name("foobar"+prefix+"a");

    CHECK_EQUAL("foo"+prefix+"a",a->fqName());
    CHECK_EQUAL(":c",c->fqName());
    // some variables are now not local, so Name() will be qualified
    CHECK_EQUAL("foo"+prefix+"a",a->name());
    CHECK_EQUAL(":c",c->name());
    // and check valueIds
    CHECK_EQUAL(vpref+"a",a->valueId());
    CHECK_EQUAL(":c",c->valueId());
    
    // test scoping in a Godley table
    int godleyId=addGodleyTable(0,0);
    GodleyIcon& godley=godleyItems[godleyId];
    godley.table.resize(4,3);
    godley.table.cell(0,1)="local";
    godley.table.cell(0,2)=":global";
    // row 1 is initial conditions
    godley.table.cell(2,1)="l1";
    godley.table.cell(2,2)=a->fqName();
    godley.table.cell(3,1)=b->fqName();
    godley.table.cell(3,2)=c->fqName();
    godley.update();

    CHECK_EQUAL(2, godley.stockVars.size());
    CHECK_EQUAL(-1, godley.stockVars[0]->scope());
    CHECK_EQUAL(-1, godley.stockVars[1]->scope());
    CHECK_EQUAL(4, godley.flowVars.size());
    CHECK_EQUAL(-1, godley.flowVars[0]->scope());
    CHECK_EQUAL(a->scope(), godley.flowVars[1]->scope());
    CHECK_EQUAL(b->scope(), godley.flowVars[2]->scope());
    CHECK_EQUAL(c->scope(), godley.flowVars[3]->scope());

    // variable cloning
    int caid=copyVariable(aid);
    CHECK_EQUAL(a->scope(),variables[caid]->scope());
    CHECK_EQUAL(-1,variables[caid]->group);
    int cbid=copyVariable(bid);
    VariablePtr cb=variables[cbid];
    CHECK_EQUAL(b->scope(),variables[cbid]->scope());
    CHECK_EQUAL(-1,variables[cbid]->group);
    int ccid=copyVariable(cid);
    CHECK_EQUAL(c->scope(),variables[ccid]->scope());
    CHECK_EQUAL(-1,variables[ccid]->group);
  }

  TEST(inGroup)
  {
    IntrusiveMap<int, GroupIcon> groups;
    {
      GroupIcon& g=groups[0];
      g.moveTo(50,50);
      g.width=g.height=50;
    }
    {
      GroupIcon& g=groups[1];
      g.moveTo(100,100);
      g.width=g.height=100;
    }
    groups[0].addGroup(groups[1]);
    {
      GroupIcon& g=groups[2];
      g.moveTo(200,200);
      g.width=g.height=100;
    }
    InGroup igt;
    igt.initGroupList(groups);
    CHECK_EQUAL(0, igt.containingGroup(40,40));
    CHECK_EQUAL(1, igt.containingGroup(110,110));
    CHECK_EQUAL(2, igt.containingGroup(160,160));
    CHECK_EQUAL(-1, igt.containingGroup(200,100));
    
  }

}
