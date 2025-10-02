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
#include "minsky_epilogue.h"
#undef True
#include <gtest/gtest.h>
using namespace minsky;
using namespace std;
using MathDAG::differentiateName;

class BinOpFixture : public Minsky, public ::testing::Test
{
public:
  LocalMinsky lm{*this};
  VariablePtr offs{VariableType::constant};
  OperationPtr t{OperationType::time};
  OperationPtr plus{OperationType::add};
  OperationPtr tsq{OperationType::multiply};
  OperationPtr tcube{OperationType::multiply};
  //  OperationPtr minus{OperationType::subtract};
  //  OperationPtr pow{OperationType::pow};
  OperationPtr deriv{OperationType::differentiate};
  VariablePtr f{VariableType::flow,"f"}; // to receive results of function before differentiation
  VariablePtr df{VariableType::flow,"df"}; // to receive results of function after differentiation
  IntOp& integ;
  BinOpFixture(): 
    integ(dynamic_cast<IntOp&>
          (*(model->addItem(OperationPtr(OperationType::integrate)))))
  {
    dynamic_cast<VarConstant&>(*offs).init("0.1");
    model->addItem(offs);
    model->addItem(t);
    model->addItem(plus);
    model->addItem(tsq);
    model->addItem(tcube);
    model->addItem(deriv);
    model->addItem(f);
    model->addWire(new Wire(t->ports(0),plus->ports(1)));
    model->addWire(new Wire(offs->ports(0),plus->ports(2)));
    model->addWire(new Wire(plus->ports(0),tsq->ports(1)));
    model->addWire(new Wire(plus->ports(0),tsq->ports(2)));
    model->addWire(new Wire(plus->ports(0),tcube->ports(1)));
    model->addWire(new Wire(tsq->ports(0),tcube->ports(2)));
    model->addWire(new Wire(deriv->ports(0), integ.ports(1)));
    model->addItem(df);
    model->addWire(new Wire(deriv->ports(0), df->ports(1)));
    
    stepMin=1e-12;
    stepMax=1e-9;
    epsAbs=0.001;
    epsRel=0.01;
    order=4;

  }
};

class MinskyTest : public Minsky, public ::testing::Test
{
public:
  LocalMinsky lm{*this};
};

TEST(Derivative, differentiateName)
  {
    string dx=differentiateName("x");
    string d2x=differentiateName(dx);
    string d3x=differentiateName(d2x);
    EXPECT_EQ("dx/dt",dx);
    EXPECT_EQ("d^{2}x/dt^{2}",d2x);
    EXPECT_EQ("d^{3}x/dt^{3}",d3x);
    EXPECT_EQ("dd^nx/dt^n/dt",differentiateName("d^nx/dt^n"));
    // if user enters an invalid formula, such as \frac{d^2}{dt^3}x, then it should be processed as a name
    EXPECT_EQ("dd^2x/dt^3/dt",differentiateName("d^2x/dt^3"));
  }

  TEST_F(BinOpFixture,subtract)
  {
    OperationPtr minus{OperationType::subtract};
    model->addItem(minus);
    model->addWire(new Wire(t->ports(0),minus->ports(1)));
    model->addWire(new Wire(tsq->ports(0),minus->ports(2)));
    model->addWire(new Wire(minus->ports(0), deriv->ports(1)));
    model->addWire(new Wire(minus->ports(0), f->ports(1)));

    reset(); 
    running=true;
    nSteps=1;step(); // ensure f is evaluated
    // set the constant of integration to the value of f at t=0
    double f0=f->value();
    integ.intVar->value(f0);
    nSteps=1000; step();
    EXPECT_NEAR(1, f->value()/integ.intVar->value(), 0.003);
    EXPECT_TRUE(abs(f->value()-f0)>0.000001*f0); // checks that evolution of function value occurs
   
  }

  TEST_F(BinOpFixture,pow)
  {
    OperationPtr pow{OperationType::pow};
    model->addItem(pow);
    model->addWire(new Wire(plus->ports(0),pow->ports(1)));
    model->addWire(new Wire(pow->ports(0), deriv->ports(1)));
    model->addWire(new Wire(pow->ports(0), f->ports(1)));

    // firstly check when base port is unwired
    CHECK_THROW(reset(),std::exception);
    model->addWire(new Wire(tsq->ports(0),pow->ports(2)));

    reset();
    running=true;
    nSteps=1;step(); // ensure f is evaluated
    // set the constant of integration to the value of f at t=0
    double f0=f->value();
    integ.intVar->value(f0);
    nSteps=1000; step();step();
    EXPECT_NEAR(1, f->value()/integ.intVar->value(), 0.003);
    EXPECT_TRUE(abs(f->value()-f0)>0.0000001*f0); // checks that evolution of function value occurs
   
  }

  TEST_F(BinOpFixture,log)
  {
    OperationPtr log{OperationType::log};
    model->addItem(log);
    OperationPtr exp{OperationType::exp};
    model->addItem(exp);


    model->addWire(new Wire(plus->ports(0),exp->ports(1)));
    model->addWire(new Wire(exp->ports(0),log->ports(1)));
    model->addWire(new Wire(log->ports(0), deriv->ports(1)));
    model->addWire(new Wire(log->ports(0), f->ports(1)));

    // firstly check when base port is unwired 
    CHECK_THROW(reset(),std::exception);
    
    model->addWire(new Wire(tsq->ports(0),log->ports(2)));

    
    reset(); 
    running=true;
    nSteps=1;step(); // ensure f is evaluated
    // set the constant of integration to the value of f at t=0
    double f0=f->value();
    integ.intVar->value(f0);
    running=true;
    nSteps=1000; step();
    EXPECT_NEAR(1, f->value()/integ.intVar->value(), 0.003);
    EXPECT_TRUE(abs(f->value()-f0)>0.00001*f0); // checks that evolution of function value occurs
   
  }
  
  TEST_F(BinOpFixture,comparisonOps)
  {
    auto ops={OperationType::lt,OperationType::le,OperationType::eq};
    for (auto op: ops)
      {
        OperationPtr opp{op};
        model->addItem(opp);
        
        model->addWire(new Wire(t->ports(0),opp->ports(1)));
        model->addWire(new Wire(tsq->ports(0),opp->ports(2)));
        model->addWire(new Wire(opp->ports(0), deriv->ports(1)));
        model->addWire(new Wire(opp->ports(0), f->ports(1)));

        CHECK_THROW(reset(),error);
        model->deleteItem(*opp);
      }

    /// boolean operations have zero derivative
    auto ops2={OperationType::and_,OperationType::or_,OperationType::not_};
    
    for (auto op: ops2)
      {
        OperationPtr opp{op};
        model->addItem(opp);
       
        model->addWire(new Wire(t->ports(0),opp->ports(1)));
        if (opp->portsSize()>2)
          model->addWire(new Wire(tsq->ports(0),opp->ports(2)));
        model->addWire(new Wire(opp->ports(0), deriv->ports(1)));
        model->addWire(new Wire(opp->ports(0), f->ports(1)));

        reset(); 
        running=true;
        nSteps=1;step(); // ensure f is evaluated
        EXPECT_EQ(0, df->value());
        model->deleteItem(*opp);
      }

    auto ops3={OperationType::min,OperationType::max};
    for (auto op: ops3)
      {
        OperationPtr opp{op};
        model->addItem(opp);
        model->addWire(new Wire(opp->ports(0), deriv->ports(1)));
        model->addWire(new Wire(opp->ports(0), f->ports(1)));

        // no inputs should evaluate to zero
        reset(); 
        running=true;
        nSteps=1;step(); // ensure f is evaluated
        EXPECT_EQ(0, df->value());
        
        auto opWire=model->addWire(new Wire(t->ports(0),opp->ports(1)));

        
        // check first with single input wired
        save(OperationType::typeName(op)+".mky");
        reset(); 
        running=true;
        nSteps=1;step(); // ensure f is evaluated
        // set the constant of integration to the value of f at t=0
        double f0=f->value();
        integ.intVar->value(f0);
        running=true;
        nSteps=1000; step();
        EXPECT_NEAR(1, f->value()/integ.intVar->value(), 0.003);
        EXPECT_TRUE(abs(f->value()-f0)>0.00001*f0); // checks that evolution of function value occurs

        // check other single input wired
        model->removeWire(*opWire);
        model->addWire(new Wire(tsq->ports(0),opp->ports(2)));
        reset(); 
        running=true;
        nSteps=1;step(); // ensure f is evaluated
        // set the constant of integration to the value of f at t=0
        f0=f->value();
        integ.intVar->value(f0);
        running=true;
        nSteps=1000; step();
        EXPECT_NEAR(1, f->value()/integ.intVar->value(), 0.003);
        EXPECT_TRUE(abs(f->value()-f0)>0.00001*f0); // checks that evolution of function value occurs
       
        // now check with two inputs wired
        model->addWire(new Wire(t->ports(0),opp->ports(1)));
        reset(); 
        running=true;
        nSteps=1;step(); // ensure f is evaluated
        // set the constant of integration to the value of f at t=0
        f0=f->value();
        integ.intVar->value(f0);
        nSteps=1000; step();
        EXPECT_NEAR(1, f->value()/integ.intVar->value(), 0.003);
        EXPECT_TRUE(abs(f->value()-f0)>0.00001*f0); // checks that evolution of function value occurs

        model->deleteItem(*opp);
      }

  }

  TEST_F(BinOpFixture,singleArgFuncs)
    {
      // test functions
      OperationPtr funOp;
      for (int op=OperationType::integrate; op<OperationType::numOps; ++op)
        {
          //TODO for now, ignore tensor operations
          switch (OperationType::classify(OperationType::Type(op)))
            {
            case OperationType::reduction:
            case OperationType::scan:
            case OperationType::tensor:
            case OperationType::binop:
            case OperationType::constop:
            case OperationType::statistics:
              continue;
            default:
              break;
            }
          cout << OperationType::typeName(op) << endl;
          model->removeItem(*funOp);
          funOp.reset(OperationBase::create(OperationType::Type(op)));
          garbageCollect();
          model->addItem(funOp);
          model->addWire(new Wire(tcube->ports(0), funOp->ports(1)));
          model->addWire(new Wire(funOp->ports(0), f->ports(1)));
          model->addWire(new Wire(funOp->ports(0), deriv->ports(1)));
          save(OperationType::typeName(op)+".mky");
          switch (OperationType::Type(op))
            {
            case OperationType::floor: case OperationType::frac:
            case OperationType::index:
            case OperationType::data:
              CHECK_THROW(reset(), ecolab::error);
              continue;
            case OperationType::ravel:
            case OperationType::not_:
              continue; // test not meaningful for a step function
            default:
              reset(); 
            }
          running=true;
          nSteps=1;step(); // ensure f is evaluated
          // set the constant of integration to the value of f at t=0
          double f0=f->value();
          integ.intVar->value(f0);			  
          nSteps=1000; step();
          EXPECT_NEAR(1, f->value()/integ.intVar->value(), 0.003);
          EXPECT_TRUE(abs(f->value()-f0)>0.00000000001*f0); // checks that evolution of function value occurs
        }
    }
  
  TEST_F(MinskyTest,noInputs)
    {
      using namespace MathDAG;
      SystemOfEquations se(*this);
      for (auto op=0; op<OperationType::numOps; ++op)
        {
          NodePtr funOp{OperationDAGBase::create(OperationType::Type(op))};
          try
            {
              auto derivative=funOp->derivative(se);
              auto constant=dynamic_cast<ConstantDAG*>(derivative.get());
          
              if (constant)
                {
                  if (op==OperationType::time)
                    EXPECT_EQ("1", constant->value);
                  else
                    EXPECT_EQ("0", constant->value);
                }
            }
          catch (...) {} // ignore code that shouldn't be executed
        }
    }
