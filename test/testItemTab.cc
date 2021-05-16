/*
  @copyright Steve Keen 2021
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
using namespace std;

namespace
{
  struct TestFixture: public Minsky
  {
    LocalMinsky lm;
    TestFixture(): lm(*this) {}
  };
}

SUITE(ItemTab)
{
  TEST_FIXTURE(TestFixture, populateItemVector)
    {
      VariablePtr a(VariableType::parameter, "a");
      VariablePtr b(VariableType::parameter, "b");
      VariablePtr c(VariableType::flow, "c");
      auto intOp=make_shared<IntOp>();
      model->addItem(a);
      model->addItem(b);
      model->addItem(c);
      model->addItem(intOp);
      a->toggleVarTabDisplay();
      b->toggleVarTabDisplay();
      c->toggleVarTabDisplay();
      intOp->intVar->toggleVarTabDisplay();
      parameterTab.populateItemVector();
      CHECK_EQUAL(2,parameterTab.itemVector.size());
      set<ItemPtr> params(parameterTab.itemVector.begin(), parameterTab.itemVector.end());
      CHECK_EQUAL(1, params.count(a));
      CHECK_EQUAL(1, params.count(b));

      // parameters are always displayed in the tab
      
      variableTab.populateItemVector();
      CHECK_EQUAL(2,variableTab.itemVector.size());
      set<ItemPtr> vars(variableTab.itemVector.begin(), variableTab.itemVector.end());
      CHECK_EQUAL(1, vars.count(c));
      CHECK_EQUAL(1, vars.count(intOp->intVar));

      intOp->intVar->toggleVarTabDisplay();
      variableTab.populateItemVector();
      CHECK_EQUAL(1,variableTab.itemVector.size());
      set<ItemPtr> vars1(variableTab.itemVector.begin(), variableTab.itemVector.end());
      CHECK_EQUAL(1, vars1.count(c));
      CHECK_EQUAL(0, vars1.count(intOp->intVar));
      
    }
}
