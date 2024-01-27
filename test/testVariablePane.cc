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
  struct MinskyFixture: public Minsky
  {
    LocalMinsky lm;
    MinskyFixture(): lm(*this)
    {
    }
  };
}

SUITE(VariablePane)
{
  TEST_FIXTURE(MinskyFixture,updateAndDraw)
    {
      load("1Free.mky");
      variablePane.updateWithHeight(100);
      CHECK(variableValues.size()<=variablePane.numRows()*variablePane.numCols());
      variablePane.renderToSVG("1FreeAllVariableTab.svg");
      variablePane.deselect(VariableType::parameter);
      variablePane.deselect(VariableType::stock);
      variablePane.update();
      variablePane.renderToSVG("1FreeNoParamStock.svg");
      variablePane.select(VariableType::parameter);
      variablePane.update();
      variablePane.renderToSVG("1FreeNoStock.svg");
    }
  
  TEST_FIXTURE(MinskyFixture,emplace)
    {
      load("1Free.mky");
      variablePane.updateWithHeight(100);
      CHECK_EQUAL("Variable:undefined", variablePane.cell(variablePane.numRows()-1,variablePane.numCols()-1).variable().classType());
      CHECK(variablePane.cell(0,0).width()>0);
      variablePane.cell(0,0).emplace();
      CHECK(canvas.itemFocus);
      auto v=canvas.itemFocus->variableCast();
      CHECK(v);
      CHECK_EQUAL(variablePane.cell(0,0).variable().classType(), canvas.itemFocus->classType());
    }
}
