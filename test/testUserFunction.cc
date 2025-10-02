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
#include <gtest/gtest.h>
#include "selection.h"
#include "userFunction.h"
#include "minsky.h"
#include "minsky_epilogue.h"

using namespace minsky;
using namespace std;

class MinskyFixture : public Minsky, public ::testing::Test
{
public:
  LocalMinsky lm{*this};
};


TEST(UserFunction, symbolNames1)
  {
    UserFunction f("test","input/referenceInput");
    vector<string> expected{"input","referenceInput"};
    EXPECT_TRUE(f.symbolNames()==expected);
  }

TEST_F(MinskyFixture, referenceMinskyVariables)
  {
    variableValues.emplace(":foo", VariableType::flow);
    variableValues.emplace(":bar", VariableType::flow);
    variableValues[":foo"]->allocValue()=3.0;
    variableValues[":bar"]->allocValue()=5.0;
    UserFunction f("test","foo+bar");
    f.compile();
    EXPECT_EQ(8, f({}));
  }

TEST_F(MinskyFixture, arguments)
  {
    UserFunction f("test","x+y");
    f.argNames={"x","y"};
    f.compile();
    EXPECT_EQ(7, f({3,4}));
  }  

TEST_F(MinskyFixture, error)
  {
    UserFunction f("test","x+");
    f.argNames={"x","y"};
    EXPECT_THROW(f.compile(), std::exception);
  }  

TEST_F(MinskyFixture, stringProcessing)
  {
     UserFunction f("test(x)","x:='foo\\'s bar'[]");
     f.compile(); // checks this is syntactically valid
     auto sn=f.symbolNames();
     EXPECT_EQ(1,sn.size());
     EXPECT_EQ("x",sn[0]);
  }

TEST_F(MinskyFixture, units)
  {
    variableValues.emplace(":foo", VariableType::flow);
    variableValues.emplace(":bar", VariableType::flow);
    variableValues[":foo"]->allocValue()=3.0;
    variableValues[":bar"]->allocValue()=5.0;
    variableValues[":foo"]->setUnits("m");
    variableValues[":bar"]->setUnits("s");
    timeUnit="s";
    UserFunction f("test","foo+bar");
    EXPECT_THROW(f.units(true),std::exception);
    f.expression="bar+time";
    EXPECT_EQ("s",f.units(true).str());
    f.expression="foo*bar";
    EXPECT_EQ("m s",f.units(true).str());
    f.expression="exp(foo)";
    EXPECT_THROW(f.units(true),std::exception);
  }
