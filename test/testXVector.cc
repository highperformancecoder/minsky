/*
  @copyright Steve Keen 2018
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

#include "variableType.h"
#include "evalOp.h"
#include "selection.h"
#include <ecolab_epilogue.h>
#include <UnitTest++/UnitTest++.h>
#include <exception>
using namespace minsky;

SUITE(XVector)
{
  TEST(evalOpVector)
    {
      VariableValue from1(VariableType::flow), from2(VariableType::flow),
        to(VariableType::flow);
      from1.xVector={{"a",{{0,"a1"},{1,"a2"},{2,"a3"}}},{"b",{{0,"b1"},{1,"b3"},{2,"b4"}}}};
      from2.xVector={{"c",{{0,"c1"},{1,"c2"},{2,"c3"}}},{"b",{{0,"b2"},{1,"b3"},{2,"b4"},{3,"b5"}}}};
      from1.allocValue();
      from2.allocValue();
      EvalOpPtr e(OperationType::add, to, from1, from2);
      CHECK_EQUAL(3,to.xVector.size());
      map<string, VariableValue::XVector> m;
      for (auto& i: to.xVector)
        m[i.name]=i;
      CHECK_EQUAL(3,m["a"].size());
      CHECK(m["a"]==from1.xVector[0]);
      CHECK(m["c"]==from2.xVector[0]);
      VariableValue::XVector t1{"b",{{1,"b3"},{2,"b4"}}};
      CHECK(m["b"]==t1);

      CHECK_EQUAL(3, to.dims().size());
      vector<size_t> d{to.xVector[0].size(),to.xVector[1].size(),to.xVector[2].size()};
      CHECK_ARRAY_EQUAL(d, to.dims(), d.size());
      CHECK_EQUAL(18,to.numElements());

      // the following assumes that to's xVector is {"a","b","c"}. If
      // different order, then this test will need to be modified
      CHECK_EQUAL("a",to.xVector[0].name);
      CHECK_EQUAL("b",to.xVector[1].name);
      CHECK_EQUAL("c",to.xVector[2].name);
      vector<unsigned> i1{3,4,5,6,7,8,3,4,5,6,7,8,3,4,5,6,7,8};
      vector<unsigned> i2{3,3,3,6,6,6,4,4,4,7,7,7,5,5,5,8,8,8};
      for (auto& i:i1) i+=from1.idx();
      for (auto& i:i2) i+=from2.idx();
      CHECK_EQUAL(i1.size(),e->in1.size());
      CHECK_EQUAL(i2.size(),e->in2.size());
      CHECK_ARRAY_EQUAL(i1,e->in1,i1.size());
      CHECK_ARRAY_EQUAL(i2,e->in2,i2.size());
    }
}
