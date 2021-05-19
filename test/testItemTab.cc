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
      model->addWire(*a, *c, 1);
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

    TEST_FIXTURE(ParVarTabFixture, checkCells)
    {
      variableTab.surface=make_shared<ecolab::cairo::Surface>(cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,NULL));
      ecolab::Pango pango(surface.cairo());
      CHECK_EQUAL(2,variableTab.itemVector.size());
      // name
      pango.setMarkup(latexToPango(variableTab.itemVector[0]->variableCast()->name()));
      CHECK_EQUAL(pango.width(),variableTab.cell(1,0).width());
      // definition
      pango.setMarkup(variableTab.itemVector[0]->variableCast()->definition());
      CHECK_EQUAL(pango.width(),variableTab.cell(1,1).width());
      pango.setMarkup(variableTab.itemVector[1]->variableCast()->definition());
      CHECK_EQUAL(pango.width(),variableTab.cell(1,2).width());
      // test other columns maybe?
    }

    TEST_FIXTURE(ParVarTabFixture, itemAt)
    {
      auto item0=variableTab.itemVector[0];
      CHECK_EQUAL(item0, variableTab.itemAt(item0->itemTabX, item0->itemTabY));
      auto item1=variableTab.itemVector[1];
      CHECK_EQUAL(item1, variableTab.itemAt(item1->itemTabX, item1->itemTabY));
    }

    TEST_FIXTURE(ParVarTabFixture, moveItem)
    {
      auto item0=variableTab.itemVector[0];
      item0->itemTabX=100; item0->itemTabY=200;
      double x=item0->itemTabX, y=item0->itemTabY;
      variableTab.mouseDownCommon(x,y);
      variableTab.mouseUp(x+10, y+10);
      CHECK_EQUAL(x+10, item0->itemTabX);
      CHECK_EQUAL(y+10, item0->itemTabY);
    }
}
