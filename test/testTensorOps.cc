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
      EvalOpPtr sum(OperationType::sum, nullptr, to, from);
      EvalOpPtr prod(OperationType::product, nullptr, to, from);
      EvalOpPtr inf(OperationType::infimum, nullptr, to, from);
      EvalOpPtr sup(OperationType::supremum, nullptr, to, from);
      EvalOpPtr any(OperationType::any, nullptr, to, from);
      EvalOpPtr all(OperationType::all, nullptr, to, from);
      EvalOpPtr infIndex(OperationType::infIndex, nullptr, to, from);
      EvalOpPtr supIndex(OperationType::supIndex, nullptr, to, from);
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

      from.begin()[1]=-1;
      from.begin()[3]=10;
      infIndex->eval();
      CHECK_EQUAL(1,to.value());
      supIndex->eval();
      CHECK_EQUAL(3,to.value());
    }
  
  TEST(scan)
    {
      VariableValue from(VariableType::flow), to(VariableType::flow);
      from.dims({5}); to.dims({5});
      from.allocValue();
      
      EvalOpPtr sum(OperationType::runningSum, nullptr, to, from);
      Operation<OperationType::runningSum> opSum;
      sum->setTensorParams(from,opSum);
      
      EvalOpPtr prod(OperationType::runningProduct, nullptr, to, from);
      Operation<OperationType::runningProduct> opProduct;
      prod->setTensorParams(from,opProduct);
      
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

  TEST(indexGather)
    {
      VariableValue from(VariableType::flow), to(VariableType::flow);
      from.dims({5}); to.dims({5});
      from.allocValue();
      for (auto i=from.begin(); i!=from.end(); ++i)
        *i=(i-from.begin())%2;
      EvalOpPtr index(OperationType::index, nullptr, to, from);
      index->eval();
      vector<double> expected{1,3};
      CHECK_ARRAY_EQUAL(expected,to.begin(),2);
      for (size_t i=3; i<to.numElements(); ++i)
        CHECK(std::isnan(to.begin()[i]));

      // apply gather to the orignal vector and the index results.
      VariableValue gathered(VariableType::flow); gathered.dims({5});
      EvalOpPtr gather(OperationType::gather, nullptr, gathered, from, to);
      gather->eval();
      // replace nans with -1 to make comparison test simpler
      for (auto g=gathered.begin(); g!=gathered.end(); ++g)
        if (!finite(*g)) *g=-1;
      expected={-1,1,-1,1,-1};
      CHECK_ARRAY_EQUAL(expected,gathered.begin(),5);

      // another example - check form corner cases
      vector<double> data{0.36,0.412,0.877,0.437,0.751};
      memcpy(from.begin(),&data[0],data.size()*sizeof(data[0]));
      
      index->eval();
      expected={2,4};
      CHECK_ARRAY_EQUAL(expected,to.begin(),2);
      for (size_t i=3; i<to.numElements(); ++i)
        CHECK(std::isnan(to.begin()[i]));

      // apply gather to the orignal vector and the index results.
      gather->eval();
      // replace nans with -1 to make comparison test simpler
      for (auto g=gathered.begin(); g!=gathered.end(); ++g)
        if (!finite(*g)) *g=-1;
      expected={-1,-1,0.877,-1,0.751};
      CHECK_ARRAY_EQUAL(expected,gathered.begin(),5);
    }
}
