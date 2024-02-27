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
#include <UnitTest++/UnitTest++.h>
#include "selection.h"
#include "userFunction.h"
#include "minsky.h"
#include "minsky_epilogue.h"

using namespace minsky;
using namespace std;

struct MinskyFixture: public Minsky
{
  LocalMinsky lm{*this};
};


SUITE(UserFunction)
{
  TEST(symbolNames1)
    {
      UserFunction f("test","input/referenceInput");
      vector<string> expected{"input","referenceInput"};
      CHECK(f.symbolNames()==expected);
    }

  TEST_FIXTURE(MinskyFixture, referenceMinskyVariables)
    {
      variableValues.emplace(":foo", VariableType::flow);
      variableValues.emplace(":bar", VariableType::flow);
      variableValues[":foo"]->allocValue()=3.0;
      variableValues[":bar"]->allocValue()=5.0;
      UserFunction f("test","foo+bar");
      f.compile();
      CHECK_EQUAL(8, f({}));
    }

  TEST_FIXTURE(MinskyFixture, arguments)
    {
      UserFunction f("test","x+y");
      f.argNames={"x","y"};
      f.compile();
      CHECK_EQUAL(7, f({3,4}));
    }  

  TEST_FIXTURE(MinskyFixture, error)
    {
      UserFunction f("test","x+");
      f.argNames={"x","y"};
      CHECK_THROW(f.compile(), std::exception);
    }  

  TEST_FIXTURE(MinskyFixture, stringProcessing)
    {
       UserFunction f("test(x)","x:='foo\\'s bar'[]");
       f.compile(); // checks this is syntactically valid
       auto sn=f.symbolNames();
       CHECK_EQUAL(1,sn.size());
       CHECK_EQUAL("x",sn[0]);
    }

  TEST_FIXTURE(MinskyFixture, units)
    {
      variableValues.emplace(":foo", VariableType::flow);
      variableValues.emplace(":bar", VariableType::flow);
      variableValues[":foo"]->allocValue()=3.0;
      variableValues[":bar"]->allocValue()=5.0;
      variableValues[":foo"]->setUnits("m");
      variableValues[":bar"]->setUnits("s");
      timeUnit="s";
      UserFunction f("test","foo+bar");
      CHECK_THROW(f.units(true),std::exception);
      f.expression="bar+time";
      CHECK_EQUAL("s",f.units(true).str());
      f.expression="foo*bar";
      CHECK_EQUAL("m s",f.units(true).str());
      f.expression="exp(foo)";
      CHECK_THROW(f.units(true),std::exception);
    }

}
