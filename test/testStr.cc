/*
  @copyright Steve Keen 2020
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

#include "str.h"
using namespace minsky;

#include <UnitTest++/UnitTest++.h>

SUITE(Str)
{
  TEST(numBytes)
    {
      CHECK_EQUAL(strlen("A"),numBytes("A"[0]));
      CHECK_EQUAL(strlen("π"),numBytes("π"[0]));
      CHECK_EQUAL(strlen("㐀"),numBytes("㐀"[0]));
      CHECK_EQUAL(strlen("𪜢"),numBytes("𪜢"[0]));
    }

  TEST(prevIndex)
    {
      CHECK_EQUAL(0, prevIndex("1A", 0));
      CHECK_EQUAL(1, prevIndex("1A", strlen("1A")));
      CHECK_EQUAL(1, prevIndex("1π", strlen("1π")));
      CHECK_EQUAL(1, prevIndex("1㐀", strlen("1㐀")));
      CHECK_EQUAL(1, prevIndex("1𪜢", strlen("1𪜢")));
    }
}
