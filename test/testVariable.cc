/*
  @copyright Steve Keen 2013
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

#include "cairoItems.h"
#include "variableValue.h"
#include "group.h"
#include "selection.h"
#include "lasso.h"
#include "valueId.h"
#include "minsky_epilogue.h"
#include <gtest/gtest.h>

using namespace minsky;

TEST(Variable, scoping)
  {
    EXPECT_EQ(0, scope(":foo"));
    EXPECT_THROW(scope("furry[1]:foo"), std::exception);
    EXPECT_EQ(1, scope("1:foo"));
    EXPECT_THROW(scope("foo"), ecolab::error);
    
    EXPECT_EQ("foo", uqName(":foo"));
    EXPECT_EQ("foo", uqName("furry[1]:foo"));
    EXPECT_EQ("foo", uqName("1:foo"));
    EXPECT_EQ("foo", uqName("foo"));

    EXPECT_EQ(":foo",minsky::valueId(0,"foo"));
    EXPECT_EQ("1:foo",minsky::valueIdCanonical(1,"foo"));
    EXPECT_EQ(":foo",minsky::valueId(":foo"));
    EXPECT_THROW(minsky::valueId("foo"), ecolab::error);

  }

TEST(Variable, typeMismatch)
  {
    // Test for ticket 1473 - prevent creating variables of different types with the same name
    auto& minsky=cminsky();
    minsky.model->clear();
    
    // Create a flow variable with name "x"
    auto var1 = minsky.model->addItem(VariablePtr(VariableType::flow, "x"));
    EXPECT_EQ(VariableType::flow, var1->type());
    
    // Try to create a stock variable with the same name - should throw an error
    auto var2 = minsky.model->addItem(VariablePtr(VariableType::stock, "x"));
    EXPECT_THROW(var2->name("x"), std::exception);
    
    // Try to create a parameter variable with the same name - should also throw an error  
    auto var3 = minsky.model->addItem(VariablePtr(VariableType::parameter, "x"));
    EXPECT_THROW(var3->name("x"), std::exception);
    
    // Creating a variable with the same name and type should work
    auto var4 = minsky.model->addItem(VariablePtr(VariableType::flow, "x"));
    EXPECT_NO_THROW(var4->name("x"));
    EXPECT_EQ(VariableType::flow, var4->type());
  }
