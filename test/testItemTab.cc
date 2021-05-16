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
#include <cairo_base.h> 
#include "minsky_epilogue.h"
#include <UnitTest++/UnitTest++.h>

using namespace minsky;
using namespace std;

namespace
{
  struct TestFixture: public Minsky
  {
    LocalMinsky lm;
    ecolab::cairo::Surface surface{cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,NULL)};
    TestFixture(): lm(*this) {}
  };

  struct ParVarTabFixture: public TestFixture
  {
    VariablePtr a{VariableType::parameter, "a"};
    VariablePtr b{VariableType::parameter, "b"};
    VariablePtr c{VariableType::flow, "c"};
    shared_ptr<IntOp> intOp=make_shared<IntOp>();
     ParVarTabFixture()
    {
      model->addItem(a);
      model->addItem(b);
      model->addItem(c);
      model->addItem(intOp);
      a->toggleVarTabDisplay();
      b->toggleVarTabDisplay();
      c->toggleVarTabDisplay();
      intOp->intVar->toggleVarTabDisplay();
      parameterTab.populateItemVector();
      variableTab.populateItemVector();
    }
  };
  
}

SUITE(ItemTab)
{
  TEST_FIXTURE(ParVarTabFixture, populateItemVector)
    {
      CHECK_EQUAL(2,parameterTab.itemVector.size());
      set<ItemPtr> params(parameterTab.itemVector.begin(), parameterTab.itemVector.end());
      CHECK_EQUAL(1, params.count(a));
      CHECK_EQUAL(1, params.count(b));

      // parameters are always displayed in the tab
      
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

  TEST_FIXTURE(ParVarTabFixture, colXrowY)
    {
      parameterTab.draw(surface.cairo());
      // TODO: this test is horribly implementation specific
      CHECK_EQUAL(1, parameterTab.colLeftMargin.count(0));
      CHECK(parameterTab.colLeftMargin[0].size()>1);
      CHECK_EQUAL(2, parameterTab.colX(parameterTab.colLeftMargin[0][1]));

      for (auto& i: parameterTab.colLeftMargin[0])
        cout << i << " ";
      cout << endl;
      for (auto& i: parameterTab.rowTopMargin)
        cout << i << " ";
      cout << endl;
      
      CHECK_EQUAL(2, parameterTab.rowTopMargin.size());
      CHECK_EQUAL(1, parameterTab.rowY(parameterTab.rowTopMargin[1]));
    }

}
