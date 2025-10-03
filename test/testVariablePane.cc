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
  class VariablePaneSuite : public Minsky, public ::testing::Test
  {
  public:
    LocalMinsky lm;
    VariablePaneSuite(): lm(*this)
    {
    }
  };
}

TEST_F(VariablePaneSuite, updateAndDraw)
  {
    load("1Free.mky");
    variablePane.updateWithHeight(100);
    // count the number of non-temporary variables
    size_t nonTempCount=0;
    for (auto& [valueId, v]: variableValues)
      if (v->type()!=VariableType::tempFlow)
        nonTempCount++;
    EXPECT_TRUE(nonTempCount<=variablePane.numRows()*variablePane.numCols());
    variablePane.renderToSVG("1FreeAllVariableTab.svg");
    variablePane.deselect(VariableType::parameter);
    variablePane.deselect(VariableType::stock);
    variablePane.update();
    variablePane.renderToSVG("1FreeNoParamStock.svg");
    variablePane.select(VariableType::parameter);
    variablePane.update();
    variablePane.renderToSVG("1FreeNoStock.svg");
  }

TEST_F(VariablePaneSuite, emplace)
  {
    load("1Free.mky");
    variablePane.updateWithHeight(100);
    EXPECT_EQ("Variable:undefined", variablePane.cell(variablePane.numRows()-1,variablePane.numCols()-1).variable().classType());
    EXPECT_TRUE(variablePane.cell(0,0).width()>0);
    variablePane.cell(0,0).emplace();
    EXPECT_TRUE(canvas.itemFocus);
    auto v=canvas.itemFocus->variableCast();
    EXPECT_TRUE(v);
    EXPECT_EQ(variablePane.cell(0,0).variable().classType(), canvas.itemFocus->classType());
  }
