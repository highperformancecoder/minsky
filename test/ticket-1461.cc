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
#undef True
#include <gtest/gtest.h>
using namespace minsky;

namespace
{
  class Ticket1461 : public Minsky, public ::testing::Test
  {
  public:
    LocalMinsky lm;
    string savedMessage;
    Ticket1461(): lm(*this){}
    void message(const string& x) override {savedMessage=x;}
  };


TEST_F(Ticket1461, clone)
  {
    canvas.addVariable("foo",VariableType::flow);
    auto var=canvas.itemFocus->variableCast();
    EXPECT_TRUE(var);
    var->tooltip("hello");
    VariablePtr clone(var->clone());
    EXPECT_EQ("hello",clone->tooltip());
  }

TEST_F(Ticket1461, copy)
  {
    canvas.addVariable("foo", VariableType::flow);
    auto& var=*canvas.itemFocus->variableCast();
    var.tooltip("hello");
    var.setUnits("m");
    canvas.selection.insertItem(canvas.itemFocus);
    canvas.itemFocus.reset();
    copy();
    paste();
    auto& varCopy=*canvas.itemFocus->variableCast();
    EXPECT_TRUE(&var!=&varCopy);
    EXPECT_EQ("hello",varCopy.tooltip());
    EXPECT_EQ("m",varCopy.unitsStr());
  }

}
