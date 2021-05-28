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

SUITE(UserFunction)
{
  TEST(symbolNames1)
    {
      UserFunction f("test","input/referenceInput");
      vector<string> expected{"input","referenceInput"};
      CHECK(f.symbolNames()==expected);
    }

  TEST(referenceMinskyVariables)
    {
      Minsky minsky;
      LocalMinsky lm(minsky);
      minsky.variableValues[":foo"]->allocValue()=3.0;
      minsky.variableValues[":bar"]->allocValue()=5.0;
      UserFunction f("test","foo+bar");
      f.compile();
      CHECK_EQUAL(8, f({}));
    }

  TEST(arguments)
    {
      UserFunction f("test","x+y");
      f.argNames={"x","y"};
      f.compile();
      CHECK_EQUAL(7, f({3,4}));
    }  

  TEST(error)
    {
      UserFunction f("test","x+");
      f.argNames={"x","y"};
      CHECK_THROW(f.compile(), std::exception);
    }  
}
