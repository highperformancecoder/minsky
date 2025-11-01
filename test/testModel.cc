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

#include <gtest/gtest.h>

using namespace minsky;
using namespace std;

namespace
{

  struct ModelSuite: public Minsky, public ::testing::Test
  {
    GroupPtr group0;
    ItemPtr a,b,c;
    WirePtr ab,bc; 
    LocalMinsky lm;

    ModelSuite(): lm(*this)
    {
      a=model->addItem(new Variable<VariableType::flow>("a"));
      b=model->addItem(new Variable<VariableType::flow>("b"));
      c=model->addItem(new Variable<VariableType::flow>("c"));
      // create 3 variables, wire them and add first two to a group,
      // leaving 3rd external
      a->moveTo(100,100);
      b->moveTo(200,100);
      c->moveTo(300,100);
      EXPECT_EQ(2,a->portsSize());
      EXPECT_EQ(2,b->portsSize());
      EXPECT_EQ(2,c->portsSize());

      ab=model->addWire(new Wire(a->ports(0), b->ports(1)));
      bc=model->addWire(new Wire(b->ports(0), c->ports(1)));
      checkWiresConsistent();

      group0=model->addGroup(new Group);
      checkWiresConsistent();
      group0->addItem(a);
      checkWiresConsistent();
      group0->addItem(b);
      group0->splitBoundaryCrossingWires();
     
      EXPECT_EQ(3,group0->items.size());
      EXPECT_EQ(2,group0->wires.size());
      EXPECT_EQ(1,model->items.size());
      EXPECT_EQ(4,model->numItems());
      EXPECT_EQ(3,model->numWires());
      checkWiresConsistent();

      // add a couple of time operators, to ensure the group has finite size
      model->addItem(OperationPtr(OperationType::time))->moveTo(100,75);
      model->addItem(OperationPtr(OperationType::time))->moveTo(200,125);
      canvas.select({50,50,250,150});
      save("TestGroupFixture.mky");
      EXPECT_TRUE(model->uniqueItems());
    }

    void checkWiresConsistent() {
      for (auto& i: model->items)
        for (size_t pi=0; pi<i->portsSize(); ++pi)
          if (auto p=i->ports(pi).lock())
            for (auto& w: p->wires())
              {
                EXPECT_TRUE(w);
                EXPECT_TRUE(p->input() || p==w->from());
                EXPECT_TRUE(!p->input() || p==w->to());
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

  // gtest requires all fixtures to be derived from Test, alas!
  class CanvasFixture: public Canvas, public ::testing::Test
  {
    Minsky lminsky;
    LocalMinsky lm;
  public:
    CanvasFixture(): lm(lminsky) {}
  };
  class GodleyIconFixture: public GodleyIcon, public ::testing::Test {};
  class GroupFixture: public Group, public ::testing::Test {};
}


TEST_F(ModelSuite, accessibleVars)
{
  vector<string> globalAccessibleVars{"c"};
  vector<string> group0AccessibleVars{":c","a","b","output1"};
  group0->makeSubroutine();
  EXPECT_EQ(globalAccessibleVars.size(), model->accessibleVars().size());
  for (size_t _i=0; _i<globalAccessibleVars.size(); ++_i) EXPECT_EQ(globalAccessibleVars[_i], model->accessibleVars()[_i]);
  EXPECT_EQ(group0AccessibleVars.size(), group0->accessibleVars().size());
  for (size_t _i=0; _i<group0AccessibleVars.size(); ++_i) EXPECT_EQ(group0AccessibleVars[_i], group0->accessibleVars()[_i]);
  EXPECT_EQ(group0AccessibleVars.size(), a->variableCast()->accessibleVars().size());
  for (size_t _i=0; _i<group0AccessibleVars.size(); ++_i) EXPECT_EQ(group0AccessibleVars[_i], a->variableCast()->accessibleVars()[_i]);
}

TEST_F(ModelSuite, makeSubroutine)
{
  group0->makeSubroutine();
  for (auto& i: group0->items)
    if (auto v=i->variableCast())
      EXPECT_TRUE(v->rawName()[0]!=':');
}
  
TEST_F(ModelSuite, SelectGroup)
{
  auto& g=*model->addGroup(new Group);
  g.addItem(a);
  g.addItem(b);
  EXPECT_EQ(2, g.items.size());
  EXPECT_EQ(1, g.wires.size());
  EXPECT_TRUE(&g==a->group.lock().get());
  EXPECT_TRUE(!a->visible());
  EXPECT_TRUE(&g==b->group.lock().get());
  EXPECT_TRUE(!b->visible()); 
  EXPECT_TRUE(c->group.lock()==model);
  EXPECT_TRUE(c->visible());
  EXPECT_TRUE(!model->findWire(*ab)->visible());
  EXPECT_TRUE(model->findWire(*bc)->visible());
  EXPECT_TRUE(find(g.wires.begin(), g.wires.end(), ab) != g.wires.end()); 
  EXPECT_TRUE(model->uniqueItems());
  model->removeGroup(g); // why is this needed???
}

TEST_F(ModelSuite, addVariable)
{
  group0->addItem(c);
  EXPECT_TRUE(model->uniqueItems());
  EXPECT_EQ(4,group0->items.size());
  EXPECT_EQ(2,model->items.size());
  EXPECT_EQ(3,group0->wires.size());
  EXPECT_EQ(0,model->wires.size());
    
  checkWiresConsistent();

  // now check removal
  group0->group.lock()->addItem(c);

  EXPECT_EQ(3,group0->items.size());
  EXPECT_EQ(2,group0->wires.size());
  EXPECT_EQ(3,model->items.size());
  EXPECT_EQ(1,model->wires.size());
}

TEST_F(ModelSuite, addIntegral)
{
  unsigned numItems=model->numItems();
  auto integ=make_shared<IntOp>();
  model->addItem(integ);
  EXPECT_EQ(numItems+2,model->numItems());
  EXPECT_TRUE(integ->group.lock()==model);
  EXPECT_TRUE(integ->intVar->group.lock()==model);
  group0->addItem(integ);
  EXPECT_TRUE(integ->group.lock()==group0);
  EXPECT_TRUE(integ->intVar->group.lock()==group0);
}
  
TEST_F(ModelSuite, addBookmark)
{
  model->addBookmark("bookmark0");
  EXPECT_EQ("bookmark0",model->bookmarkList()[model->bookmarks.size()-1]);
  EXPECT_EQ(1,model->bookmarks.size());
  auto& b0=*model->bookmarks.begin();
  double x0 =b0.x, y0=b0.y;	  
  model->moveTo(100,100);
  model->addBookmark("bookmark1");
  EXPECT_EQ("bookmark1",model->bookmarkList().back());
  EXPECT_EQ(2,model->bookmarks.size());
  auto& b1=*model->bookmarks.find(Bookmark("bookmark1"));
  double x1=b1.x,y1=b1.y;
  EXPECT_EQ(x1,model->x());
  EXPECT_EQ(y1,model->y());
  model->moveTo(200,200);
  model->addBookmark("bookmark2");
  EXPECT_EQ("bookmark2",model->bookmarkList().back());
  EXPECT_EQ(3,model->bookmarks.size());
  auto& b2=*model->bookmarks.find(Bookmark("bookmark2"));
  double x2=b2.x,y2=b2.y;
  EXPECT_EQ(x2,model->x());
  EXPECT_EQ(y2,model->y());
  model->moveTo(300,300);
  model->addBookmark("bookmark3");
  EXPECT_EQ("bookmark3",model->bookmarkList().back());
  EXPECT_EQ(4,model->bookmarks.size());
  auto& b3=*model->bookmarks.find(Bookmark("bookmark3"));
  double x3=b3.x,y3=b3.y;
  EXPECT_EQ(x3,model->x());
  EXPECT_EQ(y3,model->y());	  
  model->gotoBookmark(0);
  EXPECT_EQ(x0,model->x());
  EXPECT_EQ(y0,model->y());
  EXPECT_TRUE(x0!=x1 && y0!=y1 && x0!=x2 && y0!=y2 && x0!=x3 && y0!=y3);
  model->deleteBookmark(model->bookmarks.size()-1);
  model->deleteBookmark(model->bookmarks.size()-1);
  model->deleteBookmark(model->bookmarks.size()-1);
  model->deleteBookmark(model->bookmarks.size()-1);
  EXPECT_EQ(0,model->bookmarks.size()); 	   	          
}  
  
// check that removing then adding an item leaves the group idempotent
TEST_F(ModelSuite, removeAddItem)
{
  EXPECT_EQ(1,group0->createdIOvariables.size());
  EXPECT_EQ(3,model->items.size());
  model->addItem(a);
  group0->splitBoundaryCrossingWires();
  save("x1.mky");
  EXPECT_EQ(3,group0->items.size()); // extra io var created
  EXPECT_EQ(4,model->items.size());
  EXPECT_EQ(2,group0->createdIOvariables.size());
  EXPECT_EQ(4,model->numWires());
  group0->addItem(a);
  group0->splitBoundaryCrossingWires();
  EXPECT_EQ(3,group0->items.size());
  EXPECT_EQ(3,model->items.size());
  EXPECT_EQ(1,group0->createdIOvariables.size());
  EXPECT_EQ(3,model->numWires());
  EXPECT_EQ(3,group0->items.size());
}

TEST_F(ModelSuite, displayPlot)
{
  auto plot=new PlotWidget;
  group0->addItem(plot);
  plot->makeDisplayPlot();
  EXPECT_TRUE(group0->displayPlot.get()==plot);
  group0->removeDisplayPlot();
  EXPECT_TRUE(!group0->displayPlot.get());
}

TEST_F(ModelSuite, findGroup)
{
  EXPECT_TRUE(model->findGroup(*group0)==group0);
}

TEST_F(ModelSuite, copy)
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
  EXPECT_NEAR(group0->bb.width(), g->bb.width(), 1e-2);
  EXPECT_NEAR(group0->bb.height(), g->bb.height(), 1e-2);
  EXPECT_NEAR(group0->x(),g->x(), 1e-2);
  EXPECT_NEAR(group0->y(),g->y(), 1e-2);
  for (size_t _i=0; _i<4; ++_i) EXPECT_NEAR(group0->cBounds().data()[_i], g->cBounds().data()[_i], 1e-2);

  save("copy.mky");
      
  EXPECT_EQ(group0->items.size(), g->items.size());
  EXPECT_EQ(group0->inVariables.size(), g->inVariables.size());
  EXPECT_EQ(group0->outVariables.size(), g->outVariables.size());
  EXPECT_TRUE(g->createdIOvariables.empty());
  EXPECT_EQ(group0->wires.size(), g->wires.size());
  EXPECT_EQ(group0->groups.size(), g->groups.size());
  EXPECT_NEAR(group0->x(),g->x(),1e-2);
  EXPECT_NEAR(group0->y(),g->y(),1e-2);
      
  for (size_t i=0; i<group0->items.size(); i++)
    {
      EXPECT_TRUE(group0->items[i]!=g->items[i]);
      EXPECT_EQ(group0->items[i]->classType(),g->items[i]->classType());
      EXPECT_NEAR(group0->items[i]->x(),g->items[i]->x(),1e-2);
      EXPECT_NEAR(group0->items[i]->y(),g->items[i]->y(),1e-2);
    }

  for (size_t i=0; i<group0->inVariables.size(); i++)
    {
      EXPECT_TRUE(group0->inVariables[i]!=g->inVariables[i]);
      EXPECT_NEAR(group0->inVariables[i]->x(), g->inVariables[i]->x(),1e-2);
      EXPECT_NEAR(group0->inVariables[i]->y(), g->inVariables[i]->y(),1e-2);
    }
  for (size_t i=0; i<group0->outVariables.size(); i++)
    {
      EXPECT_TRUE(group0->outVariables[i]!=g->outVariables[i]);
      EXPECT_NEAR(group0->outVariables[i]->x(), g->outVariables[i]->x(),1e-2);
      EXPECT_NEAR(group0->outVariables[i]->y(), g->outVariables[i]->y(),1e-2);
    }
        
  for (size_t i=0; i<group0->wires.size(); i++)
    {
      auto w1=group0->wires[i], w2=g->wires[i];
      EXPECT_TRUE(w1!=w2);
      EXPECT_TRUE(w1->to()->item().group.lock()==group0);
      EXPECT_TRUE(w1->from()->item().group.lock()==group0);
      EXPECT_TRUE(w2->to()->item().group.lock()==g);
      EXPECT_TRUE(w2->from()->item().group.lock()==g);
      auto c1=w1->coords(), c2=w2->coords();
      EXPECT_EQ(c1.size(), c2.size());
      for (size_t _i=0; _i<c1.size(); ++_i) EXPECT_NEAR(c1.data()[_i], c2.data()[_i], 1e-2);
    }
  for (size_t i=0; i<group0->groups.size(); i++)
    {
      EXPECT_TRUE(group0->groups[i]!=g->groups[i]);
    }
}

  
TEST_F(GroupFixture, GroupRecursiveDo)
{
  addItem(new Operation<OperationType::exp>);
  auto g=addGroup(new Group);
  g->addItem(new Operation<OperationType::ln>);
  EXPECT_TRUE(recursiveDo(&GroupItems::items,[&](Items&,Items::iterator i)
  {return dynamic_cast<Operation<OperationType::exp>*>(i->get());}));
  EXPECT_TRUE(recursiveDo(&GroupItems::items,[&](Items&,Items::iterator i)
  {return dynamic_cast<Operation<OperationType::ln>*>(i->get());}));
  EXPECT_TRUE(!recursiveDo(&GroupItems::items,[&](Items&,Items::iterator i)
  {return dynamic_cast<Operation<OperationType::add>*>(i->get());}));
}
  
TEST_F(ModelSuite, removeGroup)
{
  auto g=model->removeGroup(*group0);
  EXPECT_TRUE(g==group0);
  EXPECT_TRUE(find(model->groups.begin(),model->groups.end(),group0)==model->groups.end());
}
   
TEST_F(ModelSuite,moveContents)
{
  group0->addItem(new Group);
  unsigned numItems=model->numItems();
  unsigned numGroups=model->numGroups();
  unsigned numWires=model->numWires();
  EXPECT_TRUE(!group0->empty());
  model->moveContents(*group0);
  EXPECT_TRUE(group0->empty());
  EXPECT_EQ(numItems, model->numItems());
  EXPECT_EQ(numGroups, model->numGroups());
  EXPECT_EQ(numWires, model->numWires());
  EXPECT_THROW(group0->moveContents(*model), error);
}

TEST_F(GroupFixture, checkAddIORegion)
{
  EXPECT_EQ(IORegion::input, inIORegion(x()-0.5*iWidth()*zoomFactor(), y()));
  EXPECT_EQ(IORegion::output, inIORegion(x()+0.5*iWidth()*zoomFactor(), y()));
  VariablePtr inp(VariableType::flow,"input");
  VariablePtr outp(VariableType::flow,"output");
  inp->moveTo(x()-0.5*iWidth()*zoomFactor(), y());
  addItem(inp);
  checkAddIORegion(inp);
  outp->moveTo(x()+0.5*iWidth()*zoomFactor(), y());
  addItem(outp);
  checkAddIORegion(outp);
  EXPECT_EQ(1,inVariables.size());
  EXPECT_EQ(1,outVariables.size());
  EXPECT_EQ("input",inVariables[0]->name());
  EXPECT_EQ("output",outVariables[0]->name());
}
    


TEST_F(ModelSuite, getItemAt)
{
  // zoom to display group0 to make a & b visible
  canvas.item=group0;
  canvas.zoomToDisplay();
  canvas.getItemAt(a->x()+2,a->y()+2);
  EXPECT_TRUE(a==canvas.item);
  canvas.getItemAt(b->x()-2,b->y()-2);
  EXPECT_TRUE(b==canvas.item);
  canvas.getItemAt(c->x()-2,c->y()+2);
  EXPECT_TRUE(c==canvas.item);
  model->setZoom(1);
  canvas.getItemAt(group0->x()-2,group0->y()+2);
  EXPECT_TRUE(group0==canvas.item);
}
  
TEST_F(ModelSuite, getWireAt)
{
  auto from=a->ports(0).lock(), to=b->ports(1).lock();
  float x=0.5f*(from->x()+to->x())+1;
  float y=0.5f*(from->y()+to->y())+1;
  canvas.getWireAt(x,y);
  EXPECT_TRUE(canvas.wire==ab);
}
  
TEST_F(CanvasFixture,findVariableDefinition)
{
  model=cminsky().model;
  VariablePtr var1(VariableType::flow,"foo");
  VariablePtr var2(VariableType::flow,"foo");
  OperationPtr op(OperationType::exp);      
  item=model->addItem(var1);
  model->addItem(var2);
  model->addItem(op);
  // initially, foo is undefined, so should return false
  EXPECT_TRUE(!findVariableDefinition());
  EXPECT_TRUE(item==var1);
  model->addWire(new Wire(op->ports(0),var2->ports(1)));
  EXPECT_TRUE(findVariableDefinition());
  EXPECT_TRUE(itemIndicator==var2);
  model->removeItem(*var2);

  shared_ptr<IntOp> integ(new IntOp);
  integ->description("foo");
  model->addItem(integ);
  item=var1;
  EXPECT_TRUE(findVariableDefinition());
  EXPECT_TRUE(itemIndicator==integ);

  model->items.clear();
  shared_ptr<GodleyIcon> godley(new GodleyIcon);
  model->addItem(godley);
  godley->table.resize(3,2);
  godley->table.cell(0,1)="foo";
  godley->table.cell(0,2)="bar";
  godley->update();
  item=VariablePtr(VariableType::stock,"foo");
  EXPECT_TRUE(findVariableDefinition());
  EXPECT_TRUE(itemIndicator==godley);

  // on a non-variable, findVariableDefinition should return false
  item=godley;
  EXPECT_TRUE(!findVariableDefinition());

}

TEST_F(ModelSuite,moveItem)
{
  cairo::Surface surf(cairo_recording_surface_create(CAIRO_CONTENT_COLOR,nullptr));
  c->draw(surf.cairo());// reposition ports
  EXPECT_TRUE(c->clickType(c->x(),c->y()) == ClickType::onItem); 
  canvas.mouseDown(c->x(),c->y());
  canvas.mouseUp(400,500);
  EXPECT_EQ(400,c->x());
  EXPECT_EQ(500,c->y());
}
    
TEST_F(ModelSuite,resizeVariable)
{
  c->moveTo(400,300);
  c->updateBoundingBox();
  cairo::Surface surf(cairo_recording_surface_create(CAIRO_CONTENT_COLOR,nullptr));
  c->draw(surf.cairo());// reposition ports
  float xc=c->right(), yc=c->bottom();      
  EXPECT_TRUE(c->clickType(xc,yc) == ClickType::onResize);
  canvas.mouseDown(xc,yc);
  canvas.mouseUp(600,800);
  EXPECT_NEAR(600, c->right(),4*portRadiusMult);
  EXPECT_NEAR(800, c->bottom(),4*portRadiusMult);
}    

TEST_F(ModelSuite,resizeOperation)
{
  OperationPtr add(OperationType::add);
  model->addItem(add);
  add->moveTo(400,300);
  cairo::Surface surf(cairo_recording_surface_create(CAIRO_CONTENT_COLOR,nullptr));
  add->draw(surf.cairo());// reposition ports
  float xc=add->right(), yc=add->bottom();      
  EXPECT_TRUE(add->clickType(xc,yc) == ClickType::onResize); 
  canvas.mouseDown(xc,yc);
  canvas.mouseUp(600,800);
  EXPECT_NEAR(600,add->right(),4*portRadiusMult);
  EXPECT_NEAR(800,add->bottom(),4*portRadiusMult);
}    

TEST_F(ModelSuite,onSlider)
{
  auto cc=model->addItem(new Variable<VariableType::flow>("cc"));
  cc->moveTo(500,300);
  auto cv=dynamic_cast<VariableBase*>(cc.get());
  cv->iWidth(10);
  cv->value(1000);
  cv->sliderMin(0);
  cv->sliderMax(2000);
  cv->sliderStep(20);
  // work out where slider is located
  RenderVariable rv(*cv);
  float xc=cv->x()+rv.handlePos(), yc=cv->y()-rv.height();
  EXPECT_EQ(ClickType::inItem, cv->clickType(xc,yc));
  canvas.mouseDown(xc,yc);
  xc+=5;
  canvas.mouseUp(xc,yc);
  // check handle and value changed
  EXPECT_NEAR(xc, cv->x()+rv.handlePos(), 1);
  EXPECT_TRUE(cv->value()>1000);
  // check variable hasn't moved
  EXPECT_EQ(500,cv->x());
  EXPECT_EQ(300,cv->y());

  // now check that value is clamped to max/min
  canvas.mouseDown(xc,yc);
  canvas.mouseUp(xc+100,yc);
  EXPECT_EQ(cv->sliderMax(), cv->value());
  xc=cv->x()+rv.handlePos();
  canvas.mouseDown(xc,yc);
  canvas.mouseUp(xc-100,yc);
  EXPECT_EQ(cv->sliderMin(), cv->value());
}

TEST_F(ModelSuite,lasso)
{
  canvas.selection.clear();
  canvas.mouseDown(250,0); //Adjusted for new shape of operation icons. For ticket 362.
  canvas.mouseUp(350,150);
  EXPECT_EQ(1,canvas.selection.items.size());
  EXPECT_TRUE(find(canvas.selection.items.begin(),canvas.selection.items.end(),c) !=canvas.selection.items.end());

  // now check when the first click is in the bounding box of an icon, but outside it
  OperationPtr op(OperationType::time);
  model->addItem(op);
  op->moveTo(500,500);
  float x=524, y=524;               // adjusted for 2*portRadius near corners, for feature 94
  EXPECT_TRUE(op->contains(x-12,y-12));
  EXPECT_EQ(ClickType::outside, op->clickType(x,y));
  canvas.selection.clear();
  canvas.mouseDown(x,y);
  canvas.mouseUp(x-17,y-17);
  EXPECT_EQ(1,canvas.selection.items.size());
  EXPECT_TRUE(find(canvas.selection.items.begin(),canvas.selection.items.end(),op) !=canvas.selection.items.end());

  group0->updateBoundingBox(); //why? for Travis.
      
  // test that groups can be selected
  EXPECT_TRUE(!group0->displayContents());
  float w=0.5*group0->width()+10, h=0.5*group0->height()+10;
  x=group0->x()-w; y=group0->y()-h;
  // nw -> se selection
  canvas.mouseDown(x,y);
  canvas.mouseUp(x+2*w,y+2*h);
  EXPECT_EQ(0,canvas.selection.items.size());
  EXPECT_EQ(1,canvas.selection.groups.size());
  EXPECT_TRUE(find(canvas.selection.groups.begin(),canvas.selection.groups.end(),group0) !=canvas.selection.groups.end());

  // ne -> sw selection
  canvas.selection.clear();
  canvas.mouseDown(x+2*w,y);
  canvas.mouseUp(x-2*w,y+2*h);
  EXPECT_EQ(0,canvas.selection.items.size());
  EXPECT_EQ(1,canvas.selection.groups.size());
  EXPECT_TRUE(find(canvas.selection.groups.begin(),canvas.selection.groups.end(),group0) !=canvas.selection.groups.end());
      
  // se -> nw selection
  canvas.selection.clear();
  canvas.mouseDown(x+2*w,y+2*h);
  canvas.mouseUp(x,y);
  EXPECT_EQ(0,canvas.selection.items.size());
  EXPECT_EQ(1,canvas.selection.groups.size());
  EXPECT_TRUE(find(canvas.selection.groups.begin(),canvas.selection.groups.end(),group0) !=canvas.selection.groups.end());
      
  // sw -> ne selection
  canvas.selection.clear();
  canvas.mouseDown(x,y+2*h);
  canvas.mouseUp(x+2*w,y-2*h);
  EXPECT_EQ(0,canvas.selection.items.size());
  EXPECT_EQ(1,canvas.selection.groups.size());
  EXPECT_TRUE(find(canvas.selection.groups.begin(),canvas.selection.groups.end(),group0) !=canvas.selection.groups.end());
}

TEST_F(CanvasFixture, wires)
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
  EXPECT_EQ(4, coords.size());

  // drag up, should insert a handle
  float x=0.5*(coords[0]+coords[2]), y=0.5*(coords[1]+coords[3]);
  mouseDown(x,y);
  x+=10;
  mouseUp(x,y);
  coords=w->coords();
  EXPECT_EQ(6, coords.size());
  EXPECT_EQ(x,coords[2]);
  EXPECT_EQ(y,coords[3]);
}

TEST_F(CanvasFixture, godleyResize)
{
  model.reset(new Group);
  addGodley();
  auto i=itemFocus;
  double w=i->width(), h=i->height();
  double x=i->x(), y=i->y();

  mouseDown(x+0.5*w, y+0.5*h);
  mouseUp(x+w, y+h);
  EXPECT_NEAR(1.5*w,i->width(),0.1);
  EXPECT_NEAR(1.5*h,i->height(),0.1);
}

TEST_F(CanvasFixture, groupResize)
{
  model.reset(new Group);
  model->self=model;
  addGroup();
  auto& group=dynamic_cast<Group&>(*itemFocus);
  group.updateBoundingBox();
  group.relZoom=0.5; // ensure displayContents is false
  double w=group.iWidth()*group.zoomFactor(), h=group.iHeight()*group.zoomFactor();
  double x=group.x(), y=group.y(), z=group.relZoom;
  EXPECT_TRUE(group.clickType(group.right(),group.top()) == ClickType::onResize);

  mouseDown(group.right(), group.bottom());
  mouseUp(x+w, y+h);
  EXPECT_NEAR(x+w,group.right(),1);
  EXPECT_NEAR(y+h,group.bottom(),1); 
}

TEST_F(CanvasFixture, moveIntoThenOutOfGroup)
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
        
  EXPECT_TRUE(b->group.lock()==model);
  EXPECT_EQ(1,model->numWires());
  EXPECT_EQ(2,model->numItems());
  EXPECT_EQ(0,g->inVariables.size());

  // move b into group.
  mouseDown(b->x()+5,b->y()+5);   
  mouseUp(g->x()+5,g->y()+5);  // small offset added because resize handles grabbed otherwise, for feature 94. don't understand why?
  EXPECT_TRUE(b->group.lock()==g);
  EXPECT_EQ(2,model->numWires());
  EXPECT_EQ(3,model->numItems());
  EXPECT_EQ(1,g->inVariables.size());

  // move b out of group
  item=g;
  zoomToDisplay();
  mouseDown(b->x()+5,b->y()+5);  
  mouseUp(200,200);
  EXPECT_TRUE(b->group.lock()==model);
  EXPECT_EQ(1,model->numWires());
  EXPECT_EQ(2,model->numItems());
  EXPECT_EQ(0,g->inVariables.size());
}
    
TEST_F(CanvasFixture, mouseFocus)
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

  EXPECT_TRUE(!a->mouseFocus);
  EXPECT_TRUE(!b->mouseFocus);
  EXPECT_TRUE(!w->mouseFocus);

  mouseMove(a->x(),a->y());
  EXPECT_TRUE(a->mouseFocus);
  EXPECT_TRUE(!b->mouseFocus);
  EXPECT_TRUE(!w->mouseFocus);

  mouseMove(b->x(),b->y());
  EXPECT_TRUE(!a->mouseFocus);
  EXPECT_TRUE(b->mouseFocus);
  EXPECT_TRUE(!w->mouseFocus);

  mouseMove(0.5*(a->x()+b->x()),0.5*(a->y()+b->y()));
  EXPECT_TRUE(!a->mouseFocus);
  EXPECT_TRUE(!b->mouseFocus);
  EXPECT_TRUE(w->mouseFocus);
        
}

TEST_F(CanvasFixture, removeItemFromItsGroup)
{
  model.reset(new Group);
  model->self=model;
  auto g0=model->addGroup(new Group);
  auto g1=g0->addGroup(new Group);
  auto a=g1->addItem(new Operation<OperationType::exp>);
  EXPECT_TRUE(a->group.lock()==g1);
  EXPECT_TRUE(a->visible()==g1->displayContents());
  item=a;
  removeItemFromItsGroup();
  EXPECT_TRUE(a->group.lock()==g0);
  EXPECT_TRUE(a->visible()==g0->displayContents());
  item=a;
  removeItemFromItsGroup();
  EXPECT_TRUE(a->group.lock()==model);
  EXPECT_TRUE(a->visible());
}

TEST_F(ModelSuite,selectAllVariables)
{
  model->moveContents(*group0);
  canvas.item=a;
  canvas.copyItem();
  canvas.mouseUp(500,500);
  canvas.selectAllVariables();
  EXPECT_EQ(2,canvas.selection.items.size());
  for (auto i: canvas.selection.items)
    {
      auto ii=dynamic_cast<VariableBase*>(i.get());
      EXPECT_TRUE(ii);
      if (ii) EXPECT_EQ(dynamic_pointer_cast<VariableBase>(a)->valueId(), ii->valueId());
    }

  canvas.item=b;
  canvas.selectAllVariables();
  EXPECT_EQ(1,canvas.selection.items.size());
  EXPECT_TRUE(canvas.selection.items[0]==b);

  canvas.item=group0;
  canvas.selectAllVariables();
  EXPECT_TRUE(canvas.selection.empty());

  // check integrals are selected too
  auto integ=new IntOp;
  canvas.item=model->addItem(integ);
  integ->description("foo");
  save("foo.mky");
  canvas.selectAllVariables();
  EXPECT_EQ(2,canvas.selection.items.size());
  EXPECT_TRUE(canvas.selection.items[0]==integ->intVar);
  EXPECT_TRUE(canvas.selection.items[1].get()==integ);
}
    
TEST_F(ModelSuite,renameAllInstances)
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
  EXPECT_EQ(numItems, model->numItems());
  unsigned count=0;
  for (auto i: model->items)
    if (auto v=dynamic_cast<VariableBase*>(i.get()))
      {
        EXPECT_TRUE(v->valueId()!=":a");
        if (v->valueId()==":foobar")
          count++;
      }
  EXPECT_EQ(2,count); // should be 1 from first godley table, and the original a

  // check that the Godley table got updated
  EXPECT_EQ("foobar",gi->table.cell(2,1));
  EXPECT_EQ(":foobar",gi->table.cell(2,2));
  EXPECT_EQ("a",gi2->table.cell(2,1)); // local var, not target of rename
  EXPECT_EQ(":foobar",gi2->table.cell(2,2));

        
  // check no renaming should happen when item is not a variable 
  canvas.item=group0;
  canvas.renameAllInstances("foobar1");
  for (auto i: model->items)
    if (auto v=dynamic_cast<VariableBase*>(i.get()))
      EXPECT_TRUE(v->name()!="foobar1");
        
  // check integrals are renamed too
  auto integ=new IntOp;
  canvas.item=model->addItem(integ);
  integ->description("foo");
  canvas.renameAllInstances("bar");
  EXPECT_TRUE(integ->description()=="bar");
}

TEST_F(ModelSuite,ungroupItem)
{
  unsigned originalNumItems=model->numItems();
  unsigned originalNumGroups=model->numGroups();
  canvas.item=group0;
  canvas.ungroupItem();
  EXPECT_EQ(originalNumItems, model->numItems());
  EXPECT_EQ(originalNumGroups-1, model->numGroups());
}

TEST_F(ModelSuite,copyItem)
{
  model->addItem(a);
  unsigned originalNumItems=model->numItems();
  unsigned originalNumGroups=model->numGroups();
  canvas.item=a;
  canvas.copyItem();
  EXPECT_EQ(originalNumItems+1, model->numItems());
  EXPECT_EQ(dynamic_cast<VariableBase*>(a.get())->valueId(),
            dynamic_cast<VariableBase*>(model->items.back().get())->valueId());

  auto integ=new IntOp;
  canvas.item=model->addItem(integ);
  integ->description("foo");
  originalNumItems=model->numItems();
  canvas.copyItem();
  EXPECT_EQ(originalNumItems+1, model->numItems());
  EXPECT_EQ(integ->intVar->valueId(),
            dynamic_cast<VariableBase*>(model->items.back().get())->valueId());
}

TEST_F(ModelSuite,openGroupInCanvas)
{
  // remove a from group0, which should add an invariable
  model->addItem(a);
  group0->splitBoundaryCrossingWires();
  EXPECT_TRUE(!group0->inVariables.empty());
  EXPECT_TRUE(!group0->outVariables.empty());
  canvas.openGroupInCanvas(group0);
  EXPECT_TRUE(canvas.model==group0);
  EXPECT_TRUE(group0->displayContents());
  canvas.openGroupInCanvas(model);
  EXPECT_TRUE(canvas.model==model);
}

TEST_F(CanvasFixture,copyVars)
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
  EXPECT_EQ(originalNumItems+godley->flowVars().size(),model->numItems());
  // Check that the number of items in selection after copyAllFlowVars() is equal to the number of flowVars attached to the Godley Icon. For ticket 1039.
  EXPECT_EQ(godley->flowVars().size(),selection.items.size());
        
  //Check that there are two copies of the flowVars orginally attached to the Godley Icon. For ticket 1039.
  map<string,int> idCnt;
  for (auto& i: model->items)
    if (auto v=i->variableCast())
      idCnt[v->valueId()]++;
  for (auto v: godley->flowVars())
    EXPECT_EQ(2, idCnt[v->valueId()]);                
        
  originalNumItems=model->numItems();
  copyAllStockVars();
  EXPECT_EQ(originalNumItems+godley->stockVars().size(),model->numItems());
  // Check that the number of items in selection after copyAllStockVars() is equal to the number of stockVars attached to the Godley Icon. For ticket 1039.
  EXPECT_EQ(godley->stockVars().size(),selection.items.size());     
        
  //Check that there are two copies of the stockVars orginally attached to the Godley Icon. For ticket 1039.
  idCnt.clear();
  for (auto& i: model->items)
    if (auto v=i->variableCast())
      if (v->isStock()) idCnt[v->valueId()]++;
  for (auto v: godley->stockVars())
    EXPECT_EQ(2, idCnt[v->valueId()]);    
}

TEST_F(ModelSuite,handleArrows)
{
  // need a variable that is not defined
  auto v=model->addItem(new Variable<VariableType::parameter>("v"))->variableCast();
        
  v->value(1000);
  v->sliderMin(0);
  v->sliderMax(2000);
  v->sliderStep(100);
  canvas.keyPress({0xff52,"",0,v->x(),v->y()});
  EXPECT_EQ(1100,v->value());
  canvas.keyPress({0xff51,"",0,v->x(),v->y()});
  EXPECT_EQ(1000,v->value());
  for (size_t i=0; i<20; ++i)
    canvas.keyPress({0xff52,"",0,v->x(),v->y()});
  EXPECT_EQ(2000,v->value());
  for (size_t i=0; i<30; ++i)
    canvas.keyPress({0xff51,"",0,v->x(),v->y()});
  EXPECT_EQ(0,v->value());
}
    
TEST_F(ModelSuite,selectVar)
{
  EXPECT_TRUE(!group0->outVariables.empty());
  auto v=group0->outVariables[0];
  canvas.item=group0;
  EXPECT_TRUE(canvas.selectVar(v->x(), v->y()));
  EXPECT_TRUE(canvas.item==v);
  canvas.item=group0;
  EXPECT_TRUE(!canvas.selectVar(500,500));
  canvas.item.reset();
  EXPECT_TRUE(!canvas.selectVar(500,500));
}

TEST_F(CanvasFixture, addStuff)
{
  model.reset(new Group);
  model->self=model;
  addVariable("foo",VariableType::flow);
  auto v=dynamic_cast<VariableBase*>(itemFocus.get());
  EXPECT_TRUE(v);
  EXPECT_EQ("foo",v->name());
  EXPECT_TRUE(model==itemFocus->group.lock());

  addNote("some text");
  EXPECT_TRUE(itemFocus);
  EXPECT_EQ("some text", itemFocus->detailedText());

  addGroup();
  EXPECT_TRUE(dynamic_cast<Group*>(itemFocus.get()));
  EXPECT_TRUE(model==itemFocus->group.lock());

  addSwitch();
  EXPECT_TRUE(dynamic_cast<SwitchIcon*>(itemFocus.get()));
  EXPECT_TRUE(model==itemFocus->group.lock());
}

TEST_F(CanvasFixture, groupSelection)
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
  EXPECT_TRUE(newG);
  EXPECT_EQ(2,newG->items.size());
  EXPECT_EQ(1,newG->groups.size());
  EXPECT_EQ(1,newG->wires.size());
  EXPECT_TRUE(model->uniqueItems());
  EXPECT_TRUE(model->nocycles());
}

TEST(TensorOps, handles)
{
  Operation<OperationType::sin> item;
  item.ports(0).lock()->moveTo(0,0);
  item.ports(1).lock()->moveTo(10,10);
  Wire wire(item.ports(0),item.ports(1),{0,0,3,4,6,7,10,10});
  EXPECT_EQ(0, wire.nearestHandle(3.1, 3.9));
  EXPECT_EQ(1, wire.nearestHandle(6.2, 7.5));
  // should not have inserted anything yet
  EXPECT_EQ(8, wire.coords().size());

  // check for midpoint insertion
  EXPECT_EQ(0, wire.nearestHandle(1.6, 2));
  EXPECT_EQ(10, wire.coords().size());
  EXPECT_NEAR(1.5, wire.coords()[2],0.01);
  EXPECT_NEAR(2, wire.coords()[3],0.01);
      
  EXPECT_EQ(2, wire.nearestHandle(4.5, 5.5));
  EXPECT_EQ(12, wire.coords().size());
  EXPECT_NEAR(4.5, wire.coords()[6],0.01);
  EXPECT_NEAR(5.5, wire.coords()[7],0.01);

  EXPECT_EQ(4, wire.nearestHandle(10, 10));
  EXPECT_EQ(14, wire.coords().size());
  EXPECT_NEAR(8, wire.coords()[10],0.01);
  EXPECT_NEAR(8.5, wire.coords()[11],0.01);

  wire.editHandle(0,2.2,3.3);
  EXPECT_NEAR(2.2, wire.coords()[2],0.01);
  EXPECT_NEAR(3.3, wire.coords()[3],0.01);
}


TEST_F(GodleyIconFixture, select)
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
  EXPECT_EQ(1,flowVars().size());
  EXPECT_EQ(1,stockVars().size());
  for (auto& i: flowVars())
    EXPECT_TRUE(i==select(i->x(),i->y()));
  for (auto& i: stockVars())
    EXPECT_TRUE(i==select(i->x(),i->y()));
  EXPECT_TRUE(!select(x(),y()));
}
  
TEST_F(ModelSuite, update)
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
  table.assetClass(1,GodleyAssetClass::asset);
  table.assetClass(2,GodleyAssetClass::liability);

  // for initial conditions below
  model->addItem(VariablePtr(VariableType::parameter,"x"));
  model->addItem(VariablePtr(VariableType::parameter,"y"));
      
  VariablePtr v(VariableType::stock,"stock1");
  model->addItem(v);
  v->init("x");
  v=VariablePtr(VariableType::stock,"stock2");
  model->addItem(v);
  v->init("y");
  godley->update();
  EXPECT_TRUE(table.initialConditionRow(1));
  EXPECT_EQ("x",table.cell(1,1));
  EXPECT_EQ("y",table.cell(1,2));

  // now remove a column
  table.deleteCol(3);
  godley->update();
  map<string,unsigned> varCount;
  for (auto& i: model->items)
    if (auto v=dynamic_cast<VariableBase*>(i.get()))
      varCount[v->name()]++;

  EXPECT_EQ(2,varCount["stock1"]);
  EXPECT_EQ(1,varCount["stock2"]);
  EXPECT_EQ(1,varCount["flow1"]);
  EXPECT_EQ(0,varCount["flow2"]);
}


TEST_F(ModelSuite,saveGroupAndInsert)
{
  unsigned origNumItems=model->numItems();
  unsigned origNumGroups=model->numGroups();
  unsigned numItemsInGroup=group0->numItems();
  saveGroupAsFile(*group0,"/tmp/testGroup.mky");
  insertGroupFromFile("/tmp/testGroup.mky");
  auto group1=dynamic_pointer_cast<Group>(canvas.itemFocus);
  EXPECT_EQ(origNumGroups+1, model->numGroups());
  EXPECT_EQ(origNumItems+numItemsInGroup, model->numItems());
  EXPECT_EQ(numItemsInGroup, group1->numItems());
}


TEST_F(ModelSuite, description)
{
  auto intop=new IntOp;
  model->addItem(intop);
  model->addItem(a);
  model->addWire(new Wire(intop->ports(0),a->ports(1)));
  intop->description("a");
  // should cowardly refuse, and give a different name
  EXPECT_EQ("a1",intop->description());
  EXPECT_EQ(1,intop->ports(0).lock()->wires().size());
  EXPECT_TRUE(intop->ports(0).lock()->wires()[0]->to()==a->ports(1).lock());
  auto intop2=new IntOp;
  model->addItem(intop2);
  intop2->description(intop->description());
  EXPECT_TRUE(intop2->description()!=intop->description());
}


template <ButtonWidgetEnums::RowCol RC>
struct ButtonWidgetFixture: public GodleyIcon, public ButtonWidget<RC>, public ::testing::Test
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
    table.assetClass(1,GodleyAssetClass::asset);
    table.assetClass(2,GodleyAssetClass::liability);
    table.assetClass(3,GodleyAssetClass::equity);
  }
};

typedef ButtonWidgetFixture<ButtonWidgetEnums::row> RowButtonWidgetFixture;
typedef ButtonWidgetFixture<ButtonWidgetEnums::col> ColButtonWidgetFixture;
  
TEST_F(RowButtonWidgetFixture, RowButtonWidget)
{
  auto origData=table.getData();
  idx=2;
  invoke(0);
  EXPECT_EQ(5,table.rows());
  // check row inserted
  for (size_t i=0; i<table.cols(); ++i)
    EXPECT_EQ("",table.cell(3,i));
    
  idx=3;
  invoke(buttonSpacing);
  EXPECT_EQ(4,table.rows());
  // check row deleted
  for (size_t i=0; i<table.cols(); ++i)
    EXPECT_EQ("c3"+str(i),table.cell(3,i));
  EXPECT_TRUE(table.getData()==origData);               
        
  // now check arrow functionality  
  idx=2; pos=second;                     
  invoke(2*buttonSpacing);
  EXPECT_EQ(4,table.rows());
  // check row swapped with next
  for (size_t i=0; i<table.cols(); ++i)
    EXPECT_EQ("c1"+str(i),table.cell(1,i));      
  for (size_t i=0; i<table.cols(); ++i)           
    EXPECT_EQ("c2"+str(i),table.cell(3,i));        // second row can only be swapped with third row
  for (size_t i=0; i<table.cols(); ++i)
    EXPECT_EQ("c3"+str(i),table.cell(2,i));             
    
  pos=middle;
  invoke(2*buttonSpacing);
  EXPECT_EQ(4,table.rows());
  for (size_t i=0; i<table.cols(); ++i)
    EXPECT_EQ("c1"+str(i),table.cell(2,i));
  for (size_t i=0; i<table.cols(); ++i)
    EXPECT_EQ("c2"+str(i),table.cell(3,i));
  for (size_t i=0; i<table.cols(); ++i)
    EXPECT_EQ("c3"+str(i),table.cell(1,i));
  invoke(3*buttonSpacing);
  EXPECT_EQ(4,table.rows());
  for (size_t i=0; i<table.cols(); ++i)
    EXPECT_EQ("c1"+str(i),table.cell(3,i));
  for (size_t i=0; i<table.cols(); ++i)
    EXPECT_EQ("c2"+str(i),table.cell(2,i));
  for (size_t i=0; i<table.cols(); ++i)
    EXPECT_EQ("c3"+str(i),table.cell(1,i));
    
  pos=last;
  invoke(2*buttonSpacing);
  EXPECT_EQ(4,table.rows());
  for (size_t i=0; i<table.cols(); ++i)
    EXPECT_EQ("c1"+str(i),table.cell(3,i));
  for (size_t i=0; i<table.cols(); ++i)
    EXPECT_EQ("c2"+str(i),table.cell(1,i));
  for (size_t i=0; i<table.cols(); ++i)
    EXPECT_EQ("c3"+str(i),table.cell(2,i));

    
  // should be no 4th button on first, second & last
  pos=first;
  auto saveData=table.getData();
  invoke(3*buttonSpacing);
  EXPECT_TRUE(table.getData()==saveData);
  pos=second;
  invoke(3*buttonSpacing);
  EXPECT_TRUE(table.getData()==saveData);      
  pos=last;
  invoke(3*buttonSpacing);
  EXPECT_TRUE(table.getData()==saveData);
}
  
TEST_F(ColButtonWidgetFixture, ColButtonWidget)
{
  auto origData=table.getData();
  idx=2;
  invoke(0);
  EXPECT_EQ(5,table.cols());
  // check row inserted
  for (size_t i=0; i<table.rows(); ++i)
    EXPECT_EQ("",table.cell(i,3));
    
  idx=3;
  invoke(buttonSpacing);
  EXPECT_EQ(4,table.cols());
  // check col deleted
  for (size_t i=0; i<table.rows(); ++i)
    EXPECT_EQ("c"+str(i)+"3",table.cell(i,3));
  EXPECT_TRUE(table.getData()==origData);
    
  // now check arrow functionality
  idx=2; pos=first;
  invoke(2*buttonSpacing);
  EXPECT_EQ(5,table.cols());
  // check row swapped with next
  for (size_t i=0; i<table.rows(); ++i)
    EXPECT_EQ("c"+str(i)+"1",table.cell(i,1));
  for (size_t i=0; i<table.rows(); ++i)
    EXPECT_EQ("c"+str(i)+"2",table.cell(i,4));
  // extra column inserted for liability class
  for (size_t i=0; i<table.rows(); ++i) 
    EXPECT_EQ("",table.cell(i,2));
  for (size_t i=0; i<table.rows(); ++i)
    EXPECT_EQ("c"+str(i)+"3",table.cell(i,3));
    
  pos=middle; idx=3;
  invoke(2*buttonSpacing);
  EXPECT_EQ(4,table.cols());
  for (size_t i=0; i<table.rows(); ++i)
    EXPECT_EQ("c"+str(i)+"1",table.cell(i,1));
  for (size_t i=0; i<table.rows(); ++i)
    EXPECT_EQ("c"+str(i)+"2",table.cell(i,3));
  for (size_t i=0; i<table.rows(); ++i)
    EXPECT_EQ("c"+str(i)+"3",table.cell(i,2));
  idx=2;
  invoke(3*buttonSpacing);
  EXPECT_EQ(5,table.cols());
  for (size_t i=0; i<table.rows(); ++i)
    EXPECT_EQ("c"+str(i)+"1",table.cell(i,1));
  for (size_t i=0; i<table.rows(); ++i)
    EXPECT_EQ("c"+str(i)+"2",table.cell(i,3));
  for (size_t i=0; i<table.rows(); ++i)
    EXPECT_EQ("c"+str(i)+"3",table.cell(i,4));
  for (size_t i=0; i<table.rows(); ++i)
    EXPECT_EQ("",table.cell(i,2));

  // should be no 4th button on first
  pos=first;
  auto saveData=table.getData();
  invoke(3*buttonSpacing);
  EXPECT_TRUE(table.getData()==saveData);
      
  // should be no button on last
  pos=last;
  invoke(buttonSpacing);
  EXPECT_TRUE(table.getData()==saveData);
  invoke(2*buttonSpacing);
  EXPECT_TRUE(table.getData()==saveData);      
  invoke(3*buttonSpacing);
  EXPECT_TRUE(table.getData()==saveData);
}

struct EmbedGodleyIcon
{
  GodleyIcon embeddedGodleyIcon;
};
  
struct GodleyTableWindowFixture: private EmbedGodleyIcon, public GodleyTableWindow, public ::testing::Test
{
  GodleyTableWindowFixture(): GodleyTableWindow(embeddedGodleyIcon) {}
};
  
TEST_F(GodleyTableWindowFixture, mouseMove)
{
  godleyIcon().table.cell(1,1)="hello";
  surface.reset(new ecolab::cairo::Surface
                (cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,NULL)));
  redraw(0,0,0,0);
  double x=colLeftMargin[1]+10, y=topTableOffset+rowHeight+5;
  EXPECT_EQ(1, colX(x));
  EXPECT_EQ(1, rowY(y));
  mouseMove(x,y);
  EXPECT_EQ(1, hoverRow);
  EXPECT_EQ(1, hoverCol);
  x=2*ButtonWidget<row>::buttonSpacing+1;
  EXPECT_EQ(rowWidget, GodleyTableWindow::clickType(x,y));
  mouseMove(x,y);
  EXPECT_EQ(2,rowWidgets[1].mouseOver());
  x=colLeftMargin[1]+ButtonWidget<row>::buttonSpacing+1;
  y=5+columnButtonsOffset;
  mouseMove(x,y);
  EXPECT_EQ(1,colWidgets[1].mouseOver());
  mouseMove(0,0);
  for (auto& i: rowWidgets) EXPECT_EQ(-1, i.mouseOver());
  for (auto& i: colWidgets) EXPECT_EQ(-1, i.mouseOver());
}                                
  
TEST_F(GodleyTableWindowFixture, mouseSelect)
{
  godleyIcon().table.cell(1,1)="hello";
  surface.reset(new ecolab::cairo::Surface
                (cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,NULL)));
  redraw(0,0,0,0);
  double x=colLeftMargin[1], y=topTableOffset+rowHeight+5;
  EXPECT_EQ(1, colX(x));
  EXPECT_EQ(1, rowY(y));
  mouseDown(x,y);
  mouseMoveB1(x+10,y);
  mouseUp(x+10,y);
  EXPECT_EQ(1, selectedRow);
  EXPECT_EQ(1, selectedCol);
  EXPECT_EQ(0,insertIdx);
  EXPECT_TRUE(selectIdx>insertIdx);
  EXPECT_TRUE(!cminsky().clipboard.getClipboard().empty());

      
}
  
TEST_F(GodleyTableWindowFixture, mouseButtons)
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
  EXPECT_EQ(colWidget, clickType(x,y));
  mouseDown(x,y);
  // should have invoked moving column 1 left
  EXPECT_EQ("r1c2",godleyIcon().table.cell(1,2));
  EXPECT_EQ("r2c2",godleyIcon().table.cell(2,2));
  EXPECT_EQ("r1c1",godleyIcon().table.cell(1,3));
  EXPECT_EQ("r2c1",godleyIcon().table.cell(2,3));
      
  x=2*ButtonWidget<row>::buttonSpacing+1, y=5+topTableOffset+rowHeight;
  EXPECT_EQ(rowWidget, clickType(x,y));
  mouseDown(x,y);
  // Row 1 cannot move down. For ticket 1064
  EXPECT_EQ("r2c2",godleyIcon().table.cell(2,2));
  EXPECT_EQ("r1c2",godleyIcon().table.cell(1,2));
      
  x=2*ButtonWidget<row>::buttonSpacing+1, y=5+topTableOffset+2*rowHeight;
  EXPECT_EQ(rowWidget, clickType(x,y));
  mouseDown(x,y);
  // should have invoked moving row 2 down, in effect swapping row 2 and 3's contents
  EXPECT_EQ("r3c2",godleyIcon().table.cell(2,2));
  EXPECT_EQ("r2c2",godleyIcon().table.cell(3,2));
      
}
  
TEST_F(GodleyTableWindowFixture, moveRowColCell)
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
  EXPECT_EQ(1,colX(x));
  EXPECT_EQ(0,rowY(y));
  mouseDown(x,y);
  x=colLeftMargin[2]+10;
  EXPECT_EQ(2,colX(x));
  mouseMoveB1(x,y);
  mouseUp(x,y);
  EXPECT_EQ("col1",godleyIcon().table.cell(0,3));

  x=leftTableOffset+10;
  y=topTableOffset+rowHeight+5;
  EXPECT_EQ(0,colX(x));
  EXPECT_EQ(1,rowY(y));
  mouseDown(x,y);
  y+=rowHeight;
  EXPECT_EQ(2,rowY(y));
  mouseMoveB1(x,y);
  mouseUp(x,y);
  EXPECT_EQ("row1",godleyIcon().table.cell(1,0));  //Cannot swap cells (1,0) and (2,0) by click dragging. For ticket 1064/1066
  EXPECT_EQ("row2",godleyIcon().table.cell(2,0));

  // check moving a cell
  x=colLeftMargin[1]+10;
  y=topTableOffset+rowHeight+5;
  EXPECT_EQ(1,colX(x));
  EXPECT_EQ(1,rowY(y));
  godleyIcon().table.cell(1,1)="cell11";
  mouseDown(x,y);
  x=colLeftMargin[2]+10;
  y+=rowHeight;
  mouseMoveB1(x,y);
  mouseUp(x,y);
  EXPECT_EQ("cell11",godleyIcon().table.cell(2,2));
}

#define XK_MISCELLANY
#define XK_XKB_KEYS
#include <X11/keysymdef.h>
  
TEST_F(GodleyTableWindowFixture, keyPress)
{
  godleyIcon().table.resize(3,4);
  selectedCol=1;
  selectedRow=1;
  selectIdx=insertIdx=0;
  keyPress({'a',"a"}); keyPress({'b',"b"}); keyPress({'b',"b"}); insertIdx=2; keyPress({XK_Delete,""});
  keyPress({'c',"c"}); keyPress({'c',"c"}); keyPress({XK_BackSpace,""});
  EXPECT_EQ("abc",godleyIcon().table.cell(1,1));

  godleyIcon().table.cell(0,1)="stock1";
  EXPECT_EQ(0,godleyIcon().stockVars().size());
  keyPress({XK_Return,""}); // should cause update to be called
  // unfortunately, this is a freestanding GodleyIcon, so update has no effect
  //      EXPECT_EQ(1,godleyIcon().stockVars().size());
  //      EXPECT_EQ("stock1",godleyIcon().stockVars()[0]->name());
  EXPECT_EQ(-1,selectedCol);
  EXPECT_EQ(-1,selectedRow);

      
  selectedCol=1;
  selectedRow=1;
  godleyIcon().table.savedText="abc";
  keyPress({'d',"d"}); keyPress({XK_Escape,""}); // should revert to previous
  EXPECT_EQ("abc",godleyIcon().table.cell(1,1));
  EXPECT_EQ(-1,selectedCol);
  EXPECT_EQ(-1,selectedRow);

  // tab, arrow movement
  selectedCol=1;
  selectedRow=1;
  selectIdx=insertIdx=0;
  keyPress({XK_Right,""});
  EXPECT_EQ(1,insertIdx);
  keyPress({XK_Left,""});
  EXPECT_EQ(0,insertIdx);
  // (1,0) cell no longer selectable, selectedRow and selectedCol become -1
  keyPress({XK_Left,""});
  EXPECT_EQ(-1,selectedCol);   
  EXPECT_EQ(-1,selectedRow);      
  keyPress({XK_Tab,""});          //  No cell selected, selectedRow = 0 and selectedCol = 1 when tab button is pressed, see godleyTableWindow line 633
  EXPECT_EQ(1,selectedCol);  
  keyPress({XK_Tab,""});
  EXPECT_EQ(2,selectedCol);
  keyPress({XK_Right,""});     
  EXPECT_EQ(3,selectedCol);  
  keyPress({XK_Down,""});
  EXPECT_EQ(1,selectedRow);  // selectedRow = 0 from initial tab press 
  keyPress({XK_Tab,""});
  EXPECT_EQ(0,selectedCol);  
  keyPress({XK_Left,""});
  EXPECT_EQ(3,selectedCol);   
  EXPECT_EQ(1,selectedRow);   
  keyPress({XK_Right,""});
  EXPECT_EQ(0,selectedCol);   // Moved down one row  
  EXPECT_EQ(2,selectedRow);
  keyPress({XK_ISO_Left_Tab,""});
  EXPECT_EQ(3,selectedCol);
  keyPress({XK_ISO_Left_Tab,""});
  EXPECT_EQ(2,selectedCol);
  EXPECT_EQ(1,selectedRow);    
  keyPress({XK_Right,""});
  EXPECT_EQ(3,selectedCol);
  EXPECT_EQ(1,selectedRow);       
  keyPress({XK_Tab,""}); // check wrap around
  EXPECT_EQ(0,selectedCol);
  EXPECT_EQ(2,selectedRow);

  // cut, copy paste
  selectedCol=1;
  selectedRow=1;
  selectIdx=0;
  insertIdx=1;
  cminsky().clipboard.putClipboard("");
  keyPress({XK_Control_L,""}); keyPress({'c',"\x3"}); //copy
  EXPECT_EQ("a",cminsky().clipboard.getClipboard());
  cminsky().clipboard.putClipboard("");
  keyPress({XK_Control_L,""}); keyPress({'x',"\x18"});  //cut
  EXPECT_EQ("a",cminsky().clipboard.getClipboard());
  EXPECT_EQ("bc",godleyIcon().table.cell(1,1));
  keyPress({XK_Control_L,""}); keyPress({'v',"\x16"});  //paste
  EXPECT_EQ("abc",godleyIcon().table.cell(1,1));

  // initial cell movement when nothing selected
  selectedCol=-1; selectedRow=-1;
  keyPress({XK_Tab,""});
  EXPECT_EQ(1,selectedCol);
  EXPECT_EQ(0,selectedRow);

  selectedCol=-1; selectedRow=-1;
  keyPress({XK_ISO_Left_Tab,""});
  EXPECT_EQ(3,selectedCol);
  EXPECT_EQ(2,selectedRow);
      
  selectedCol=-1; selectedRow=-1;
  keyPress({XK_Left,""});
  EXPECT_EQ(3,selectedCol);
  EXPECT_EQ(0,selectedRow);
      
  selectedCol=-1; selectedRow=-1;
  keyPress({XK_Right,""});
  EXPECT_EQ(1,selectedCol);
  EXPECT_EQ(0,selectedRow);
      
  selectedCol=-1; selectedRow=-1;
  keyPress({XK_Down,""});
  EXPECT_EQ(0,selectedCol);
  EXPECT_EQ(2,selectedRow);    // Initial Conditions cell cannot be selected. For ticket 1064
      
  selectedCol=-1; selectedRow=-1;
  keyPress({XK_Up,""});
  EXPECT_EQ(0,selectedCol);
  EXPECT_EQ(2,selectedRow);
      
}
  
TEST_F(GodleyTableWindowFixture, addDelVars)
{
  surface.reset(new ecolab::cairo::Surface
                (cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,NULL)));
  redraw(0,0,0,0);
  double x=colLeftMargin[1]+10, y=topTableOffset+5+rowHeight;
  EXPECT_EQ(1,colX(x));
  EXPECT_EQ(1,rowY(y));
  auto& t=godleyIcon().table;
  t.cell(1,0)="row1";
  t.cell(0,1)="col1";
  t.cell(0,2)="col2";
  t.cell(0,3)="col3";
  addFlow(y);
  EXPECT_EQ(3,t.rows());
  EXPECT_EQ("",t.cell(2,0));
  addFlow(y+rowHeight);
  EXPECT_EQ(4,t.rows());
  EXPECT_EQ("",t.cell(3,0));      
  t.cell(3,0)="row3";
  deleteFlow(y+rowHeight);
  EXPECT_EQ(3,t.rows()); 
  EXPECT_EQ("row3",t.cell(2,0));
  deleteFlow(y);                        // Check that row1 cannot be deleted. For ticket 1064
  EXPECT_EQ(3,t.rows()); 
  EXPECT_EQ("row3",t.cell(2,0));      

  addStockVar(x);
  EXPECT_EQ(5,t.cols());
  EXPECT_EQ("",t.cell(0,2));
  t.cell(0,2)="newCol";

  deleteStockVar(x);
  EXPECT_EQ(4,t.cols());
  EXPECT_EQ("newCol",t.cell(0,1));
}

TEST_F(GodleyTableWindowFixture, undoRedo)
{
  auto& t=godleyIcon().table;
  t.cell(1,0)="row1";
  pushHistory();
  t.cell(1,0)="xxx";
  undo(1);
  EXPECT_EQ("row1",t.cell(1,0));
  undo(-1);
  EXPECT_EQ("xxx",t.cell(1,0));
}

TEST_F(ModelSuite, copyBetweenCols)
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
  EXPECT_EQ(1,gw.colXZoomed(x1));
  EXPECT_EQ(2,gw.colXZoomed(x2));
  EXPECT_EQ(2,gw.rowYZoomed(y));

  // move cell from asset to liability
  gw.mouseDown(x2,y);
  gw.mouseUp(x1,y);
  gw.update();

  // this scenario should not add extra rows
  EXPECT_EQ(3,godley1->table.rows());
  EXPECT_EQ(3,godley2->table.rows());
  EXPECT_EQ(4,godley1->table.cols());
  EXPECT_EQ(4,godley2->table.cols());

  EXPECT_EQ("",godley1->table.cell(2,1));
  EXPECT_EQ("",godley2->table.cell(2,2));
  FlowCoef fc(godley1->table.cell(2,2));
  EXPECT_EQ("foobar",fc.name);
  EXPECT_EQ(1,fc.coef);
  fc=FlowCoef(godley2->table.cell(2,1));
  EXPECT_EQ("foobar",fc.name);
  EXPECT_EQ(1,fc.coef);
}
   
TEST_F(ModelSuite, almalgamateLines)
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
  EXPECT_EQ(3,godley1->table.rows());
  FlowCoef fc(godley1->table.cell(2,1));
  EXPECT_EQ("foobar",fc.name);
  EXPECT_EQ(-1,fc.coef);

  EXPECT_EQ(3,godley2->table.rows());
  fc=FlowCoef(godley2->table.cell(2,2));
  EXPECT_EQ("foobar",fc.name);
  EXPECT_EQ(-1,fc.coef);

  // Now add a name to the row, to fix things
  godley1->table.resize(4,4);
  godley1->table.cell(3,1)="2foobar";
  balanceDuplicateColumns(*godley1,1);
  godley1->table.cell(2,0)="tax";
  balanceDuplicateColumns(*godley2,2);
       
  // extra row should not be amalgamated on godley1, but amalgamated on godley2
  EXPECT_EQ(4,godley1->table.rows());
  fc=FlowCoef(godley1->table.cell(2,1));
  EXPECT_EQ("foobar",fc.name);
  EXPECT_EQ(-1,fc.coef);
  fc=FlowCoef(godley1->table.cell(3,1));
  EXPECT_EQ("foobar",fc.name);
  EXPECT_EQ(2,fc.coef);

  EXPECT_EQ(3,godley2->table.rows());
  fc=FlowCoef(godley2->table.cell(2,2));
  EXPECT_EQ("foobar",fc.name);
  EXPECT_EQ(1,fc.coef);
      
}

TEST_F(ModelSuite, saveAsGroup)
{
  group0->inVariables.push_back(a);
  group0->makeSubroutine();
  save("foo.mky");
  EXPECT_TRUE(group0->inVariables.size());
  EXPECT_TRUE(group0->outVariables.size());
  saveGroupAsFile(*group0,"group0.mky");
  insertGroupFromFile("group0.mky");
  Group& newGroup=dynamic_cast<Group&>(*canvas.itemFocus);
  // check I/O variables
  EXPECT_EQ(group0->inVariables.size(),newGroup.inVariables.size());
  EXPECT_EQ(group0->outVariables.size(),newGroup.outVariables.size());
  for (int i=0; i<group0->inVariables.size(); ++i)
    EXPECT_EQ(group0->inVariables[i]->name(), newGroup.inVariables[i]->name());
  for (int i=0; i<group0->outVariables.size(); ++i)
    EXPECT_EQ(group0->outVariables[i]->name(), newGroup.outVariables[i]->name());
  // check items
  EXPECT_EQ(group0->items.size(), newGroup.items.size());
}
     

