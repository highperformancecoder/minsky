/*
  @copyright Steve Keen 2014
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

#include <equations.h>
#include <minsky.h>
#include <ecolab_epilogue.h>
#include <UnitTest++/UnitTest++.h>
using namespace minsky;
using namespace std;
using MathDAG::differentiateName;

struct BinOpFixture: public Minsky
{
  LocalMinsky lm{*this};
  OperationPtr offs{OperationType::constant};
  OperationPtr t{OperationType::time};
  OperationPtr plus{OperationType::add};
  OperationPtr tsq{OperationType::multiply};
  OperationPtr minus{OperationType::subtract};
  OperationPtr pow{OperationType::pow};
  OperationPtr deriv{OperationType::differentiate};
  VariablePtr f; // to receive results of function before differentiation
  IntOp& integ;
  BinOpFixture(): 
    f(VariableType::flow,"f"),
    integ(dynamic_cast<IntOp&>
          (*(operations[getNewId()]=OperationPtr(OperationType::integrate))))
  {
    dynamic_cast<Constant&>(*offs).value=0.1;
    operations[getNewId()]=offs;
    operations[getNewId()]=t;
    operations[getNewId()]=plus;
    operations[getNewId()]=tsq;
    operations[getNewId()]=minus;
    operations[getNewId()]=pow;
    operations[getNewId()]=deriv;
    variables.addVariable(f);
    addWire(Wire(t->ports()[0],plus->ports()[1]));
    addWire(Wire(offs->ports()[0],plus->ports()[2]));
    addWire(Wire(plus->ports()[0],tsq->ports()[2]));
    addWire(Wire(deriv->ports()[0], integ.ports()[1]));
    variables.makeConsistent();

    stepMin=1e-6;
    stepMax=1e-3;
    epsAbs=0.001;
    epsRel=0.01;
    order=4;

  }
};

SUITE(Derivative)
{
  TEST(differentiateName)
  {
    string dx=differentiateName("x");
    string d2x=differentiateName(dx);
    string d3x=differentiateName(d2x);
    CHECK_EQUAL("dx/dt",dx);
    CHECK_EQUAL("d^{2}x/dt^{2}",d2x);
    CHECK_EQUAL("d^{3}x/dt^{3}",d3x);
    CHECK_EQUAL("dd^nx/dt^n/dt",differentiateName("d^nx/dt^n"));
    // if user enters an invalid formula, such as \frac{d^2}{dt^3}x, then it should be processed as a name
    CHECK_EQUAL("dd^2x/dt^3/dt",differentiateName("d^2x/dt^3"));
  }

  TEST_FIXTURE(BinOpFixture,subtract)
  {
    addWire(Wire(t->ports()[0],minus->ports()[1]));
    addWire(Wire(tsq->ports()[0],minus->ports()[2]));
    addWire(Wire(minus->ports()[0], deriv->ports()[1]));
    addWire(Wire(minus->ports()[0], f->ports()[1]));

    reset(); 
    nSteps=1;step(); // ensure f is evaluated
    // set the constant of integration to the value of f at t=0
    double f0=f->value();
    integ.getIntVar()->value(f0);
    nSteps=800; step();
    CHECK_CLOSE(1, f->value()/integ.getIntVar()->value(), 0.003);
    CHECK(abs(f->value()-f0)>0.1*f0); // checks that evolution of function value occurs
   
  }

  TEST_FIXTURE(BinOpFixture,pow)
  {
    addWire(Wire(plus->ports()[0],pow->ports()[1]));
    addWire(Wire(tsq->ports()[0],pow->ports()[2]));
    addWire(Wire(pow->ports()[0], deriv->ports()[1]));
    addWire(Wire(pow->ports()[0], f->ports()[1]));

    reset(); 
    nSteps=1;step(); // ensure f is evaluated
    // set the constant of integration to the value of f at t=0
    double f0=f->value();
    integ.getIntVar()->value(f0);
    nSteps=800; step();
    CHECK_CLOSE(1, f->value()/integ.getIntVar()->value(), 0.003);
    CHECK(abs(f->value()-f0)>0.1*f0); // checks that evolution of function value occurs
   
  }

  TEST_FIXTURE(BinOpFixture,log)
  {
    OperationPtr log{OperationType::log};
    operations[getNewId()]=log;
    OperationPtr exp{OperationType::exp};
    operations[getNewId()]=exp;


    addWire(Wire(plus->ports()[0],exp->ports()[1]));
    addWire(Wire(exp->ports()[0],log->ports()[1]));
    addWire(Wire(tsq->ports()[0],log->ports()[2]));
    addWire(Wire(log->ports()[0], deriv->ports()[1]));
    addWire(Wire(log->ports()[0], f->ports()[1]));

    reset(); 
    nSteps=1;step(); // ensure f is evaluated
    // set the constant of integration to the value of f at t=0
    double f0=f->value();
    integ.getIntVar()->value(f0);
    nSteps=800; step();
    CHECK_CLOSE(1, f->value()/integ.getIntVar()->value(), 0.003);
    CHECK(abs(f->value()-f0)>0.1*f0); // checks that evolution of function value occurs
   
  }
  
  TEST_FIXTURE(BinOpFixture,singleArgFuncs)
    {
      // test functions
      int funOp=getNewId();
      for (int op=OperationType::sqrt; op<OperationType::numOps; ++op)
        {
          cout << OperationType::typeName(op) << endl;
          OperationPtr fn{OperationType::Type(op)};
          operations[funOp]=fn;
          garbageCollect();
          addWire(Wire(plus->ports()[0], fn->ports()[1]));
          addWire(Wire(fn->ports()[0], f->ports()[1]));
          addWire(Wire(fn->ports()[0], deriv->ports()[1]));
          reset(); 
          nSteps=1;step(); // ensure f is evaluated
          // set the constant of integration to the value of f at t=0
          double f0=f->value();
          integ.getIntVar()->value(f0);
          nSteps=800; step();
          CHECK_CLOSE(1, f->value()/integ.getIntVar()->value(), 0.003);
          CHECK(abs(f->value()-f0)>0.1*f0); // checks that evolution of function value occurs
        }
    }
}
