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
#include "minskyTensorOps.h"
#include "minsky.h"
#include "minsky_epilogue.h"
#include <UnitTest++/UnitTest++.h>
using namespace minsky;

#include <exception>
using namespace std;

#include <boost/date_time.hpp>
using namespace boost;
using namespace boost::posix_time;
using namespace boost::gregorian;

// convenience structure to avoid putting items into Minsky
// note privately derived from EvalCommon pointer to get around initialisation order issues
struct Eval: private std::shared_ptr<EvalCommon>, public TensorEval
{
  template <class Op>
  Eval(VariableBase& result, Op& op):
    shared_ptr<EvalCommon>(new EvalCommon),
    TensorEval(*result.vValue(), *this,
               TensorOpFactory().create(op,TensorsFromPort(*this))) {}
  
  void operator()() {TensorEval::eval(ValueVector::flowVars.data(), ValueVector::stockVars.data());}
};

struct TestFixture
{
  Variable<VariableType::flow> from, to;
  VariableValue& fromVal;
  TestFixture(): from("from"), to("to"), fromVal(*from.vValue()) {
    fromVal.hypercube(Hypercube(vector<unsigned>{5}));
    for (auto& i: fromVal)
      i=&i-&fromVal[0]+1;
  }

  template <OperationType::Type op>
  void evalOp()
  {
    Operation<op> theOp;
    Wire w1(from.ports[0], theOp.ports[1]), w2(theOp.ports[0], to.ports[1]);
    Eval(to, theOp)();
  }
  template <OperationType::Type op>
  void checkReduction(double v)
  {
    evalOp<op>();
    CHECK_EQUAL(v,to.vValue()->value());
  }
};

struct MinskyFixture: public Minsky
{
  LocalMinsky lm{*this};
};

SUITE(TensorOps)
{
  TEST_FIXTURE(TestFixture, reduction)
    {
      checkReduction<OperationType::sum>(15);
      checkReduction<OperationType::product>(120);
      checkReduction<OperationType::infimum>(1);
      checkReduction<OperationType::supremum>(5);
      checkReduction<OperationType::any>(1);
      checkReduction<OperationType::all>(1);

      fromVal[3]=0;
      checkReduction<OperationType::all>(0);

      for (auto& i: fromVal)
        i=0;
      checkReduction<OperationType::any>(0);
      checkReduction<OperationType::all>(0);

      fromVal[1]=-1;
      fromVal[3]=10;
      checkReduction<OperationType::infIndex>(1);
      checkReduction<OperationType::supIndex>(3);
    }
  
  TEST_FIXTURE(TestFixture, scan)
    {
      for (auto& i: fromVal)
        i=2;
      
      evalOp<OperationType::runningSum>();
      {
        auto& toVal=*to.vValue();
        for (size_t i=0; i<toVal.size(); ++i)
          CHECK_EQUAL(2*(i+1),toVal[i]);
      }
      
      evalOp<OperationType::runningProduct>();
      {
        auto& toVal=*to.vValue();
        for (size_t i=0; i<toVal.size(); ++i)
          CHECK_EQUAL(pow(2,i+1),toVal[i]);
      }
    }

  TEST_FIXTURE(TestFixture, indexGather)
    {
      auto& toVal=*to.vValue();
      for (auto& i: fromVal)
        i=(&i-&fromVal[0])%2;
      evalOp<OperationType::index>();
      vector<double> expected{1,3};
      CHECK_ARRAY_EQUAL(expected,toVal.begin(),2);
      for (size_t i=3; i<toVal.size(); ++i)
        CHECK(std::isnan(toVal[i]));

      // apply gather to the orignal vector and the index results.
      Operation<OperationType::gather> gatherOp;
      Variable<VariableType::flow> gatheredVar("gathered");
      Wire w1(from.ports[0], gatherOp.ports[1]);
      Wire w2(to.ports[0], gatherOp.ports[2]);
      Wire w3(gatherOp.ports[0], gatheredVar.ports[1]);

      auto& gathered=*gatheredVar.vValue();
      Eval eval(gatheredVar, gatherOp);
      eval();
      
      // replace nans with -1 to make comparison test simpler
      for (auto& g: gathered)
        if (!finite(g)) g=-1;
      expected={1,1,-1,-1,-1};
      CHECK_ARRAY_EQUAL(expected,gathered.begin(),5);

      // another example - check for corner cases
      vector<double> data{0.36,0.412,0.877,0.437,0.751};
      memcpy(fromVal.begin(),&data[0],data.size()*sizeof(data[0]));
      
      eval();
      expected={1,3};
      CHECK_ARRAY_EQUAL(expected,toVal.begin(),2);
      for (size_t i=3; i<toVal.size(); ++i)
        CHECK(std::isnan(toVal[i]));

      // replace nans with -1 to make comparison test simpler
      for (auto& g: gathered)
        if (!finite(g)) g=-1;
      expected={0.412, 0.437, -1, -1, -1};
      CHECK_ARRAY_EQUAL(expected,gathered.begin(),5);
    }

  TEST_FIXTURE(MinskyFixture, tensorUnOpFactory)
    {
      TensorOpFactory factory;
      auto ev=make_shared<EvalCommon>();
      TensorsFromPort tp(ev);
      Variable<VariableType::flow> src("src"), dest("dest");
      src.init("iota(5)");
      variableValues.reset();
      CHECK_EQUAL(1,src.vValue()->rank());
      CHECK_EQUAL(5,src.vValue()->size());
      for (OperationType::Type op=OperationType::copy; op<OperationType::innerProduct;
           op=OperationType::Type(op+1))
        {
          OperationPtr o(op);
          CHECK_EQUAL(2, o->numPorts());
          Wire w1(src.ports[0], o->ports[1]), w2(o->ports[0], dest.ports[1]);
          TensorEval eval(*dest.vValue(), ev, factory.create(*o,tp));
          eval.eval(ValueVector::flowVars.data(), ValueVector::stockVars.data());
          switch (OperationType::classify(op))
            {
            case OperationType::function:
              {
                // just check that scalar functions are performed elementwise
                CHECK_EQUAL(src.vValue()->size(), dest.vValue()->size());
                unique_ptr<ScalarEvalOp> scalarOp(ScalarEvalOp::create(op));
                CHECK(scalarOp.get());
                for (size_t i=0; i<src.vValue()->size(); ++i)
                  {
                    double x=scalarOp->evaluate((*src.vValue())[i]);
                    double y=(*dest.vValue())[i];
                    if (finite(x)||finite(y))
                      CHECK_EQUAL(scalarOp->evaluate((*src.vValue())[i]), (*dest.vValue())[i]);
                  }
                break;
              }
            case OperationType::reduction:
              CHECK_EQUAL(0, dest.vValue()->rank());
              CHECK_EQUAL(1, dest.vValue()->size());
              break;
            case OperationType::scan:
              CHECK_EQUAL(1, dest.vValue()->rank());
              CHECK_EQUAL(src.vValue()->size(), dest.vValue()->size());
              break;
            default:
              CHECK(false);
              break;
            }
        }
    }
  
}
