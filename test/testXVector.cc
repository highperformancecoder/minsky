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
#include "xvector.h"
#include <ecolab_epilogue.h>
#include <UnitTest++/UnitTest++.h>
using namespace minsky;

#include <exception>
using namespace std;

#include <boost/date_time.hpp>
using namespace boost;
using namespace boost::posix_time;
using namespace boost::gregorian;

SUITE(XVector)
{
  TEST(evalOpVector)
    {
      VariableValue from1(VariableType::flow), from2(VariableType::flow),
        to(VariableType::flow);
      from1.xVector={{"a",{"a1","a2","a3"}},{"b",{"b1","b3","b4"}}};
      from2.xVector={{"c",{"c1","c2","c3"}},{"b",{"b2","b3","b4","b5"}}};
      from1.allocValue();
      from2.allocValue();
      EvalOpPtr e(OperationType::add, to, from1, from2);
      CHECK_EQUAL(3,to.xVector.size());
      map<string, XVector> m;
      for (auto& i: to.xVector)
        m[i.name]=i;
      CHECK_EQUAL(3,m["a"].size());
      CHECK(m["a"]==from1.xVector[0]);
      CHECK(m["c"]==from2.xVector[0]);
      XVector t1{"b",{"b3","b4"}};
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
      for (size_t i=0; i<i2.size(); ++i)
        CHECK_EQUAL(i2[i],e->in2[i][0].idx);

      e=EvalOpPtr(OperationType::add, to, to, from1);
      CHECK_EQUAL(e->in1.size(),e->in2.size());
      CHECK_EQUAL(i1.size(),e->in2.size());
      for (size_t i=0; i<i2.size(); ++i)
        CHECK_EQUAL(i1[i],e->in2[i][0].idx);

      // target incompatible dimension with source
      CHECK_THROW(EvalOpPtr(OperationType::add, from1, from1, from2), std::exception);

      to.xVector.clear();
      e=EvalOpPtr(OperationType::copy, to, from1, from2);
      CHECK(to.xVector==from1.xVector);
      CHECK_EQUAL(from1.numElements(), e->in1.size());
      for (size_t i=0; i<e->in1.size(); ++i)
        CHECK_EQUAL(from1.idx()+i, e->in1[i]);

      to.makeXConformant(from1);
      to.makeXConformant(from2);
      e=EvalOpPtr(OperationType::copy, to, from2);
      CHECK_EQUAL(3, to.dims().size());
      {
        vector<size_t> d{to.xVector[0].size(),to.xVector[1].size(),to.xVector[2].size()};
        CHECK_ARRAY_EQUAL(d, to.dims(), d.size());
      }
      CHECK_EQUAL(18,to.numElements());
      CHECK_EQUAL(i2.size(),e->in1.size());
      CHECK_ARRAY_EQUAL(i2,e->in1,i1.size());
    }

  TEST_FIXTURE(XVector, push_back)
    {
      // firstly check the simple string case
      push_back("foo");
      CHECK(front().type()==typeid(std::string));
      CHECK_EQUAL("foo",str(front()));
      clear();
      // now check values
      dimension.type=Dimension::value;
      push_back("0.5");
      CHECK(front().type()==typeid(double));
      CHECK_EQUAL(0.5,stod(str(front())));
      CHECK_THROW(push_back("foo"),std::exception);
      clear();
      // now check dates and times

      dimension.type=Dimension::time;
      dimension.units="%Y-Q%Q";
      push_back("2018-Q2");
      
      CHECK_EQUAL(ptime(date(2018,Apr,1)), any_cast<ptime>(back()));
      CHECK_THROW(push_back("2-2018"),std::exception);
      
      dimension.units="Q%Q-%Y";
      push_back("Q2-2018");
      CHECK_EQUAL(ptime(date(2018,Apr,1)), any_cast<ptime>(back()));
      CHECK_THROW(push_back("2-2018"),std::exception);
      
      dimension.units="Q%Q";
      CHECK_THROW(push_back("Q1"),std::exception);

      dimension.units="%Y-%m-%d";
      push_back("2018-04-01");
      CHECK_EQUAL(ptime(date(2018,Apr,1)), any_cast<ptime>(back()));
      CHECK_THROW(push_back("2-2018"),std::exception);

      dimension.units.clear();
      push_back("2018-04-01");
      CHECK_EQUAL(ptime(date(2018,Apr,1)), any_cast<ptime>(back()));
      CHECK_THROW(push_back("foo"),std::exception);

    }
}
