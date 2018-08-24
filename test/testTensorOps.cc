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

SUITE(TensorOps)
{
  TEST(reduction)
    {
      VariableValue from(VariableType::flow), to(VariableType::flow);
      from.dims({5});
      from.allocValue();
      EvalOpPtr sum(OperationType::sum, to, from);
      EvalOpPtr prod(OperationType::product, to, from);
      EvalOpPtr inf(OperationType::infimum, to, from);
      EvalOpPtr sup(OperationType::supremum, to, from);
      EvalOpPtr any(OperationType::any, to, from);
      EvalOpPtr all(OperationType::all, to, from);
      for (auto i=from.begin(); i!=from.end(); ++i)
        *i=i-from.begin()+1;
      sum->eval();
      CHECK_EQUAL(15,to.value());
      prod->eval();
      CHECK_EQUAL(120,to.value());
      inf->eval();
      CHECK_EQUAL(1,to.value());
      sup->eval();
      CHECK_EQUAL(5,to.value());
      any->eval();
      CHECK_EQUAL(1,to.value());
      all->eval();
      CHECK_EQUAL(1,to.value());
      from.begin()[3]=0;
      any->eval();
      CHECK_EQUAL(1,to.value());
      all->eval();
      CHECK_EQUAL(0,to.value());
      for (auto i=from.begin(); i!=from.end(); ++i)
        *i=0;
      any->eval();
      CHECK_EQUAL(0,to.value());
      all->eval();
      CHECK_EQUAL(0,to.value());
    }
  
  TEST(scan)
    {
      VariableValue from(VariableType::flow), to(VariableType::flow);
      from.dims({5}); to.dims({5});
      from.allocValue();
      EvalOpPtr sum(OperationType::runningSum, to, from);
      EvalOpPtr prod(OperationType::runningProduct, to, from);
      for (auto i=from.begin(); i!=from.end(); ++i)
        *i=2;
      sum->eval();
      CHECK_EQUAL(5,to.numElements());
      for (size_t i=0; i<to.numElements(); ++i)
        CHECK_EQUAL(2*(i+1),to.value(i));
      prod->eval();
      CHECK_EQUAL(5,to.numElements());
      for (size_t i=0; i<to.numElements(); ++i)        
        CHECK_EQUAL(pow(2,i+1),to.value(i));
    }

}
