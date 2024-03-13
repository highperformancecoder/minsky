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
#include <UnitTest++/UnitTest++.h>
using namespace minsky;

namespace
{
  struct TestFixture: public Minsky
  {
    LocalMinsky lm;
    TestFixture(): lm(*this)
    {
    }
  };
}

SUITE(Phillips)
{
  TEST_FIXTURE(TestFixture,fromInit)
    {
      load("1Free.mky");
      phillipsDiagram.clear();
      phillipsDiagram.init();
      set<string> stocks;
      for (auto& i: model->items)
        if (auto v=dynamic_cast<Variable<VariableType::stock>*>(i.get()))
          stocks.insert(v->rawName());
      CHECK_EQUAL(stocks.size(), phillipsDiagram.stocks.size());
      for (auto& [name,phillipsStock]: phillipsDiagram.stocks)
        {
          CHECK(stocks.count(name));
          CHECK(stocks.count(phillipsStock.rawName()));
        }
      CHECK_EQUAL(3,phillipsDiagram.flows.size());
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
      CHECK(phillipsDiagram.stocks.empty());
      CHECK(phillipsDiagram.flows.empty());
      
      // now check this change can be persisted
      load("1FreePhillipsMutated");
      phillipsDiagram.init();
      phillipsDiagram.renderToSVG("1FreePhillipsMutatedLoaded.svg");
      CHECK(!phillipsDiagram.stocks.empty());
      CHECK(!phillipsDiagram.flows.empty());
      auto& newPhillipsStock=phillipsDiagram.stocks.begin()->second;
      CHECK_CLOSE(newX, newPhillipsStock.x(),0.1);
      CHECK_CLOSE(newY, newPhillipsStock.y(),0.1);
      auto& newFlow=phillipsDiagram.flows.begin()->second;
      CHECK_EQUAL(newCoords.size(), newFlow.coords().size());
      CHECK_ARRAY_CLOSE(newCoords, newFlow.coords(), newCoords.size(),0.1); 

    }

    TEST_FIXTURE(TestFixture,emptyModelClears)
    {
      load("1Free.mky");
      phillipsDiagram.init();
      CHECK(!phillipsDiagram.stocks.empty());
      CHECK(!phillipsDiagram.flows.empty());
      // remove controlled items first to prevent foot-on-foot errors
      for (auto& i: model->items) i->removeControlledItems();
      model->items.clear();
      phillipsDiagram.init();
      CHECK(phillipsDiagram.stocks.empty());
      CHECK(phillipsDiagram.flows.empty());
    }

    TEST_FIXTURE(TestFixture,panning)
      {
        phillipsDiagram.moveTo(100,100);
        vector expected{100,100};
        CHECK_ARRAY_EQUAL(expected,phillipsDiagram.position(),2);
      }

    
    TEST_FIXTURE(TestFixture,movingItem)
      {
        load("1Free.mky");
        phillipsDiagram.init();
        auto& phillipsStock=phillipsDiagram.stocks.begin()->second;
        float x=100, y=100;
        CHECK(abs(phillipsStock.x()-x)>1 && abs(phillipsStock.y()-y)>1);
        phillipsDiagram.mouseDown(phillipsStock.x(),phillipsStock.y());
        phillipsDiagram.mouseUp(x,y);
        CHECK_CLOSE(x,phillipsStock.x(),0.1);
        CHECK_CLOSE(y,phillipsStock.y(),0.1);
      }

      TEST_FIXTURE(TestFixture,rotateItem)
      {
        load("1Free.mky");
        phillipsDiagram.init();
        auto& phillipsStock=phillipsDiagram.stocks.begin()->second;
        float x=phillipsStock.x()+100, y=phillipsStock.y()+100;
        CHECK_EQUAL(0,phillipsStock.rotation());
        phillipsDiagram.startRotatingItem(phillipsStock.x(),phillipsStock.y());
        phillipsDiagram.mouseUp(x,y);
        CHECK_CLOSE(45,phillipsStock.rotation(),0.1);
      }
      
      TEST_FIXTURE(TestFixture,bendFlow)
      {
        load("1Free.mky");
        phillipsDiagram.init();
        phillipsDiagram.renderToSVG("1FreePhillips.svg");
        auto& flow=phillipsDiagram.flows.begin()->second;
        auto coords=flow.coords();
        float x=0.5*(coords[0]+coords[2]), y=0.5*(coords[1]+coords[3]);
        phillipsDiagram.mouseMove(x,y);
        CHECK(flow.mouseFocus);
        phillipsDiagram.mouseDown(x,y);
        x+=20; y-=20;
        phillipsDiagram.mouseUp(x,y);
        CHECK_EQUAL(6,flow.coords().size());
        vector expected{coords[0],coords[1],x,y,coords[2],coords[3]};
        CHECK_ARRAY_CLOSE(expected, flow.coords(),flow.coords().size(), 0.1);
      }

      
}
