/*
  @copyright Steve Keen 2022
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
    string savedMessage;
    TestFixture(): lm(*this)
    {
    }
    void message(const string& x) override {savedMessage=x;}
  };
}

SUITE(Ticket1461)
{
  TEST(clone)
    {
      Variable<VariableType::flow> var;
      var.tooltip="hello";
      auto clone=var.clone();
      CHECK_EQUAL("hello",clone->tooltip);
    }

  TEST_FIXTURE(TestFixture, copy)
    {
      canvas.addVariable("foo", VariableType::flow);
      auto& var=*canvas.itemFocus->variableCast();
      var.tooltip="hello";
      var.setUnits("m");
      canvas.selection.insertItem(canvas.itemFocus);
      copy();
      paste();
      auto& varCopy=*canvas.itemFocus->variableCast();
      CHECK(&var!=&varCopy);
      CHECK_EQUAL("hello",varCopy.tooltip);
      CHECK_EQUAL("m",varCopy.unitsStr());
    }

}

