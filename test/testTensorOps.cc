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
#include "userFunction.h"
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
  Eval(VariableBase& result, const OperationPtr& op):
    shared_ptr<EvalCommon>(new EvalCommon),
    TensorEval(result.vValue(), *this,
               TensorOpFactory().create(op,TensorsFromPort(*this))) {}
  
  void operator()() {TensorEval::eval(ValueVector::flowVars.data(), ValueVector::flowVars.size(), ValueVector::stockVars.data());}
};

struct TestFixture
{
  Minsky dummyM;
  LocalMinsky lm{dummyM};

  GroupPtr g{new Group}; // allow itemPtrFromThis() to work.
  VariablePtr from{VariableType::flow,"from"}, to{VariableType::flow,"to"};
  VariableValue& fromVal;
  TestFixture(): fromVal(*from->vValue()) {
    g->self=g;
    g->addItem(from);
    g->addItem(to);
    fromVal.hypercube(Hypercube(vector<unsigned>{5}));
    for (auto& i: fromVal)
      i=&i-&fromVal[0]+1;
  }

  template <OperationType::Type op>
  void evalOp(const std::string& axis="", double arg=0)
  {
    OperationPtr theOp(op);
    g->addItem(theOp); 
    theOp->axis=axis;
    theOp->arg=arg;
    Wire w1(from->ports(0), theOp->ports(1)), w2(theOp->ports(0), to->ports(1));
    Eval(*to, theOp)();
  }
  template <OperationType::Type op, class F>
  void checkReduction(F f)
  {
    // reduce all
    evalOp<op>();
    double ref=fromVal[0];
    for (size_t i=1; i<fromVal.size(); ++i) ref=f(ref, fromVal[i]);
    CHECK_EQUAL(ref,to->vValue()->value());

    vector<unsigned> dims{5,5};
    fromVal.hypercube(Hypercube(dims));
    auto& toVal=*to->vValue();
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
      CHECK_EQUAL(1,to->vValue()->value());
      evalOp<OperationType::supIndex>();
      CHECK_EQUAL(3,to->vValue()->value());
    }
  
  TEST_FIXTURE(TestFixture, scan)
    {
      for (auto& i: fromVal)
        i=2;
      
      evalOp<OperationType::runningSum>();
      {
        auto& toVal=*to->vValue();
        for (size_t i=0; i<toVal.size(); ++i)
          CHECK_EQUAL(2*(i+1),toVal[i]);
      }
      
      evalOp<OperationType::runningProduct>();
      {
        auto& toVal=*to->vValue();
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
        auto& toVal=*to->vValue();
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
        auto& toVal=*to->vValue();
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
        auto& toVal=*to->vValue();
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
        auto& toVal=*to->vValue();
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
        auto& toVal=*to->vValue();
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
      for (auto& i: *to->vValue())
        if (std::isfinite(i))
          CHECK_EQUAL(1,i);
      evalOp<OperationType::difference>("1",delta);
      for (auto& i: *to->vValue())
        if (std::isfinite(i))
          CHECK_EQUAL(5,i);
      delta=2;
      evalOp<OperationType::difference>("0",delta);
      for (auto& i: *to->vValue())
        if (std::isfinite(i))
          CHECK_EQUAL(2,i);
      evalOp<OperationType::difference>("1",delta);
      for (auto& i: *to->vValue())
        if (std::isfinite(i))
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
      CHECK_EQUAL(4, to->vValue()->hypercube().dims()[0]);
      for (auto& i: *to->vValue())
        CHECK_EQUAL(1,i);
      CHECK_EQUAL(1, to->vValue()->hypercube().xvectors[0][0].value);

      evalOp<OperationType::difference>("",delta=-1);
      CHECK_EQUAL(4, to->vValue()->hypercube().dims()[0]);
      for (auto& i: *to->vValue())
        CHECK_EQUAL(-1,i);
      CHECK_EQUAL(0, to->vValue()->hypercube().xvectors[0][0].value);
                  
      evalOp<OperationType::difference>("",delta=2);
      CHECK_EQUAL(3, to->vValue()->hypercube().dims()[0]);
      for (auto& i: *to->vValue())
        CHECK_EQUAL(2,i);
      CHECK_EQUAL(2, to->vValue()->hypercube().xvectors[0][0].value);

      // check that the sparse code works as expected
      fromVal.index({0,1,2,3,4});
      evalOp<OperationType::difference>("",delta=1);
      CHECK_EQUAL(4, to->vValue()->hypercube().dims()[0]);
      for (auto& i: *to->vValue())
        CHECK_EQUAL(1,i);
      CHECK_EQUAL(1, to->vValue()->hypercube().xvectors[0][0].value);
      vector<size_t> ii{0,1,2,3};
      CHECK_ARRAY_EQUAL(ii, to->vValue()->index(), 4);
    }
  
  TEST_FIXTURE(TestFixture, difference2D_II)
    {
      vector<unsigned> dims{5,5,5};
      fromVal.hypercube(Hypercube(dims));
      size_t cnt=0;
      for (size_t i=0; i<dims[2]; ++i)
        for (size_t j=0; j<dims[1]; ++j)
          for (size_t k=0; k<dims[0]; ++k)
            fromVal[cnt++]=i+j+k;

      int delta=1;
      evalOp<OperationType::difference>("1",delta=1);
      CHECK_EQUAL(4, to->vValue()->hypercube().dims()[1]);
      for (auto& i: *to->vValue())
        if (std::isfinite(i))
          CHECK_EQUAL(1,i);
      CHECK_EQUAL(1, to->vValue()->hypercube().xvectors[1][0].value);

      evalOp<OperationType::difference>("1",delta=-1);
      CHECK_EQUAL(4, to->vValue()->hypercube().dims()[1]);
      for (auto& i: *to->vValue())
        if (std::isfinite(i))
          CHECK_EQUAL(-1,i);
      CHECK_EQUAL(0, to->vValue()->hypercube().xvectors[1][0].value);
                  
      evalOp<OperationType::difference>("1",delta=2);
      CHECK_EQUAL(3, to->vValue()->hypercube().dims()[1]);
      for (auto& i: *to->vValue())
        if (std::isfinite(i))
          CHECK_EQUAL(2,i);
      CHECK_EQUAL(2, to->vValue()->hypercube().xvectors[1][0].value);

      // check that the sparse code works as expected
      fromVal.index({3,8,13,16,32,64});
      for (size_t i=0; i<fromVal.size(); ++i)
        fromVal[i]=(i%5)+(i/5)%5+(i/5);
      evalOp<OperationType::difference>("1",delta=1);
      CHECK_EQUAL(4, to->vValue()->hypercube().dims()[1]);
      cnt=0;
      for (auto& i: *to->vValue())
        if (std::isfinite(i))
          {
            CHECK_EQUAL(1,i);
            cnt++;
          }
      CHECK_EQUAL(2,cnt);
      CHECK_EQUAL(1, to->vValue()->hypercube().xvectors[1][0].value);
      vector<size_t> ii{3,8};
      CHECK_ARRAY_EQUAL(ii, to->vValue()->index(), ii.size());
      
      evalOp<OperationType::difference>("1",delta=-1);
      CHECK_EQUAL(4, to->vValue()->hypercube().dims()[1]);
      cnt=0;
      for (auto& i: *to->vValue())
        if (std::isfinite(i))
          {
            CHECK_EQUAL(-1,i);
            cnt++;
          }
      CHECK_EQUAL(2,cnt);
      CHECK_EQUAL(0, to->vValue()->hypercube().xvectors[1][0].value);
      vector<size_t> i2{3,8};
      CHECK_ARRAY_EQUAL(i2, to->vValue()->index(), i2.size());
    }

  TEST_FIXTURE(TestFixture, gatherInterpolateValue)
    {
      vector<double> x{0,1.2,2.5,3,4};
      Hypercube hc(vector<unsigned>{unsigned(x.size())});
      for (size_t i=0; i<x.size(); ++i)
        hc.xvectors[0][i]=x[i];
      fromVal.hypercube(hc);
      for (size_t i=0; i<x.size(); ++i)
        fromVal[i]=x[i];
      OperationPtr gatherOp(OperationType::gather);
      Variable<VariableType::flow> gatheredVar("gathered");
      Wire w1(from->ports(0), gatherOp->ports(1));
      Wire w2(to->ports(0), gatherOp->ports(2));   
      Wire w3(gatherOp->ports(0), gatheredVar.ports(1));
      
      auto& gathered=*gatheredVar.vValue();
      auto& toVal=*to->vValue();
      Eval eval(gatheredVar, gatherOp);
      for (size_t i=0; i<x.size(); ++i)
        {
          toVal[0]=i;
          eval();
          CHECK_EQUAL(0,gathered.rank());
          CHECK_CLOSE(i, gathered[0], 1E-4);
        }
    }
  
  TEST_FIXTURE(TestFixture, gatherInterpolateDate)
    {
      using boost::gregorian::date;
      using boost::gregorian::Jan;
      using boost::gregorian::Feb;
      using boost::gregorian::Jun;
      vector<ptime> x;
      x.emplace_back(date(1990,Jan,1));
      x.emplace_back(date(1991,Feb,1));
      x.emplace_back(date(1992,Jun,1));
      x.emplace_back(date(1993,Jan,1));
      x.emplace_back(date(1994,Jan,1));
      Hypercube hc(vector<unsigned>{unsigned(x.size())});
      hc.xvectors[0].dimension=Dimension(Dimension::time,"");
      for (size_t i=0; i<x.size(); ++i)
        {
          hc.xvectors[0][i]=x[i];
        }
      fromVal.hypercube(hc);
      for (size_t i=0; i<x.size(); ++i)
        fromVal[i]=x[i].date().year()+x[i].date().day_of_year()/365.0;
      OperationPtr gatherOp(OperationType::gather);
      Variable<VariableType::flow> gatheredVar("gathered");
      Wire w1(from->ports(0), gatherOp->ports(1));
      Wire w2(to->ports(0), gatherOp->ports(2));   
      Wire w3(gatherOp->ports(0), gatheredVar.ports(1));
      
      auto& gathered=*gatheredVar.vValue();
      auto& toVal=*to->vValue();
      Eval eval(gatheredVar, gatherOp);
      for (size_t i=1990; i<1990+x.size(); ++i)
        {
          toVal[0]=i;
          eval();
          CHECK_EQUAL(0,gathered.rank());
          CHECK_CLOSE(i, gathered[0],0.01);
        }
    }
  
  TEST_FIXTURE(TestFixture, gatherExtractRowColumn)
    {
      from->init("rand(3,5)");
      minsky::minsky().variableValues.resetValue(fromVal);
      OperationPtr gatherOp(OperationType::gather);
      Variable<VariableType::flow> gatheredVar("gathered");
      Wire w1(from->ports(0), gatherOp->ports(1));
      Wire w2(to->ports(0), gatherOp->ports(2));   
      Wire w3(gatherOp->ports(0), gatheredVar.ports(1));
      auto& gathered=*gatheredVar.vValue();
      auto& toVal=*to->vValue();

      CHECK_EQUAL(2, fromVal.rank());
      {
        // extract row
        gatherOp->axis="0";
        Eval eval(gatheredVar, gatherOp);
        for (size_t i=0; i<fromVal.shape()[0]; ++i)
          {
            toVal[0]=i;
            eval();
            CHECK_EQUAL(1,gathered.rank());
            vector<double> expected;
            for (size_t j=0; j<fromVal.shape()[1]; ++j)
              expected.push_back(fromVal[i+fromVal.shape()[0]*j]);
            CHECK_EQUAL(expected.size(), gathered.size());
            CHECK_ARRAY_CLOSE(expected.data(), &gathered[0], expected.size(), 1e-4);
          }
      }

      {
        // extract column
        gatherOp->axis="1";
        Eval eval(gatheredVar, gatherOp);
        for (size_t i=0; i<fromVal.shape()[1]; ++i)
          {
            toVal[0]=i;
            eval();
            CHECK_EQUAL(1,gathered.rank());
            vector<double> expected;
            for (size_t j=0; j<fromVal.shape()[0]; ++j)
              expected.push_back(fromVal[j+fromVal.shape()[0]*i]);
            CHECK_EQUAL(expected.size(), gathered.size());
            CHECK_ARRAY_CLOSE(expected.data(), &gathered[0], expected.size(), 1e-4);
          }
      }
    }

  
  TEST_FIXTURE(TestFixture, indexGather)
    {
      auto& toVal=*to->vValue();
      for (auto& i: fromVal)
        i=(&i-&fromVal[0])%2;
      evalOp<OperationType::index>();
      vector<double> expected{1,3};
      CHECK_ARRAY_EQUAL(expected,toVal.begin(),2);
      for (size_t i=3; i<toVal.size(); ++i)
        CHECK(std::isnan(toVal[i]));

      // apply gather to the orignal vector and the index results.
      OperationPtr gatherOp(OperationType::gather);
      Variable<VariableType::flow> gatheredVar("gathered");
      Wire w1(from->ports(0), gatherOp->ports(1));
      Wire w2(to->ports(0), gatherOp->ports(2));
      Wire w3(gatherOp->ports(0), gatheredVar.ports(1));

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
      memcpy(fromVal.begin(),data.data(),data.size()*sizeof(data[0]));
      
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

   TEST_FIXTURE(TestFixture, sparseGather)
    {
      auto& toVal=*to->vValue();
      fromVal.index({1,3,5});
      for (auto& i: fromVal)
        i=(&i-&fromVal[0])%2;

      toVal.hypercube(Hypercube(vector<unsigned>{3}));
      toVal[0]=0; toVal[1]=1; toVal[2]=3;
      
      // apply gather to the orignal vector.
      OperationPtr gatherOp(OperationType::gather);
      Variable<VariableType::flow> gatheredVar("gathered");
      Wire w1(from->ports(0), gatherOp->ports(1));
      Wire w2(to->ports(0), gatherOp->ports(2));
      Wire w3(gatherOp->ports(0), gatheredVar.ports(1));

      auto& gathered=*gatheredVar.vValue();
      Eval eval(gatheredVar, gatherOp);
      eval();
      
      // replace nans with -1 to make comparison test simpler
      for (auto& g: gathered)
        if (!finite(g)) g=-1;
      vector<double> expected={-1,0,1};
      CHECK_EQUAL(expected.size(), gathered.size());
      CHECK_ARRAY_EQUAL(expected,gathered.begin(),expected.size());

    }

 
   TEST_FIXTURE(TestFixture, sparse2Gather)
    {
      auto& toVal=*to->vValue();
      for (auto& i: fromVal)
        i=(&i-&fromVal[0])%2;

      toVal.index({1,3,5});
      toVal.hypercube(Hypercube({6}));
      toVal[0]=0; toVal[1]=1; toVal[2]=3;
      
      // apply gather to the orignal vector.
      OperationPtr gatherOp(OperationType::gather);
      Variable<VariableType::flow> gatheredVar("gathered");
      Wire w1(from->ports(0), gatherOp->ports(1));
      Wire w2(to->ports(0), gatherOp->ports(2));
      Wire w3(gatherOp->ports(0), gatheredVar.ports(1));

      auto& gathered=*gatheredVar.vValue();
      Eval eval(gatheredVar, gatherOp);
      eval();
      
      // replace nans with -1 to make comparison test simpler
      for (auto& g: gathered)
        if (!finite(g)) g=-1;
      vector<double> expected={0,1,1};
      CHECK_EQUAL(expected.size(), gathered.size());
      CHECK_ARRAY_EQUAL(expected,gathered.begin(),expected.size());
      CHECK_ARRAY_EQUAL(gathered.index(), toVal.index(), toVal.index().size());                

    }

    TEST_FIXTURE(TestFixture, sparse3DGather)
    {
      fromVal.hypercube(Hypercube({2,5,3}));
      fromVal.index({4,11,16,21});
      
      auto& toVal=*to->vValue();
      for (auto& i: fromVal)
        i=(&i-&fromVal[0]);

      toVal.index({0,2,3});
      toVal.hypercube(Hypercube({6}));
      toVal[0]=0; toVal[1]=2; toVal[2]=3;
      
      // apply gather to the orignal vector.
      OperationPtr gatherOp(OperationType::gather);
      gatherOp->axis="1";
      Variable<VariableType::flow> gatheredVar("gathered");
      Wire w1(from->ports(0), gatherOp->ports(1));
      Wire w2(to->ports(0), gatherOp->ports(2));
      Wire w3(gatherOp->ports(0), gatheredVar.ports(1));

      auto& gathered=*gatheredVar.vValue();
      Eval eval(gatheredVar, gatherOp);
      eval();
      
      double n=nan("");
      vector<double> expected={n,n,0,n,n,n, n,n,n,n,n,n, n,n,n,2,n,n, 1,n,n,n,n,n, n,n,n,n,n,n, 3,n,n,n,n,n};
      CHECK_EQUAL(expected.size(), gathered.hypercube().numElements());
      for (size_t i=0; i<expected.size(); ++i)
        if (isnan(expected[i]))
          CHECK(isnan(gathered.atHCIndex(i)));
        else
          CHECK_EQUAL(expected[i], gathered.atHCIndex(i));
    }

    TEST_FIXTURE(TestFixture, gatherExceptions)
      {
        OperationPtr gatherOp(OperationType::gather);
        Variable<VariableType::flow> gatheredVar("gathered");
        Wire w1(from->ports(0), gatherOp->ports(1));
        Wire w2(to->ports(0), gatherOp->ports(2));
        Wire w3(gatherOp->ports(0), gatheredVar.ports(1));
        
        fromVal.hypercube(Hypercube());
        CHECK_THROW(Eval(gatheredVar, gatherOp), std::exception);

        fromVal.hypercube(Hypercube({3,4}));
        CHECK_THROW(Eval(gatheredVar, gatherOp), std::exception);
      }

    TEST_FIXTURE(MinskyFixture, gatherBackElement)
    {
      VariablePtr x(VariableType::parameter,"x"), i(VariableType::parameter,"i"), z(VariableType::flow,"z");
      x->init("iota(5)");
      i->init("4");
      model->addItem(x);
      model->addItem(i);
      model->addItem(z);
      OperationPtr gather(OperationType::gather);
      model->addItem(gather);
      model->addWire(*x,*gather,1);
      model->addWire(*i,*gather,2);
      model->addWire(*gather,*z,1);
      model->addWire(*z, *model->addItem(new Sheet), 1);
      reset();
      auto& zz=*z->vValue();
      CHECK_EQUAL(0, zz.rank());
      CHECK_EQUAL(4, zz[0]);
    }

  TEST_FIXTURE(TestFixture, indexGatherTensorStringArgs)
    {
      vector<XVector> x{{"x",{Dimension::string,""}}, {"y",{Dimension::string,""}},
                                                        {"z",{Dimension::string,""}}};
      for (int i=0; i<2; ++i) 
        x[0].push_back(to_string(i));
      for (int i=0; i<3; ++i) 
        x[1].push_back(to_string(i));
      for (int i=0; i<5; ++i) 
        x[2].push_back(to_string(i));
      Hypercube fromHC(x);

      x.resize(2); x[1].resize(2); // trim to 2x2
      x[0].name="t0";
      x[1].name="t1";
      Hypercube toHC(x);
      
      vector<int> checkV={2,3,5};
      CHECK_ARRAY_EQUAL(checkV, fromHC.dims(), checkV.size());
      checkV={2,2};
      CHECK_ARRAY_EQUAL(checkV, toHC.dims(),checkV.size());

      auto& toVal=*to->vValue();
      auto& fromVal=*from->vValue();
      toVal.hypercube(toHC);
      fromVal.hypercube(fromHC);

      vector<double> fv;
      for (size_t i=0; i<fromHC.dims()[2]; ++i)
        for (size_t j=0; j<fromHC.dims()[1]; ++j)
          for (size_t k=0; k<fromHC.dims()[0]; ++k)
            fv.push_back(i+j+k);

      toVal[0]=-1;  // out of lower bound
      toVal[1]=4;   // out of upper bound
      toVal[2]=1.3; // interpolated
      toVal[3]=2;   // on exact point

      CHECK_EQUAL(fv.size(), fromVal.size());
      memcpy(&fromVal[0], fv.data(), sizeof(double)*fv.size());
      
      // apply gather to the orignal vector and the index results.
      OperationPtr gatherOp(OperationType::gather);
      gatherOp->axis="y";
      Variable<VariableType::flow> gatheredVar("gathered");
      Wire w1(from->ports(0), gatherOp->ports(1));
      Wire w2(to->ports(0), gatherOp->ports(2));
      Wire w3(gatherOp->ports(0), gatheredVar.ports(1));

      auto& gathered=*gatheredVar.vValue();
      Eval eval(gatheredVar, gatherOp);
      eval();
      
      vector<size_t> expectedDims{2,2,2,5};
      CHECK_EQUAL(expectedDims.size(), gathered.rank());
      CHECK_ARRAY_EQUAL(expectedDims, gathered.hypercube().dims(), expectedDims.size());

      auto& gxv=gathered.hypercube().xvectors;
      CHECK_EQUAL("t0",gxv[0].name);
      CHECK_EQUAL("t1",gxv[1].name);
      CHECK_EQUAL("x",gxv[2].name);
      CHECK_EQUAL("z",gxv[3].name);

      for (size_t i=0; i<expectedDims[3]; ++i)
        for (size_t j=0; j<expectedDims[2]; ++j)
          {
            CHECK(isnan(gathered[(i*expectedDims[2]+j)*toVal.size()]));
            CHECK(isnan(gathered[(i*expectedDims[2]+j)*toVal.size()+1]));
            CHECK_CLOSE(i+j+1.3, gathered[(i*expectedDims[2]+j)*toVal.size()+2],0.01);
            CHECK_EQUAL(i+j+2, gathered[(i*expectedDims[2]+j)*toVal.size()+3]);
          }
    }

  TEST_FIXTURE(TestFixture, indexGatherTensorValueArgs)
    {
      vector<XVector> x{{"x",{Dimension::string,""}}, {"y",{Dimension::value,""}},
                                                        {"z",{Dimension::string,""}}};
      for (int i=0; i<2; ++i) 
        x[0].push_back(to_string(i));
      x[1].push_back(1900.0); x[1].push_back(1950.0); x[1].push_back(1970.0);
      for (int i=0; i<5; ++i) 
        x[2].push_back(to_string(i));
      Hypercube fromHC(x);

      x.resize(2); x[1].resize(2); // trim to 2x2
      Hypercube toHC(x);
      
      vector<int> checkV={2,3,5};
      CHECK_ARRAY_EQUAL(checkV, fromHC.dims(), checkV.size());
      checkV={2,2};
      CHECK_ARRAY_EQUAL(checkV, toHC.dims(),checkV.size());

      auto& toVal=*to->vValue();
      auto& fromVal=*from->vValue();
      toVal.hypercube(toHC);
      fromVal.hypercube(fromHC);

      vector<double> fv;
      for (size_t i=0; i<fromHC.dims()[2]; ++i)
        for (size_t j=0; j<fromHC.dims()[1]; ++j)
          for (size_t k=0; k<fromHC.dims()[0]; ++k)
            fv.push_back(i+j+k);

      toVal[0]=1890;  // out of lower bound
      toVal[1]=1980;   // out of upper bound
      toVal[2]=1930; // interpolated
      toVal[3]=1950;   // on exact point

      CHECK_EQUAL(fv.size(), fromVal.size());
      memcpy(&fromVal[0], fv.data(), sizeof(double)*fv.size());
      
      // apply gather to the orignal vector and the index results.
      OperationPtr gatherOp(OperationType::gather);
      gatherOp->axis="y";
      Variable<VariableType::flow> gatheredVar("gathered");
      Wire w1(from->ports(0), gatherOp->ports(1));
      Wire w2(to->ports(0), gatherOp->ports(2));
      Wire w3(gatherOp->ports(0), gatheredVar.ports(1));

      auto& gathered=*gatheredVar.vValue();
      Eval eval(gatheredVar, gatherOp);
      eval();
      
      vector<size_t> expectedDims{2,2,2,5};
      CHECK_EQUAL(expectedDims.size(), gathered.rank());
      CHECK_ARRAY_EQUAL(expectedDims, gathered.hypercube().dims(), expectedDims.size());

      for (size_t i=0; i<expectedDims[3]; ++i)
        for (size_t j=0; j<expectedDims[2]; ++j)
          {
            CHECK(isnan(gathered[(i*expectedDims[2]+j)*toVal.size()]));
            CHECK(isnan(gathered[(i*expectedDims[2]+j)*toVal.size()+1]));
            CHECK_CLOSE(i+j+0.6, gathered[(i*expectedDims[2]+j)*toVal.size()+2],0.01);
            CHECK_EQUAL(i+j+1, gathered[(i*expectedDims[2]+j)*toVal.size()+3]);
          }
    }

  TEST_FIXTURE(TestFixture, indexGatherTensorTimeArgs)
    {
      vector<XVector> x{{"x",{Dimension::string,""}}, {"y",{Dimension::time,""}},
                                                        {"z",{Dimension::string,""}}};
      for (int i=0; i<2; ++i) 
        x[0].push_back(to_string(i));
      x[1].push_back(ptime(date(1900,Jan,1))); x[1].push_back(ptime(date(1950,Jan,1))); x[1].push_back(ptime(date(1970,Jan,1)));
      for (int i=0; i<5; ++i) 
        x[2].push_back(to_string(i));
      Hypercube fromHC(x);

      x.resize(2); x[1].resize(2); // trim to 2x2
      Hypercube toHC(x);
      
      vector<int> checkV={2,3,5};
      CHECK_ARRAY_EQUAL(checkV, fromHC.dims(), checkV.size());
      checkV={2,2};
      CHECK_ARRAY_EQUAL(checkV, toHC.dims(),checkV.size());

      auto& toVal=*to->vValue();
      auto& fromVal=*from->vValue();
      toVal.hypercube(toHC);
      fromVal.hypercube(fromHC);

      vector<double> fv;
      for (size_t i=0; i<fromHC.dims()[2]; ++i)
        for (size_t j=0; j<fromHC.dims()[1]; ++j)
          for (size_t k=0; k<fromHC.dims()[0]; ++k)
            fv.push_back(i+j+k);

      toVal[0]=1890;  // out of lower bound
      toVal[1]=1980;   // out of upper bound
      toVal[2]=1930; // interpolated
      toVal[3]=1950;   // on exact point

      CHECK_EQUAL(fv.size(), fromVal.size());
      memcpy(&fromVal[0], fv.data(), sizeof(double)*fv.size());
      
      // apply gather to the orignal vector and the index results.
      OperationPtr gatherOp(OperationType::gather);
      gatherOp->axis="y";
      Variable<VariableType::flow> gatheredVar("gathered");
      Wire w1(from->ports(0), gatherOp->ports(1));
      Wire w2(to->ports(0), gatherOp->ports(2));
      Wire w3(gatherOp->ports(0), gatheredVar.ports(1));

      auto& gathered=*gatheredVar.vValue();
      Eval eval(gatheredVar, gatherOp);
      eval();
      
      vector<size_t> expectedDims{2,2,2,5};
      CHECK_EQUAL(expectedDims.size(), gathered.rank());
      CHECK_ARRAY_EQUAL(expectedDims, gathered.hypercube().dims(), expectedDims.size());

      for (size_t i=0; i<expectedDims[3]; ++i)
        for (size_t j=0; j<expectedDims[2]; ++j)
          {
            CHECK(isnan(gathered[(i*expectedDims[2]+j)*toVal.size()]));
            CHECK(isnan(gathered[(i*expectedDims[2]+j)*toVal.size()+1]));
            CHECK_CLOSE(i+j+0.6, gathered[(i*expectedDims[2]+j)*toVal.size()+2],0.01);
            CHECK_EQUAL(i+j+1, gathered[(i*expectedDims[2]+j)*toVal.size()+3]);
          }
    }

  
  
  TEST_FIXTURE(MinskyFixture, tensorUnOpFactory)
    {
      TensorOpFactory factory;
      auto ev=make_shared<EvalCommon>();
      TensorsFromPort tp(ev);
      VariablePtr src(VariableType::flow,"src"), dest(VariableType::flow,"dest");
      model->addItem(src); model->addItem(dest);
      src->init("iota(5)");
      variableValues.reset();
      CHECK_EQUAL(1,src->vValue()->rank());
      CHECK_EQUAL(5,src->vValue()->size());
      for (OperationType::Type op=OperationType::copy; op<OperationType::innerProduct;
           op=OperationType::Type(op+1))
        {
          OperationPtr o(op);
          model->addItem(o);
          CHECK_EQUAL(2, o->numPorts());
          Wire w1(src->ports(0), o->ports(1)), w2(o->ports(0), dest->ports(1));
          TensorEval eval(dest->vValue(), ev, factory.create(o,tp));
          eval.eval(ValueVector::flowVars.data(), ValueVector::flowVars.size(), ValueVector::stockVars.data());
          switch (OperationType::classify(op))
            {
            case OperationType::function:
              {
                // just check that scalar functions are performed elementwise
                CHECK_EQUAL(src->vValue()->size(), dest->vValue()->size());
                unique_ptr<ScalarEvalOp> scalarOp(ScalarEvalOp::create(op,o));
                CHECK(scalarOp.get());
                for (size_t i=0; i<src->vValue()->size(); ++i)
                  {
                    double x=scalarOp->evaluate((*src->vValue())[i]);
                    double y=(*dest->vValue())[i];
                    if (finite(x)||finite(y))
                      CHECK_EQUAL(x,y);
                  }
                break;
              }
            case OperationType::reduction:
              CHECK_EQUAL(0, dest->vValue()->rank());
              CHECK_EQUAL(1, dest->vValue()->size());
              break;
            case OperationType::scan:
              CHECK_EQUAL(1, dest->vValue()->rank());
              if (op==OperationType::difference || op==OperationType::differencePlus) //TODO should difference really be a scan?
                CHECK_EQUAL(src->vValue()->size()-1, dest->vValue()->size());
              else
                CHECK_EQUAL(src->vValue()->size(), dest->vValue()->size());
              break;
            default:
              CHECK(false);
              break;
            }
          model->removeItem(*o);
        }
    }
  
  TEST_FIXTURE(MinskyFixture, tensorBinOpFactory)
    {
      TensorOpFactory factory;
      auto ev=make_shared<EvalCommon>();
      TensorsFromPort tp(ev);
      VariablePtr src1(VariableType::flow,"src1"),
        src2(VariableType::flow,"src2"), dest(VariableType::flow,"dest");
      model->addItem(src1); model->addItem(src2); model->addItem(dest);
      src1->init("iota(5)");
      src2->init("one(5)");
      variableValues.reset();
      CHECK_EQUAL(1,src1->vValue()->rank());
      CHECK_EQUAL(5,src1->vValue()->size());
      CHECK_EQUAL(1,src2->vValue()->rank());
      CHECK_EQUAL(5,src2->vValue()->size());
      for (OperationType::Type op=OperationType::add; op<OperationType::copy;
           op=OperationType::Type(op+1))
        {
          OperationPtr o(op);
          model->addItem(o);
          if (auto f=dynamic_cast<UserFunction*>(o.get()))
            {
              f->expression="x+y";
              f->compile();
            }
          CHECK_EQUAL(3, o->numPorts());
          Wire w1(src1->ports(0), o->ports(1)), w2(src2->ports(0), o->ports(2)),
            w3(o->ports(0), dest->ports(1));
          TensorEval eval(dest->vValue(), ev, factory.create(o,tp));
          eval.eval(ValueVector::flowVars.data(), ValueVector::flowVars.size(), ValueVector::stockVars.data());
          CHECK_EQUAL(src1->vValue()->size(), dest->vValue()->size());
          CHECK_EQUAL(src2->vValue()->size(), dest->vValue()->size());
          unique_ptr<ScalarEvalOp> scalarOp(ScalarEvalOp::create(op,o));
          for (size_t i=0; i<src1->vValue()->size(); ++i)
            {
              double x=scalarOp->evaluate((*src1->vValue())[i], (*src2->vValue())[i]);
              double y=(*dest->vValue())[i];
              if (finite(x)||finite(y))
                CHECK_EQUAL(x,y);
            }
        }
    }

  template <OperationType::Type op, class F, class F2>
    void multiWireTest(double identity, F f, F2 f2)
  {
    //cout << OperationType::typeName(op)<<endl;
    OperationPtr o(op);
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

  TEST_FIXTURE(MinskyFixture, binOp)
    {
      // same example as examples/binaryInterpolation.mky
      VariablePtr x1(VariableType::parameter,"x1"), x2(VariableType::parameter,"x2");
      model->addItem(x1); model->addItem(x2);
      auto& x1Val=*x1->vValue();
      auto& x2Val=*x2->vValue();
      Hypercube hc1({7}), hc2=hc1;
      x1Val.hypercube(hc1);
      x2Val.hypercube(hc2);
      x1Val={1,2,2,1,3,2,1};
      x2Val={3,4,5,6,7,8,2};
      OperationPtr add(OperationType::add);
      model->addItem(add);
      VariablePtr result(VariableType::flow,"result");
      model->addItem(result);
      Wire w1(x1->ports(0),add->ports(1));
      Wire w2(x2->ports(0),add->ports(2));
      Wire w3(add->ports(0), result->ports(1));
      auto ev=make_shared<EvalCommon>();
      TensorEval eval(variableValues[":result"], ev, tensorOpFactory.create(add,TensorsFromPort(ev)));
      eval.eval(ValueVector::flowVars.data(), ValueVector::flowVars.size(), ValueVector::stockVars.data());
      CHECK_EQUAL(x1Val.size(),result->vValue()->size());

      vector<double> expected{4,6,7,7,10,10,3};
      CHECK_ARRAY_CLOSE(expected, result->vValue()->begin(), 7, 0.001);
    }

  TEST_FIXTURE(MinskyFixture, binOpInterpolation1D)
    {
      // same example as examples/binaryInterpolation.mky
      VariablePtr x1(VariableType::parameter,"x1"), x2(VariableType::parameter,"x2");
      model->addItem(x1); model->addItem(x2);
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
      model->addItem(add);
      VariablePtr result(VariableType::flow,"result");
      model->addItem(result);
      Wire w1(x1->ports(0),add->ports(1));
      Wire w2(x2->ports(0),add->ports(2));
      Wire w3(add->ports(0), result->ports(1));
      auto ev=make_shared<EvalCommon>();
      TensorEval eval(variableValues[":result"], ev, tensorOpFactory.create(add,TensorsFromPort(ev)));
      eval.eval(ValueVector::flowVars.data(), ValueVector::flowVars.size(), ValueVector::stockVars.data());
      CHECK_EQUAL(5,result->vValue()->size());

      vector<double> expected{5,5.5,5,7.5,7};
      CHECK_ARRAY_CLOSE(expected, result->vValue()->begin(), 5, 0.001);
    }
  
  TEST_FIXTURE(MinskyFixture, binOpInterpolation1Dunsorted)
    {
      // same example as examples/binaryInterpolation.mky
      VariablePtr x1(VariableType::parameter,"x1"), x2(VariableType::parameter,"x2");
      model->addItem(x1); model->addItem(x2);
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
      model->addItem(add);
      VariablePtr result(VariableType::flow,"result");
      model->addItem(result);
      Wire w1(x1->ports(0),add->ports(1));
      Wire w2(x2->ports(0),add->ports(2));
      Wire w3(add->ports(0), result->ports(1));
      auto ev=make_shared<EvalCommon>();
      TensorEval eval(variableValues[":result"], ev, tensorOpFactory.create(add,TensorsFromPort(ev)));
      eval.eval(ValueVector::flowVars.data(), ValueVector::flowVars.size(), ValueVector::stockVars.data());
      CHECK_EQUAL(5,result->vValue()->size());

      vector<double> expected{5,5.5,5,7.5,7};
      CHECK_ARRAY_CLOSE(expected, result->vValue()->begin(), 5, 0.001);
    }
  
  TEST_FIXTURE(MinskyFixture, binOpInterpolation2D)
    {
      // same example as examples/binaryInterpolation.mky
      VariablePtr x1(VariableType::parameter,"x1"), x2(VariableType::parameter,"x2");
      model->addItem(x1); model->addItem(x2);
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
      model->addItem(add);
      VariablePtr result(VariableType::flow,"result");
      model->addItem(result);
      Wire w1(x1->ports(0),add->ports(1));
      Wire w2(x2->ports(0),add->ports(2));
      Wire w3(add->ports(0), result->ports(1));
      auto ev=make_shared<EvalCommon>();
      TensorEval eval(variableValues[":result"], ev, tensorOpFactory.create(add,TensorsFromPort(ev)));
      eval.eval(ValueVector::flowVars.data(), ValueVector::flowVars.size(), ValueVector::stockVars.data());

      CHECK_EQUAL(9, result->vValue()->size());
      vector<double> expected{7,8.5,10,12.3333,13.8333,15.3333,15,16.5,18};
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
      expected={2.12132, 2.12132, 2.12132, 2.12132, 2.12132, 2.12132, 2.12132, 2.12132, 2.12132};
      CHECK_ARRAY_CLOSE(expected, *chain.back(), 9, 1e-4);

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
        CHECK(i.type==Dimension::time);
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
    scan.setArgument(arg,{"0",0});
    CHECK_EQUAL(arg->rank(), scan.rank());
    CHECK(scan.size()>1);
        
    TensorVal tv;
    tv=scan;

    CHECK_EQUAL(tv.size(), scan.size());
    CHECK_ARRAY_EQUAL(tv.hypercube().dims(), scan.hypercube().dims(), scan.rank());
    for (size_t i=0; i<tv.size(); ++i)
      CHECK_EQUAL(scan[i], tv[i]);
  }

  TEST(permuteAxis)
  {
    // 5x5 example
    Hypercube hc{5,5};
    auto dense=make_shared<TensorVal>(hc);
    for (size_t i=0; i<dense->size(); ++i) (*dense)[i]=i;
    PermuteAxis pa;
    pa.setArgument(dense,{"0",0});
    vector<size_t> permutation{1,4,3};
    pa.setPermutation(permutation);
    CHECK_EQUAL(2, pa.rank());
    CHECK_EQUAL(3, pa.hypercube().dims()[0]);
    CHECK_EQUAL(5, pa.hypercube().dims()[1]);
    CHECK_EQUAL(15, pa.size());
        
    for (size_t i=0; i<pa.size(); ++i)
      {
        switch (i%3)
          {
          case 0:
            CHECK_EQUAL(1, int(pa[i])%5);
            break;
          case 1:
            CHECK_EQUAL(4, int(pa[i])%5);
            break;
          case 2:
            CHECK_EQUAL(3, int(pa[i])%5);
            break;
          }
      }

    pa.setArgument(dense,{"1",0});
    pa.setPermutation(permutation);
    CHECK_EQUAL(2, pa.rank());
    CHECK_EQUAL(5, pa.hypercube().dims()[0]);
    CHECK_EQUAL(3, pa.hypercube().dims()[1]);
    CHECK_EQUAL(15, pa.size());
        
    for (size_t i=0; i<pa.size(); ++i)
      {
        switch (i/5)
          {
          case 0:
            CHECK_EQUAL(1, int(pa[i])/5);
            break;
          case 1:
            CHECK_EQUAL(4, int(pa[i])/5);
            break;
          case 2:
            CHECK_EQUAL(3, int(pa[i])/5);
            break;
          }
      }

        
        
    auto sparse=make_shared<TensorVal>(hc);
    sparse->index(std::set<size_t>{2,4,5,8,10,11,15,20});
    for (size_t i=0; i<sparse->size(); ++i) (*sparse)[i]=sparse->index()[i];

    pa.setArgument(sparse,{"0",0});
    pa.setPermutation(permutation);
    CHECK_EQUAL(2, pa.rank());
    CHECK_EQUAL(3, pa.hypercube().dims()[0]);
    CHECK_EQUAL(5, pa.hypercube().dims()[1]);
    CHECK_EQUAL(3, pa.size());
        
    for (size_t i=0; i<pa.size(); ++i)
      {
        auto splitted=pa.hypercube().splitIndex(pa.index()[i]);
        switch (splitted[0])
          {
          case 0:
            CHECK_EQUAL(1, int(pa[i])%5);
            break;
          case 1:
            CHECK_EQUAL(4, int(pa[i])%5);
            break;
          case 2:
            CHECK_EQUAL(3, int(pa[i])%5);
            break;
          default:
            CHECK(false);
          }
      }
    pa.setArgument(sparse,{"1",0});
    pa.setPermutation(permutation);
    CHECK_EQUAL(2, pa.rank());
    CHECK_EQUAL(3, pa.hypercube().dims()[1]);
    CHECK_EQUAL(5, pa.hypercube().dims()[0]);
    CHECK_EQUAL(4, pa.size());
        
    for (size_t i=0; i<pa.size(); ++i)
      {
        auto splitted=pa.hypercube().splitIndex(pa.index()[i]);
        switch (splitted[1])
          {
          case 0:
            CHECK_EQUAL(1, int(pa[i])/5);
            break;
          case 1:
            CHECK_EQUAL(4, int(pa[i])/5);
            break;
          case 2:
            CHECK_EQUAL(3, int(pa[i])/5);
            break;
          default:
            CHECK(false);
          }
      }
        
  }

  TEST(dimLabels)
  {
    vector<XVector> x{{"x",{Dimension::string,""}}, {"y",{Dimension::string,""}},
                                                      {"z",{Dimension::string,""}}};
    Hypercube hc(x);
    vector<string> expected{"x","y","z"};
    CHECK_EQUAL(expected.size(), hc.dimLabels().size());
    CHECK_ARRAY_EQUAL(expected, hc.dimLabels(), expected.size());
  }

  struct OuterFixture: public MinskyFixture
  {
    VariablePtr x{VariableType::parameter,"x"};
    VariablePtr y{VariableType::parameter,"y"};
    VariablePtr z{VariableType::flow,"z"};
    OperationPtr outer{OperationType::outerProduct};
    OuterFixture()
    {
      model->addItem(x);
      model->addItem(y);
      model->addItem(z);
      model->addItem(outer);
      auto& xx=x->vValue()->tensorInit;
      xx.index({1,3});
      xx.hypercube({5});
      xx[0]=1;
      xx[1]=3;
      y->init("iota(5)");
    }
  };
  
  TEST_FIXTURE(OuterFixture, sparseOuterProduct)
    {
      model->addWire(*x,*outer,1);
      model->addWire(*x,*outer,2);
      model->addWire(*outer,*z,1);
      reset();
      auto& zz=*z->vValue();
      CHECK_EQUAL(2, zz.rank());
      vector<unsigned> expectedIndex={6,8,16,18};
      CHECK_EQUAL(expectedIndex.size(), zz.size());
      CHECK_ARRAY_EQUAL(expectedIndex, zz.index(), expectedIndex.size());
      vector<double> zValues(&zz[0], &zz[0]+zz.size());
      vector<double> expectedValues={1,3,3,9};
      CHECK_EQUAL(expectedValues.size(), zz.size());
      CHECK_ARRAY_EQUAL(expectedValues, zValues, expectedValues.size());
    }
  


TEST_FIXTURE(OuterFixture, sparse1OuterProduct)
    {
      model->addWire(*y,*outer,1);
      model->addWire(*x,*outer,2);
      model->addWire(*outer,*z,1);
      reset();
      auto& zz=*z->vValue();
      CHECK_EQUAL(2, zz.rank());
      vector<unsigned> expectedIndex={5,6,7,8,9,15,16,17,18,19};
      CHECK_EQUAL(expectedIndex.size(), zz.size());
      CHECK_ARRAY_EQUAL(expectedIndex, zz.index(), expectedIndex.size());
      vector<double> zValues(&zz[0], &zz[0]+zz.size());
      vector<double> expectedValues={0,1,2,3,4,0,3,6,9,12};
      CHECK_EQUAL(expectedValues.size(), zz.size());
      CHECK_ARRAY_EQUAL(expectedValues, zValues, expectedValues.size());
    }
TEST_FIXTURE(OuterFixture, sparse2OuterProduct)
    {
      model->addWire(*x,*outer,1);
      model->addWire(*y,*outer,2);
      model->addWire(*outer,*z,1);
      reset();
      auto& zz=*z->vValue();
      CHECK_EQUAL(2, zz.rank());
      vector<unsigned> expectedIndex={1,3,6,8,11,13,16,18,21,23};
      CHECK_EQUAL(expectedIndex.size(), zz.size());
      CHECK_ARRAY_EQUAL(expectedIndex, zz.index(), expectedIndex.size());
      vector<double> zValues(&zz[0], &zz[0]+zz.size());
      vector<double> expectedValues={0,0,1,3,2,6,3,9,4,12};
      CHECK_EQUAL(expectedValues.size(), zz.size());
      CHECK_ARRAY_EQUAL(expectedValues, zValues, expectedValues.size());
    }

 TEST_FIXTURE(TestFixture,TensorVarValAssignment)
   {
     auto ev=make_shared<EvalCommon>();
     double fv[100], sv[10];
     ev->update(fv,sizeof(fv)/sizeof(fv[0]),sv);
     auto startTimestamp=ev->timestamp();
     TensorVarVal tvv(to->vValue(),ev); 
     tvv=fromVal;
     CHECK_EQUAL(fromVal.rank(), tvv.rank());
     CHECK_ARRAY_EQUAL(fromVal.shape().data(), tvv.shape().data(), fromVal.rank());
     CHECK_ARRAY_CLOSE(&fromVal[0], &tvv[0], fromVal.size(), 1e-5);
     
     CHECK(ev->timestamp()>startTimestamp);
     CHECK_EQUAL(fv,ev->flowVars());
     CHECK_EQUAL(sv,ev->stockVars());
     CHECK_EQUAL(sv,ev->stockVars());
     CHECK_EQUAL(sizeof(fv)/sizeof(fv[0]),ev->fvSize());
   }

 struct CorrelationFixture: public TestFixture
 {
   VariablePtr from1{VariableType::flow,"from1"};
   VariableValue& from1Val=*from1->vValue();
   CorrelationFixture() {
     g->addItem(from1);
     TensorVal x(vector<unsigned>{5,5}), y(vector<unsigned>{5,5});
     //TensorVal x(vector<unsigned>{2,2}), y(vector<unsigned>{2,2});
     x=std::vector<double>{
       0.401195,0.984229,0.637603,0.457079,0.163259,
       0.153241,0.761564,0.576274,0.799264,0.717473,
       0.292201,0.491054,0.381334,0.712326,0.290808,
       0.921837,0.071269,0.990526,0.048759,0.656821,
       0.228753,0.756031,0.290251,0.346473,0.140935
     };
     y=std::vector<double>{
       0.721763,0.961509,0.696964,0.547983,0.153318,
       0.989830,0.431140,0.924805,0.072694,0.186429,
       0.997384,0.774908,0.835645,0.865869,0.349889,
       0.330321,0.353306,0.106463,0.593529,0.184363,
       0.914338,0.619335,0.512865,0.607043,0.528224
     };
     fromVal=x; from1Val=y;
   }
 };
 
 TEST_FIXTURE(CorrelationFixture,covariance)
   {
     // calculated on Octave cov(x,y)
     vector<double> cov{
       -8.3229e-02,   6.1468e-02,  -6.7649e-02,   7.3800e-02,  -2.3324e-02,
       -2.7181e-02,   5.6054e-02,  -2.9875e-02,   2.0813e-02,  -5.6449e-02,
       -8.7004e-02,   7.7607e-02,  -5.4858e-02,   9.5311e-02,  -1.2319e-02,
       2.4160e-02,  -3.1403e-02,  -1.5013e-02,  -2.6450e-02,  -4.7382e-02,
       -1.9252e-02,   4.4654e-03,  -3.3002e-02,  -6.5478e-04,  -2.4050e-02
     };

     OperationPtr covOp(OperationType::covariance);
     covOp->axis="1";
     g->addItem(covOp);
     Wire w1(from->ports(0),covOp->ports(1)), w2(from1->ports(0),covOp->ports(2)), w3(covOp->ports(0),to->ports(1));
     Eval(*to, covOp)();
     
     auto& toVal=*to->vValue();
     CHECK_EQUAL(cov.size(), toVal.size());
     CHECK_ARRAY_CLOSE(cov, &toVal[0], toVal.size(), 1e-4);
   }
 
 TEST_FIXTURE(CorrelationFixture,rho)
   {
     // calculated on Octave (cov(x,y)./(std(x)'*std(y)))'
     vector<double> rho{
       -0.970729,   0.627315,  -0.888322,   0.877548,  -0.302967,
       -0.357572,   0.645244,  -0.442479,   0.279151,  -0.827059,
       -0.877474,   0.684865,  -0.622907,   0.980007,  -0.138368,
       0.274212,  -0.311870,  -0.191840,  -0.306059,  -0.598950,
       -0.397234,   0.080619,  -0.766631,  -0.013774,  -0.552663
     };

     OperationPtr rhoOp(OperationType::rho);
     rhoOp->axis="1";
     g->addItem(rhoOp);
     Wire w1(from->ports(0),rhoOp->ports(1)), w2(from1->ports(0),rhoOp->ports(2)), w3(rhoOp->ports(0),to->ports(1));
     Eval(*to, rhoOp)();
     
     auto& toVal=*to->vValue();
     CHECK_EQUAL(rho.size(), toVal.size());
     CHECK_ARRAY_CLOSE(rho, &toVal[0], toVal.size(), 1e-4);
   }

  TEST_FIXTURE(CorrelationFixture,selfCovariance)
   {
     // calculated on Octave cov(x)
     vector<double> cov{
   9.3537e-02,  -8.2565e-02,   7.1559e-02,  -7.6511e-02,   2.7879e-02,
  -8.2565e-02,   1.2217e-01,  -5.4696e-02,   5.6276e-02,  -5.0898e-02,
   7.1559e-02,  -5.4696e-02,   7.3793e-02,  -4.6796e-02,   4.6805e-02,
  -7.6511e-02,   5.6276e-02,  -4.6796e-02,   8.9992e-02,   1.2587e-03,
   2.7879e-02,  -5.0898e-02,   4.6805e-02,   1.2587e-03,   7.5410e-02
     };

     OperationPtr covOp(OperationType::covariance);
     covOp->axis="1";
     g->addItem(covOp);
     Wire w1(from->ports(0),covOp->ports(1)), w3(covOp->ports(0),to->ports(1));
     Eval(*to, covOp)();
     
     auto& toVal=*to->vValue();
     CHECK_EQUAL(cov.size(), toVal.size());
     CHECK_ARRAY_CLOSE(cov, &toVal[0], toVal.size(), 1e-4);
   }

 TEST_FIXTURE(CorrelationFixture,nonConformantCovariance)
   {
     from1Val.hypercube(vector<unsigned>{5,4});

     OperationPtr covOp(OperationType::covariance);
     covOp->axis="1";
     g->addItem(covOp);
     Wire w1(from->ports(0),covOp->ports(1)), w2(from1->ports(0),covOp->ports(2)), w3(covOp->ports(0),to->ports(1));
     CHECK_THROW(Eval(*to, covOp), std::exception);
   }
 
 TEST_FIXTURE(CorrelationFixture,dimensionNotFound)
   {
     OperationPtr covOp(OperationType::covariance);
     covOp->axis="foo";
     g->addItem(covOp);
     Wire w1(from->ports(0),covOp->ports(1)), w2(from1->ports(0),covOp->ports(2)), w3(covOp->ports(0),to->ports(1));
     CHECK_THROW(Eval(*to, covOp), std::exception);
   }
 
 TEST_FIXTURE(CorrelationFixture,vectorCovariance)
   {
     fromVal.hypercube(vector<unsigned>{5});
     from1Val.hypercube(vector<unsigned>{5});
     // calculated on Octave cov(x,y)
     vector<double> cov{0.081669};

     OperationPtr covOp(OperationType::covariance);
     g->addItem(covOp);
     Wire w1(from->ports(0),covOp->ports(1)), w2(from1->ports(0),covOp->ports(2)), w3(covOp->ports(0),to->ports(1));
     Eval(*to, covOp)();
     
     auto& toVal=*to->vValue();
     CHECK_EQUAL(cov.size(), toVal.size());
     CHECK_ARRAY_CLOSE(cov, &toVal[0], toVal.size(), 1e-4);
   }

  TEST_FIXTURE(CorrelationFixture,lineLinearRegression)
   {
     TensorVal x(vector<unsigned>{6}); x=vector<double>{0,1,2,3,4,5};
     TensorVal y(vector<unsigned>{6}); y=vector<double>{1,2,3,4,5,6};
     fromVal=y;
     from1Val=x;
     
     // line y=x+1
     vector<double> result={1,2,3,4,5,6};

     OperationPtr op(OperationType::linearRegression);
     g->addItem(op);
     Wire w1(from->ports(0),op->ports(1)), w2(from1->ports(0),op->ports(2)), w3(op->ports(0),to->ports(1));
     Eval(*to, op)();
     
     auto& toVal=*to->vValue();
     CHECK_EQUAL(result.size(), toVal.size());
     CHECK_ARRAY_CLOSE(result, &toVal[0], toVal.size(), 1e-4);
   }

  TEST_FIXTURE(CorrelationFixture,xvectorValueLinearRegression)
   {
     Hypercube hc;
     hc.xvectors.emplace_back("0", Dimension(Dimension::value,""), vector<any>{0,1,2,3,4,5});
     TensorVal y(hc); y=vector<double>{1,2,3,4,5,6};
     fromVal=y;
     
     // line y=x+1
     vector<double> result={1,2,3,4,5,6};

     OperationPtr op(OperationType::linearRegression);
     g->addItem(op);
     Wire w1(from->ports(0),op->ports(1)), w3(op->ports(0),to->ports(1));
     Eval(*to, op)();
     
     auto& toVal=*to->vValue();
     CHECK_EQUAL(result.size(), toVal.size());
     CHECK_ARRAY_CLOSE(result, &toVal[0], toVal.size(), 1e-4);
   }

  TEST_FIXTURE(CorrelationFixture,xvectorTimeLinearRegression)
   {
     Hypercube hc;
     hc.xvectors.emplace_back("0", Dimension(Dimension::time,""),
                              vector<any>{
                                ptime(date(1970,Jan,1)),
                                ptime(date(1971,Jan,1)),
                                ptime(date(1972,Jan,1)),
                                ptime(date(1973,Jan,1)),
                                ptime(date(1974,Jan,1)),
                                ptime(date(1975,Jan,1)),
                              });
     TensorVal y(hc); y=vector<double>{1,2,3,4,5,6};
     fromVal=y;
     
     // line y=x+1
     vector<double> result={1,2,3,4,5,6};

     OperationPtr op(OperationType::linearRegression);
     g->addItem(op);
     Wire w1(from->ports(0),op->ports(1)), w3(op->ports(0),to->ports(1));
     Eval(*to, op)();
     
     auto& toVal=*to->vValue();
     CHECK_EQUAL(result.size(), toVal.size());
     // larger tolerance to allow for years not all being the same length
     CHECK_ARRAY_CLOSE(result, &toVal[0], toVal.size(), 1e-1);
   }

  TEST_FIXTURE(CorrelationFixture,xvectorStringLinearRegression)
   {
     Hypercube hc;
     hc.xvectors.emplace_back("0", Dimension(Dimension::string,""), vector<any>{"a","b","c","d","e","f"});
     TensorVal y(hc); y=vector<double>{1,2,3,4,5,6};
     fromVal=y;
     
     // line y=x+1
     vector<double> result={1,2,3,4,5,6};

     OperationPtr op(OperationType::linearRegression);
     g->addItem(op);
     Wire w1(from->ports(0),op->ports(1)), w3(op->ports(0),to->ports(1));
     Eval(*to, op)();
     
     auto& toVal=*to->vValue();
     CHECK_EQUAL(result.size(), toVal.size());
     CHECK_ARRAY_CLOSE(result, &toVal[0], toVal.size(), 1e-4);
   }

  TEST_FIXTURE(CorrelationFixture,vectorLinearRegression)
   {
     TensorVal x(vector<unsigned>{8}); x=vector<double>{0,0,1,1,2,2,3,3};
     TensorVal y(vector<unsigned>{8}); y=vector<double>{0,2,1,3,2,4,3,5};
     fromVal=y;
     from1Val=x;
     
     // line y=x+1
     vector<double> result{1,1,2,2,3,3,4,4};

     OperationPtr op(OperationType::linearRegression);
     g->addItem(op);
     Wire w1(from->ports(0),op->ports(1)), w2(from1->ports(0),op->ports(2)), w3(op->ports(0),to->ports(1));
     Eval(*to, op)();
     
     auto& toVal=*to->vValue();
     CHECK_EQUAL(result.size(), toVal.size());
     CHECK_ARRAY_CLOSE(result, &toVal[0], toVal.size(), 1e-4);
   }

  TEST_FIXTURE(CorrelationFixture,matrixLinearRegression)
   {
     TensorVal x(vector<unsigned>{8,2}); x=vector<double>{0,0,1,1,2,2,3,3,0,0,1,1,2,2,3,3};
     TensorVal y(vector<unsigned>{8,2}); y=vector<double>{0,2,1,3,2,4,3,5,1,3,3,5,5,7,7,9};
     fromVal=y;
     from1Val=x;
     
     // lines y=x+1, y=2x+2
     vector<double> result{1,1,2,2,3,3,4,4,2,2,4,4,6,6,8,8};

     OperationPtr op(OperationType::linearRegression);
     op->axis="0";
     g->addItem(op);
     Wire w1(from->ports(0),op->ports(1)), w2(from1->ports(0),op->ports(2)), w3(op->ports(0),to->ports(1));
     Eval(*to, op)();
     
     auto& toVal=*to->vValue();
     CHECK_EQUAL(result.size(), toVal.size());
     CHECK_ARRAY_CLOSE(result, &toVal[0], toVal.size(), 1e-4);
     CHECK_EQUAL(x.rank(), toVal.rank());
     CHECK_ARRAY_EQUAL(x.hypercube().dims(), toVal.hypercube().dims(), x.rank());
   }

  TEST_FIXTURE(CorrelationFixture,allMatrixLinearRegression)
   {
     TensorVal x(vector<unsigned>{8,2}); x=vector<double>{0,0,1,1,2,2,3,3,0,0,1,1,2,2,3,3};
     TensorVal y(vector<unsigned>{8,2}); y=vector<double>{0,2,1,3,2,4,3,5,0,2,1,3,2,4,3,5};
     fromVal=y;
     from1Val=x;
     
     // lines y=x+1, y=2x+2
     vector<double> result{1,1,2,2,3,3,4,4,1,1,2,2,3,3,4,4};

     OperationPtr op(OperationType::linearRegression);
     g->addItem(op);
     Wire w1(from->ports(0),op->ports(1)), w2(from1->ports(0),op->ports(2)), w3(op->ports(0),to->ports(1));
     Eval(*to, op)();
     
     auto& toVal=*to->vValue();
     CHECK_EQUAL(result.size(), toVal.size());
     CHECK_ARRAY_CLOSE(result, &toVal[0], toVal.size(), 1e-4);
     CHECK_EQUAL(x.rank(), toVal.rank());
     CHECK_ARRAY_EQUAL(x.hypercube().dims(), toVal.hypercube().dims(), x.rank());
   }


 
}
