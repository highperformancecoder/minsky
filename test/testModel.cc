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

#include "cairoItems.h"
#include "operation.h"
#include "group.h"
#include "minsky.h"
#include "godleyTableWindow.h"
#include "minsky_epilogue.h"

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
      a=model->addItem(new Variable<VariableType::flow>("a"));
      b=model->addItem(new Variable<VariableType::flow>("b"));
      c=model->addItem(new Variable<VariableType::flow>("c"));
      // create 3 variables, wire them and add first two to a group,
      // leaving 3rd external
      a->moveTo(100,100);
      b->moveTo(200,100);
      c->moveTo(300,100);
      CHECK_EQUAL(2,a->portsSize());
      CHECK_EQUAL(2,b->portsSize());
      CHECK_EQUAL(2,c->portsSize());

      ab=model->addWire(new Wire(a->ports(0), b->ports(1)));
      bc=model->addWire(new Wire(b->ports(0), c->ports(1)));
      checkWiresConsistent();

      group0=model->addGroup(new Group);
      checkWiresConsistent();
      group0->addItem(a);
      checkWiresConsistent();
      group0->addItem(b);
      group0->splitBoundaryCrossingWires();
     
      CHECK_EQUAL(3,group0->items.size());
      CHECK_EQUAL(2,group0->wires.size());
      CHECK_EQUAL(1,model->items.size());
      CHECK_EQUAL(4,model->numItems());
      CHECK_EQUAL(3,model->numWires());
      checkWiresConsistent();

      // add a couple of time operators, to ensure the group has finite size
      model->addItem(OperationPtr(OperationType::time))->moveTo(100,75);
      model->addItem(OperationPtr(OperationType::time))->moveTo(200,125);
      canvas.select({50,50,250,150});
      save("TestGroupFixture.mky");
      CHECK(model->uniqueItems());
    }

    void checkWiresConsistent() {
      for (auto& i: model->items)
        for (size_t pi=0; pi<i->portsSize(); ++pi)
          if (auto p=i->ports(pi).lock())
            for (auto& w: p->wires())
              {
                CHECK(w);
                CHECK(p->input() || p==w->from());
                CHECK(!p->input() || p==w->to());
              }
      for (auto& g: model->groups)
        for (auto& i: g->items)
          for (size_t pi=0; pi<i->portsSize(); ++pi)
            if (auto p=i->ports(pi).lock())
              for (auto& w: p->wires())
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
  TEST_FIXTURE(TestFixture, accessibleVars)
    {
      vector<string> globalAccessibleVars{"c"};
      vector<string> group0AccessibleVars{":c","a","b","output1"};
      group0->makeSubroutine();
      CHECK_EQUAL(globalAccessibleVars.size(), model->accessibleVars().size());
      CHECK_ARRAY_EQUAL(globalAccessibleVars, model->accessibleVars(), globalAccessibleVars.size());
      CHECK_EQUAL(group0AccessibleVars.size(), group0->accessibleVars().size());
      CHECK_ARRAY_EQUAL(group0AccessibleVars, group0->accessibleVars(), group0AccessibleVars.size());
      CHECK_EQUAL(group0AccessibleVars.size(), a->variableCast()->accessibleVars().size());
      CHECK_ARRAY_EQUAL(group0AccessibleVars, a->variableCast()->accessibleVars(), group0AccessibleVars.size());
    }

  TEST_FIXTURE(TestFixture, makeSubroutine)
    {
      group0->makeSubroutine();
      for (auto& i: group0->items)
        if (auto v=i->variableCast())
          CHECK(v->rawName()[0]!=':');
    }
  
    TEST_FIXTURE(TestFixture, SelectGroup)
    {
      auto& g=*model->addGroup(new Group);
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
      model->removeGroup(g); // why is this needed???
    }

  TEST_FIXTURE(TestFixture, addVariable)
    {
      group0->addItem(c);
      CHECK(model->uniqueItems());
      CHECK_EQUAL(4,group0->items.size());
      CHECK_EQUAL(2,model->items.size());
      CHECK_EQUAL(3,group0->wires.size());
      CHECK_EQUAL(0,model->wires.size());
    
      checkWiresConsistent();

      // now check removal
      group0->group.lock()->addItem(c);

      CHECK_EQUAL(3,group0->items.size());
      CHECK_EQUAL(2,group0->wires.size());
      CHECK_EQUAL(3,model->items.size());
      CHECK_EQUAL(1,model->wires.size());
    }

  TEST_FIXTURE(TestFixture, addIntegral)
    {
      unsigned numItems=model->numItems();
      auto integ=make_shared<IntOp>();
      model->addItem(integ);
      CHECK_EQUAL(numItems+2,model->numItems());
      CHECK(integ->group.lock()==model);
      CHECK(integ->intVar->group.lock()==model);
      group0->addItem(integ);
      CHECK(integ->group.lock()==group0);
      CHECK(integ->intVar->group.lock()==group0);
    }
  
  TEST_FIXTURE(TestFixture, addBookmark)
    {
      model->addBookmark("bookmark0");
      CHECK_EQUAL("bookmark0",model->bookmarkList()[model->bookmarks.size()-1]);
      CHECK_EQUAL(1,model->bookmarks.size());
      auto& b0=*model->bookmarks.begin();
      double x0 =b0.x, y0=b0.y;	  
      model->moveTo(100,100);
      model->addBookmark("bookmark1");
      CHECK_EQUAL("bookmark1",model->bookmarkList().back());
      CHECK_EQUAL(2,model->bookmarks.size());
      auto& b1=*model->bookmarks.find(Bookmark("bookmark1"));
      double x1=b1.x,y1=b1.y;
      CHECK_EQUAL(x1,model->x());
      CHECK_EQUAL(y1,model->y());
      model->moveTo(200,200);
      model->addBookmark("bookmark2");
      CHECK_EQUAL("bookmark2",model->bookmarkList().back());
      CHECK_EQUAL(3,model->bookmarks.size());
      auto& b2=*model->bookmarks.find(Bookmark("bookmark2"));
      double x2=b2.x,y2=b2.y;
      CHECK_EQUAL(x2,model->x());
      CHECK_EQUAL(y2,model->y());
      model->moveTo(300,300);
      model->addBookmark("bookmark3");
      CHECK_EQUAL("bookmark3",model->bookmarkList().back());
      CHECK_EQUAL(4,model->bookmarks.size());
      auto& b3=*model->bookmarks.find(Bookmark("bookmark3"));
      double x3=b3.x,y3=b3.y;
      CHECK_EQUAL(x3,model->x());
      CHECK_EQUAL(y3,model->y());	  
      model->gotoBookmark(0);
      CHECK_EQUAL(x0,model->x());
      CHECK_EQUAL(y0,model->y());
      CHECK(x0!=x1 && y0!=y1 && x0!=x2 && y0!=y2 && x0!=x3 && y0!=y3);
      model->deleteBookmark(model->bookmarks.size()-1);
      model->deleteBookmark(model->bookmarks.size()-1);
      model->deleteBookmark(model->bookmarks.size()-1);
      model->deleteBookmark(model->bookmarks.size()-1);
      CHECK_EQUAL(0,model->bookmarks.size()); 	   	          
    }  
  
  // check that removing then adding an item leaves the group idempotent
  TEST_FIXTURE(TestFixture, removeAddItem)
    {
      CHECK_EQUAL(1,group0->createdIOvariables.size());
      CHECK_EQUAL(3,model->items.size());
      model->addItem(a);
      group0->splitBoundaryCrossingWires();
      save("x1.mky");
      CHECK_EQUAL(3,group0->items.size()); // extra io var created
      CHECK_EQUAL(4,model->items.size());
      CHECK_EQUAL(2,group0->createdIOvariables.size());
      CHECK_EQUAL(4,model->numWires());
      group0->addItem(a);
      group0->splitBoundaryCrossingWires();
      CHECK_EQUAL(3,group0->items.size());
      CHECK_EQUAL(3,model->items.size());
      CHECK_EQUAL(1,group0->createdIOvariables.size());
      CHECK_EQUAL(3,model->numWires());
      CHECK_EQUAL(3,group0->items.size());
    }

  TEST_FIXTURE(TestFixture, displayPlot)
    {
      auto plot=new PlotWidget;
      group0->addItem(plot);
      plot->makeDisplayPlot();
      CHECK(group0->displayPlot.get()==plot);
      group0->removeDisplayPlot();
      CHECK(!group0->displayPlot.get());
    }

  TEST_FIXTURE(TestFixture, findGroup)
    {
      CHECK(model->findGroup(*group0)==group0);
    }

  TEST_FIXTURE(TestFixture, copy)
    {
      auto t=model->addItem(new minsky::Time);
      model->addWire(new Wire(t->ports(0),a->ports(1)));
      group0->splitBoundaryCrossingWires();
      group0->addGroup(new Group);
      group0->title="group0";
      auto g=model->addGroup(group0->copy());
      g->title="g";
      // set same zoom factors to ensure copied items have same zooms
      group0->setZoom(1);
      g->setZoom(1);
      // force rendering to ensure everything is placed the same
      group0->updateBoundingBox();
      g->updateBoundingBox();
      CHECK_CLOSE(group0->bb.width(), g->bb.width(), 1e-2);
      CHECK_CLOSE(group0->bb.height(), g->bb.height(), 1e-2);
      CHECK_CLOSE(group0->x(),g->x(), 1e-2);
      CHECK_CLOSE(group0->y(),g->y(), 1e-2);
      CHECK_ARRAY_CLOSE(group0->cBounds().data(), g->cBounds().data(), 4, 1e-2);

      save("copy.mky");
      
      CHECK_EQUAL(group0->items.size(), g->items.size());
      CHECK_EQUAL(group0->inVariables.size(), g->inVariables.size());
      CHECK_EQUAL(group0->outVariables.size(), g->outVariables.size());
      CHECK(g->createdIOvariables.empty());
      CHECK_EQUAL(group0->wires.size(), g->wires.size());
      CHECK_EQUAL(group0->groups.size(), g->groups.size());
      CHECK_CLOSE(group0->x(),g->x(),1e-2);
      CHECK_CLOSE(group0->y(),g->y(),1e-2);
      
      for (size_t i=0; i<group0->items.size(); i++)
        {
          CHECK(group0->items[i]!=g->items[i]);
          CHECK_EQUAL(group0->items[i]->classType(),g->items[i]->classType());
          CHECK_CLOSE(group0->items[i]->x(),g->items[i]->x(),1e-2);
          CHECK_CLOSE(group0->items[i]->y(),g->items[i]->y(),1e-2);
        }

      for (size_t i=0; i<group0->inVariables.size(); i++)
        {
          CHECK(group0->inVariables[i]!=g->inVariables[i]);
          CHECK_CLOSE(group0->inVariables[i]->x(), g->inVariables[i]->x(),1e-2);
          CHECK_CLOSE(group0->inVariables[i]->y(), g->inVariables[i]->y(),1e-2);
        }
      for (size_t i=0; i<group0->outVariables.size(); i++)
        {
          CHECK(group0->outVariables[i]!=g->outVariables[i]);
          CHECK_CLOSE(group0->outVariables[i]->x(), g->outVariables[i]->x(),1e-2);
          CHECK_CLOSE(group0->outVariables[i]->y(), g->outVariables[i]->y(),1e-2);
        }
        
      for (size_t i=0; i<group0->wires.size(); i++)
        {
          auto w1=group0->wires[i], w2=g->wires[i];
          CHECK(w1!=w2);
          CHECK(w1->to()->item().group.lock()==group0);
          CHECK(w1->from()->item().group.lock()==group0);
          CHECK(w2->to()->item().group.lock()==g);
          CHECK(w2->from()->item().group.lock()==g);
          auto c1=w1->coords(), c2=w2->coords();
          CHECK_EQUAL(c1.size(), c2.size());
          CHECK_ARRAY_CLOSE(c1.data(), c2.data(), c1.size(), 1e-2);
        }
      for (size_t i=0; i<group0->groups.size(); i++)
        {
          CHECK(group0->groups[i]!=g->groups[i]);
        }
    }

  
  TEST_FIXTURE(Group, GroupRecursiveDo)
    {
      addItem(new Operation<OperationType::exp>);
      auto g=addGroup(new Group);
      g->addItem(new Operation<OperationType::ln>);
      CHECK(recursiveDo(&GroupItems::items,[&](Items&,Items::iterator i)
                                           {return dynamic_cast<Operation<OperationType::exp>*>(i->get());}));
      CHECK(recursiveDo(&GroupItems::items,[&](Items&,Items::iterator i)
                                           {return dynamic_cast<Operation<OperationType::ln>*>(i->get());}));
      CHECK(!recursiveDo(&GroupItems::items,[&](Items&,Items::iterator i)
                                            {return dynamic_cast<Operation<OperationType::add>*>(i->get());}));
    }
  
  TEST_FIXTURE(TestFixture, removeGroup)
    {
      auto g=model->removeGroup(*group0);
      CHECK(g==group0);
      CHECK(find(model->groups.begin(),model->groups.end(),group0)==model->groups.end());
    }
   
  TEST_FIXTURE(TestFixture,moveContents)
    {
      group0->addItem(new Group);
      unsigned numItems=model->numItems();
      unsigned numGroups=model->numGroups();
      unsigned numWires=model->numWires();
      CHECK(!group0->empty());
      model->moveContents(*group0);
      CHECK(group0->empty());
      CHECK_EQUAL(numItems, model->numItems());
      CHECK_EQUAL(numGroups, model->numGroups());
      CHECK_EQUAL(numWires, model->numWires());
      CHECK_THROW(group0->moveContents(*model), error);
    }

  TEST_FIXTURE(Group, checkAddIORegion)
    {
      CHECK_EQUAL(IORegion::input, inIORegion(x()-0.5*iWidth()*zoomFactor(), y()));
      CHECK_EQUAL(IORegion::output, inIORegion(x()+0.5*iWidth()*zoomFactor(), y()));
      VariablePtr inp(VariableType::flow,"input");
      VariablePtr outp(VariableType::flow,"output");
      inp->moveTo(x()-0.5*iWidth()*zoomFactor(), y());
      addItem(inp);
      checkAddIORegion(inp);
      outp->moveTo(x()+0.5*iWidth()*zoomFactor(), y());
      addItem(outp);
      checkAddIORegion(outp);
      CHECK_EQUAL(1,inVariables.size());
      CHECK_EQUAL(1,outVariables.size());
      CHECK_EQUAL("input",inVariables[0]->name());
      CHECK_EQUAL("output",outVariables[0]->name());
    }
    
}

SUITE(Canvas)
{
  TEST_FIXTURE(TestFixture, getItemAt)
    {
      // zoom to display group0 to make a & b visible
      canvas.item=group0;
      canvas.zoomToDisplay();
      canvas.getItemAt(a->x()+2,a->y()+2);
      CHECK(a==canvas.item);
      canvas.getItemAt(b->x()-2,b->y()-2);
      CHECK(b==canvas.item);
      canvas.getItemAt(c->x()-2,c->y()+2);
      CHECK(c==canvas.item);
      model->setZoom(1);
      canvas.getItemAt(group0->x()-2,group0->y()+2);
      CHECK(group0==canvas.item);
    }
  
  TEST_FIXTURE(TestFixture, getWireAt)
    {
      auto from=a->ports(0).lock(), to=b->ports(1).lock();
      float x=0.5f*(from->x()+to->x())+1;
      float y=0.5f*(from->y()+to->y())+1;
      canvas.getWireAt(x,y);
      CHECK(canvas.wire==ab);
    }
  
  TEST_FIXTURE(Canvas,findVariableDefinition)
    {
      model=cminsky().model;
      VariablePtr var1(VariableType::flow,"foo");
      VariablePtr var2(VariableType::flow,"foo");
      OperationPtr op(OperationType::exp);      
      item=model->addItem(var1);
      model->addItem(var2);
      model->addItem(op);
      // initially, foo is undefined, so should return false
      CHECK(!findVariableDefinition());
      CHECK(item==var1);
      model->addWire(new Wire(op->ports(0),var2->ports(1)));
      CHECK(findVariableDefinition());
      CHECK(item==var2);
      model->removeItem(*var2);

      shared_ptr<IntOp> integ(new IntOp);
      integ->description("foo");
      model->addItem(integ);
      item=var1;
      CHECK(findVariableDefinition());
      CHECK(item==integ);
      model->removeItem(*integ);
      integ->removeControlledItems();
      
      shared_ptr<GodleyIcon> godley(new GodleyIcon);
      model->addItem(godley);
      godley->table.resize(3,2);
      godley->table.cell(0,1)="foo";
      godley->table.cell(0,2)="bar";
      godley->update();
      item=var1;
      CHECK(findVariableDefinition());
      CHECK(item==godley);

      // on a non-variable, findVariableDefinition should returns false
      CHECK(!findVariableDefinition());

    }

    TEST_FIXTURE(TestFixture,moveItem)
    {
      cairo::Surface surf(cairo_recording_surface_create(CAIRO_CONTENT_COLOR,nullptr));
      c->draw(surf.cairo());// reposition ports
      CHECK(c->clickType(c->x(),c->y()) == ClickType::onItem); 
      canvas.mouseDown(c->x(),c->y());
      canvas.mouseUp(400,500);
      CHECK_EQUAL(400,c->x());
      CHECK_EQUAL(500,c->y());
    }
    
    TEST_FIXTURE(TestFixture,resizeVariable)
    {
      c->moveTo(400,300);
      c->updateBoundingBox();
      cairo::Surface surf(cairo_recording_surface_create(CAIRO_CONTENT_COLOR,nullptr));
      c->draw(surf.cairo());// reposition ports
      float xc=c->right(), yc=c->bottom();      
      CHECK(c->clickType(xc,yc) == ClickType::onResize);
      canvas.mouseDown(xc,yc);
      canvas.mouseUp(600,800);
      CHECK_CLOSE(600, c->right(),4*portRadiusMult);
      CHECK_CLOSE(800, c->bottom(),4*portRadiusMult);
    }    

    TEST_FIXTURE(TestFixture,resizeOperation)
    {
      OperationPtr add(OperationType::add);
      model->addItem(add);
      add->moveTo(400,300);
      cairo::Surface surf(cairo_recording_surface_create(CAIRO_CONTENT_COLOR,nullptr));
      add->draw(surf.cairo());// reposition ports
      float xc=add->right(), yc=add->bottom();      
      CHECK(add->clickType(xc,yc) == ClickType::onResize); 
      canvas.mouseDown(xc,yc);
      canvas.mouseUp(600,800);
      CHECK_CLOSE(600,add->right(),4*portRadiusMult);
      CHECK_CLOSE(800,add->bottom(),4*portRadiusMult);
    }    

    TEST_FIXTURE(TestFixture,onSlider)
    {
      auto cc=model->addItem(new Variable<VariableType::flow>("cc"));
      cc->moveTo(500,300);
      auto cv=dynamic_cast<VariableBase*>(cc.get());
      cv->value(1000);
      cv->sliderMin=0;
      cv->sliderMax=2000;
      // work out where slider is located
      RenderVariable rv(*cv);
      float xc=cv->x()+rv.handlePos(), yc=cv->y()-rv.height();
      CHECK_EQUAL(ClickType::onSlider, cv->clickType(xc,yc));
      canvas.mouseDown(xc,yc);
      xc+=5;
      canvas.mouseUp(xc,yc);
      // check handle and value changed
      CHECK_CLOSE(xc, cv->x()+rv.handlePos(), 0.1);
      CHECK(cv->value()>1000);
      // check variable hasn't moved
      CHECK_EQUAL(500,cv->x());
      CHECK_EQUAL(300,cv->y());

      // now check that value is clamped to max/min
      canvas.mouseDown(xc,yc);
      canvas.mouseUp(xc+100,yc);
      CHECK_EQUAL(cv->sliderMax, cv->value());
      xc=cv->x()+rv.handlePos();
      canvas.mouseDown(xc,yc);
      canvas.mouseUp(xc-100,yc);
      CHECK_EQUAL(cv->sliderMin, cv->value());
    }

    TEST_FIXTURE(TestFixture,lasso)
    {
      canvas.selection.clear();
      canvas.mouseDown(250,0); //Adjusted for new shape of operation icons. For ticket 362.
      canvas.mouseUp(350,150);
      CHECK_EQUAL(1,canvas.selection.items.size());
      CHECK(find(canvas.selection.items.begin(),canvas.selection.items.end(),c) !=canvas.selection.items.end());

      // now check when the first click is in the bounding box of an icon, but outside it
      OperationPtr op(OperationType::time);
      model->addItem(op);
      op->moveTo(500,500);
      float x=524, y=524;               // adjusted for 2*portRadius near corners, for feature 94
      CHECK(op->contains(x-12,y-12));
      CHECK_EQUAL(ClickType::outside, op->clickType(x,y));
      canvas.selection.clear();
      canvas.mouseDown(x,y);
      canvas.mouseUp(x-17,y-17);
      CHECK_EQUAL(1,canvas.selection.items.size());
      CHECK(find(canvas.selection.items.begin(),canvas.selection.items.end(),op) !=canvas.selection.items.end());

      group0->updateBoundingBox(); //why? for Travis.
      
      // test that groups can be selected
      CHECK(!group0->displayContents());
      float w=0.5*group0->width()+10, h=0.5*group0->height()+10;
      x=group0->x()-w; y=group0->y()-h;
      // nw -> se selection
      canvas.mouseDown(x,y);
      canvas.mouseUp(x+2*w,y+2*h);
      CHECK_EQUAL(0,canvas.selection.items.size());
      CHECK_EQUAL(1,canvas.selection.groups.size());
      CHECK(find(canvas.selection.groups.begin(),canvas.selection.groups.end(),group0) !=canvas.selection.groups.end());

      // ne -> sw selection
      canvas.selection.clear();
      canvas.mouseDown(x+2*w,y);
      canvas.mouseUp(x-2*w,y+2*h);
      CHECK_EQUAL(0,canvas.selection.items.size());
      CHECK_EQUAL(1,canvas.selection.groups.size());
      CHECK(find(canvas.selection.groups.begin(),canvas.selection.groups.end(),group0) !=canvas.selection.groups.end());
      
      // se -> nw selection
      canvas.selection.clear();
      canvas.mouseDown(x+2*w,y+2*h);
      canvas.mouseUp(x,y);
      CHECK_EQUAL(0,canvas.selection.items.size());
      CHECK_EQUAL(1,canvas.selection.groups.size());
      CHECK(find(canvas.selection.groups.begin(),canvas.selection.groups.end(),group0) !=canvas.selection.groups.end());
      
      // sw -> ne selection
      canvas.selection.clear();
      canvas.mouseDown(x,y+2*h);
      canvas.mouseUp(x+2*w,y-2*h);
      CHECK_EQUAL(0,canvas.selection.items.size());
      CHECK_EQUAL(1,canvas.selection.groups.size());
      CHECK(find(canvas.selection.groups.begin(),canvas.selection.groups.end(),group0) !=canvas.selection.groups.end());
    }

    TEST_FIXTURE(Canvas, wires)
      {
        model.reset(new Group);
        OperationPtr a(OperationType::exp);
        model->addItem(a);
        a->moveTo(100,100);
        OperationPtr b(OperationType::exp);
        model->addItem(b);
        b->moveTo(200,200);
        WirePtr w(new Wire(a->ports(0),b->ports(1)));
        model->addWire(w);
        auto coords=w->coords();
        CHECK_EQUAL(4, coords.size());

        // drag up, should insert a handle
        float x=0.5*(coords[0]+coords[2]), y=0.5*(coords[1]+coords[3]);
        mouseDown(x,y);
        x+=10;
        mouseUp(x,y);
        coords=w->coords();
        CHECK_EQUAL(6, coords.size());
        CHECK_EQUAL(x,coords[2]);
        CHECK_EQUAL(y,coords[3]);
      }

    TEST_FIXTURE(Canvas, godleyResize)
      {
        model.reset(new Group);
        addGodley();
        auto i=itemFocus;
        double w=i->width(), h=i->height();
        double x=i->x(), y=i->y();

        mouseDown(x+0.5*w, y+0.5*h);
        mouseUp(x+w, y+h);
        CHECK_CLOSE(1.5*w,i->width(),0.1);
        CHECK_CLOSE(1.5*h,i->height(),0.1);
      }

    TEST_FIXTURE(Canvas, groupResize)
      {
        model.reset(new Group);
        model->self=model;
        addGroup();
        auto& group=dynamic_cast<Group&>(*itemFocus);
        group.updateBoundingBox();
        group.relZoom=0.5; // ensure displayContents is false
        double w=group.iWidth()*group.zoomFactor(), h=group.iHeight()*group.zoomFactor();
        double x=group.x(), y=group.y(), z=group.relZoom;
        CHECK(group.clickType(group.right(),group.top()) == ClickType::onResize);

        mouseDown(group.right(), group.bottom());
        mouseUp(x+w, y+h);
        CHECK_CLOSE(x+w,group.right(),1);
        CHECK_CLOSE(y+h,group.bottom(),1); 
      }

    TEST_FIXTURE(Canvas, moveIntoThenOutOfGroup)
      {
        model.reset(new Group);
        model->self=model;
        OperationPtr a(OperationType::exp);
        model->addItem(a);
        a->moveTo(100,100);
        OperationPtr b(OperationType::exp);
        model->addItem(b);
        b->moveTo(200,200);
        model->addWire(*a,*b,1);

        auto g=model->addGroup(new Group);
        
        CHECK(b->group.lock()==model);
        CHECK_EQUAL(1,model->numWires());
        CHECK_EQUAL(2,model->numItems());
        CHECK_EQUAL(0,g->inVariables.size());

        // move b into group.
        mouseDown(b->x()+5,b->y()+5);   
        mouseUp(g->x()+5,g->y()+5);  // small offset added because resize handles grabbed otherwise, for feature 94. don't understand why?
        CHECK(b->group.lock()==g);
        CHECK_EQUAL(2,model->numWires());
        CHECK_EQUAL(3,model->numItems());
        CHECK_EQUAL(1,g->inVariables.size());

        // move b out of group
        item=g;
        zoomToDisplay();
        mouseDown(b->x()+5,b->y()+5);  
        mouseUp(200,200);
        CHECK(b->group.lock()==model);
        CHECK_EQUAL(1,model->numWires());
        CHECK_EQUAL(2,model->numItems());
        CHECK_EQUAL(0,g->inVariables.size());
      }
    
    TEST_FIXTURE(Canvas, mouseFocus)
      {
        model.reset(new Group);
        model->self=model;
        OperationPtr a(OperationType::exp);
        model->addItem(a);
        a->moveTo(100,100);
        a->ports(0).lock()->moveTo(110,100); // normally this is done inside draw()
        OperationPtr b(OperationType::exp);
        model->addItem(b);
        b->moveTo(200,200);
        b->ports(1).lock()->moveTo(190,200); // normally this is done inside draw()
        auto w=model->addWire(*a,*b,1);

        CHECK(!a->mouseFocus);
        CHECK(!b->mouseFocus);
        CHECK(!w->mouseFocus);

        mouseMove(a->x(),a->y());
        CHECK(a->mouseFocus);
        CHECK(!b->mouseFocus);
        CHECK(!w->mouseFocus);

        mouseMove(b->x(),b->y());
        CHECK(!a->mouseFocus);
        CHECK(b->mouseFocus);
        CHECK(!w->mouseFocus);

        mouseMove(0.5*(a->x()+b->x()),0.5*(a->y()+b->y()));
        CHECK(!a->mouseFocus);
        CHECK(!b->mouseFocus);
        CHECK(w->mouseFocus);
        
      }

    TEST_FIXTURE(Canvas, removeItemFromItsGroup)
      {
        model.reset(new Group);
        model->self=model;
        auto g0=model->addGroup(new Group);
        auto g1=g0->addGroup(new Group);
        auto a=g1->addItem(new Operation<OperationType::exp>);
        CHECK(a->group.lock()==g1);
        CHECK(a->visible()==g1->displayContents());
        item=a;
        removeItemFromItsGroup();
        CHECK(a->group.lock()==g0);
        CHECK(a->visible()==g0->displayContents());
        item=a;
        removeItemFromItsGroup();
        CHECK(a->group.lock()==model);
        CHECK(a->visible());
      }

    TEST_FIXTURE(TestFixture,selectAllVariables)
      {
        model->moveContents(*group0);
        canvas.item=a;
        canvas.copyItem();
        canvas.mouseUp(500,500);
        canvas.selectAllVariables();
        CHECK_EQUAL(2,canvas.selection.items.size());
        for (auto i: canvas.selection.items)
          {
            auto ii=dynamic_cast<VariableBase*>(i.get());
            CHECK(ii);
            if (ii) CHECK_EQUAL(dynamic_pointer_cast<VariableBase>(a)->valueId(), ii->valueId());
          }

        canvas.item=b;
        canvas.selectAllVariables();
        CHECK_EQUAL(1,canvas.selection.items.size());
        CHECK(canvas.selection.items[0]==b);

        canvas.item=group0;
        canvas.selectAllVariables();
        CHECK(canvas.selection.empty());

        // check integrals are selected too
        auto integ=new IntOp;
        canvas.item=model->addItem(integ);
        integ->description("foo");
        canvas.selectAllVariables();
        CHECK_EQUAL(1,canvas.selection.items.size());
        CHECK(canvas.selection.items[0]==integ->intVar);
      }
    
    TEST_FIXTURE(TestFixture,renameAllInstances)
      {
        
        model->moveContents(*group0);

        // add a top level Godley table
        GodleyIcon* gi=new GodleyIcon;
        model->addItem(gi);
        gi->table.resize(3,3);
        gi->table.cell(2,1)="a";
        gi->table.cell(2,2)=":a";
        gi->update();

        // add a copy of the above into the group
        auto gi2=new GodleyIcon;
        gi2->table=gi->table;
        group0->addItem(gi2);
        gi2->update();
        
        canvas.item=a;
        auto numItems=model->numItems();
        canvas.renameAllInstances("foobar");
        CHECK_EQUAL(numItems, model->numItems());
        unsigned count=0;
        for (auto i: model->items)
          if (auto v=dynamic_cast<VariableBase*>(i.get()))
            {
              CHECK(v->valueId()!=":a");
              if (v->valueId()==":foobar")
                count++;
            }
        CHECK_EQUAL(2,count); // should be 1 from first godley table, and the original a

        // check that the Godley table got updated
        CHECK_EQUAL("foobar",gi->table.cell(2,1));
        CHECK_EQUAL(":foobar",gi->table.cell(2,2));
        CHECK_EQUAL("a",gi2->table.cell(2,1)); // local var, not target of rename
        CHECK_EQUAL(":foobar",gi2->table.cell(2,2));

        
        // check no renaming should happen when item is not a variable 
        canvas.item=group0;
        canvas.renameAllInstances("foobar1");
        for (auto i: model->items)
          if (auto v=dynamic_cast<VariableBase*>(i.get()))
            CHECK(v->name()!="foobar1");
        
        // check integrals are renamed too
        auto integ=new IntOp;
        canvas.item=model->addItem(integ);
        integ->description("foo");
        canvas.renameAllInstances("bar");
        CHECK(integ->description()=="bar");
      }

    TEST_FIXTURE(TestFixture,ungroupItem)
      {
        unsigned originalNumItems=model->numItems();
        unsigned originalNumGroups=model->numGroups();
        canvas.item=group0;
        canvas.ungroupItem();
        CHECK_EQUAL(originalNumItems, model->numItems());
        CHECK_EQUAL(originalNumGroups-1, model->numGroups());
      }

    TEST_FIXTURE(TestFixture,copyItem)
      {
        model->addItem(a);
        unsigned originalNumItems=model->numItems();
        unsigned originalNumGroups=model->numGroups();
        canvas.item=a;
        canvas.copyItem();
        CHECK_EQUAL(originalNumItems+1, model->numItems());
        CHECK_EQUAL(dynamic_cast<VariableBase*>(a.get())->valueId(),
               dynamic_cast<VariableBase*>(model->items.back().get())->valueId());

        auto integ=new IntOp;
        canvas.item=model->addItem(integ);
        integ->description("foo");
        originalNumItems=model->numItems();
        canvas.copyItem();
        CHECK_EQUAL(originalNumItems+1, model->numItems());
        CHECK_EQUAL(integ->intVar->valueId(),
          dynamic_cast<VariableBase*>(model->items.back().get())->valueId());
      }

    TEST_FIXTURE(TestFixture,openGroupInCanvas)
      {
        // remove a from group0, which should add an invariable
        model->addItem(a);
        group0->splitBoundaryCrossingWires();
        CHECK(!group0->inVariables.empty());
        CHECK(!group0->outVariables.empty());
        canvas.openGroupInCanvas(group0);
        CHECK(canvas.model==group0);
        CHECK(group0->displayContents());
        canvas.openGroupInCanvas(model);
        CHECK(canvas.model==model);
      }

    TEST_FIXTURE(Canvas,copyVars)
      {
        model.reset(new Group);
        model->self=model;
        auto godley=new GodleyIcon;
        item=model->addItem(godley);

        // simple Godley table with 2 stock and 2 flow vars
        godley->table.resize(3,3);
        godley->table.cell(0,1)="foo";
        godley->table.cell(0,2)="bar";
        godley->table.cell(2,1)="x";
        godley->table.cell(2,2)="y";
        godley->update();

        unsigned originalNumItems=model->numItems();
        copyAllFlowVars();
        CHECK_EQUAL(originalNumItems+godley->flowVars().size(),model->numItems());
        // Check that the number of items in selection after copyAllFlowVars() is equal to the number of flowVars attached to the Godley Icon. For ticket 1039.
        CHECK_EQUAL(godley->flowVars().size(),selection.items.size());
        
        //Check that there are two copies of the flowVars orginally attached to the Godley Icon. For ticket 1039.
        map<string,int> idCnt;
        for (auto& i: model->items)
          if (auto v=i->variableCast())
             idCnt[v->valueId()]++;
        for (auto v: godley->flowVars())
        CHECK_EQUAL(2, idCnt[v->valueId()]);                
        
        originalNumItems=model->numItems();
        copyAllStockVars();
        CHECK_EQUAL(originalNumItems+godley->stockVars().size(),model->numItems());
        // Check that the number of items in selection after copyAllStockVars() is equal to the number of stockVars attached to the Godley Icon. For ticket 1039.
        CHECK_EQUAL(godley->stockVars().size(),selection.items.size());     
        
        //Check that there are two copies of the stockVars orginally attached to the Godley Icon. For ticket 1039.
        idCnt.clear();
        for (auto& i: model->items)
          if (auto v=i->variableCast())
             if (v->isStock()) idCnt[v->valueId()]++;
        for (auto v: godley->stockVars())
        CHECK_EQUAL(2, idCnt[v->valueId()]);    
      }

    TEST_FIXTURE(TestFixture,handleArrows)
      {
        auto cv=dynamic_cast<VariableBase*>(c.get());
        cv->value(1000);
        cv->sliderMin=0;
        cv->sliderMax=2000;
        cv->sliderStep=100;
        canvas.keyPress({0xff52,"",0,c->x(),c->y()});
        CHECK_EQUAL(1100,cv->value());
        canvas.keyPress({0xff51,"",0,c->x(),c->y()});
        CHECK_EQUAL(1000,cv->value());
        for (size_t i=0; i<20; ++i)
          canvas.keyPress({0xff52,"",0,c->x(),c->y()});
        CHECK_EQUAL(2000,cv->value());
        for (size_t i=0; i<30; ++i)
          canvas.keyPress({0xff51,"",0,c->x(),c->y()});
        CHECK_EQUAL(0,cv->value());
      }
    
     TEST_FIXTURE(TestFixture,selectVar)
      {
        CHECK(!group0->outVariables.empty());
        auto v=group0->outVariables[0];
        canvas.item=group0;
        CHECK(canvas.selectVar(v->x(), v->y()));
        CHECK(canvas.item==v);
        canvas.item=group0;
        CHECK(!canvas.selectVar(500,500));
        canvas.item.reset();
        CHECK(!canvas.selectVar(500,500));
      }

     TEST_FIXTURE(Canvas, addStuff)
       {
         model.reset(new Group);
         model->self=model;
         addVariable("foo",VariableType::flow);
         auto v=dynamic_cast<VariableBase*>(itemFocus.get());
         CHECK(v);
         CHECK_EQUAL("foo",v->name());
         CHECK(model==itemFocus->group.lock());

         addNote("some text");
         CHECK(itemFocus);
         CHECK_EQUAL("some text", itemFocus->detailedText);

         addGroup();
         CHECK(dynamic_cast<Group*>(itemFocus.get()));
         CHECK(model==itemFocus->group.lock());

         addSwitch();
         CHECK(dynamic_cast<SwitchIcon*>(itemFocus.get()));
         CHECK(model==itemFocus->group.lock());
       }

     TEST_FIXTURE(Canvas, groupSelection)
       {
         model.reset(new Group);
         model->self=model;
         auto t=model->addItem(new Operation<OperationType::time>);
         auto e=model->addItem(new Operation<OperationType::exp>);
         model->addWire(new Wire(t->ports(0),e->ports(1)));
         auto g=model->addGroup(new Group);
         selection.items.push_back(t);
         selection.items.push_back(e);
         selection.groups.push_back(g);
         groupSelection();
         auto newG=model->groups.back();
         CHECK(newG);
         CHECK_EQUAL(2,newG->items.size());
         CHECK_EQUAL(1,newG->groups.size());
         CHECK_EQUAL(1,newG->wires.size());
         CHECK(model->uniqueItems());
         CHECK(model->nocycles());
       }
}

SUITE(Wire)
{
  TEST(handles)
    {
      Operation<OperationType::sin> item;
      item.ports(0).lock()->moveTo(0,0);
      item.ports(1).lock()->moveTo(10,10);
      Wire wire(item.ports(0),item.ports(1),{0,0,3,4,6,7,10,10});
      CHECK_EQUAL(0, wire.nearestHandle(3.1, 3.9));
      CHECK_EQUAL(1, wire.nearestHandle(6.2, 7.5));
      // should not have inserted anything yet
      CHECK_EQUAL(8, wire.coords().size());

      // check for midpoint insertion
      CHECK_EQUAL(0, wire.nearestHandle(1.6, 2));
      CHECK_EQUAL(10, wire.coords().size());
      CHECK_CLOSE(1.5, wire.coords()[2],0.01);
      CHECK_CLOSE(2, wire.coords()[3],0.01);
      
      CHECK_EQUAL(2, wire.nearestHandle(4.5, 5.5));
      CHECK_EQUAL(12, wire.coords().size());
      CHECK_CLOSE(4.5, wire.coords()[6],0.01);
      CHECK_CLOSE(5.5, wire.coords()[7],0.01);

      CHECK_EQUAL(4, wire.nearestHandle(10, 10));
      CHECK_EQUAL(14, wire.coords().size());
      CHECK_CLOSE(8, wire.coords()[10],0.01);
      CHECK_CLOSE(8.5, wire.coords()[11],0.01);

      wire.editHandle(0,2.2,3.3);
      CHECK_CLOSE(2.2, wire.coords()[2],0.01);
      CHECK_CLOSE(3.3, wire.coords()[3],0.01);
    }
}

SUITE(GodleyIcon)
{
  TEST_FIXTURE(GodleyIcon, select)
    {
      GodleyIcon::svgRenderer.setResource("bank.svg");
      
      table.resize(3,2);
      table.cell(2,1)="flow1";
      table.cell(0,1)="stock1";
      toggleVariableDisplay();
      update();
      // TODO - shouldn't be needed, but there is some font problem causing bottomMargin to be calculated incorrectly
      
      scaleIcon(5*bottomMargin(),5*leftMargin());
      update();
      CHECK_EQUAL(1,flowVars().size());
      CHECK_EQUAL(1,stockVars().size());
      for (auto& i: flowVars())
          CHECK(i==select(i->x(),i->y()));
       for (auto& i: stockVars())
         CHECK(i==select(i->x(),i->y()));
      CHECK(!select(x(),y()));
    }
  
  TEST_FIXTURE(TestFixture, update)
    {
      auto godley=new GodleyIcon;
      model->addItem(godley);
      auto& table=godley->table;
      table.resize(3,3);
      table.cell(2,1)="flow1";
      table.cell(2,2)="flow2";
      table.cell(0,1)="stock1";
      table.cell(0,2)="stock2";
      table.doubleEntryCompliant=true;
      table._assetClass(1,GodleyAssetClass::asset);
      table._assetClass(2,GodleyAssetClass::liability);

      VariablePtr v(VariableType::stock,"stock1");
      model->addItem(v);
      v->init("x");
      v=VariablePtr(VariableType::stock,"stock2");
      model->addItem(v);
      v->init("y");
      godley->update();
      CHECK(table.initialConditionRow(1));
      CHECK_EQUAL("x",table.cell(1,1));
      CHECK_EQUAL("y",table.cell(1,2));

      // now remove a column
      table.deleteCol(3);
      godley->update();
      map<string,unsigned> varCount;
      for (auto& i: model->items)
        if (auto v=dynamic_cast<VariableBase*>(i.get()))
          varCount[v->name()]++;

      CHECK_EQUAL(2,varCount["stock1"]);
      CHECK_EQUAL(1,varCount["stock2"]);
      CHECK_EQUAL(1,varCount["flow1"]);
      CHECK_EQUAL(0,varCount["flow2"]);
    }

}

SUITE(Minsky)
{
    TEST_FIXTURE(TestFixture,saveGroupAndInsert)
    {
      unsigned origNumItems=model->numItems();
      unsigned origNumGroups=model->numGroups();
      unsigned numItemsInGroup=group0->numItems();
      saveGroupAsFile(*group0,"/tmp/testGroup.mky");
      insertGroupFromFile("/tmp/testGroup.mky");
      auto group1=dynamic_pointer_cast<Group>(canvas.itemFocus);
      CHECK_EQUAL(origNumGroups+1, model->numGroups());
      CHECK_EQUAL(origNumItems+numItemsInGroup, model->numItems());
      CHECK_EQUAL(numItemsInGroup, group1->numItems());
    }
}

SUITE(Integrate)
{
  TEST_FIXTURE(TestFixture, description)
    {
      auto intop=new IntOp;
      model->addItem(intop);
      model->addItem(a);
      model->addWire(new Wire(intop->ports(0),a->ports(1)));
      intop->description("a");
      // should cowardly refuse, and give a different name
      CHECK_EQUAL("a1",intop->description());
      CHECK_EQUAL(1,intop->ports(0).lock()->wires().size());
      CHECK(intop->ports(0).lock()->wires()[0]->to()==a->ports(1).lock());
      auto intop2=new IntOp;
      model->addItem(intop2);
      intop2->description(intop->description());
      CHECK(intop2->description()!=intop->description());
    }
}

SUITE(GodleyTableWindow)
{
  template <ButtonWidgetEnums::RowCol RC>
    struct ButtonWidgetFixture: public GodleyIcon, public ButtonWidget<RC>
  {
    using ButtonWidget<RC>::first;
    using ButtonWidget<RC>::second;
    using ButtonWidget<RC>::middle;
    using ButtonWidget<RC>::last;
    using ButtonWidget<RC>::idx;
    using ButtonWidget<RC>::pos;
    using ButtonWidget<RC>::invoke;
    using ButtonWidget<RC>::buttonSpacing;
    ButtonWidgetFixture(): ButtonWidget<RC>(static_cast<GodleyIcon&>(*this))
    {
      table.resize(4,4);
      for (size_t r=0; r<4; ++r)
        for (size_t c=0; c<4; ++c)
          table.cell(r,c)="c"+str(r)+str(c);
      table._assetClass(1,GodleyAssetClass::asset);
      table._assetClass(2,GodleyAssetClass::liability);
      table._assetClass(3,GodleyAssetClass::equity);
    }
  };

  typedef ButtonWidgetFixture<ButtonWidgetEnums::row> RowButtonWidgetFixture;
  typedef ButtonWidgetFixture<ButtonWidgetEnums::col> ColButtonWidgetFixture;
  
  TEST_FIXTURE(RowButtonWidgetFixture, RowButtonWidget)
    {
      auto origData=table.getData();
      idx=2;
      invoke(0);
      CHECK_EQUAL(5,table.rows());
      // check row inserted
      for (size_t i=0; i<table.cols(); ++i)
        CHECK_EQUAL("",table.cell(3,i));
    
      idx=3;
      invoke(buttonSpacing);
      CHECK_EQUAL(4,table.rows());
      // check row deleted
      for (size_t i=0; i<table.cols(); ++i)
        CHECK_EQUAL("c3"+str(i),table.cell(3,i));
      CHECK(table.getData()==origData);               
        
      // now check arrow functionality  
      idx=2; pos=second;                     
      invoke(2*buttonSpacing);
      CHECK_EQUAL(4,table.rows());
      // check row swapped with next
      for (size_t i=0; i<table.cols(); ++i)
        CHECK_EQUAL("c1"+str(i),table.cell(1,i));      
      for (size_t i=0; i<table.cols(); ++i)           
        CHECK_EQUAL("c2"+str(i),table.cell(3,i));        // second row can only be swapped with third row
      for (size_t i=0; i<table.cols(); ++i)
        CHECK_EQUAL("c3"+str(i),table.cell(2,i));             
    
      pos=middle;
      invoke(2*buttonSpacing);
      CHECK_EQUAL(4,table.rows());
      for (size_t i=0; i<table.cols(); ++i)
        CHECK_EQUAL("c1"+str(i),table.cell(2,i));
      for (size_t i=0; i<table.cols(); ++i)
        CHECK_EQUAL("c2"+str(i),table.cell(3,i));
      for (size_t i=0; i<table.cols(); ++i)
        CHECK_EQUAL("c3"+str(i),table.cell(1,i));
      invoke(3*buttonSpacing);
      CHECK_EQUAL(4,table.rows());
      for (size_t i=0; i<table.cols(); ++i)
        CHECK_EQUAL("c1"+str(i),table.cell(3,i));
      for (size_t i=0; i<table.cols(); ++i)
        CHECK_EQUAL("c2"+str(i),table.cell(2,i));
      for (size_t i=0; i<table.cols(); ++i)
        CHECK_EQUAL("c3"+str(i),table.cell(1,i));
    
      pos=last;
      invoke(2*buttonSpacing);
      CHECK_EQUAL(4,table.rows());
      for (size_t i=0; i<table.cols(); ++i)
        CHECK_EQUAL("c1"+str(i),table.cell(3,i));
      for (size_t i=0; i<table.cols(); ++i)
        CHECK_EQUAL("c2"+str(i),table.cell(1,i));
      for (size_t i=0; i<table.cols(); ++i)
        CHECK_EQUAL("c3"+str(i),table.cell(2,i));

    
      // should be no 4th button on first, second & last
      pos=first;
      auto saveData=table.getData();
      invoke(3*buttonSpacing);
      CHECK(table.getData()==saveData);
      pos=second;
      invoke(3*buttonSpacing);
      CHECK(table.getData()==saveData);      
      pos=last;
      invoke(3*buttonSpacing);
      CHECK(table.getData()==saveData);
}
  
  TEST_FIXTURE(ColButtonWidgetFixture, ColButtonWidget)
    {
      auto origData=table.getData();
      idx=2;
      invoke(0);
      CHECK_EQUAL(5,table.cols());
      // check row inserted
      for (size_t i=0; i<table.rows(); ++i)
        CHECK_EQUAL("",table.cell(i,3));
    
      idx=3;
      invoke(buttonSpacing);
      CHECK_EQUAL(4,table.cols());
      // check col deleted
      for (size_t i=0; i<table.rows(); ++i)
        CHECK_EQUAL("c"+str(i)+"3",table.cell(i,3));
      CHECK(table.getData()==origData);
    
      // now check arrow functionality
      idx=2; pos=first;
      invoke(2*buttonSpacing);
      CHECK_EQUAL(5,table.cols());
      // check row swapped with next
      for (size_t i=0; i<table.rows(); ++i)
        CHECK_EQUAL("c"+str(i)+"1",table.cell(i,1));
      for (size_t i=0; i<table.rows(); ++i)
        CHECK_EQUAL("c"+str(i)+"2",table.cell(i,4));
      // extra column inserted for liability class
      for (size_t i=0; i<table.rows(); ++i) 
        CHECK_EQUAL("",table.cell(i,2));
      for (size_t i=0; i<table.rows(); ++i)
        CHECK_EQUAL("c"+str(i)+"3",table.cell(i,3));
    
      pos=middle; idx=3;
      invoke(2*buttonSpacing);
      CHECK_EQUAL(4,table.cols());
      for (size_t i=0; i<table.rows(); ++i)
        CHECK_EQUAL("c"+str(i)+"1",table.cell(i,1));
      for (size_t i=0; i<table.rows(); ++i)
        CHECK_EQUAL("c"+str(i)+"2",table.cell(i,3));
      for (size_t i=0; i<table.rows(); ++i)
        CHECK_EQUAL("c"+str(i)+"3",table.cell(i,2));
      idx=2;
      invoke(3*buttonSpacing);
      CHECK_EQUAL(5,table.cols());
      for (size_t i=0; i<table.rows(); ++i)
        CHECK_EQUAL("c"+str(i)+"1",table.cell(i,1));
      for (size_t i=0; i<table.rows(); ++i)
        CHECK_EQUAL("c"+str(i)+"2",table.cell(i,3));
      for (size_t i=0; i<table.rows(); ++i)
        CHECK_EQUAL("c"+str(i)+"3",table.cell(i,4));
      for (size_t i=0; i<table.rows(); ++i)
        CHECK_EQUAL("",table.cell(i,2));

      // should be no 4th button on first
      pos=first;
      auto saveData=table.getData();
      invoke(3*buttonSpacing);
      CHECK(table.getData()==saveData);
      
      // should be no button on last
      pos=last;
      invoke(buttonSpacing);
      CHECK(table.getData()==saveData);
      invoke(2*buttonSpacing);
      CHECK(table.getData()==saveData);      
      invoke(3*buttonSpacing);
      CHECK(table.getData()==saveData);
    }

  struct EmbedGodleyIcon
  {
    GodleyIcon embeddedGodleyIcon;
  };
  
  struct GodleyTableWindowFixture: private EmbedGodleyIcon, public GodleyTableWindow
  {
    GodleyTableWindowFixture(): GodleyTableWindow(embeddedGodleyIcon)
    {
      Tk_Init(interp()); // required for clipboard operations
    }
  };
  
  TEST_FIXTURE(GodleyTableWindowFixture, mouseMove)
    {
      godleyIcon().table.cell(1,1)="hello";
      surface.reset(new ecolab::cairo::Surface
                    (cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,NULL)));
      redraw(0,0,0,0);
      double x=colLeftMargin[1]+10, y=topTableOffset+rowHeight+5;
      CHECK_EQUAL(1, colX(x));
      CHECK_EQUAL(1, rowY(y));
      mouseMove(x,y);
      CHECK_EQUAL(1, hoverRow);
      CHECK_EQUAL(1, hoverCol);
      x=2*ButtonWidget<row>::buttonSpacing+1;
      CHECK_EQUAL(rowWidget, GodleyTableWindow::clickType(x,y));
      mouseMove(x,y);
      CHECK_EQUAL(2,rowWidgets[1].mouseOver());
      x=colLeftMargin[1]+ButtonWidget<row>::buttonSpacing+1;
      y=5+columnButtonsOffset;
      mouseMove(x,y);
      CHECK_EQUAL(1,colWidgets[1].mouseOver());
      mouseMove(0,0);
      for (auto& i: rowWidgets) CHECK_EQUAL(-1, i.mouseOver());
      for (auto& i: colWidgets) CHECK_EQUAL(-1, i.mouseOver());
    }
  
  TEST_FIXTURE(GodleyTableWindowFixture, mouseSelect)
    {
      Tk_Init(interp()); // required for clipboard operations
      godleyIcon().table.cell(1,1)="hello";
      surface.reset(new ecolab::cairo::Surface
                    (cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,NULL)));
      redraw(0,0,0,0);
      double x=colLeftMargin[1], y=topTableOffset+rowHeight+5;
      CHECK_EQUAL(1, colX(x));
      CHECK_EQUAL(1, rowY(y));
      mouseDown(x,y);
      mouseMoveB1(x+10,y);
      mouseUp(x+10,y);
      CHECK_EQUAL(1, selectedRow);
      CHECK_EQUAL(1, selectedCol);
      CHECK_EQUAL(0,insertIdx);
      CHECK(selectIdx>insertIdx);
      CHECK(!cminsky().clipboard.getClipboard().empty());

      
    }
  
  TEST_FIXTURE(GodleyTableWindowFixture, mouseButtons)
    {
      godleyIcon().table.resize(4,4);
      godleyIcon().table.cell(0,1)="col1";  
      godleyIcon().table.cell(0,2)="col2";
      godleyIcon().table.cell(1,1)="r1c1";
      godleyIcon().table.cell(1,2)="r1c2";
      godleyIcon().table.cell(2,1)="r2c1";
      godleyIcon().table.cell(2,2)="r2c2";
      godleyIcon().table.cell(3,1)="r3c1";
      godleyIcon().table.cell(3,2)="r3c2";      
      
      surface.reset(new ecolab::cairo::Surface
                    (cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,NULL)));
      redraw(0,0,0,0);
      double x=colLeftMargin[1]+2*ButtonWidget<row>::buttonSpacing+1, y=5+columnButtonsOffset;
      CHECK_EQUAL(colWidget, clickType(x,y));
      mouseDown(x,y);
      // should have invoked moving column 1 left
      CHECK_EQUAL("r1c2",godleyIcon().table.cell(1,2));
      CHECK_EQUAL("r2c2",godleyIcon().table.cell(2,2));
      CHECK_EQUAL("r1c1",godleyIcon().table.cell(1,3));
      CHECK_EQUAL("r2c1",godleyIcon().table.cell(2,3));
      
      x=2*ButtonWidget<row>::buttonSpacing+1, y=5+topTableOffset+rowHeight;
      CHECK_EQUAL(rowWidget, clickType(x,y));
      mouseDown(x,y);
      // Row 1 cannot move down. For ticket 1064
      CHECK_EQUAL("r2c2",godleyIcon().table.cell(2,2));
      CHECK_EQUAL("r1c2",godleyIcon().table.cell(1,2));
      
      x=2*ButtonWidget<row>::buttonSpacing+1, y=5+topTableOffset+2*rowHeight;
      CHECK_EQUAL(rowWidget, clickType(x,y));
      mouseDown(x,y);
      // should have invoked moving row 2 down, in effect swapping row 2 and 3's contents
      CHECK_EQUAL("r3c2",godleyIcon().table.cell(2,2));
      CHECK_EQUAL("r2c2",godleyIcon().table.cell(3,2));
      
    }
  
  TEST_FIXTURE(GodleyTableWindowFixture, moveRowColCell)
    {
      godleyIcon().table.resize(3,4);
      godleyIcon().table.cell(0,1)="col1";
      godleyIcon().table.cell(0,2)="col2";
      godleyIcon().table.cell(1,0)="row1";
      godleyIcon().table.cell(2,0)="row2";
      godleyIcon().table.cell(1,1)="r1c1";
      godleyIcon().table.cell(1,2)="r1c2";
      godleyIcon().table.cell(2,1)="r2c1";
      godleyIcon().table.cell(2,2)="r2c2";
      surface.reset(new ecolab::cairo::Surface
                    (cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,NULL)));
      redraw(0,0,0,0);
      double x=colLeftMargin[1]+10, y=topTableOffset+5;
      CHECK_EQUAL(1,colX(x));
      CHECK_EQUAL(0,rowY(y));
      mouseDown(x,y);
      x=colLeftMargin[2]+10;
      CHECK_EQUAL(2,colX(x));
      mouseMoveB1(x,y);
      mouseUp(x,y);
      CHECK_EQUAL("col1",godleyIcon().table.cell(0,3));

      x=leftTableOffset+10;
      y=topTableOffset+rowHeight+5;
      CHECK_EQUAL(0,colX(x));
      CHECK_EQUAL(1,rowY(y));
      mouseDown(x,y);
      y+=rowHeight;
      CHECK_EQUAL(2,rowY(y));
      mouseMoveB1(x,y);
      mouseUp(x,y);
      CHECK_EQUAL("row1",godleyIcon().table.cell(1,0));  //Cannot swap cells (1,0) and (2,0) by click dragging. For ticket 1064/1066
      CHECK_EQUAL("row2",godleyIcon().table.cell(2,0));

      // check moving a cell
      x=colLeftMargin[1]+10;
      y=topTableOffset+rowHeight+5;
      CHECK_EQUAL(1,colX(x));
      CHECK_EQUAL(1,rowY(y));
      godleyIcon().table.cell(1,1)="cell11";
      mouseDown(x,y);
      x=colLeftMargin[2]+10;
      y+=rowHeight;
      mouseMoveB1(x,y);
      mouseUp(x,y);
      CHECK_EQUAL("cell11",godleyIcon().table.cell(2,2));
    }

#define XK_MISCELLANY
#define XK_XKB_KEYS
#include <X11/keysymdef.h>
  
  TEST_FIXTURE(GodleyTableWindowFixture, keyPress)
    {
      godleyIcon().table.resize(3,4);
      selectedCol=1;
      selectedRow=1;
      selectIdx=insertIdx=0;
      keyPress({'a',"a"}); keyPress({'b',"b"}); keyPress({'b',"b"}); insertIdx=2; keyPress({XK_Delete,""});
      keyPress({'c',"c"}); keyPress({'c',"c"}); keyPress({XK_BackSpace,""});
      CHECK_EQUAL("abc",godleyIcon().table.cell(1,1));

      godleyIcon().table.cell(0,1)="stock1";
      CHECK_EQUAL(0,godleyIcon().stockVars().size());
      keyPress({XK_Return,""}); // should cause update to be called
      // unfortunately, this is a freestanding GodleyIcon, so update has no effect
//      CHECK_EQUAL(1,godleyIcon().stockVars().size());
//      CHECK_EQUAL("stock1",godleyIcon().stockVars()[0]->name());
      CHECK_EQUAL(-1,selectedCol);
      CHECK_EQUAL(-1,selectedRow);

      
      selectedCol=1;
      selectedRow=1;
      godleyIcon().table.savedText="abc";
      keyPress({'d',"d"}); keyPress({XK_Escape,""}); // should revert to previous
      CHECK_EQUAL("abc",godleyIcon().table.cell(1,1));
      CHECK_EQUAL(-1,selectedCol);
      CHECK_EQUAL(-1,selectedRow);

      // tab, arrow movement
      selectedCol=1;
      selectedRow=1;
      selectIdx=insertIdx=0;
      keyPress({XK_Right,""});
      CHECK_EQUAL(1,insertIdx);
      keyPress({XK_Left,""});
      CHECK_EQUAL(0,insertIdx);
      // (1,0) cell no longer selectable, selectedRow and selectedCol become -1
      keyPress({XK_Left,""});
      CHECK_EQUAL(-1,selectedCol);   
      CHECK_EQUAL(-1,selectedRow);      
      keyPress({XK_Tab,""});          //  No cell selected, selectedRow = 0 and selectedCol = 1 when tab button is pressed, see godleyTableWindow line 633
      CHECK_EQUAL(1,selectedCol);  
      keyPress({XK_Tab,""});
      CHECK_EQUAL(2,selectedCol);
      keyPress({XK_Right,""});     
      CHECK_EQUAL(3,selectedCol);  
      keyPress({XK_Down,""});
      CHECK_EQUAL(1,selectedRow);  // selectedRow = 0 from initial tab press 
      keyPress({XK_Tab,""});
      CHECK_EQUAL(0,selectedCol);  
      keyPress({XK_Left,""});
      CHECK_EQUAL(3,selectedCol);   
      CHECK_EQUAL(1,selectedRow);   
      keyPress({XK_Right,""});
      CHECK_EQUAL(0,selectedCol);   // Moved down one row  
      CHECK_EQUAL(2,selectedRow);
      keyPress({XK_ISO_Left_Tab,""});
      CHECK_EQUAL(3,selectedCol);
      keyPress({XK_ISO_Left_Tab,""});
      CHECK_EQUAL(2,selectedCol);
      CHECK_EQUAL(1,selectedRow);    
      keyPress({XK_Right,""});
      CHECK_EQUAL(3,selectedCol);
      CHECK_EQUAL(1,selectedRow);       
      keyPress({XK_Tab,""}); // check wrap around
      CHECK_EQUAL(0,selectedCol);
      CHECK_EQUAL(2,selectedRow);

      // cut, copy paste
      selectedCol=1;
      selectedRow=1;
      selectIdx=0;
      insertIdx=1;
      cminsky().clipboard.putClipboard("");
      keyPress({XK_Control_L,""}); keyPress({'c',"\x3"}); //copy
      CHECK_EQUAL("a",cminsky().clipboard.getClipboard());
      cminsky().clipboard.putClipboard("");
      keyPress({XK_Control_L,""}); keyPress({'x',"\x18"});  //cut
      CHECK_EQUAL("a",cminsky().clipboard.getClipboard());
      CHECK_EQUAL("bc",godleyIcon().table.cell(1,1));
      keyPress({XK_Control_L,""}); keyPress({'v',"\x16"});  //paste
      CHECK_EQUAL("abc",godleyIcon().table.cell(1,1));

      // initial cell movement when nothing selected
      selectedCol=-1; selectedRow=-1;
      keyPress({XK_Tab,""});
      CHECK_EQUAL(1,selectedCol);
      CHECK_EQUAL(0,selectedRow);

      selectedCol=-1; selectedRow=-1;
      keyPress({XK_ISO_Left_Tab,""});
      CHECK_EQUAL(3,selectedCol);
      CHECK_EQUAL(2,selectedRow);
      
      selectedCol=-1; selectedRow=-1;
      keyPress({XK_Left,""});
      CHECK_EQUAL(3,selectedCol);
      CHECK_EQUAL(0,selectedRow);
      
      selectedCol=-1; selectedRow=-1;
      keyPress({XK_Right,""});
      CHECK_EQUAL(1,selectedCol);
      CHECK_EQUAL(0,selectedRow);
      
      selectedCol=-1; selectedRow=-1;
      keyPress({XK_Down,""});
      CHECK_EQUAL(0,selectedCol);
      CHECK_EQUAL(2,selectedRow);    // Initial Conditions cell cannot be selected. For ticket 1064
      
      selectedCol=-1; selectedRow=-1;
      keyPress({XK_Up,""});
      CHECK_EQUAL(0,selectedCol);
      CHECK_EQUAL(2,selectedRow);
      
    }
  
  TEST_FIXTURE(GodleyTableWindowFixture, addDelVars)
    {
      surface.reset(new ecolab::cairo::Surface
                    (cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,NULL)));
      redraw(0,0,0,0);
      double x=colLeftMargin[1]+10, y=topTableOffset+5+rowHeight;
      CHECK_EQUAL(1,colX(x));
      CHECK_EQUAL(1,rowY(y));
      auto& t=godleyIcon().table;
      t.cell(1,0)="row1";
      t.cell(0,1)="col1";
      t.cell(0,2)="col2";
      t.cell(0,3)="col3";
      addFlow(y);
      CHECK_EQUAL(3,t.rows());
      CHECK_EQUAL("",t.cell(2,0));
      addFlow(y+rowHeight);
      CHECK_EQUAL(4,t.rows());
      CHECK_EQUAL("",t.cell(3,0));      
      t.cell(3,0)="row3";
      deleteFlow(y+rowHeight);
      CHECK_EQUAL(3,t.rows()); 
      CHECK_EQUAL("row3",t.cell(2,0));
      deleteFlow(y);                        // Check that row1 cannot be deleted. For ticket 1064
      CHECK_EQUAL(3,t.rows()); 
      CHECK_EQUAL("row3",t.cell(2,0));      

      addStockVar(x);
      CHECK_EQUAL(5,t.cols());
      CHECK_EQUAL("",t.cell(0,2));
      t.cell(0,2)="newCol";

      deleteStockVar(x);
      CHECK_EQUAL(4,t.cols());
      CHECK_EQUAL("newCol",t.cell(0,1));
    }

   TEST_FIXTURE(GodleyTableWindowFixture, undoRedo)
     {
       auto& t=godleyIcon().table;
       t.cell(1,0)="row1";
       pushHistory();
       t.cell(1,0)="xxx";
       undo(1);
       CHECK_EQUAL("row1",t.cell(1,0));
       undo(-1);
       CHECK_EQUAL("xxx",t.cell(1,0));
     }

   TEST_FIXTURE(TestFixture, copyBetweenCols)
     {
       // test scenario in bug #1212, where item is dragged from one column to another in linked tables
       auto godley1=dynamic_pointer_cast<GodleyIcon>(model->addItem(new GodleyIcon));
       godley1->table.resize(3,4);
       godley1->table.cell(0,1)="foo";
       godley1->table.cell(0,2)="bar";
       godley1->table.cell(2,1)="foobar";

       // linked table assets are liabilities and vice versa
       auto godley2=new GodleyIcon;
       model->addItem(godley2);
       godley2->table.resize(3,4);
       godley2->table.cell(0,2)="foo";
       godley2->table.cell(0,1)="bar";
       godley2->table.cell(2,2)="foobar";

       godley1->update();
       godley2->update();

       GodleyTableWindow gw(*godley1);
       // render GodleyTableWindow to compute column/row boundaries
       ecolab::cairo::Surface surf(cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,nullptr));
       gw.draw(surf.cairo());
       // positions of the 2,1 and 2,2 cells
       double x1=gw.colLeftMargin[1]+5, x2=gw.colLeftMargin[2]+5, y=2*gw.rowHeight+gw.topTableOffset;
       CHECK_EQUAL(1,gw.colXZoomed(x1));
       CHECK_EQUAL(2,gw.colXZoomed(x2));
       CHECK_EQUAL(2,gw.rowYZoomed(y));

       // move cell from asset to liability
       gw.mouseDown(x2,y);
       gw.mouseUp(x1,y);
       gw.update();

       // this scenario should not add extra rows
       CHECK_EQUAL(3,godley1->table.rows());
       CHECK_EQUAL(3,godley2->table.rows());
       CHECK_EQUAL(4,godley1->table.cols());
       CHECK_EQUAL(4,godley2->table.cols());

       CHECK_EQUAL("",godley1->table.cell(2,1));
       CHECK_EQUAL("",godley2->table.cell(2,2));
       FlowCoef fc(godley1->table.cell(2,2));
       CHECK_EQUAL("foobar",fc.name);
       CHECK_EQUAL(1,fc.coef);
       fc=FlowCoef(godley2->table.cell(2,1));
       CHECK_EQUAL("foobar",fc.name);
       CHECK_EQUAL(1,fc.coef);
     }
   
     TEST_FIXTURE(TestFixture, almalgamateLines)
     {
       // test scenario in bug #1212, where item is dragged from one column to another in linked tables
       auto godley1=dynamic_pointer_cast<GodleyIcon>(model->addItem(new GodleyIcon));
       godley1->table.resize(4,4);
       godley1->table.cell(0,1)="foo";
       godley1->table.cell(0,2)="bar";
       godley1->table.cell(2,1)="foobar";
       godley1->table.cell(3,1)="-2foobar";

       // linked table assets are liabilities and vice versa
       auto godley2=new GodleyIcon;
       model->addItem(godley2);
       godley2->table.resize(3,4);
       godley2->table.cell(0,2)="foo";
       godley2->table.cell(0,1)="bar";
       godley2->table.cell(2,2)="-foobar";

       balanceDuplicateColumns(*godley2,2);
       
       // row 2 & 3 should be amalgamated, and the sign changed
       CHECK_EQUAL(3,godley1->table.rows());
       FlowCoef fc(godley1->table.cell(2,1));
       CHECK_EQUAL("foobar",fc.name);
       CHECK_EQUAL(-1,fc.coef);

       CHECK_EQUAL(3,godley2->table.rows());
       fc=FlowCoef(godley2->table.cell(2,2));
       CHECK_EQUAL("foobar",fc.name);
       CHECK_EQUAL(-1,fc.coef);

       // Now add a name to the row, to fix things
       godley1->table.resize(4,4);
       godley1->table.cell(3,1)="2foobar";
       balanceDuplicateColumns(*godley1,1);
       godley1->table.cell(2,0)="tax";
       balanceDuplicateColumns(*godley2,2);
       
       // extra row should not be amalgamated on godley1, but amalgamated on godley2
       CHECK_EQUAL(4,godley1->table.rows());
       fc=FlowCoef(godley1->table.cell(2,1));
       CHECK_EQUAL("foobar",fc.name);
       CHECK_EQUAL(-1,fc.coef);
       fc=FlowCoef(godley1->table.cell(3,1));
       CHECK_EQUAL("foobar",fc.name);
       CHECK_EQUAL(2,fc.coef);

       CHECK_EQUAL(3,godley2->table.rows());
       fc=FlowCoef(godley2->table.cell(2,2));
       CHECK_EQUAL("foobar",fc.name);
       CHECK_EQUAL(1,fc.coef);
      
     }

     
     
}
