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
#undef True
#include <gtest/gtest.h>
using namespace minsky;

#include <exception>
using namespace std;

#include <boost/date_time.hpp>
using namespace boost;
using namespace boost::posix_time;
using namespace boost::gregorian;
using civita::any;

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

class TensorOpSuite : public ::testing::Test
{
public:
  Minsky dummyM;
  LocalMinsky lm{dummyM};

  GroupPtr g{new Group}; // allow itemPtrFromThis() to work.
  VariablePtr from{VariableType::flow,"from"}, to{VariableType::flow,"to"};
  VariableValue& fromVal;
  TensorOpSuite(): fromVal(*from->vValue()) {
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
    EXPECT_EQ(ref,to->vValue()->value());

    vector<unsigned> dims{5,5};
    fromVal.hypercube(Hypercube(dims));
    auto& toVal=*to->vValue();
    for (auto& i: fromVal)
      i=&i-&fromVal[0]+1;

    // reduce over first dimension
    evalOp<op>("0");
    EXPECT_EQ(1, toVal.rank());
    for (size_t i=0; i<toVal.size(); ++i)
      {
        double ref=fromVal[dims[0]*i];
        for (size_t j=1; j<dims[0]; ++j)
          ref=f(ref,fromVal[j+dims[0]*i]);
        EXPECT_EQ(ref,toVal[i]);
      }
      
    evalOp<op>("1");
    EXPECT_EQ(1, toVal.rank());
    for (size_t i=0; i<toVal.size(); ++i)
      {
        double ref=fromVal[i];
        for (size_t j=1; j<dims[1]; ++j)
          ref=f(ref,fromVal[i+dims[0]*j]);
        EXPECT_EQ(ref,toVal[i]);
      }
  }
};

class TensorOpSuite1: public TensorOpSuite {};

class MinskyTensorOpSuite : public Minsky, public ::testing::Test
{
public:
  LocalMinsky lm{*this};
};

TEST_F(TensorOpSuite, reduction)
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
  EXPECT_EQ(1,to->vValue()->value());
  evalOp<OperationType::supIndex>();
  EXPECT_EQ(3,to->vValue()->value());
}
  
TEST_F(TensorOpSuite, scan)
{
  for (auto& i: fromVal)
    i=2;
      
  evalOp<OperationType::runningSum>();
  {
    auto& toVal=*to->vValue();
    for (size_t i=0; i<toVal.size(); ++i)
      EXPECT_EQ(2*(i+1),toVal[i]);
  }
      
  evalOp<OperationType::runningProduct>();
  {
    auto& toVal=*to->vValue();
    for (size_t i=0; i<toVal.size(); ++i)
      EXPECT_EQ(pow(2,i+1),toVal[i]);
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
          EXPECT_EQ(ref,toVal({i,j}));
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
          EXPECT_EQ(ref,toVal({i,j}));
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
          EXPECT_EQ(ref,toVal({i,j}));
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
          EXPECT_EQ(ref,toVal({i,j}));
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
          EXPECT_EQ(ref,toVal({i,j}));
        }
  }
}

TEST_F(TensorOpSuite, difference2D)
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
      EXPECT_EQ(1,i);
  evalOp<OperationType::difference>("1",delta);
  for (auto& i: *to->vValue())
    if (std::isfinite(i))
      EXPECT_EQ(5,i);
  delta=2;
  evalOp<OperationType::difference>("0",delta);
  for (auto& i: *to->vValue())
    if (std::isfinite(i))
      EXPECT_EQ(2,i);
  evalOp<OperationType::difference>("1",delta);
  for (auto& i: *to->vValue())
    if (std::isfinite(i))
      EXPECT_EQ(10,i);
}
TEST_F(TensorOpSuite, difference1D)
{
  vector<unsigned> dims{5};
  fromVal.hypercube(Hypercube(dims));
  int cnt=0;
  for (auto& i: fromVal)
    i=cnt++;

  int delta=1;
  evalOp<OperationType::difference>("",delta=1);
  EXPECT_EQ(4, to->vValue()->hypercube().dims()[0]);
  for (auto& i: *to->vValue())
    EXPECT_EQ(1,i);
  EXPECT_EQ(1, to->vValue()->hypercube().xvectors[0][0].value);

  evalOp<OperationType::difference>("",delta=-1);
  EXPECT_EQ(4, to->vValue()->hypercube().dims()[0]);
  for (auto& i: *to->vValue())
    EXPECT_EQ(-1,i);
  EXPECT_EQ(0, to->vValue()->hypercube().xvectors[0][0].value);
                  
  evalOp<OperationType::difference>("",delta=2);
  EXPECT_EQ(3, to->vValue()->hypercube().dims()[0]);
  for (auto& i: *to->vValue())
    EXPECT_EQ(2,i);
  EXPECT_EQ(2, to->vValue()->hypercube().xvectors[0][0].value);

  // check that the sparse code works as expected
  fromVal.index({0,1,2,3,4});
  evalOp<OperationType::difference>("",delta=1);
  EXPECT_EQ(4, to->vValue()->hypercube().dims()[0]);
  for (auto& i: *to->vValue())
    EXPECT_EQ(1,i);
  EXPECT_EQ(1, to->vValue()->hypercube().xvectors[0][0].value);
  vector<size_t> ii{0,1,2,3};
  for (size_t _i=0; _i<4; ++_i) EXPECT_EQ(ii[_i], to->vValue()->index()[_i]);
}
  
TEST_F(TensorOpSuite, difference2D_II)
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
  EXPECT_EQ(4, to->vValue()->hypercube().dims()[1]);
  for (auto& i: *to->vValue())
    if (std::isfinite(i))
      EXPECT_EQ(1,i);
  EXPECT_EQ(1, to->vValue()->hypercube().xvectors[1][0].value);

  evalOp<OperationType::difference>("1",delta=-1);
  EXPECT_EQ(4, to->vValue()->hypercube().dims()[1]);
  for (auto& i: *to->vValue())
    if (std::isfinite(i))
      EXPECT_EQ(-1,i);
  EXPECT_EQ(0, to->vValue()->hypercube().xvectors[1][0].value);
                  
  evalOp<OperationType::difference>("1",delta=2);
  EXPECT_EQ(3, to->vValue()->hypercube().dims()[1]);
  for (auto& i: *to->vValue())
    if (std::isfinite(i))
      EXPECT_EQ(2,i);
  EXPECT_EQ(2, to->vValue()->hypercube().xvectors[1][0].value);

  // check that the sparse code works as expected
  fromVal.index({3,8,13,16,32,64});
  for (size_t i=0; i<fromVal.size(); ++i)
    fromVal[i]=(i%5)+(i/5)%5+(i/5);
  evalOp<OperationType::difference>("1",delta=1);
  EXPECT_EQ(4, to->vValue()->hypercube().dims()[1]);
  cnt=0;
  for (auto& i: *to->vValue())
    if (std::isfinite(i))
      {
        EXPECT_EQ(1,i);
        cnt++;
      }
  EXPECT_EQ(2,cnt);
  EXPECT_EQ(1, to->vValue()->hypercube().xvectors[1][0].value);
  vector<size_t> ii{3,8};
  for (size_t _i=0; _i<ii.size(); ++_i) EXPECT_EQ(ii[_i], to->vValue()->index()[_i]);
      
  evalOp<OperationType::difference>("1",delta=-1);
  EXPECT_EQ(4, to->vValue()->hypercube().dims()[1]);
  cnt=0;
  for (auto& i: *to->vValue())
    if (std::isfinite(i))
      {
        EXPECT_EQ(-1,i);
        cnt++;
      }
  EXPECT_EQ(2,cnt);
  EXPECT_EQ(0, to->vValue()->hypercube().xvectors[1][0].value);
  vector<size_t> i2{3,8};
  for (size_t _i=0; _i<i2.size(); ++_i) EXPECT_EQ(i2[_i], to->vValue()->index()[_i]);
}

TEST_F(TensorOpSuite, gatherInterpolateValue)
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
      EXPECT_EQ(0,gathered.rank());
      EXPECT_NEAR(i, gathered[0], 1E-4);
    }
}
  
TEST_F(TensorOpSuite, gatherInterpolateDate)
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
      EXPECT_EQ(0,gathered.rank());
      EXPECT_NEAR(i, gathered[0],0.01);
    }
}
  
TEST_F(TensorOpSuite, gatherExtractRowColumn)
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

  EXPECT_EQ(2, fromVal.rank());
  {
    // extract row
    gatherOp->axis="0";
    Eval eval(gatheredVar, gatherOp);
    for (size_t i=0; i<fromVal.shape()[0]; ++i)
      {
        toVal[0]=i;
        eval();
        EXPECT_EQ(1,gathered.rank());
        vector<double> expected;
        for (size_t j=0; j<fromVal.shape()[1]; ++j)
          expected.push_back(fromVal[i+fromVal.shape()[0]*j]);
        EXPECT_EQ(expected.size(), gathered.size());
        for (size_t _i=0; _i<expected.size(); ++_i) EXPECT_NEAR(expected[_i], gathered[_i], 1e-4);
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
        EXPECT_EQ(1,gathered.rank());
        vector<double> expected;
        for (size_t j=0; j<fromVal.shape()[0]; ++j)
          expected.push_back(fromVal[j+fromVal.shape()[0]*i]);
        EXPECT_EQ(expected.size(), gathered.size());
        for (size_t _i=0; _i<expected.size(); ++_i) EXPECT_NEAR(expected[_i], gathered[_i], 1e-4);
      }
  }
}

  
TEST_F(TensorOpSuite, indexGather)
{
  auto& toVal=*to->vValue();
  for (auto& i: fromVal)
    i=(&i-&fromVal[0])%2;
  evalOp<OperationType::index>();
  vector<double> expected{1,3};
  for (size_t _i=0; _i<2; ++_i) EXPECT_EQ(expected[_i], toVal.begin()[_i]);
  for (size_t i=3; i<toVal.size(); ++i)
    EXPECT_TRUE(std::isnan(toVal[i]));

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
  for (size_t _i=0; _i<5; ++_i) EXPECT_EQ(expected[_i], gathered.begin()[_i]);

  // another example - check for corner cases
  vector<double> data{0.36,0.412,0.877,0.437,0.751};
  memcpy(fromVal.begin(),data.data(),data.size()*sizeof(data[0]));
      
  eval();
  expected={1,3};
  for (size_t _i=0; _i<2; ++_i) EXPECT_EQ(expected[_i], toVal.begin()[_i]);
  for (size_t i=3; i<toVal.size(); ++i)
    EXPECT_TRUE(std::isnan(toVal[i]));

  // replace nans with -1 to make comparison test simpler
  for (auto& g: gathered)
    if (!finite(g)) g=-1;
  expected={0.412, 0.437, -1, -1, -1};
  for (size_t _i=0; _i<5; ++_i) EXPECT_EQ(expected[_i], gathered.begin()[_i]);
}

TEST_F(TensorOpSuite, sparseGather)
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
  EXPECT_EQ(expected.size(), gathered.size());
  for (size_t _i=0; _i<expected.size(); ++_i) EXPECT_EQ(expected[_i], gathered.begin()[_i]);

}

 
TEST_F(TensorOpSuite, sparse2Gather)
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
  EXPECT_EQ(expected.size(), gathered.size());
  for (size_t _i=0; _i<expected.size(); ++_i) EXPECT_EQ(expected[_i], gathered.begin()[_i]);
  for (size_t _i=0; _i<toVal.index().size(); ++_i) EXPECT_EQ(gathered.index()[_i], toVal.index()[_i]);

}

TEST_F(TensorOpSuite, sparse3DGather)
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
  EXPECT_EQ(expected.size(), gathered.hypercube().numElements());
  for (size_t i=0; i<expected.size(); ++i)
    if (isnan(expected[i]))
      EXPECT_TRUE(isnan(gathered.atHCIndex(i)));
    else
      EXPECT_EQ(expected[i], gathered.atHCIndex(i));
}

TEST_F(TensorOpSuite, gatherExceptions)
{
  OperationPtr gatherOp(OperationType::gather);
  Variable<VariableType::flow> gatheredVar("gathered");
  Wire w1(from->ports(0), gatherOp->ports(1));
  Wire w2(to->ports(0), gatherOp->ports(2));
  Wire w3(gatherOp->ports(0), gatheredVar.ports(1));
        
  fromVal.hypercube(Hypercube());
  EXPECT_THROW(Eval(gatheredVar, gatherOp)(), std::exception);

  fromVal.hypercube(Hypercube({3,4}));
  EXPECT_THROW(Eval(gatheredVar, gatherOp)(), std::exception);
}

TEST_F(MinskyTensorOpSuite, gatherBackElement)
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
  EXPECT_EQ(0, zz.rank());
  EXPECT_EQ(4, zz[0]);
}

TEST_F(TensorOpSuite, indexGatherTensorStringArgs)
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
  for (size_t _i=0; _i<checkV.size(); ++_i) EXPECT_EQ(checkV[_i], fromHC.dims()[_i]);
  checkV={2,2};
  for (size_t _i=0; _i<checkV.size(); ++_i) EXPECT_EQ(checkV[_i], toHC.dims()[_i]);

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

  EXPECT_EQ(fv.size(), fromVal.size());
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
      
  vector<size_t> expectedDims{4,2,5};
  EXPECT_EQ(expectedDims.size(), gathered.rank());
  for (size_t _i=0; _i<expectedDims.size(); ++_i) EXPECT_EQ(expectedDims[_i], gathered.hypercube().dims()[_i]);

  auto& gxv=gathered.hypercube().xvectors;
  EXPECT_EQ("y",gxv[0].name);
  EXPECT_EQ("x",gxv[1].name);
  EXPECT_EQ("z",gxv[2].name);

  for (size_t i=0; i<expectedDims[2]; ++i)
    for (size_t j=0; j<expectedDims[1]; ++j)
      {
        EXPECT_TRUE(isnan(gathered[(i*expectedDims[1]+j)*toVal.size()]));
        EXPECT_TRUE(isnan(gathered[(i*expectedDims[1]+j)*toVal.size()+1]));
        EXPECT_NEAR(i+j+1.3, gathered[(i*expectedDims[1]+j)*toVal.size()+2],0.01);
        EXPECT_EQ(i+j+2, gathered[(i*expectedDims[1]+j)*toVal.size()+3]);
      }
}

TEST_F(TensorOpSuite, indexGatherTensorValueArgs)
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
  for (size_t _i=0; _i<checkV.size(); ++_i) EXPECT_EQ(checkV[_i], fromHC.dims()[_i]);
  checkV={2,2};
  for (size_t _i=0; _i<checkV.size(); ++_i) EXPECT_EQ(checkV[_i], toHC.dims()[_i]);

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

  EXPECT_EQ(fv.size(), fromVal.size());
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
      
  vector<size_t> expectedDims{4,2,5};
  EXPECT_EQ(expectedDims.size(), gathered.rank());
  for (size_t _i=0; _i<expectedDims.size(); ++_i) EXPECT_EQ(expectedDims[_i], gathered.hypercube().dims()[_i]);

  for (size_t i=0; i<expectedDims[2]; ++i)
    for (size_t j=0; j<expectedDims[1]; ++j)
      {
        EXPECT_TRUE(isnan(gathered[(i*expectedDims[1]+j)*toVal.size()]));
        EXPECT_TRUE(isnan(gathered[(i*expectedDims[1]+j)*toVal.size()+1]));
        EXPECT_NEAR(i+j+0.6, gathered[(i*expectedDims[1]+j)*toVal.size()+2],0.01);
        EXPECT_EQ(i+j+1, gathered[(i*expectedDims[1]+j)*toVal.size()+3]);
      }
}

TEST_F(TensorOpSuite, indexGatherTensorTimeArgs)
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
  for (size_t _i=0; _i<checkV.size(); ++_i) EXPECT_EQ(checkV[_i], fromHC.dims()[_i]);
  checkV={2,2};
  for (size_t _i=0; _i<checkV.size(); ++_i) EXPECT_EQ(checkV[_i], toHC.dims()[_i]);

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

  EXPECT_EQ(fv.size(), fromVal.size());
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
      
  vector<size_t> expectedDims{4,2,5};
  EXPECT_EQ(expectedDims.size(), gathered.rank());
  for (size_t _i=0; _i<expectedDims.size(); ++_i) EXPECT_EQ(expectedDims[_i], gathered.hypercube().dims()[_i]);

  for (size_t i=0; i<expectedDims[2]; ++i)
    for (size_t j=0; j<expectedDims[1]; ++j)
      {
        EXPECT_TRUE(isnan(gathered[(i*expectedDims[1]+j)*toVal.size()]));
        EXPECT_TRUE(isnan(gathered[(i*expectedDims[1]+j)*toVal.size()+1]));
        EXPECT_NEAR(i+j+0.6, gathered[(i*expectedDims[1]+j)*toVal.size()+2],0.01);
        EXPECT_EQ(i+j+1, gathered[(i*expectedDims[1]+j)*toVal.size()+3]);
      }
}

  
  
TEST_F(MinskyTensorOpSuite, tensorUnOpFactory)
{
  TensorOpFactory factory;
  auto ev=make_shared<EvalCommon>();
  TensorsFromPort tp(ev);
  VariablePtr src(VariableType::flow,"src"), dest(VariableType::flow,"dest");
  model->addItem(src); model->addItem(dest);
  src->init("iota(5)");
  variableValues.reset();
  EXPECT_EQ(1,src->vValue()->rank());
  EXPECT_EQ(5,src->vValue()->size());
  for (OperationType::Type op=OperationType::copy; op<OperationType::innerProduct;
       op=OperationType::Type(op+1))
    {
      OperationPtr o(op);
      model->addItem(o);
      EXPECT_EQ(2, o->numPorts());
      Wire w1(src->ports(0), o->ports(1)), w2(o->ports(0), dest->ports(1));
      TensorEval eval(dest->vValue(), ev, factory.create(o,tp));
      eval.eval(ValueVector::flowVars.data(), ValueVector::flowVars.size(), ValueVector::stockVars.data());
      switch (OperationType::classify(op))
        {
        case OperationType::function:
          {
            // just check that scalar functions are performed elementwise
            EXPECT_EQ(src->vValue()->size(), dest->vValue()->size());
            unique_ptr<ScalarEvalOp> scalarOp(ScalarEvalOp::create(op,o));
            EXPECT_TRUE(scalarOp.get());
            for (size_t i=0; i<src->vValue()->size(); ++i)
              {
                double x=scalarOp->evaluate((*src->vValue())[i]);
                double y=(*dest->vValue())[i];
                if (finite(x)||finite(y))
                  EXPECT_EQ(x,y);
              }
            break;
          }
        case OperationType::reduction:
          EXPECT_EQ(0, dest->vValue()->rank());
          EXPECT_EQ(1, dest->vValue()->size());
          break;
        case OperationType::scan:
          EXPECT_EQ(1, dest->vValue()->rank());
          if (op==OperationType::difference || op==OperationType::differencePlus) //TODO should difference really be a scan?
            EXPECT_EQ(src->vValue()->size()-1, dest->vValue()->size());
          else
            EXPECT_EQ(src->vValue()->size(), dest->vValue()->size());
          break;
        default:
          EXPECT_TRUE(false);
          break;
        }
      model->removeItem(*o);
    }
}
  
TEST_F(MinskyTensorOpSuite, tensorBinOpFactory)
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
  EXPECT_EQ(1,src1->vValue()->rank());
  EXPECT_EQ(5,src1->vValue()->size());
  EXPECT_EQ(1,src2->vValue()->rank());
  EXPECT_EQ(5,src2->vValue()->size());
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
      EXPECT_EQ(3, o->numPorts());
      Wire w1(src1->ports(0), o->ports(1)), w2(src2->ports(0), o->ports(2)),
        w3(o->ports(0), dest->ports(1));
      TensorEval eval(dest->vValue(), ev, factory.create(o,tp));
      eval.eval(ValueVector::flowVars.data(), ValueVector::flowVars.size(), ValueVector::stockVars.data());
      EXPECT_EQ(src1->vValue()->size(), dest->vValue()->size());
      EXPECT_EQ(src2->vValue()->size(), dest->vValue()->size());
      unique_ptr<ScalarEvalOp> scalarOp(ScalarEvalOp::create(op,o));
      for (size_t i=0; i<src1->vValue()->size(); ++i)
        {
          double x=scalarOp->evaluate((*src1->vValue())[i], (*src2->vValue())[i]);
          double y=(*dest->vValue())[i];
          if (finite(x)||finite(y))
            EXPECT_EQ(x,y);
        }
    }
}

template <OperationType::Type op, class F, class F2>
void multiWireTest(double identity, F f, F2 f2)
{
  //cout << OperationType::typeName(op)<<endl;
  OperationPtr o(op);
  auto tensorOp=TensorOpFactory().create(o);
  EXPECT_EQ(1, tensorOp->size());
  EXPECT_EQ(identity, (*tensorOp)[0]);
  Hypercube hc(vector<unsigned>{2});
  auto tv1=make_shared<TensorVal>(hc), tv2=make_shared<TensorVal>(hc);
  map<size_t,double> tv1Data{{0,1},{1,2}}, tv2Data{{0,2},{1,1}};
  *tv1=tv1Data;
  *tv2=tv2Data;
  tensorOp->setArguments(vector<TensorPtr>{tv1,tv2},vector<TensorPtr>{});
  EXPECT_EQ(f((*tv1)[0],(*tv2)[0]), (*tensorOp)[0]);
  EXPECT_EQ(f((*tv1)[1],(*tv2)[1]), (*tensorOp)[1]);
  tensorOp->setArguments(vector<TensorPtr>{},vector<TensorPtr>{tv1,tv2});
  EXPECT_EQ(f2(f((*tv1)[0],(*tv2)[0])), (*tensorOp)[0]);
  EXPECT_EQ(f2(f((*tv1)[1],(*tv2)[1])), (*tensorOp)[1]);
}

TEST_F(MinskyTensorOpSuite, tensorBinOpMultiples)
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

TEST_F(MinskyTensorOpSuite, binOp)
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
  EXPECT_EQ(x1Val.size(),result->vValue()->size());

  vector<double> expected{4,6,7,7,10,10,3};
  for (size_t _i=0; _i<7; ++_i) EXPECT_NEAR(expected[_i], result->vValue()->begin()[_i], 0.001);
}

TEST_F(MinskyTensorOpSuite, binOpInterpolation1D)
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
  EXPECT_EQ(5,result->vValue()->size());

  vector<double> expected{5,5.5,5,7.5,7};
  for (size_t _i=0; _i<5; ++_i) EXPECT_NEAR(expected[_i], result->vValue()->begin()[_i], 0.001);
}
  
TEST_F(MinskyTensorOpSuite, binOpInterpolation1Dunsorted)
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
  EXPECT_EQ(5,result->vValue()->size());

  vector<double> expected{5,5.5,5,7.5,7};
  for (size_t _i=0; _i<5; ++_i) EXPECT_NEAR(expected[_i], result->vValue()->begin()[_i], 0.001);
}
  
TEST_F(MinskyTensorOpSuite, binOpInterpolation2D)
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

  EXPECT_EQ(9, result->vValue()->size());
  vector<double> expected{7,8.5,10,12.3333,13.8333,15.3333,15,16.5,18};
  for (size_t _i=0; _i<expected.size(); ++_i) EXPECT_NEAR(expected[_i], result->vValue()->begin()[_i], 0.001);
}

struct TensorValFixture: public ::testing::Test
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
  
TEST_F(TensorValFixture, sliced2dswapped)
{
  auto sex=find_if(state.handleStates.begin(), state.handleStates.end(),
                   [](const ravel::HandleState& i){return i.description=="sex";});
  sex->sliceLabel="male";
  state.outputHandles={"date","country"};
  auto chain=createRavelChain(state, arg);
  EXPECT_EQ(2, chain.back()->rank());
  auto& chc=chain.back()->hypercube();
  auto& ahc=arg->hypercube();
  EXPECT_EQ("date",chc.xvectors[0].name);
  EXPECT_TRUE(chc.xvectors[0]==ahc.xvectors[2]);
  EXPECT_EQ("country",chc.xvectors[1].name);
  EXPECT_TRUE(chc.xvectors[1]==ahc.xvectors[0]);
  EXPECT_EQ(9,chain.back()->size());
  for (size_t i=0; i<chain.back()->size(); ++i)
    EXPECT_TRUE(ahc.splitIndex((*chain.back())[i])[1]==0); //entry is "male"
  vector<double> expected={0,1,2,6,7,8,12,13,14};
  for (size_t _i=0; _i<9; ++_i) EXPECT_EQ(expected[_i], (*chain[1])[_i]);
  expected={0,6,12,1,7,13,2,8,14};
  for (size_t _i=0; _i<9; ++_i) EXPECT_EQ(expected[_i], (*chain.back())[_i]);

  sex->sliceLabel="female";
  chain=createRavelChain(state, arg);
  EXPECT_EQ(9,chain.back()->size());
  for (size_t i=0; i<chain.back()->size(); ++i)
    EXPECT_TRUE(ahc.splitIndex((*chain.back())[i])[1]==1); //entry is "female"
      
}
TEST_F(TensorValFixture, reduction2dswapped)
{
  auto sex=find_if(state.handleStates.begin(), state.handleStates.end(),
                   [](const ravel::HandleState& i){return i.description=="sex";});
  sex->collapsed=true;
  sex->reductionOp=ravel::Op::sum;
  state.outputHandles={"date","country"};
  auto chain=createRavelChain(state, arg);
  EXPECT_EQ(2, chain.back()->rank());
  auto& chc=chain.back()->hypercube();
  auto& ahc=arg->hypercube();
  EXPECT_EQ("date",chc.xvectors[0].name);
  EXPECT_TRUE(chc.xvectors[0]==ahc.xvectors[2]);
  EXPECT_EQ("country",chc.xvectors[1].name);
  EXPECT_TRUE(chc.xvectors[1]==ahc.xvectors[0]);
  EXPECT_EQ(9,chain.back()->size());
  vector<double> expected={3,15,27,5,17,29,7,19,31};
  // above expected was in sorted index order
  auto index=chain.back()->index();
  vector<size_t> sortedIdx(index.begin(),index.end()); sort(sortedIdx.begin(),sortedIdx.end());
  for (size_t _i=0; _i<9; ++_i) EXPECT_EQ(expected[_i], chain.back()->atHCIndex(sortedIdx[_i]));

  sex->reductionOp=ravel::Op::prod;
  chain=createRavelChain(state, arg);
  expected={0,54,180,4,70,208,10,88,238};
  index=chain.back()->index();
  sortedIdx=vector<size_t>(index.begin(),index.end()); sort(sortedIdx.begin(),sortedIdx.end());
   for (size_t _i=0; _i<9; ++_i) EXPECT_EQ(expected[_i], chain.back()->atHCIndex(sortedIdx[_i]));

  sex->reductionOp=ravel::Op::av;
  chain=createRavelChain(state, arg);
  expected={1.5,7.5,13.5,2.5,8.5,14.5,3.5,9.5,15.5};
  index=chain.back()->index();
  sortedIdx=vector<size_t>(index.begin(),index.end()); sort(sortedIdx.begin(),sortedIdx.end());
  for (size_t _i=0; _i<9; ++_i) EXPECT_EQ(expected[_i], chain.back()->atHCIndex(sortedIdx[_i]));

  sex->reductionOp=ravel::Op::stddev;
  chain=createRavelChain(state, arg);
  expected={2.12132, 2.12132, 2.12132, 2.12132, 2.12132, 2.12132, 2.12132, 2.12132, 2.12132};
  index=chain.back()->index();
  sortedIdx=vector<size_t>(index.begin(),index.end()); sort(sortedIdx.begin(),sortedIdx.end());
  for (size_t _i=0; _i<9; ++_i) EXPECT_NEAR(expected[_i], chain.back()->atHCIndex(sortedIdx[_i]), 1e-4);

  sex->reductionOp=ravel::Op::min;
  chain=createRavelChain(state, arg);
  expected={0,6,12,1,7,13,2,8,14};
  index=chain.back()->index();
  sortedIdx=vector<size_t>(index.begin(),index.end()); sort(sortedIdx.begin(),sortedIdx.end());
  for (size_t _i=0; _i<9; ++_i) EXPECT_EQ(expected[_i], chain.back()->atHCIndex(sortedIdx[_i]));
      
  sex->reductionOp=ravel::Op::max;
  chain=createRavelChain(state, arg);
  expected={3,9,15,4,10,16,5,11,17};
  index=chain.back()->index();
  sortedIdx=vector<size_t>(index.begin(),index.end()); sort(sortedIdx.begin(),sortedIdx.end());
  for (size_t _i=0; _i<9; ++_i) EXPECT_EQ(expected[_i], chain.back()->atHCIndex(sortedIdx[_i]));
     
}

TEST_F(TensorValFixture, sparseSlicedRavel)
{
  state.outputHandles={"date","country"};
  auto sex=find_if(state.handleStates.begin(), state.handleStates.end(),
                   [](const ravel::HandleState& i){return i.description=="sex";});
  arg->index({0,4,8,12,16});
  sex->sliceLabel="male";
  auto chain=createRavelChain(state, arg);
  EXPECT_EQ(2, chain.back()->rank());
  EXPECT_EQ(3, chain.back()->size());
  set<size_t> expectedi{0,5,6};
  set<size_t> idx(chain[1]->index().begin(), chain[1]->index().end());
  EXPECT_TRUE(idx==expectedi);
  expectedi={0,2,7};
  idx=set<size_t>(chain.back()->index().begin(),chain.back()->index().end());
  EXPECT_TRUE(idx==expectedi);
  
  vector<double> expectedf{0,1,2,3,4};
  for (size_t _i=0; _i<3; ++_i) EXPECT_EQ(expectedf[_i], (*arg)[_i]);
  expectedf={0,2,3};
  auto index=chain[1]->index();
  vector<size_t> sortedIdx(index.begin(),index.end()); sort(sortedIdx.begin(),sortedIdx.end());
  for (size_t _i=0; _i<3; ++_i) EXPECT_EQ(expectedf[_i], chain[1]->atHCIndex(sortedIdx[_i]));
  expectedf={0,3,2};
  index=chain.back()->index();
  sortedIdx=vector<size_t>(index.begin(),index.end()); sort(sortedIdx.begin(),sortedIdx.end());
  for (size_t _i=0; _i<3; ++_i) EXPECT_EQ(expectedf[_i], chain.back()->atHCIndex(sortedIdx[_i]));

  sex->collapsed=true;
  chain=createRavelChain(state, arg);
  EXPECT_EQ(5, chain.back()->size());
  expectedi={0,1,5,6,7};
  idx=set<size_t>(chain[1]->index().begin(),chain[1]->index().end());
  EXPECT_TRUE(idx==expectedi);

  expectedi={0,2,3,5,7};
  idx=set<size_t>(chain.back()->index().begin(),chain.back()->index().end());
  EXPECT_TRUE(idx==expectedi);
  expectedf={0,1,2,3,4};
  index=chain[1]->index();
  sortedIdx=vector<size_t>(index.begin(),index.end()); sort(sortedIdx.begin(),sortedIdx.end());
  for (size_t _i=0; _i<5; ++_i) EXPECT_EQ(expectedf[_i], chain[1]->atHCIndex(sortedIdx[_i]));
  expectedf={0,3,1,4,2};
  index=chain.back()->index();
  sortedIdx=vector<size_t>(index.begin(),index.end()); sort(sortedIdx.begin(),sortedIdx.end());
  for (size_t _i=0; _i<5; ++_i) EXPECT_EQ(expectedf[_i], chain.back()->atHCIndex(sortedIdx[_i])); 
}
    
TEST_F(TensorValFixture, calipered)
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
  for (size_t _i=0; _i<expected.size(); ++_i) EXPECT_EQ(expected[_i], (*chain.back())[_i]);
  vector<double> expectedi={3};
  for (size_t _i=0; _i<expectedi.size(); ++_i) EXPECT_EQ(expectedi[_i], chain.back()->index()[_i]);
  vector<size_t> dims={2,2};
  for (size_t _i=0; _i<2; ++_i) EXPECT_EQ(dims[_i], chain.back()->shape()[_i]);
}

TEST_F(TensorValFixture, imposeDimensions)
{
  Dimensions dimensions;
  dimensions.emplace("date",Dimension{Dimension::time,"%Y"});
  arg->imposeDimensions(dimensions);
  auto& xv=arg->hypercube().xvectors[2];
  EXPECT_EQ("date",xv.name);
  EXPECT_EQ(Dimension::time,xv.dimension.type);
  EXPECT_EQ("%Y",xv.dimension.units);
  for (auto& i: xv)
    EXPECT_TRUE(i.type==Dimension::time);
}

TEST(TensorOp, tensorValVectorIndex)
{
  TensorVal tv(vector<unsigned>{5,3,2});
  for (size_t i=0; i<tv.size(); ++i) tv[i]=i;
  EXPECT_EQ(8,tv({3,1,0}));
  tv.index({1,4,8,12});
  for (size_t i=0; i<tv.size(); ++i) tv[i]=i;
  EXPECT_EQ(2,tv({3,1,0}));
  EXPECT_TRUE(isnan(tv({2,1,0})));
}
    
TEST(TensorOp, tensorValAssignment)
{
  auto arg=std::make_shared<TensorVal>(vector<unsigned>{5,3,2});
  for (size_t i=0; i<arg->size(); ++i) (*arg)[i]=i;
  Scan scan([](double& x,double y,size_t){x+=y;});
  scan.setArgument(arg,{"0",0});
  EXPECT_EQ(arg->rank(), scan.rank());
  EXPECT_TRUE(scan.size()>1);
        
  TensorVal tv;
  tv=scan;

  EXPECT_EQ(tv.size(), scan.size());
  for (size_t _i=0; _i<scan.rank(); ++_i) EXPECT_EQ(tv.hypercube().dims()[_i], scan.hypercube().dims()[_i]);
  for (size_t i=0; i<tv.size(); ++i)
    EXPECT_EQ(scan[i], tv[i]);
}

TEST(TensorOp, permuteAxis)
{
  // 5x5 example
  Hypercube hc{5,5};
  auto dense=make_shared<TensorVal>(hc);
  for (size_t i=0; i<dense->size(); ++i) (*dense)[i]=i;
  PermuteAxis pa;
  pa.setArgument(dense,{"0",0});
  vector<size_t> permutation{1,4,3};
  pa.setPermutation(permutation);
  EXPECT_EQ(2, pa.rank());
  EXPECT_EQ(3, pa.hypercube().dims()[0]);
  EXPECT_EQ(5, pa.hypercube().dims()[1]);
  EXPECT_EQ(15, pa.size());
        
  for (size_t i=0; i<pa.size(); ++i)
    {
      switch (i%3)
        {
        case 0:
          EXPECT_EQ(1, int(pa[i])%5);
          break;
        case 1:
          EXPECT_EQ(4, int(pa[i])%5);
          break;
        case 2:
          EXPECT_EQ(3, int(pa[i])%5);
          break;
        }
    }

  pa.setArgument(dense,{"1",0});
  pa.setPermutation(permutation);
  EXPECT_EQ(2, pa.rank());
  EXPECT_EQ(5, pa.hypercube().dims()[0]);
  EXPECT_EQ(3, pa.hypercube().dims()[1]);
  EXPECT_EQ(15, pa.size());
        
  for (size_t i=0; i<pa.size(); ++i)
    {
      switch (i/5)
        {
        case 0:
          EXPECT_EQ(1, int(pa[i])/5);
          break;
        case 1:
          EXPECT_EQ(4, int(pa[i])/5);
          break;
        case 2:
          EXPECT_EQ(3, int(pa[i])/5);
          break;
        }
    }

        
        
  auto sparse=make_shared<TensorVal>(hc);
  sparse->index(std::set<size_t>{2,4,5,8,10,11,15,20});
  for (size_t i=0; i<sparse->size(); ++i) (*sparse)[i]=sparse->index()[i];

  pa.setArgument(sparse,{"0",0});
  pa.setPermutation(permutation);
  EXPECT_EQ(2, pa.rank());
  EXPECT_EQ(3, pa.hypercube().dims()[0]);
  EXPECT_EQ(5, pa.hypercube().dims()[1]);
  EXPECT_EQ(3, pa.size());
        
  for (size_t i=0; i<pa.size(); ++i)
    {
      auto splitted=pa.hypercube().splitIndex(pa.index()[i]);
      switch (splitted[0])
        {
        case 0:
          EXPECT_EQ(1, int(pa[i])%5);
          break;
        case 1:
          EXPECT_EQ(4, int(pa[i])%5);
          break;
        case 2:
          EXPECT_EQ(3, int(pa[i])%5);
          break;
        default:
          EXPECT_TRUE(false);
        }
    }
  pa.setArgument(sparse,{"1",0});
  pa.setPermutation(permutation);
  EXPECT_EQ(2, pa.rank());
  EXPECT_EQ(3, pa.hypercube().dims()[1]);
  EXPECT_EQ(5, pa.hypercube().dims()[0]);
  EXPECT_EQ(4, pa.size());
        
  for (size_t i=0; i<pa.size(); ++i)
    {
      auto splitted=pa.hypercube().splitIndex(pa.index()[i]);
      switch (splitted[1])
        {
        case 0:
          EXPECT_EQ(1, int(pa[i])/5);
          break;
        case 1:
          EXPECT_EQ(4, int(pa[i])/5);
          break;
        case 2:
          EXPECT_EQ(3, int(pa[i])/5);
          break;
        default:
          EXPECT_TRUE(false);
        }
    }
        
}

TEST(TensorOp, dimLabels)
{
  vector<XVector> x{{"x",{Dimension::string,""}}, {"y",{Dimension::string,""}},
                    {"z",{Dimension::string,""}}};
  Hypercube hc(x);
  vector<string> expected{"x","y","z"};
  EXPECT_EQ(expected.size(), hc.dimLabels().size());
  for (size_t _i=0; _i<expected.size(); ++_i) EXPECT_EQ(expected[_i], hc.dimLabels()[_i]);
}

struct OuterFixture: public MinskyTensorOpSuite
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
  
TEST_F(OuterFixture, sparseOuterProduct)
{
  model->addWire(*x,*outer,1);
  model->addWire(*x,*outer,2);
  model->addWire(*outer,*z,1);
  reset();
  auto& zz=*z->vValue();
  EXPECT_EQ(2, zz.rank());
  vector<unsigned> expectedIndex={6,8,16,18};
  EXPECT_EQ(expectedIndex.size(), zz.size());
  for (size_t _i=0; _i<expectedIndex.size(); ++_i) EXPECT_EQ(expectedIndex[_i], zz.index()[_i]);
  vector<double> zValues(&zz[0], &zz[0]+zz.size());
  vector<double> expectedValues={1,3,3,9};
  EXPECT_EQ(expectedValues.size(), zz.size());
  for (size_t _i=0; _i<expectedValues.size(); ++_i) EXPECT_EQ(expectedValues[_i], zValues[_i]);
}
  


TEST_F(OuterFixture, sparse1OuterProduct)
{
  model->addWire(*y,*outer,1);
  model->addWire(*x,*outer,2);
  model->addWire(*outer,*z,1);
  reset();
  auto& zz=*z->vValue();
  EXPECT_EQ(2, zz.rank());
  vector<unsigned> expectedIndex={5,6,7,8,9,15,16,17,18,19};
  EXPECT_EQ(expectedIndex.size(), zz.size());
  for (size_t _i=0; _i<expectedIndex.size(); ++_i) EXPECT_EQ(expectedIndex[_i], zz.index()[_i]);
  vector<double> zValues(&zz[0], &zz[0]+zz.size());
  vector<double> expectedValues={0,1,2,3,4,0,3,6,9,12};
  EXPECT_EQ(expectedValues.size(), zz.size());
  for (size_t _i=0; _i<expectedValues.size(); ++_i) EXPECT_EQ(expectedValues[_i], zValues[_i]);
}
TEST_F(OuterFixture, sparse2OuterProduct)
{
  model->addWire(*x,*outer,1);
  model->addWire(*y,*outer,2);
  model->addWire(*outer,*z,1);
  reset();
  auto& zz=*z->vValue();
  EXPECT_EQ(2, zz.rank());
  vector<unsigned> expectedIndex={1,3,6,8,11,13,16,18,21,23};
  EXPECT_EQ(expectedIndex.size(), zz.size());
  for (size_t _i=0; _i<expectedIndex.size(); ++_i) EXPECT_EQ(expectedIndex[_i], zz.index()[_i]);
  vector<double> zValues(&zz[0], &zz[0]+zz.size());
  vector<double> expectedValues={0,0,1,3,2,6,3,9,4,12};
  EXPECT_EQ(expectedValues.size(), zz.size());
  for (size_t _i=0; _i<expectedValues.size(); ++_i) EXPECT_EQ(expectedValues[_i], zValues[_i]);
}

TEST_F(TensorOpSuite1,TensorVarValAssignment)
{
  auto ev=make_shared<EvalCommon>();
  double fv[100], sv[10];
  ev->update(fv,sizeof(fv)/sizeof(fv[0]),sv);
  auto startTimestamp=ev->timestamp();
  TensorVarVal tvv(to->vValue(),ev); 
  tvv=fromVal;
  EXPECT_EQ(fromVal.rank(), tvv.rank());
  for (size_t _i=0; _i<fromVal.rank(); ++_i) EXPECT_EQ(fromVal.shape()[_i], tvv.shape()[_i]);
  for (size_t _i=0; _i<fromVal.size(); ++_i) EXPECT_NEAR(fromVal[_i], tvv[_i], 1e-5);
     
  EXPECT_TRUE(ev->timestamp()>startTimestamp);
  EXPECT_EQ(fv,ev->flowVars());
  EXPECT_EQ(sv,ev->stockVars());
  EXPECT_EQ(sizeof(fv)/sizeof(fv[0]),ev->fvSize());
}

struct CorrelationSuite: public TensorOpSuite
{
  VariablePtr from1{VariableType::flow,"from1"};
  VariableValue& from1Val=*from1->vValue();
  CorrelationSuite() {
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
 
TEST_F(CorrelationSuite,covariance)
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
  EXPECT_EQ(cov.size(), toVal.size());
  for (size_t _i=0; _i<toVal.size(); ++_i) EXPECT_NEAR(cov[_i], toVal[_i], 1e-4);
}
 
TEST_F(CorrelationSuite,rho)
{
  // calculated on Octave (cov(x,y)./(std(x)'*std(y)))'
  vector<double> rho{
    -0.970729,   0.627315,  -0.888322,   0.877548,  -0.302967,
    -0.357572,   0.645244,  -0.442479,   0.279151,  -0.827059,
    -0.877474,   0.684865,  -0.622907,   0.980007,  -0.138368,
    0.274212,  -0.311870,  -0.191840,  -0.306059,  -0.598950,
    -0.397234,   0.080619,  -0.766631,  -0.013774,  -0.552663
  };

  OperationPtr rhoOp(OperationType::correlation);
  rhoOp->axis="1";
  g->addItem(rhoOp);
  Wire w1(from->ports(0),rhoOp->ports(1)), w2(from1->ports(0),rhoOp->ports(2)), w3(rhoOp->ports(0),to->ports(1));
  Eval(*to, rhoOp)();
     
  auto& toVal=*to->vValue();
  EXPECT_EQ(rho.size(), toVal.size());
  for (size_t _i=0; _i<toVal.size(); ++_i) EXPECT_NEAR(rho[_i], toVal[_i], 1e-4);
}

TEST_F(CorrelationSuite,selfCovariance)
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
  EXPECT_EQ(cov.size(), toVal.size());
  for (size_t _i=0; _i<toVal.size(); ++_i) EXPECT_NEAR(cov[_i], toVal[_i], 1e-4);
}

TEST_F(CorrelationSuite,nonConformantCovariance)
{
  from1Val.hypercube(vector<unsigned>{5,4});

  OperationPtr covOp(OperationType::covariance);
  covOp->axis="1";
  g->addItem(covOp);
  Wire w1(from->ports(0),covOp->ports(1)), w2(from1->ports(0),covOp->ports(2)), w3(covOp->ports(0),to->ports(1));
  EXPECT_THROW(Eval(*to, covOp)(), std::exception);
}
 
TEST_F(CorrelationSuite,dimensionNotFound)
{
  OperationPtr covOp(OperationType::covariance);
  covOp->axis="foo";
  g->addItem(covOp);
  Wire w1(from->ports(0),covOp->ports(1)), w2(from1->ports(0),covOp->ports(2)), w3(covOp->ports(0),to->ports(1));
  EXPECT_THROW(Eval(*to, covOp)(), std::exception);
}
 
TEST_F(CorrelationSuite,vectorCovariance)
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
  EXPECT_EQ(cov.size(), toVal.size());
  for (size_t _i=0; _i<toVal.size(); ++_i) EXPECT_NEAR(cov[_i], toVal[_i], 1e-4);
}

TEST_F(CorrelationSuite,lineLinearRegression)
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
  EXPECT_EQ(result.size(), toVal.size());
  for (size_t _i=0; _i<toVal.size(); ++_i) EXPECT_NEAR(result[_i], toVal[_i], 1e-4);
}

TEST_F(CorrelationSuite,xvectorValueLinearRegression)
{
  Hypercube hc;
  hc.xvectors.emplace_back("0", Dimension(Dimension::value,""), vector<civita::any>{0,1,2,3,4,5});
  TensorVal y(hc); y=vector<double>{1,2,3,4,5,6};
  fromVal=y;
     
  // line y=x+1
  vector<double> result={1,2,3,4,5,6};

  OperationPtr op(OperationType::linearRegression);
  g->addItem(op);
  Wire w1(from->ports(0),op->ports(1)), w3(op->ports(0),to->ports(1));
  Eval(*to, op)();
     
  auto& toVal=*to->vValue();
  EXPECT_EQ(result.size(), toVal.size());
  for (size_t _i=0; _i<toVal.size(); ++_i) EXPECT_NEAR(result[_i], toVal[_i], 1e-4);
}

TEST_F(CorrelationSuite,xvectorTimeLinearRegression)
{
  Hypercube hc;
  hc.xvectors.emplace_back("0", Dimension(Dimension::time,""),
                           vector<civita::any>{
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
  EXPECT_EQ(result.size(), toVal.size());
  // larger tolerance to allow for years not all being the same length
  for (size_t _i=0; _i<toVal.size(); ++_i) EXPECT_NEAR(result[_i], toVal[_i], 1e-1);
}

TEST_F(CorrelationSuite,xvectorStringLinearRegression)
{
  Hypercube hc;
  hc.xvectors.emplace_back("0", Dimension(Dimension::string,""), vector<civita::any>{"a","b","c","d","e","f"});
  TensorVal y(hc); y=vector<double>{1,2,3,4,5,6};
  fromVal=y;
     
  // line y=x+1
  vector<double> result={1,2,3,4,5,6};

  OperationPtr op(OperationType::linearRegression);
  g->addItem(op);
  Wire w1(from->ports(0),op->ports(1)), w3(op->ports(0),to->ports(1));
  Eval(*to, op)();
     
  auto& toVal=*to->vValue();
  EXPECT_EQ(result.size(), toVal.size());
  for (size_t _i=0; _i<toVal.size(); ++_i) EXPECT_NEAR(result[_i], toVal[_i], 1e-4);
}

TEST_F(CorrelationSuite,vectorLinearRegression)
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
  EXPECT_EQ(result.size(), toVal.size());
  for (size_t _i=0; _i<toVal.size(); ++_i) EXPECT_NEAR(result[_i], toVal[_i], 1e-4);
}

TEST_F(CorrelationSuite,matrixLinearRegression)
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
  EXPECT_EQ(result.size(), toVal.size());
  for (size_t _i=0; _i<toVal.size(); ++_i) EXPECT_NEAR(result[_i], toVal[_i], 1e-4);
  EXPECT_EQ(x.rank(), toVal.rank());
  for (size_t _i=0; _i<x.rank(); ++_i) EXPECT_EQ(x.hypercube().dims()[_i], toVal.hypercube().dims()[_i]);
}

TEST_F(CorrelationSuite,allMatrixLinearRegression)
{
  TensorVal x(vector<unsigned>{8,2}); x=vector<double>{0,0,1,1,2,2,3,3,0,0,1,1,2,2,3,3};
  TensorVal y(vector<unsigned>{8,2}); y=vector<double>{0,2,1,3,2,4,3,5,0,2,1,3,2,4,3,5};
  fromVal=y;
  from1Val=x;
     
  // lines y=x+1, y=2x+2
  vector<double> result{1,1,2,2,3,3,4,4,1,1,2,2,3,3,4,4};

  OperationPtr op(OperationType::linearRegression);
  op->axis="0";
  g->addItem(op);
  Wire w1(from->ports(0),op->ports(1)), w2(from1->ports(0),op->ports(2)), w3(op->ports(0),to->ports(1));
  Eval(*to, op)();
     
  auto& toVal=*to->vValue();
  EXPECT_EQ(result.size(), toVal.size());
  for (size_t _i=0; _i<toVal.size(); ++_i) EXPECT_NEAR(result[_i], toVal[_i], 1e-4);
  EXPECT_EQ(x.rank(), toVal.rank());
  for (size_t _i=0; _i<x.rank(); ++_i) EXPECT_EQ(x.hypercube().dims()[_i], toVal.hypercube().dims()[_i]);
}


 
