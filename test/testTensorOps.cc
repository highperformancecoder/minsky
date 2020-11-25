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
    TensorEval(result.vValue(), *this,
               TensorOpFactory().create(op,TensorsFromPort(*this))) {}
  
  void operator()() {TensorEval::eval(ValueVector::flowVars.data(), ValueVector::flowVars.size(), ValueVector::stockVars.data());}
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
  void evalOp(const std::string& axis="", double arg=0)
  {
    Operation<op> theOp;
    theOp.axis=axis;
    theOp.arg=arg;
    Wire w1(from.ports[0], theOp.ports[1]), w2(theOp.ports[0], to.ports[1]);
    Eval(to, theOp)();
  }
  template <OperationType::Type op, class F>
  void checkReduction(F f)
  {
    // reduce all
    evalOp<op>();
    double ref=fromVal[0];
    for (size_t i=1; i<fromVal.size(); ++i) ref=f(ref, fromVal[i]);
    CHECK_EQUAL(ref,to.vValue()->value());

    vector<unsigned> dims{5,5};
    fromVal.hypercube(Hypercube(dims));
    auto& toVal=*to.vValue();
    for (auto& i: fromVal)
      i=&i-&fromVal[0]+1;

    // reduce over first dimension
    evalOp<op>("0");
    CHECK_EQUAL(1, toVal.rank());
    for (size_t i=0; i<toVal.size(); ++i)
      {
        double ref=fromVal[dims[0]*i];
        for (size_t j=1; j<dims[0]; ++j)
          ref=f(ref,fromVal[j+dims[0]*i]);
        CHECK_EQUAL(ref,toVal[i]);
      }
      
    evalOp<op>("1");
    CHECK_EQUAL(1, toVal.rank());
    for (size_t i=0; i<toVal.size(); ++i)
      {
        double ref=fromVal[i];
        for (size_t j=1; j<dims[1]; ++j)
          ref=f(ref,fromVal[i+dims[0]*j]);
        CHECK_EQUAL(ref,toVal[i]);
      }
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
      checkReduction<OperationType::sum>([](double x,double y){return x+y;});
      checkReduction<OperationType::product>([](double x,double y){return x*y;});
      checkReduction<OperationType::infimum>([](double x,double y){return x<y? x: y;});
      checkReduction<OperationType::supremum>([](double x,double y){return x>y? x: y;});
      checkReduction<OperationType::any>([](double x,double y){return x>0.5 || y>0.5;});
      checkReduction<OperationType::all>([](double x,double y){return x>0.5 && y>0.5;});

      fromVal[3]=0;
      checkReduction<OperationType::all>([](double x,double y){return x>0.5 && y>0.5;});

      for (auto& i: fromVal)
        i=0;
      checkReduction<OperationType::any>([](double x,double y){return x>0.5 || y>0.5;});
      checkReduction<OperationType::all>([](double x,double y){return x>0.5 && y>0.5;});

      fromVal[1]=-1;
      fromVal[3]=100;
      evalOp<OperationType::infIndex>();
      CHECK_EQUAL(1,to.vValue()->value());
      evalOp<OperationType::supIndex>();
      CHECK_EQUAL(3,to.vValue()->value());
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

      vector<unsigned> dims{5,5};
      fromVal.hypercube(Hypercube(dims));
      for (size_t i=0; i<dims[0]; ++i)
        for (size_t j=0; j<dims[1]; ++j)
          fromVal({i,j}) = i+j*dims[0]; 

      int bin=0;
      evalOp<OperationType::runningSum>("1",bin);
      {
        auto& toVal=*to.vValue();
        for (size_t i=0; i<dims[0]; ++i)
          for (size_t j=0; j<dims[1]; ++j)
            {
              double ref=0;
              for (size_t k=0; k<=j; ++k)
                ref+=fromVal({i,k});
              CHECK_EQUAL(ref,toVal({i,j}));
            }
      }

      bin=2;
      evalOp<OperationType::runningSum>("0",bin);
      {
        auto& toVal=*to.vValue();
        for (size_t i=0; i<dims[0]; ++i)
          for (size_t j=0; j<dims[1]; ++j)
            {
              double ref=0;
              for (size_t k=max(int(i)-bin+1,0); k<=i; ++k)
                ref+=fromVal({k,j});
              CHECK_EQUAL(ref,toVal({i,j}));
            }
      }
      
      evalOp<OperationType::runningSum>("1",2);
      {
        auto& toVal=*to.vValue();
        for (size_t i=0; i<dims[0]; ++i)
          for (size_t j=0; j<dims[1]; ++j)
            {
              double ref=0;
              for (size_t k=max(int(j)-bin+1,0); k<=j; ++k)
                ref+=fromVal({i,k});
              CHECK_EQUAL(ref,toVal({i,j}));
            }
      }
      
      evalOp<OperationType::runningProduct>("0",2);
      {
        auto& toVal=*to.vValue();
        for (size_t i=0; i<dims[0]; ++i)
          for (size_t j=0; j<dims[1]; ++j)
            {
              double ref=1;
              for (size_t k=max(int(i)-bin+1,0); k<=i; ++k)
                ref*=fromVal({k,j});
              CHECK_EQUAL(ref,toVal({i,j}));
            }
      }

      evalOp<OperationType::runningProduct>("1",2);
      {
        auto& toVal=*to.vValue();
        for (size_t i=0; i<dims[0]; ++i)
          for (size_t j=0; j<dims[1]; ++j)
            {
              double ref=1;
              for (size_t k=max(int(j)-bin+1,0); k<=j; ++k)
                ref*=fromVal({i,k});
              CHECK_EQUAL(ref,toVal({i,j}));
            }
      }
    }

  TEST_FIXTURE(TestFixture, difference2D)
    {
      vector<unsigned> dims{5,5};
      fromVal.hypercube(Hypercube(dims));
      int cnt=0;
      for (auto& i: fromVal)
        i=cnt++;

      int delta=1;
      evalOp<OperationType::difference>("0",delta);
      for (auto& i: *to.vValue())
        CHECK_EQUAL(1,i);
      evalOp<OperationType::difference>("1",delta);
      for (auto& i: *to.vValue())
        CHECK_EQUAL(5,i);
      delta=2;
      evalOp<OperationType::difference>("0",delta);
      for (auto& i: *to.vValue())
        CHECK_EQUAL(2,i);
      evalOp<OperationType::difference>("1",delta);
      for (auto& i: *to.vValue())
        CHECK_EQUAL(10,i);
    }
  TEST_FIXTURE(TestFixture, difference1D)
    {
      vector<unsigned> dims{5};
      fromVal.hypercube(Hypercube(dims));
      int cnt=0;
      for (auto& i: fromVal)
        i=cnt++;

      int delta=1;
      evalOp<OperationType::difference>("",delta=1);
      CHECK_EQUAL(4, to.vValue()->hypercube().dims()[0]);
      for (auto& i: *to.vValue())
        CHECK_EQUAL(1,i);
      CHECK_EQUAL(1, any_cast<double>(to.vValue()->hypercube().xvectors[0][0]));

      evalOp<OperationType::difference>("",delta=-1);
      CHECK_EQUAL(4, to.vValue()->hypercube().dims()[0]);
      for (auto& i: *to.vValue())
        CHECK_EQUAL(-1,i);
      CHECK_EQUAL(0, any_cast<double>(to.vValue()->hypercube().xvectors[0][0]));
                  
      evalOp<OperationType::difference>("",delta=2);
      CHECK_EQUAL(3, to.vValue()->hypercube().dims()[0]);
      for (auto& i: *to.vValue())
        CHECK_EQUAL(2,i);
      CHECK_EQUAL(2, any_cast<double>(to.vValue()->hypercube().xvectors[0][0]));

      // check that the sparse code works as expected
      fromVal.index({0,1,2,3,4});
      evalOp<OperationType::difference>("",delta=1);
      CHECK_EQUAL(4, to.vValue()->hypercube().dims()[0]);
      for (auto& i: *to.vValue())
        CHECK_EQUAL(1,i);
      CHECK_EQUAL(1, any_cast<double>(to.vValue()->hypercube().xvectors[0][0]));
      vector<size_t> ii{0,1,2,3};
      CHECK_ARRAY_EQUAL(ii, to.vValue()->index(), 4);
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
          TensorEval eval(dest.vValue(), ev, factory.create(*o,tp));
          eval.eval(ValueVector::flowVars.data(), ValueVector::flowVars.size(), ValueVector::stockVars.data());
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
                      CHECK_EQUAL(x,y);
                  }
                break;
              }
            case OperationType::reduction:
              CHECK_EQUAL(0, dest.vValue()->rank());
              CHECK_EQUAL(1, dest.vValue()->size());
              break;
            case OperationType::scan:
              CHECK_EQUAL(1, dest.vValue()->rank());
              if (op==OperationType::difference) //TODO should difference really be a scan?
                CHECK_EQUAL(src.vValue()->size()-1, dest.vValue()->size());
              else
                CHECK_EQUAL(src.vValue()->size(), dest.vValue()->size());
              break;
            default:
              CHECK(false);
              break;
            }
        }
    }
  
  TEST_FIXTURE(MinskyFixture, tensorBinOpFactory)
    {
      TensorOpFactory factory;
      auto ev=make_shared<EvalCommon>();
      TensorsFromPort tp(ev);
      Variable<VariableType::flow> src1("src1"), src2("src2"), dest("dest");
      src1.init("iota(5)");
      src2.init("one(5)");
      variableValues.reset();
      CHECK_EQUAL(1,src1.vValue()->rank());
      CHECK_EQUAL(5,src1.vValue()->size());
      CHECK_EQUAL(1,src2.vValue()->rank());
      CHECK_EQUAL(5,src2.vValue()->size());
      for (OperationType::Type op=OperationType::add; op<OperationType::copy;
           op=OperationType::Type(op+1))
        {
          OperationPtr o(op);
          CHECK_EQUAL(3, o->numPorts());
          Wire w1(src1.ports[0], o->ports[1]), w2(src2.ports[0], o->ports[2]),
            w3(o->ports[0], dest.ports[1]);
          TensorEval eval(dest.vValue(), ev, factory.create(*o,tp));
          eval.eval(ValueVector::flowVars.data(), ValueVector::flowVars.size(), ValueVector::stockVars.data());
          CHECK_EQUAL(src1.vValue()->size(), dest.vValue()->size());
          CHECK_EQUAL(src2.vValue()->size(), dest.vValue()->size());
          unique_ptr<ScalarEvalOp> scalarOp(ScalarEvalOp::create(op));
          for (size_t i=0; i<src1.vValue()->size(); ++i)
            {
              double x=scalarOp->evaluate((*src1.vValue())[i], (*src2.vValue())[i]);
              double y=(*dest.vValue())[i];
              if (finite(x)||finite(y))
                CHECK_EQUAL(x,y);
            }
        }
    }

  template <OperationType::Type op, class F, class F2>
    void multiWireTest(double identity, F f, F2 f2)
  {
    //cout << OperationType::typeName(op)<<endl;
    Operation<op> o;
    auto tensorOp=TensorOpFactory().create(o);
    CHECK_EQUAL(1, tensorOp->size());
    CHECK_EQUAL(identity, (*tensorOp)[0]);
    Hypercube hc(vector<unsigned>{2});
    auto tv1=make_shared<TensorVal>(hc), tv2=make_shared<TensorVal>(hc);
    map<size_t,double> tv1Data{{0,1},{1,2}}, tv2Data{{0,2},{1,1}};
    *tv1=tv1Data;
    *tv2=tv2Data;
    tensorOp->setArguments(vector<TensorPtr>{tv1,tv2},vector<TensorPtr>{});
    CHECK_EQUAL(f((*tv1)[0],(*tv2)[0]), (*tensorOp)[0]);
    CHECK_EQUAL(f((*tv1)[1],(*tv2)[1]), (*tensorOp)[1]);
    tensorOp->setArguments(vector<TensorPtr>{},vector<TensorPtr>{tv1,tv2});
    CHECK_EQUAL(f2(f((*tv1)[0],(*tv2)[0])), (*tensorOp)[0]);
    CHECK_EQUAL(f2(f((*tv1)[1],(*tv2)[1])), (*tensorOp)[1]);
  }

  TEST_FIXTURE(MinskyFixture, tensorBinOpMultiples)
    {
      auto id=[](double x){return x;};
      multiWireTest<OperationType::add>(0, [](double x,double y){return x+y;},id);
      multiWireTest<OperationType::subtract>
        (0, [](double x,double y){return x+y;}, [](double x){return -x;});
      multiWireTest<OperationType::multiply>(1, [](double x,double y){return x*y;}, id);
      multiWireTest<OperationType::divide>
        (1, [](double x,double y){return x*y;}, [](double x){return 1/x;});
      multiWireTest<OperationType::min>
        (std::numeric_limits<double>::max(), [](double x,double y){return x<y? x: y;}, id);
      multiWireTest<OperationType::max>
        (-std::numeric_limits<double>::max(), [](double x,double y){return x>y? x: y;}, id);
      multiWireTest<OperationType::and_>(1, [](double x,double y){return x>0.5 && y>0.5;}, id);
      multiWireTest<OperationType::or_>(0, [](double x,double y){return x>0.5 || y>0.5;}, id);
    }

  TEST_FIXTURE(MinskyFixture, binOpInterpolation1D)
    {
      // same example as examples/binaryInterpolation.mky
      VariablePtr x1(VariableType::parameter,"x1"), x2(VariableType::parameter,"x2");
      auto& x1Val=*x1->vValue();
      auto& x2Val=*x2->vValue();
      Hypercube hc1({7}), hc2({3});
      hc2.xvectors[0][0]=1.0;
      hc2.xvectors[0][1]=3.0;
      hc2.xvectors[0][2]=5.0;
      x1Val.hypercube(hc1);
      x2Val.hypercube(hc2);
      x1Val={1,2,2,1,3,2,1};
      x2Val={3,4,5};
      OperationPtr add(OperationType::add);
      VariablePtr result(VariableType::flow,"result");
      Wire w1(x1->ports[0],add->ports[1]);
      Wire w2(x2->ports[0],add->ports[2]);
      Wire w3(add->ports[0], result->ports[1]);
      auto ev=make_shared<EvalCommon>();
      TensorEval eval(variableValues[":result"], ev, tensorOpFactory.create(*add,TensorsFromPort(ev)));
      eval.eval(ValueVector::flowVars.data(), ValueVector::flowVars.size(), ValueVector::stockVars.data());
      CHECK_EQUAL(x1Val.size(),result->vValue()->size());

      vector<double> expected{4,5,5.5,5,7.5,7,6};
      CHECK_ARRAY_CLOSE(expected, result->vValue()->begin(), 7, 0.001);
    }
  
  TEST_FIXTURE(MinskyFixture, binOpInterpolation1Dunsorted)
    {
      // same example as examples/binaryInterpolation.mky
      VariablePtr x1(VariableType::parameter,"x1"), x2(VariableType::parameter,"x2");
      auto& x1Val=*x1->vValue();
      auto& x2Val=*x2->vValue();
      Hypercube hc1({7}), hc2({3});
      hc2.xvectors[0][0]=1.0;
      hc2.xvectors[0][1]=5.0;
      hc2.xvectors[0][2]=3.0;
      x1Val.hypercube(hc1);
      x2Val.hypercube(hc2);
      x1Val={1,2,2,1,3,2,1};
      x2Val={3,5,4};
      OperationPtr add(OperationType::add);
      VariablePtr result(VariableType::flow,"result");
      Wire w1(x1->ports[0],add->ports[1]);
      Wire w2(x2->ports[0],add->ports[2]);
      Wire w3(add->ports[0], result->ports[1]);
      auto ev=make_shared<EvalCommon>();
      TensorEval eval(variableValues[":result"], ev, tensorOpFactory.create(*add,TensorsFromPort(ev)));
      eval.eval(ValueVector::flowVars.data(), ValueVector::flowVars.size(), ValueVector::stockVars.data());
      CHECK_EQUAL(x1Val.size(),result->vValue()->size());

      vector<double> expected{4,5,5.5,5,7.5,7,6};
      CHECK_ARRAY_CLOSE(expected, result->vValue()->begin(), 7, 0.001);
    }
  
  TEST_FIXTURE(MinskyFixture, binOpInterpolation2D)
    {
      // same example as examples/binaryInterpolation.mky
      VariablePtr x1(VariableType::parameter,"x1"), x2(VariableType::parameter,"x2");
      auto& x1Val=*x1->vValue();
      auto& x2Val=*x2->vValue();
      Hypercube hc1({4,4}), hc2({2,2});
      hc2.xvectors[0][0]=1.0;
      hc2.xvectors[0][1]=3.0;
      hc2.xvectors[1][0]=1.0;
      hc2.xvectors[1][1]=2.5;
      x1Val.hypercube(hc1);
      x2Val.hypercube(hc2);
      x1Val={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
      x2Val={1,2,3,4};
      OperationPtr add(OperationType::add);
      VariablePtr result(VariableType::flow,"result");
      Wire w1(x1->ports[0],add->ports[1]);
      Wire w2(x2->ports[0],add->ports[2]);
      Wire w3(add->ports[0], result->ports[1]);
      auto ev=make_shared<EvalCommon>();
      TensorEval eval(variableValues[":result"], ev, tensorOpFactory.create(*add,TensorsFromPort(ev)));
      eval.eval(ValueVector::flowVars.data(), ValueVector::flowVars.size(), ValueVector::stockVars.data());
      CHECK_EQUAL(x1Val.size(),result->vValue()->size());

      vector<double> expected{2,3,4.5,6,6,7,8.5,10,11.33333,12.33333,13.83333,15.3333,16,17,18.5,20};
      CHECK_EQUAL(expected.size(), result->vValue()->size());
      CHECK_ARRAY_CLOSE(expected, result->vValue()->begin(), expected.size(), 0.001);
    }

  struct TensorValFixture
  {
    ravel::RavelState state;
    std::shared_ptr<TensorVal> arg;
    TensorValFixture() {
      Hypercube hc;
      arg=make_shared<TensorVal>();
      hc.xvectors=
        {
          XVector("country",{},{"Australia","Canada","US"}),
          XVector("sex",{},{"male","female"}),
          XVector("date",{},{"2010","2011","2012"})
        };
      arg->hypercube(hc);
      for (size_t i=0; i<arg->size(); ++i) (*arg)[i]=i;
      for (auto& xv: hc.xvectors)
        {
          state.handleStates.emplace_back();
          state.handleStates.back().description=xv.name;
        }
    }
  };
  
  TEST_FIXTURE(TensorValFixture, sliced2dswapped)
    {
      auto sex=find_if(state.handleStates.begin(), state.handleStates.end(),
                       [](const ravel::HandleState& i){return i.description=="sex";});
      sex->sliceLabel="male";
      state.outputHandles={"date","country"};
      auto chain=createRavelChain(state, arg);
      CHECK_EQUAL(2, chain.back()->rank());
      auto& chc=chain.back()->hypercube();
      auto& ahc=arg->hypercube();
      CHECK_EQUAL("date",chc.xvectors[0].name);
      CHECK(chc.xvectors[0]==ahc.xvectors[2]);
      CHECK_EQUAL("country",chc.xvectors[1].name);
      CHECK(chc.xvectors[1]==ahc.xvectors[0]);
      CHECK_EQUAL(9,chain.back()->size());
      for (size_t i=0; i<chain.back()->size(); ++i)
        CHECK(ahc.splitIndex((*chain.back())[i])[1]==0); //entry is "male"
      vector<double> expected={0,1,2,6,7,8,12,13,14};
      CHECK_ARRAY_EQUAL(expected, *chain[1], 9);
      expected={0,6,12,1,7,13,2,8,14};
      CHECK_ARRAY_EQUAL(expected, *chain.back(), 9);

      sex->sliceLabel="female";
      chain=createRavelChain(state, arg);
      CHECK_EQUAL(9,chain.back()->size());
      for (size_t i=0; i<chain.back()->size(); ++i)
        CHECK(ahc.splitIndex((*chain.back())[i])[1]==1); //entry is "female"
      
    }
  TEST_FIXTURE(TensorValFixture, reduction2dswapped)
    {
      auto sex=find_if(state.handleStates.begin(), state.handleStates.end(),
                       [](const ravel::HandleState& i){return i.description=="sex";});
      sex->collapsed=true;
      sex->reductionOp=ravel::Op::sum;
      state.outputHandles={"date","country"};
      auto chain=createRavelChain(state, arg);
      CHECK_EQUAL(2, chain.back()->rank());
      auto& chc=chain.back()->hypercube();
      auto& ahc=arg->hypercube();
      CHECK_EQUAL("date",chc.xvectors[0].name);
      CHECK(chc.xvectors[0]==ahc.xvectors[2]);
      CHECK_EQUAL("country",chc.xvectors[1].name);
      CHECK(chc.xvectors[1]==ahc.xvectors[0]);
      CHECK_EQUAL(9,chain.back()->size());
      vector<double> expected={3,15,27,5,17,29,7,19,31};
      CHECK_ARRAY_EQUAL(expected, *chain.back(), 9);

      sex->reductionOp=ravel::Op::prod;
      chain=createRavelChain(state, arg);
      expected={0,54,180,4,70,208,10,88,238};
      CHECK_ARRAY_EQUAL(expected, *chain.back(), 9);

      sex->reductionOp=ravel::Op::av;
      chain=createRavelChain(state, arg);
      expected={1.5,7.5,13.5,2.5,8.5,14.5,3.5,9.5,15.5};
      CHECK_ARRAY_EQUAL(expected, *chain.back(), 9);

      sex->reductionOp=ravel::Op::stddev;
      chain=createRavelChain(state, arg);
      expected={1.5,1.5,1.5,1.5,1.5,1.5,1.5,1.5,1.5};
      CHECK_ARRAY_EQUAL(expected, *chain.back(), 9);

      sex->reductionOp=ravel::Op::min;
      chain=createRavelChain(state, arg);
      expected={0,6,12,1,7,13,2,8,14};
      CHECK_ARRAY_EQUAL(expected, *chain.back(), 9);
      
      sex->reductionOp=ravel::Op::max;
      chain=createRavelChain(state, arg);
      expected={3,9,15,4,10,16,5,11,17};
      CHECK_ARRAY_EQUAL(expected, *chain.back(), 9);
     
    }

    TEST_FIXTURE(TensorValFixture, sparseSlicedRavel)
    {
      state.outputHandles={"date","country"};
      auto sex=find_if(state.handleStates.begin(), state.handleStates.end(),
                       [](const ravel::HandleState& i){return i.description=="sex";});
      arg->index({0,4,8,12,16});
      sex->sliceLabel="male";
      auto chain=createRavelChain(state, arg);
      CHECK_EQUAL(2, chain.back()->rank());
      CHECK_EQUAL(3, chain.back()->size());
      vector<size_t> expectedi{0,5,6};
      CHECK_ARRAY_EQUAL(expectedi, chain[1]->index(),3);
      expectedi={0,2,7};
      CHECK_ARRAY_EQUAL(expectedi, chain.back()->index(),3);
      vector<double> expectedf{0,1,2,3,4};
      CHECK_ARRAY_EQUAL(expectedf, *arg,3);
      expectedf={0,2,3};
      CHECK_ARRAY_EQUAL(expectedf, *chain[1],3);
      expectedf={0,3,2};
      CHECK_ARRAY_EQUAL(expectedf, *chain.back(),3);

      sex->collapsed=true;
      chain=createRavelChain(state, arg);
      CHECK_EQUAL(5, chain.back()->size());
      expectedi={0,1,5,6,7};
      CHECK_ARRAY_EQUAL(expectedi, chain[1]->index(),3);
      expectedi={0,2,3,5,7};
      CHECK_ARRAY_EQUAL(expectedi, chain.back()->index(),3);
      expectedf={0,1,2,3,4};
      CHECK_ARRAY_EQUAL(expectedf, *chain[1], 5);
      expectedf={0,3,1,4,2};
      CHECK_ARRAY_EQUAL(expectedf, *chain.back(),5);
    }
    
    TEST_FIXTURE(TensorValFixture, calipered)
    {
      state.outputHandles={"date","country"};
      auto country=find_if(state.handleStates.begin(), state.handleStates.end(),
                       [](const ravel::HandleState& i){return i.description=="country";});
      country->minLabel="Canada";
      country->displayFilterCaliper=true;
      auto date=find_if(state.handleStates.begin(), state.handleStates.end(),
                       [](const ravel::HandleState& i){return i.description=="date";});
      date->maxLabel="2011";
      date->displayFilterCaliper=true;
      arg->index({0,4,8,12,16});
      auto chain=createRavelChain(state, arg);
      vector<double> expected={2};
      CHECK_ARRAY_EQUAL(expected, *chain.back(), expected.size());
      vector<double> expectedi={3};
      CHECK_ARRAY_EQUAL(expectedi, chain.back()->index(), expectedi.size());
      vector<size_t> dims={2,2};
      CHECK_ARRAY_EQUAL(dims, chain.back()->shape(), 2);
    }

    TEST_FIXTURE(TensorValFixture, imposeDimensions)
      {
        Dimensions dimensions;
        dimensions.emplace("date",Dimension{Dimension::time,"%Y"});
        arg->imposeDimensions(dimensions);
        auto& xv=arg->hypercube().xvectors[2];
        CHECK_EQUAL("date",xv.name);
        CHECK_EQUAL(Dimension::time,xv.dimension.type);
        CHECK_EQUAL("%Y",xv.dimension.units);
        for (auto& i: xv)
          CHECK(boost::any_cast<boost::posix_time::ptime>(&i));
      }

    TEST(sortByValue)
      {
        auto val=make_shared<TensorVal>(vector<unsigned>{5});
        vector<double> data={3,2,4,5,1};
        for (size_t i=0; i<data.size(); ++i) (*val)[i]=data[i];
        val->updateTimestamp();
        SortByValue forward(ravel::HandleSort::forward);
        forward.setArgument(val);
        CHECK_EQUAL(val->timestamp(), forward.timestamp());
        CHECK_EQUAL(val->size(), forward.size());
        for (size_t i=1; i<forward.size(); ++i)
          CHECK(forward[i]>forward[i-1]);
        vector<int> expected={4,1,0,2,3};
        for (size_t i=0; i<forward.size(); ++i)
          CHECK_EQUAL(expected[i], boost::any_cast<double>(forward.hypercube().xvectors[0][i]));
        SortByValue reverse(ravel::HandleSort::reverse);
        reverse.setArgument(val);
        for (size_t i=1; i<reverse.size(); ++i)
          CHECK(reverse[i]<reverse[i-1]);
        expected={3,2,0,1,4};
        for (size_t i=0; i<reverse.size(); ++i)
          CHECK_EQUAL(expected[i], boost::any_cast<double>(reverse.hypercube().xvectors[0][i]));
      }

    TEST(tensorValVectorIndex)
      {
        TensorVal tv(vector<unsigned>{5,3,2});
        for (size_t i=0; i<tv.size(); ++i) tv[i]=i;
        CHECK_EQUAL(8,tv({3,1,0}));
        tv.index({1,4,8,12});
        for (size_t i=0; i<tv.size(); ++i) tv[i]=i;
        CHECK_EQUAL(2,tv({3,1,0}));
        CHECK(isnan(tv({2,1,0})));
      }
    
    TEST(tensorValAssignment)
      {
        auto arg=std::make_shared<TensorVal>(vector<unsigned>{5,3,2});
        for (size_t i=0; i<arg->size(); ++i) (*arg)[i]=i;
        Scan scan([](double& x,double y,size_t){x+=y;});
        scan.setArgument(arg,"0",0);
        CHECK_EQUAL(arg->rank(), scan.rank());
        CHECK(scan.size()>1);
        
        TensorVal tv;
        tv=scan;

        CHECK_EQUAL(tv.size(), scan.size());
        CHECK_ARRAY_EQUAL(tv.hypercube().dims(), scan.hypercube().dims(), scan.rank());
        for (size_t i=0; i<tv.size(); ++i)
          CHECK_EQUAL(scan[i], tv[i]);
      }
}
