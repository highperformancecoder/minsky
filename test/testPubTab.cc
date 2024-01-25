/*
  @copyright Steve Keen 2024
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
  struct MinskyFixture: public Minsky
  {
    LocalMinsky lm;
    MinskyFixture(): lm(*this)
    {
    }
  };
}

SUITE(PubTab)
{
  TEST_FIXTURE(MinskyFixture,removeSelf)
    {
      assert(!publicationTabs.empty());
      auto numPubTabs=publicationTabs.size();
      publicationTabs[0].removeSelf();
      CHECK_EQUAL(numPubTabs-1,publicationTabs.size());
    }

    TEST_FIXTURE(PubTab,addNote)
      {
        addNote("hello",100,100);
        CHECK_EQUAL(1,items.size());
        CHECK_EQUAL("hello",items[0].itemRef->detailedText);
        CHECK(getItemAt(100,100));
        CHECK_EQUAL(100,items[0].x);
        CHECK_EQUAL(100,items[0].y);
      }

}
