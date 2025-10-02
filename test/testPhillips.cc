/*
  @copyright Steve Keen 2024
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
#include "minsky_epilogue.h"
#undef True
#include <gtest/gtest.h>
using namespace minsky;

namespace
{
  class TestFixture : public Minsky, public ::testing::Test
  {
  public:
    LocalMinsky lm;
    TestFixture(): lm(*this)
    {
    }
  };
}


TEST_F(TestFixture,fromInit)
  {
    load("1Free.mky");
    phillipsDiagram.clear();
    phillipsDiagram.init();
    set<string> stocks;
    for (auto& i: model->items)
      if (auto v=dynamic_cast<Variable<VariableType::stock>*>(i.get()))
        stocks.insert(v->rawName());
    EXPECT_EQ(stocks.size(), phillipsDiagram.stocks.size());
    for (auto& [name,phillipsStock]: phillipsDiagram.stocks)
      {
        EXPECT_TRUE(stocks.count(name));
        EXPECT_TRUE(stocks.count(phillipsStock.rawName()));
      }
    EXPECT_EQ(3,phillipsDiagram.flows.size());
    phillipsDiagram.renderToSVG("1FreePhillips.svg");
    
    // move a stock and bend a wire
    auto& phillipsStock=phillipsDiagram.stocks.begin()->second;
    phillipsStock.moveTo(phillipsStock.x()+10,phillipsStock.y()+10);
    auto& flow=phillipsDiagram.flows.begin()->second;
    auto newCoords=flow.coords();
    flow.coords({newCoords[0],newCoords[1],0.5f*(newCoords[0]+newCoords[2]),newCoords[1],newCoords[2],newCoords[3]});
    phillipsDiagram.renderToSVG("1FreePhillipsMutated.svg");
    auto newX=phillipsStock.x(), newY=phillipsStock.y();
    newCoords=flow.coords();
    
    save("1FreePhillipsMutated");
    phillipsDiagram.renderToSVG("1FreePhillipsMutated.svg");
    
    clearAllMaps(); // nb phillipsStock & flow above are now invalidated
    EXPECT_TRUE(phillipsDiagram.stocks.empty());
    EXPECT_TRUE(phillipsDiagram.flows.empty());
    
    // now check this change can be persisted
    load("1FreePhillipsMutated");
    phillipsDiagram.init();
    phillipsDiagram.renderToSVG("1FreePhillipsMutatedLoaded.svg");
    EXPECT_FALSE(phillipsDiagram.stocks.empty());
    EXPECT_FALSE(phillipsDiagram.flows.empty());
    auto& newPhillipsStock=phillipsDiagram.stocks.begin()->second;
    EXPECT_NEAR(newX, newPhillipsStock.x(),0.1);
    EXPECT_NEAR(newY, newPhillipsStock.y(),0.1);
    auto& newFlow=phillipsDiagram.flows.begin()->second;
    EXPECT_EQ(newCoords.size(), newFlow.coords().size());
    for (size_t i=0; i<newCoords.size(); ++i)
      EXPECT_NEAR(newCoords[i], newFlow.coords()[i], 0.1);

  }

  TEST_F(TestFixture,emptyModelClears)
  {
    load("1Free.mky");
    phillipsDiagram.init();
    EXPECT_FALSE(phillipsDiagram.stocks.empty());
    EXPECT_FALSE(phillipsDiagram.flows.empty());
    // remove controlled items first to prevent foot-on-foot errors
    for (auto& i: model->items) i->removeControlledItems();
    model->items.clear();
    phillipsDiagram.init();
    EXPECT_TRUE(phillipsDiagram.stocks.empty());
    EXPECT_TRUE(phillipsDiagram.flows.empty());
  }

  TEST_F(TestFixture,panning)
    {
      phillipsDiagram.moveTo(100,100);
      vector expected{100,100};
      for (size_t i=0; i<2; ++i)
        EXPECT_EQ(expected[i], phillipsDiagram.position()[i]);
    }

  
  TEST_F(TestFixture,movingItem)
    {
      load("1Free.mky");
      phillipsDiagram.init();
      auto& phillipsStock=phillipsDiagram.stocks.begin()->second;
      float x=100, y=100;
      EXPECT_TRUE(abs(phillipsStock.x()-x)>1 && abs(phillipsStock.y()-y)>1);
      phillipsDiagram.mouseDown(phillipsStock.x(),phillipsStock.y());
      phillipsDiagram.mouseUp(x,y);
      EXPECT_NEAR(x,phillipsStock.x(),0.1);
      EXPECT_NEAR(y,phillipsStock.y(),0.1);
    }

    TEST_F(TestFixture,rotateItem)
    {
      load("1Free.mky");
      phillipsDiagram.init();
      auto& phillipsStock=phillipsDiagram.stocks.begin()->second;
      float x=phillipsStock.x()+100, y=phillipsStock.y()+100;
      EXPECT_EQ(0,phillipsStock.rotation());
      phillipsDiagram.startRotatingItem(phillipsStock.x(),phillipsStock.y());
      phillipsDiagram.mouseUp(x,y);
      EXPECT_NEAR(45,phillipsStock.rotation(),0.1);
    }
    
    TEST_F(TestFixture,bendFlow)
    {
      load("1Free.mky");
      phillipsDiagram.init();
      phillipsDiagram.renderToSVG("1FreePhillips.svg");
      auto& flow=phillipsDiagram.flows.begin()->second;
      auto coords=flow.coords();
      float x=0.5*(coords[0]+coords[2]), y=0.5*(coords[1]+coords[3]);
      phillipsDiagram.mouseMove(x,y);
      EXPECT_TRUE(flow.mouseFocus);
      phillipsDiagram.mouseDown(x,y);
      x+=20; y-=20;
      phillipsDiagram.mouseUp(x,y);
      EXPECT_EQ(6,flow.coords().size());
      vector expected{coords[0],coords[1],x,y,coords[2],coords[3]};
      for (size_t i=0; i<flow.coords().size(); ++i)
        EXPECT_NEAR(expected[i], flow.coords()[i], 0.1);
    }
