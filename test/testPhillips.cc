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
      
      // now check this change can be persisted
      save("1FreePhillipsMutated");
      phillipsDiagram.renderToSVG("1FreePhillipsMutated.svg");
      clearAllMaps(); // nb phillipsStock & flow above are now invalidated
      CHECK(phillipsDiagram.stocks.empty());
      CHECK(phillipsDiagram.flows.empty());
      load("1FreePhillipsMutated");
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
}
