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
#include <UnitTest++/UnitTest++.h>
#include "variableValue.h"
#include "group.h"
#include "selection.h"
#include "lasso.h"
#include "valueId.h"
#include "minsky_epilogue.h"

using namespace minsky;

SUITE(Variable)
{
  TEST(scoping)
    {
      CHECK_EQUAL(0, scope(":foo"));
      CHECK_THROW(scope("furry[1]:foo"), std::exception);
      CHECK_EQUAL(1, scope("1:foo"));
      CHECK_THROW(scope("foo"), ecolab::error);
      
      CHECK_EQUAL("foo", uqName(":foo"));
      CHECK_EQUAL("foo", uqName("furry[1]:foo"));
      CHECK_EQUAL("foo", uqName("1:foo"));
      CHECK_EQUAL("foo", uqName("foo"));

      CHECK_EQUAL(":foo",minsky::valueId(0,"foo"));
      CHECK_EQUAL("1:foo",minsky::valueIdCanonical(1,"foo"));
      CHECK_EQUAL(":foo",minsky::valueId(":foo"));
      CHECK_THROW(minsky::valueId("foo"), ecolab::error);

    }
}
