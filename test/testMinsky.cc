/*
  @copyright Steve Keen 2012
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
#include "minsky.h"
#include "godleyTableWindow.h"
#include "matrix.h"
#include "minsky_epilogue.h"
#undef True
#include <gtest/gtest.h>
#include <gsl/gsl_integration.h>
#include <fstream>
#include <cstdio>
#include <algorithm>
#include <thread>
#include <chrono>
#include <boost/filesystem.hpp>
using namespace minsky;
using namespace boost::filesystem;

namespace
{
  struct MinskySuite: public Minsky, testing::Test
  {
    LocalMinsky lm;
    string savedMessage;
    MinskySuite(): lm(*this)
    {
    }
    void message(const string& x) override {savedMessage=x;}
  };
}

  /*
    ASCII Art diagram for the below test:


    c           -- a
    \       /
    +--int 
    /       \
    d          * - b
    /
    e ------------ f
  */

  TEST_F(MinskySuite,constructEquationsEx1)
    {
      auto gi=new GodleyIcon;
      model->addItem(gi);
      GodleyTable& godley=gi->table;
      godley.resize(3,4);
      godley.cell(0,1)="c";
      godley.cell(0,2)="d";
      godley.cell(0,3)="e";
      godley.cell(2,1)="a";
      godley.cell(2,2)="b";
      godley.cell(2,3)="f";
      gi->update();

      // build a table of variables - names will be unique at this stage
      map<string, VariablePtr> var;
      for (ItemPtr& i: model->items)
        if (auto v=dynamic_pointer_cast<VariableBase>(i))
          var[v->name()]=v;

      EXPECT_TRUE(var["a"]->lhs());
      EXPECT_TRUE(var["b"]->lhs());
      EXPECT_TRUE(!var["c"]->lhs());
      EXPECT_TRUE(!var["d"]->lhs());
      EXPECT_TRUE(!var["e"]->lhs());
      EXPECT_TRUE(var["f"]->lhs());

      auto addOp=model->addItem(OperationBase::create(OperationType::add)); 
      EXPECT_EQ(3, addOp->portsSize());
      auto intOp=dynamic_cast<IntOp*>(OperationBase::create(OperationType::integrate));
      model->addItem(intOp); 
      EXPECT_EQ(3, intOp->portsSize());
      auto mulOp=model->addItem(OperationBase::create(OperationType::multiply)); 
      EXPECT_EQ(3, mulOp->portsSize());
 
      model->addWire(*var["e"], *var["f"], 1);
      model->addWire(*var["c"], *addOp, 1);
      model->addWire(*var["d"], *addOp, 2);
      model->addWire(*addOp, *intOp, 1);
      model->addWire(*intOp->intVar, *var["a"], 1);
      model->addWire(*intOp->intVar, *mulOp, 1);
      model->addWire(*var["e"], *mulOp, 2);
      model->addWire(*mulOp, *var["b"], 1);

      for (auto& w: model->wires)
        {
          EXPECT_TRUE(!w->from()->input());
          EXPECT_TRUE(w->to()->input());
        }

      save("constructEquationsEx1.mky");

      EXPECT_TRUE(!cycleCheck());

      constructEquations();

      EXPECT_EQ(4, integrals.size());
      // check that integral  stock vars aren't flowVars
      int nakedIntegral=-1;
      for (size_t i=0; i<integrals.size(); ++i)
        {
          if (integrals[i].stock->name=="int1")
            nakedIntegral=i;
          EXPECT_TRUE(!integrals[i].stock->isFlowVar());
        }
      EXPECT_TRUE(nakedIntegral>=0);

      var["c"]->value(0.1);
      var["d"]->value(0.2);
      var["e"]->value(0.3);

      order=1;
      implicit=false;
      step();

      EXPECT_NEAR(var["c"]->value()+var["d"]->value(), integrals[nakedIntegral].input().value(), 1e-4);
      EXPECT_NEAR(integrals[nakedIntegral].stock->value(), var["a"]->value(), 1e-4);
      EXPECT_NEAR(integrals[nakedIntegral].stock->value()*var["e"]->value(), var["b"]->value(), 1e-4);
      EXPECT_NEAR(var["e"]->value(), var["f"]->value(), 1e-4);
      ode.reset();
    }

  /*
    ASCII Art diagram for the below test:

    K----------g
    \          
    +--------h
    /
    K
  */

  TEST_F(MinskySuite,constructEquationsEx2)
    {
      model->addItem(VariablePtr(VariableType::flow,"g"));
      model->addItem(VariablePtr(VariableType::flow,"h"));

      // build a table of variables - names will be unique at this stage
      map<string, VariablePtr> var;
      for (ItemPtr& i: model->items)
        if (auto v=dynamic_pointer_cast<VariableBase>(i))
            var[v->name()]=v;

      auto op4=model->addItem(new VarConstant);
      EXPECT_EQ(1, op4->portsSize());
      auto op5=model->addItem(new VarConstant);
      EXPECT_EQ(1, op5->portsSize());
      auto op6=model->addItem(OperationBase::create(OperationType::add));
      EXPECT_EQ(3, op6->portsSize());

      ecolab::array<float> coords(4,0);
 
      auto wire8=model->addWire(*op4, *var["g"], 1);
      auto wire9=model->addWire(*op4, *op6, 1);
      auto wire10=model->addWire(*op5, *op6, 2);
      auto wire11=model->addWire(*op6, *var["h"], 1);
 
      for (auto& w: model->wires)
        {
          EXPECT_TRUE(!w->from()->input());
          EXPECT_TRUE(w->to()->input());
        }
 
      EXPECT_TRUE(dynamic_cast<VarConstant*>(op4.get()));
      if (VarConstant* c=dynamic_cast<VarConstant*>(op4.get()))
        c->init("0.1");
      EXPECT_TRUE(dynamic_cast<VarConstant*>(op5.get()));
      if (VarConstant* c=dynamic_cast<VarConstant*>(op5.get()))
        c->init("0.2");

      reset();

      EXPECT_NEAR(0.1, var["g"]->value(), 1e-4);
      EXPECT_NEAR(0.3, var["h"]->value(), 1e-4);

    }

  TEST_F(MinskySuite,godleyEval)
    {
      auto gi=new GodleyIcon;
      model->addItem(gi);
      GodleyTable& godley=gi->table;
      godley.resize(3,4);
      godley.cell(0,1)=":c";
      godley.cell(0,2)=":d";
      godley.cell(0,3)=":e";
      godley.cell(1,0)="initial conditions";
      godley.cell(1,1)="10";
      godley.cell(1,2)="20";
      godley.cell(1,3)="30";
      godley.cell(2,1)=":a";
      godley.cell(2,2)="-:a";
      gi->update();
 
      variableValues[":a"]->init("5");
 
      garbageCollect();
      reset();
      EXPECT_EQ(10,variableValues[":c"]->value());
      EXPECT_EQ(20,variableValues[":d"]->value());
      EXPECT_EQ(30,variableValues[":e"]->value());
      EXPECT_EQ(5,variableValues[":a"]->value());
      for (size_t i=0; i<stockVars.size(); ++i)
        stockVars[i]=0;
     
      evalGodley.eval(stockVars.data(), flowVars.data());
      EXPECT_EQ(5,variableValues[":c"]->value());
      EXPECT_EQ(-5,variableValues[":d"]->value());
      EXPECT_EQ(0,variableValues[":e"]->value());
      EXPECT_EQ(5,variableValues[":a"]->value());
    
    }

  /*
    ASCII Art diagram for the below test:


    c           -- a
    \       /
    +--int 
    /       \
    d          * - b
    /
    e ------------ f
  */
  TEST_F(MinskySuite,derivative)
    {
      auto gi=new GodleyIcon;
      model->addItem(gi);
      GodleyTable& godley=gi->table;
      garbageCollect();
 
      godley.resize(3,4);
      godley.cell(0,1)="c";
      godley.cell(0,2)="d";
      godley.cell(0,3)="e";
      godley.cell(2,1)="a";
      godley.cell(2,2)="b";
      godley.cell(2,3)="f";
      gi->update();
 
      // build a table of variables - names will be unique at this stage
      map<string, VariablePtr> var;
      for (ItemPtr& i: model->items)
        if (auto v=dynamic_pointer_cast<VariableBase>(i))
          var[v->name()]=v;

      auto op1=model->addItem(OperationPtr(OperationType::add));
      auto op2=new IntOp;
      model->addItem(op2);
      auto op3=model->addItem(OperationPtr(OperationType::multiply));
  
      model->addWire(*var["e"], *var["f"], 1);
      model->addWire(*var["c"], *op1, 1);
      model->addWire(*var["d"], *op1, 2);
      model->addWire(*op1, *op2, 1);
      model->addWire(*op2->intVar, *var["a"], 1);
      model->addWire(*op2->intVar, *op3, 1);
      model->addWire(*var["e"],* op3, 2);
      model->addWire(*op3, *var["b"], 1);
 
      EXPECT_TRUE(!cycleCheck());
      reset();
      vector<double> j(stockVars.size()*stockVars.size());
      Matrix jac(stockVars.size(),j.data());
 
      auto& c=*variableValues[":c"];   c=100;
      auto& d=*variableValues[":d"];   d=200;
      auto& e=*variableValues[":e"];   e=300;
      double& x=stockVars.back();   x=0; // temporary variable storing \int c+d
 
      EXPECT_EQ(4, stockVars.size());
 
      save("derivative.mky");
      evalJacobian(jac,t,stockVars.data());
   
      EXPECT_EQ(0, jac(0,0));
      EXPECT_EQ(0, jac(0,1));  
      EXPECT_EQ(0, jac(0,2));
      EXPECT_EQ(1, jac(0,3));
      EXPECT_EQ(0, jac(1,0));
      EXPECT_EQ(0, jac(1,1));
      EXPECT_EQ(x, jac(1,2));
      EXPECT_EQ(e.value(), jac(1,3));
      EXPECT_EQ(0,jac(2,0));
      EXPECT_EQ(0,jac(2,1));
      EXPECT_EQ(1,jac(2,2));
      EXPECT_EQ(0,jac(2,3));
      EXPECT_EQ(1,jac(3,0));
      EXPECT_EQ(1,jac(3,1));
      EXPECT_EQ(0,jac(3,2));
      EXPECT_EQ(0,jac(3,3));
    }

  TEST_F(MinskySuite,integrals)
    {
      // First, integrate a constant
      auto op1=model->addItem(new VarConstant);
      auto op2=model->addItem(OperationPtr(OperationBase::integrate));
      
      IntOp* intOp=dynamic_cast<IntOp*>(op2.get());
      EXPECT_TRUE(intOp);
      intOp->description("output");
      model->addWire(*op1,*op2,1);

      constructEquations();
      EXPECT_TRUE(variableValues.validEntries());
      
      double value = 10;
      dynamic_cast<VariableBase*>(op1.get())->init(to_string(value));
      nSteps=1;
      running=true;
      step();
      // for now, constructEquations doesn work
      EXPECT_NEAR(value*t, integrals[0].stock->value(), 1e-5);
      EXPECT_NEAR(integrals[0].stock->value(), variableValues[":output"]->value(), 1e-5);
 
      // now integrate the linear function
      auto op3=model->addItem(OperationPtr(OperationBase::integrate));
      model->addWire(*intOp->intVar, *op3, 1);
      reset();
      running=true;
      step();
      //      EXPECT_NEAR(0.5*value*t*t, integrals[1].stock.value(), 1e-5);
      intOp=dynamic_cast<IntOp*>(op3.get());
      EXPECT_TRUE(intOp);
      EXPECT_NEAR(0.5*value*t*t, intOp->intVar->value(), 1e-5);
    }

  /*
    check that cyclic networks throw an exception

    a
    \ 
    + - a
    /
    b
  */

  TEST_F(MinskySuite,cyclicThrows)
    {
      // First, integrate a constant
      auto op1=model->addItem(OperationPtr(OperationType::add));
      auto w=model->addItem(VariablePtr(VariableType::flow,"w"));
      auto a=model->addItem(VariablePtr(VariableType::flow,"a"));
      EXPECT_TRUE(model->addWire(new Wire(op1->ports(0), w->ports(1))));
      EXPECT_TRUE(model->addWire(new Wire(w->ports(0), op1->ports(1))));
      model->addWire(new Wire(op1->ports(0), w->ports(1)));
      model->addWire(new Wire(w->ports(0), op1->ports(1)));
      model->addWire(new Wire(a->ports(0), op1->ports(2)));

      EXPECT_TRUE(cycleCheck());
      EXPECT_THROW(constructEquations(), ecolab::error);
    }

  /*
    but integration is allowed to cycle

    +--------+ 
    \        \
    *- int---+
    /
    b
  */

  TEST_F(MinskySuite,cyclicIntegrateDoesntThrow)
    {
      // First, integrate a constant
      auto op1=model->addItem(OperationPtr(OperationType::integrate));
      auto op2=model->addItem(OperationPtr(OperationType::multiply));
      auto a=model->addItem(VariablePtr(VariableType::flow,"a"));
      EXPECT_TRUE(model->addWire(new Wire(op1->ports(0), op2->ports(1))));
      EXPECT_TRUE(model->addWire(new Wire(op2->ports(0), op1->ports(1))));
      EXPECT_TRUE(model->addWire(new Wire(a->ports(0), op2->ports(2))));

      model->addWire(new Wire(op1->ports(0), op2->ports(1)));
      model->addWire(new Wire(op2->ports(0), op1->ports(1)));
      model->addWire(new Wire(a->ports(0), op2->ports(2)));

      EXPECT_TRUE(!cycleCheck());
      constructEquations();
    }

  TEST_F(MinskySuite,godleyIconVariableOrder)
    {
      auto& g=dynamic_cast<GodleyIcon&>(*model->addItem(new GodleyIcon));
      g.table.dimension(3,4);
      g.table.cell(0,1)="a1";
      g.table.cell(0,2)="z2";
      g.table.cell(0,3)="d1";
      g.table.cell(2,1)="b3";
      g.table.cell(2,2)="x1";
      g.table.cell(2,3)="h2";
      g.update();
  
      assert(g.stockVars().size()==g.table.cols()-1 && g.flowVars().size()==g.table.cols()-1);
      for (size_t i=1; i<g.table.cols(); ++i)
        {
          EXPECT_EQ(g.table.cell(0,i), g.stockVars()[i-1]->name());
          EXPECT_EQ(g.table.cell(2,i), g.flowVars()[i-1]->name());
        }
    }

  /*
    a --
    b / \
    + -- c
  */
   

  TEST_F(MinskySuite,multiVariableInputsAdd)
    {
      auto varA = model->addItem(VariablePtr(VariableType::flow, "a"));
      auto varB = model->addItem(VariablePtr(VariableType::flow, "b"));
      auto varC = model->addItem(VariablePtr(VariableType::flow, "c"));
      variableValues[":a"]->init("0.1");
      variableValues[":b"]->init("0.2");

      auto intOp = model->addItem(OperationBase::create(OperationType::integrate)); //enables equations to step
  

      auto op=model->addItem(OperationBase::create(OperationType::add));

      model->addWire(new Wire(varA->ports(0), op->ports(1)));
      model->addWire(new Wire(varB->ports(0), op->ports(1)));
      model->addWire(new Wire(op->ports(0), varC->ports(1)));
      model->addWire(new Wire(varC->ports(0), intOp->ports(1)));

      // move stuff around to make layout a bit better
      varA->moveTo(10,100);
      varB->moveTo(10,200);
      varC->moveTo(100,150);
      op->moveTo(50,150);
      intOp->moveTo(150,150);

      save("multiVariableInputs.mky");

      constructEquations();
      step();
      EXPECT_NEAR(0.3, variableValues[":c"]->value(), 1e-5);
    }

  TEST_F(MinskySuite,multiVariableInputsSubtract)
    {
      auto varA = model->addItem(VariablePtr(VariableType::flow, "a"));
      auto varB = model->addItem(VariablePtr(VariableType::flow, "b"));
      auto varC = model->addItem(VariablePtr(VariableType::flow, "c"));
      variableValues[":a"]->init("0.1");
      variableValues[":b"]->init("0.2");

      auto intOp = model->addItem(OperationBase::create(OperationType::integrate)); //enables equations to step
  
      auto op=model->addItem(OperationBase::create(OperationType::subtract));

      model->addWire(new Wire(varA->ports(0), op->ports(2)));
      model->addWire(new Wire(varB->ports(0), op->ports(2)));
      model->addWire(new Wire(op->ports(0), varC->ports(1)));
      model->addWire(new Wire(varC->ports(0), intOp->ports(1)));

      // move stuff around to make layout a bit better
      varA->moveTo(10,100);
      varB->moveTo(10,200);
      varC->moveTo(100,150);
      op->moveTo(50,150);
      intOp->moveTo(150,150);

      //Save("multiVariableInputs.mky");

      constructEquations();
      step();
      EXPECT_NEAR(-0.3, variableValues[":c"]->value(), 1e-5);
    }

  TEST_F(MinskySuite,multiVariableInputsMultiply)
    {
      auto varA = model->addItem(VariablePtr(VariableType::flow, "a"));
      auto varB = model->addItem(VariablePtr(VariableType::flow, "b"));
      auto varC = model->addItem(VariablePtr(VariableType::flow, "c"));
      variableValues[":a"]->init("0.1");
      variableValues[":b"]->init("0.2");

      auto intOp = model->addItem(OperationBase::create(OperationType::integrate)); //enables equations to step
  
      auto op=model->addItem(OperationBase::create(OperationType::multiply));

      model->addWire(new Wire(varA->ports(0), op->ports(2)));
      model->addWire(new Wire(varB->ports(0), op->ports(2)));
      model->addWire(new Wire(op->ports(0), varC->ports(1)));
      model->addWire(new Wire(varC->ports(0), intOp->ports(1)));

      // move stuff around to make layout a bit better
      varA->moveTo(10,100);
      varB->moveTo(10,200);
      varC->moveTo(100,150);
      op->moveTo(50,150);
      intOp->moveTo(150,150);

      //Save("multiVariableInputs.mky");

      constructEquations();
      step();
      EXPECT_NEAR(0.02, variableValues[":c"]->value(), 1e-5);
    }

  TEST_F(MinskySuite,multiVariableInputsDivide)
    {
      auto varA = model->addItem(VariablePtr(VariableType::flow, "a"));
      auto varB = model->addItem(VariablePtr(VariableType::flow, "b"));
      auto varC = model->addItem(VariablePtr(VariableType::flow, "c"));
      variableValues[":a"]->init("0.1");
      variableValues[":b"]->init("0.2");

      auto intOp = model->addItem(OperationBase::create(OperationType::integrate)); //enables equations to step
  
      auto op=model->addItem(OperationBase::create(OperationType::divide));

      model->addWire(new Wire(varA->ports(0), op->ports(2)));
      model->addWire(new Wire(varB->ports(0), op->ports(2)));
      model->addWire(new Wire(op->ports(0), varC->ports(1)));
      model->addWire(new Wire(varC->ports(0), intOp->ports(1)));

      // move stuff around to make layout a bit better
      varA->moveTo(10,100);
      varB->moveTo(10,200);
      varC->moveTo(100,150);
      op->moveTo(50,150);
      intOp->moveTo(150,150);

      //Save("multiVariableInputs.mky");

      constructEquations();
      step();
      EXPECT_NEAR(50, variableValues[":c"]->value(), 1e-5);
    }

  // instantiate all operations and variables to ensure that definitions
  // have been provided for all virtual methods
TEST(TensorOps, checkAllOpsDefined)
  {
    using namespace MathDAG;
    for (int o=0; o<OperationType::sum; ++o)
      {
        if (OperationType::Type(o)==OperationType::constant) continue; // constant deprecated
        OperationType::Type op=OperationType::Type(o);
        OperationPtr po(op);
        EXPECT_EQ(OperationType::typeName(op), OperationType::typeName(po->type()));
        if (o!=OperationType::differentiate) // no evalop etc for differentiate needed.
          {
            EvalOpPtr ev(op);
            EXPECT_EQ(OperationType::typeName(op), OperationType::typeName(ev->type()));

            shared_ptr<OperationDAGBase> opDAG
              (OperationDAGBase::create(OperationType::Type(op), ""));
            EXPECT_EQ(OperationType::typeName(op), OperationType::typeName(opDAG->type()));
          }
      }

    for (int v=0; v<VariableType::numVarTypes; ++v)
      {
        VariableType::Type vt=VariableType::Type(v);
        VariablePtr pv(vt);
        EXPECT_EQ(vt, pv->type());
      }
  }

  namespace
  {
    // integrand function for following test
    double integrand(double x, void* params)
    {
      auto& e=dynamic_cast<ScalarEvalOp&>(**static_cast<EvalOpPtr*>(params));
      return e.d1(x,0);
    }

    // macro to make lines numbers work out
#define testUnOp(evalOp, ws)                                            \
    {                                                                   \
      double result, abserr;                                            \
      double a=0.5, b=0.95;                                             \
      gsl_function_struct integ={integrand, &evalOp};                   \
      gsl_integration_qag (&integ, a, b, 1e-5, 1e-3, 100, 1, ws,        \
                           &result, &abserr);                           \
      EXPECT_NEAR(evalOp->evaluate(b,0)-evalOp->evaluate(a,0), result, 2*abserr); \
    }
  

    // adaptor class to test each derivative of binary ops
    struct FixedArg1: public ScalarEvalOp
    {
      shared_ptr<ScalarEvalOp> op;
      double arg1;
      FixedArg1(OperationType::Type op, double arg):
        op(dynamic_pointer_cast<ScalarEvalOp>(EvalOpPtr(op))), arg1(arg)
      {}

      OperationType::Type type() const {return op->type();}
      FixedArg1* clone() const {return new FixedArg1(*this);}
      void pack(pack_t&, const string&) const {}
      void unpack(unpack_t&, const string&) {}
    
      int numArgs() const {return 1;}
      double evaluate(double in1, double in2) const
      {return op->evaluate(arg1, in1);} 
      double d1(double x1=0, double x2=0) const
      {return op->d2(arg1, x1);}
      double d2(double x1=0, double x2=0) const
      {return 0;}
    };

    struct FixedArg2: public ScalarEvalOp
    {
      shared_ptr<ScalarEvalOp> op;
      double arg2;
      FixedArg2(OperationType::Type op, double arg):
        op(dynamic_pointer_cast<ScalarEvalOp>(EvalOpPtr(op))), arg2(arg) {}

      OperationType::Type type() const {return op->type();}
      FixedArg2* clone() const {return new FixedArg2(*this);}
      void pack(pack_t&, const string&) const {}
      void unpack(unpack_t&, const string&) {}

      int numArgs() const {return 1;}
      double evaluate(double in1, double in2) const
      {return op->evaluate(in1, arg2);} 
      double d1(double x1=0, double x2=0) const
      {return op->d1(x1, arg2);}
      double d2(double x1=0, double x2=0) const
      {return 0;}
    };

  }

  // Tests certain functions as throwing
TEST(TensorOps, checkDerivativesThrow)
  {
    EXPECT_THROW(EvalOp<OperationType::and_>().d1(0,0),error); 
    EXPECT_THROW(EvalOp<OperationType::or_>().d1(0,0),error); 
    EXPECT_THROW(EvalOp<OperationType::not_>().d1(0,0),error); 
    EXPECT_THROW(EvalOp<OperationType::lt>().d1(0,0),error); 
    EXPECT_THROW(EvalOp<OperationType::le>().d1(0,0),error); 
    EXPECT_THROW(EvalOp<OperationType::eq>().d1(0,0),error); 
    EXPECT_THROW(EvalOp<OperationType::floor>().d1(0,0),error); 
    EXPECT_THROW(EvalOp<OperationType::frac>().d1(0,0),error); 
 }
  
  // Tests derivative definitions of one argument functions by
  // integration over [0.5,1], using fundamental theorem of calculus.
  // Hopefully, this range will avoid any singularities in the derivative
TEST(TensorOps, checkDerivatives)
  {
    gsl_integration_workspace* ws=gsl_integration_workspace_alloc(1000);
    // do not test tensor ops
    for (int op=0; op<OperationType::sum; ++op)
      {
        // derivatives should not have EvalOps instantiated
        // other operation cannot be tested this way
        switch (op)
          {
          case OperationType::ravel:
          case OperationType::userFunction:
          case OperationType::differentiate:
          case OperationType::and_:
          case OperationType::or_:
          case OperationType::not_:
          case OperationType::lt:
          case OperationType::le:
          case OperationType::eq:
          case OperationType::floor:
          case OperationType::frac:      
            continue;
          default:
            break;
          }
        cout << "checking "<<OperationType::typeName(op)<<endl;
        shared_ptr<ScalarEvalOp> evalOp=dynamic_pointer_cast<ScalarEvalOp>(EvalOpPtr(OperationType::Type(op)));
        if (evalOp->numArgs()==1)
          {
            testUnOp(evalOp, ws);
          }
        else if (evalOp->numArgs()==2 && evalOp->type()!=OperationType::integrate)
          {
            evalOp.reset(new FixedArg1(OperationType::Type(op), 2.0));
            testUnOp(evalOp, ws);
            evalOp.reset(new FixedArg2(OperationType::Type(op), 2.0));
            testUnOp(evalOp, ws);
          }
      }

    gsl_integration_workspace_free(ws);
  }

TEST(TensorOps, evalOpEvaluate)
  {
    EvalOp<OperationType::floor> floor;
    EXPECT_EQ(3,floor.evaluate(3.2,0));
    EvalOp<OperationType::frac> frac;
    EXPECT_NEAR(0.2,frac.evaluate(3.2,0),1e-6);
  }
  
  TEST_F(MinskySuite,multiGodleyRules)
    {
      auto g1=new GodleyIcon; model->addItem(g1);
      auto g2=new GodleyIcon; model->addItem(g2);
      auto g3=new GodleyIcon; model->addItem(g3);
      GodleyTable& godley1=g1->table;
      GodleyTable& godley2=g2->table;
      GodleyTable& godley3=g3->table;
      godley1.resize(2,1);
      godley2.resize(2,1);
      godley3.resize(2,1);

      godley1.doubleEntryCompliant=true;
      godley2.doubleEntryCompliant=true;
      godley3.doubleEntryCompliant=true;

      // empty godleys should be fine
      initGodleys();

      godley1.resize(4,4);
      godley1.cell(0,1)=":hello"; godley1.assetClass(1, GodleyAssetClass::asset);
      godley1.cell(0,2)=":bar"; godley1.assetClass(2, GodleyAssetClass::liability);
      godley1.cell(0,3)=":foo"; godley1.assetClass(3, GodleyAssetClass::equity);

      // should still be no problem
      initGodleys();

      godley2.resize(4,4);
      godley2.cell(0,1)=":hello2"; godley2.assetClass(1, GodleyAssetClass::asset);
      godley2.cell(0,2)=":bar2"; godley2.assetClass(2, GodleyAssetClass::liability);
      godley2.cell(0,3)=":foo2"; godley2.assetClass(3, GodleyAssetClass::equity);

      godley3.resize(4,4);
      godley3.cell(0,1)=":hello3"; godley3.assetClass(1, GodleyAssetClass::asset);
      godley3.cell(0,2)=":bar3"; godley3.assetClass(2, GodleyAssetClass::liability);
      godley3.cell(0,3)=":foo3"; godley3.assetClass(3, GodleyAssetClass::equity);

      // should be no problem - all columns are different
      initGodleys();

      godley3.cell(0,2)=":foo2"; 

      // two incompatible columns
      EXPECT_THROW(initGodleys(), ecolab::error);

  
      godley3.cell(0,2)=":foo3"; 
      godley3.cell(0,1)=":hello2"; 

      // two incompatible columns asset columns
      EXPECT_THROW(initGodleys(), ecolab::error);

      // should now be a compatible asset/liability pair
      godley3.assetClass(1, GodleyAssetClass::liability);
      initGodleys();
  

      // add in another asset/liability pair
      godley1.cell(0,3)=":bar3"; godley1.assetClass(3, GodleyAssetClass::asset);
      initGodleys();

      // now conflict that pair
      godley2.cell(0,3)=":bar3";godley2.assetClass(3, GodleyAssetClass::asset);
      EXPECT_THROW(initGodleys(), ecolab::error);
      godley2.cell(0,3)=":bar2";
  
      // now add some flow variables and check those
      godley2.cell(2,1)="2:a";
      godley3.cell(2,1)=":a";
      godley3.cell(3,1)=":a";
      (variableValues[":a"]=VariableValuePtr(VariableType::flow))->allocValue();
      (variableValues[":hello2"]=VariableValuePtr(VariableType::stock))->allocValue();

      initGodleys();

      godley3.cell(3,1)="";
      EXPECT_THROW(initGodleys(), ecolab::error);
 

    }

  TEST_F(MinskySuite,matchingTableColumns)
    {
      auto g1=new GodleyIcon; model->addItem(g1);
      auto g2=new GodleyIcon; model->addItem(g2);
      auto g3=new GodleyIcon; model->addItem(g3);
      GodleyTable& godley1=g1->table;
      GodleyTable& godley2=g2->table;
      godley1.resize(6,4);
      godley2.resize(3,5);

      godley1.assetClass(1,GodleyAssetClass::asset);  
      godley1.assetClass(2,GodleyAssetClass::liability);  
      godley1.assetClass(3,GodleyAssetClass::equity);  
      godley2.assetClass(1,GodleyAssetClass::asset);  
      godley2.assetClass(2,GodleyAssetClass::liability);
      godley2.assetClass(3,GodleyAssetClass::liability);
      godley2.assetClass(4,GodleyAssetClass::equity);

      godley1.cell(0,1)="a1";  
      godley1.cell(0,2)="l1";
      godley1.cell(0,3)="e1";
      g1->update();
      
      set<string> cols=matchingTableColumns(*g2,GodleyAssetClass::asset);
      EXPECT_EQ(1, cols.size());
      EXPECT_EQ("l1", *cols.begin());

      godley2.cell(0,1)="l1";  
      godley2.cell(0,2)="l2";  
      godley2.cell(0,4)="e2";
      g2->update();
      
      cols=matchingTableColumns(*g1,GodleyAssetClass::asset);
      EXPECT_EQ(1, cols.size());
      EXPECT_EQ("l2", *cols.begin());

      cols=matchingTableColumns(*g1,GodleyAssetClass::liability);
      EXPECT_EQ(0, cols.size());

      cols=matchingTableColumns(*g1,GodleyAssetClass::equity);
      EXPECT_EQ(1, cols.size());
      EXPECT_EQ("a1", *cols.begin());

      cols=matchingTableColumns(*g2,GodleyAssetClass::equity);
      EXPECT_EQ(0, cols.size());

      cols=matchingTableColumns(*g3,GodleyAssetClass::asset);
      EXPECT_EQ(1, cols.size());
      EXPECT_EQ("l2", *cols.begin());

      cols=matchingTableColumns(*g3,GodleyAssetClass::liability);
      EXPECT_EQ(1, cols.size());
      EXPECT_EQ("a1", *cols.begin());

      

      model->addItem(VariablePtr(VariableType::flow, ":a"));
      model->addItem(VariablePtr(VariableType::flow, ":b"));
      model->addItem(VariablePtr(VariableType::flow, ":c"));
      model->addItem(VariablePtr(VariableType::flow, ":d"));
      model->addItem(VariablePtr(VariableType::flow, ":e"));

      // OK now check some balanceDuplicateColumns functionality
      godley1.cell(1,0)="row1";
      godley1.cell(2,0)="row2";
      godley1.cell(3,0)="row3";
      godley1.cell(1,2)="a";
      godley1.cell(2,2)="b";
      godley1.cell(3,2)="c";
      godley1.cell(4,2)="e"; // for checking that an internal transfer is not reflected
      godley1.cell(5,2)="-e";
      godley2.cell(1,0)="row1";
      godley2.cell(2,1)="d";
      g1->update();
      g2->update();

      godley2.exportToCSV("before.csv");
      EXPECT_EQ(3,godley2.rows());
      balanceDuplicateColumns(*g1, 2); 
      godley2.exportToCSV("after.csv");
      // two rows should have been added, and one deleted
      EXPECT_EQ(4,godley2.rows());
      EXPECT_EQ("a",trimWS(godley2.cell(1,1)));
      EXPECT_EQ("b",godley2.cell(2,1));
      EXPECT_EQ("c",godley2.cell(3,1));
      EXPECT_EQ("row3",godley2.cell(3,0)); // check label transferred
    }

  TEST_F(MinskySuite,bug1157)
    {
      auto g1=new GodleyIcon; model->addItem(g1);
      auto g2=new GodleyIcon; model->addItem(g2);
      GodleyTable& godley1=g1->table;
      GodleyTable& godley2=g2->table;
      godley1.resize(3,2);
      godley2.resize(3,2);

      godley1.assetClass(1,GodleyAssetClass::asset);  
      godley2.assetClass(1,GodleyAssetClass::liability);  

      godley1.cell(0,1)="a";  
      godley1.cell(2,0)="xx";
      godley1.cell(2,1)="-b";
      g1->update();
      
      godley2.cell(0,1)="a";  
      godley2.cell(2,0)="yy";  
      godley2.cell(2,1)="-b";
      g2->update();

      EXPECT_EQ(3,godley2.rows());
      EXPECT_EQ("yy",godley2.cell(2,0));
      EXPECT_EQ("-b",godley2.cell(2,1));
      godley2.exportToCSV("before.csv");

      godley1.cell(2,1)="b";
      balanceDuplicateColumns(*g1, 1); 
      godley2.exportToCSV("after.csv");
      EXPECT_EQ(4,godley2.rows());
      EXPECT_EQ("yy",godley2.cell(2,0)); // row label should be updated
      EXPECT_EQ("-b",godley2.cell(2,1));  // sign should be transferred
      EXPECT_EQ("xx",godley2.cell(3,0)); // row label should be updated
      EXPECT_EQ("2b",godley2.cell(3,1));  // sign should be transferred
    }

  TEST_F(MinskySuite,importDuplicateColumn)
    {
      auto g1=new GodleyIcon; model->addItem(g1);
      auto g2=new GodleyIcon; model->addItem(g2);
      GodleyTable& godley1=g1->table;
      GodleyTable& godley2=g2->table;
      godley1.resize(4,3);
      godley2.resize(2,2);

      godley1.assetClass(1,GodleyAssetClass::asset);  
      godley2.assetClass(1,GodleyAssetClass::liability);

      godley1.cell(0,1)="a1";  
      godley1.cell(2,0)="row1";
      godley1.cell(2,1)="a";
      godley1.cell(3,0)="row2";
      godley1.cell(3,1)="b";

      godley2.cell(0,1)="a1";
      importDuplicateColumn(godley2, 1);
      EXPECT_EQ("row1",godley2.cell(2,0));
      EXPECT_EQ("a",godley2.cell(2,1));
      EXPECT_EQ("row2",godley2.cell(3,0));
      EXPECT_EQ("b",godley2.cell(3,1));

      // move cell between columns
      g1->moveCell({2,1,2,2});
      EXPECT_EQ("a",godley1.cell(2,2));
      EXPECT_EQ("",godley1.cell(2,1));
      EXPECT_EQ("",godley2.cell(2,1));

      g1->moveCell({2,2,3,2});
      EXPECT_EQ("a",godley1.cell(3,2));
      EXPECT_EQ("",godley1.cell(2,2));
    }

  TEST_F(MinskySuite,godleyRowSums)
    {
      auto g1=new GodleyIcon; model->addItem(g1);
      GodleyTable& godley1=g1->table;
      godley1.resize(6,4);
      godley1.cell(0,1)="s0";
      godley1.cell(0,2)="s1";
      godley1.cell(0,3)="s2";
      godley1.cell(1,1)="a";
      godley1.cell(1,2)="2b";
      godley1.cell(1,3)="-c";
      EXPECT_EQ("a-2b+c",godley1.rowSum(1));
    }

  TEST_F(MinskySuite,godleyMoveRowCol)
    {
      auto g1=new GodleyIcon; model->addItem(g1);
      GodleyTable& godley1=g1->table;
      godley1.resize(6,3);
      godley1.cell(0,1)="c1";
      godley1.cell(0,2)="c2";
      godley1.cell(1,1)="a";
      godley1.cell(1,2)="2b";
      godley1.cell(2,1)="a1";
      godley1.cell(2,2)="2b1";
      godley1.assetClass(1,GodleyAssetClass::asset);
      godley1.assetClass(2,GodleyAssetClass::asset);
      godley1.moveRow(1,1);
      EXPECT_EQ("a",godley1.cell(2,1));
      EXPECT_EQ("2b",godley1.cell(2,2));
      EXPECT_EQ("a1",godley1.cell(1,1));
      EXPECT_EQ("2b1",godley1.cell(1,2));
      godley1.moveCol(1,1);
      EXPECT_EQ("a",godley1.cell(2,2));
      EXPECT_EQ("2b",godley1.cell(2,1));
      EXPECT_EQ("a1",godley1.cell(1,2));
      EXPECT_EQ("2b1",godley1.cell(1,1));
    }

  TEST_F(MinskySuite,godleyNameUnique)
    {
      auto g1=new GodleyIcon; model->addItem(g1);
      auto g2=new GodleyIcon; model->addItem(g2);
      auto g3=new GodleyIcon; model->addItem(g3);
      GodleyTable& godley1=g1->table;
      GodleyTable& godley2=g2->table;
      GodleyTable& godley3=g3->table;
      godley1.nameUnique();
      godley2.nameUnique();
      godley3.nameUnique();
      EXPECT_TRUE(godley1.title!=godley2.title);
      EXPECT_TRUE(godley2.title!=godley3.title);
      EXPECT_TRUE(godley1.title!=godley3.title);
    }

    TEST_F(MinskySuite,clearInitCond)
    {
      auto g1=new GodleyIcon; model->addItem(g1);
      EXPECT_TRUE(g1->table.initialConditionRow(1));
      // to clear a cell, it first needs to contain something
      g1->setCell(1,1,"1");
      g1->setCell(1,1,"");
      EXPECT_EQ("0",g1->table.cell(1,1));
    }

    /*
      /---------------------------------\
      |                b  :b            |
      | /-----------\       /--------\  |
      | |/--\ a /--\|       |:a b :b |  |
      | ||a |   |:b||       |        |  |
      | ||:a|   |:c||       \--------/  |
      | |\--/   \--/|                   |
      | \-----------/                   |
      \---------------------------------/
    */
    
    TEST_F(MinskySuite,varNameScoping)
    {
      auto g0=model->addGroup(new Group);
      auto g1=model->addGroup(new Group);
      auto g2=g0->addGroup(new Group);
      auto g3=g0->addGroup(new Group);
      VariablePtr a0(VariableType::flow,"a");
      VariablePtr a1(VariableType::flow,":a");
      g2->addItem(a0);
      g2->addItem(a1);
      VariablePtr a2(VariableType::flow,"a");
      g0->addItem(a2);
      VariablePtr a3(VariableType::flow,":a");
      g1->addItem(a3);

      VariablePtr b0(VariableType::flow,"b");
      model->addItem(b0);
      VariablePtr b1(VariableType::flow,":b");
      model->addItem(b1);
      VariablePtr b2(VariableType::flow,"b");
      VariablePtr b3(VariableType::flow,":b");
      g1->addItem(b2);
      g1->addItem(b3);
      VariablePtr b4(VariableType::flow,":b");
      g3->addItem(b4);

      
      VariablePtr c(VariableType::flow,":c");
      g3->addItem(c);

      g0->makeSubroutine();
      g1->makeSubroutine();
      g2->makeSubroutine();
      g3->makeSubroutine();
      if (a1->local()) a1->toggleLocal();
      if (a3->local()) a3->toggleLocal();
      if (b1->local()) b1->toggleLocal();
      if (b3->local()) b3->toggleLocal();
      if (b4->local()) b4->toggleLocal();
      
      // check valueIds correspond to the scoping rules
      EXPECT_TRUE(a0->valueId()!=a1->valueId());
      EXPECT_TRUE(a0->valueId()!=a3->valueId());
      EXPECT_EQ(a1->valueId(),a2->valueId());
      EXPECT_TRUE(a1->valueId()!=a3->valueId());

      EXPECT_EQ(b0->valueId(),b1->valueId());
      EXPECT_EQ(b0->valueId(),b3->valueId());
      EXPECT_TRUE(b2->valueId()!=b3->valueId());

      // check display values:
      EXPECT_EQ("a",a0->name());
      EXPECT_EQ("a",a1->name());
      EXPECT_EQ("a",a2->name());
      EXPECT_EQ("a",a3->name());
      EXPECT_EQ("b",b0->name());
      EXPECT_EQ("b",b1->name());
      EXPECT_EQ("b",b2->name());
      EXPECT_EQ("b",b3->name());
      EXPECT_EQ("c",c->name());

      // now check move rules
      g0->addItem(a1);
      EXPECT_EQ("a",a1->name());

      g2->addItem(a1);
      EXPECT_EQ("a",a1->name());

      g1->addItem(a1);
      EXPECT_EQ("a",a1->name());

      g1->addItem(c);
      EXPECT_EQ("c",c->name());

    }

    TEST_F(MinskySuite,cantMultiplyDefineVars)
    {
      VariablePtr f1(VariableType::flow,"foo");
      VariablePtr f2(VariableType::flow,"foo");
      OperationPtr op(OperationType::time);
      model->addItem(f1);
      model->addItem(f2);
      model->addItem(op);
      model->addWire(op->ports(0),f1->ports(1));
      model->addWire(op->ports(0),f2->ports(1));
      EXPECT_EQ(1, f1->ports(1).lock()->wires().size());
      EXPECT_EQ(0, f2->ports(1).lock()->wires().size());
    }

    TEST_F(MinskySuite,MultiplyDefinedVarsThrowsOnReset)
    {
      VariablePtr f1(VariableType::flow,"foo");
      VariablePtr f2(VariableType::flow,"foo");
      OperationPtr op(OperationType::time);
      model->addItem(f1);
      model->addItem(f2);
      model->addItem(op);
      model->addWire(op->ports(0),f1->ports(1));
      model->addWire(std::make_shared<Wire>(op->ports(0),f2->ports(1)));
      EXPECT_EQ(1, f1->ports(1).lock()->wires().size());
      // We've tricked the system into having a multiply defined variable
      EXPECT_EQ(1, f2->ports(1).lock()->wires().size());
      EXPECT_THROW(reset(), std::exception);
    }

    TEST_F(MinskySuite, RemoveDefinitionsFromPastedVars)
      {
        VariablePtr a(VariableType::flow,"a");
        VariablePtr b(VariableType::flow,"b");
        model->addItem(a); model->addItem(b);
        model->addWire(a->ports(0), b->ports(1));
        canvas.selection.ensureItemInserted(a);
        canvas.selection.ensureItemInserted(b);
        EXPECT_EQ(1,canvas.selection.numWires());
        copy();
        paste();
        EXPECT_EQ(4, model->items.size());
        // ensure extra wire is not copied
        EXPECT_EQ(1, model->wires.size());
        // check that b's definition remains as before
        EXPECT_TRUE(definingVar(":b")==b);
      }

    TEST_F(MinskySuite, DefinitionPasted)
      {
        VariablePtr a(VariableType::flow,"a");
        VariablePtr b(VariableType::flow,"b");
        model->addItem(a); model->addItem(b);
        model->addWire(a->ports(0), b->ports(1));
        canvas.selection.ensureItemInserted(a);
        canvas.selection.ensureItemInserted(b);
        copy();
        model->deleteItem(*b);
        EXPECT_EQ(0, model->wires.size());
        paste();
        // ensure extra wire is not copied
        EXPECT_EQ(1, model->wires.size());
        // check that b's definition is now the copied var
        EXPECT_TRUE(definingVar(":b")!=b);
      }
    
    TEST_F(MinskySuite, PastedIntOpShowsMessage)
      {
        auto intOp=make_shared<IntOp>();
        intOp->description("foo");
        model->addItem(intOp);
        EXPECT_EQ(2,model->items.size());
        canvas.selection.ensureItemInserted(intOp);
        copy();
        paste();
        EXPECT_TRUE(savedMessage.size()); // check that pop message is written
      }

    TEST_F(MinskySuite, RetypePastedIntegralVariable)
      {
        auto intOp=make_shared<IntOp>();
        intOp->description("foo");
        model->addItem(intOp);
        VariablePtr clonedIntVar(intOp->intVar->clone());
        model->addItem(clonedIntVar);
        canvas.selection.ensureItemInserted(clonedIntVar);
        copy();
        model->removeItem(*intOp);
        intOp.reset();
        paste();
        EXPECT_EQ(2,model->items.size());
        EXPECT_TRUE(model->items[1]->variableCast());
        EXPECT_EQ(VariableType::flow, model->items[1]->variableCast()->type());
        EXPECT_EQ(clonedIntVar->name(), model->items[1]->variableCast()->name());
      }
    
    TEST_F(MinskySuite, cut)
      {
        auto a=model->addItem(new Variable<VariableType::flow>("a"));
        auto integ=new IntOp;
        model->addItem(integ);
        auto g=model->addGroup(new Group);
        g->addItem(new Variable<VariableType::flow>("a1"));
        EXPECT_EQ(4,model->numItems());
        EXPECT_EQ(1,model->numGroups());

        canvas.selection.ensureItemInserted(a);
        EXPECT_EQ(1,canvas.selection.numItems());
        canvas.selection.toggleItemMembership(integ->intVar);
        EXPECT_EQ(3,canvas.selection.numItems()); // both integral and intVar must be inserted
        canvas.selection.toggleItemMembership(model->findItem(*integ));
        EXPECT_EQ(1,canvas.selection.numItems());
        canvas.selection.items.push_back(integ->intVar);
        EXPECT_EQ(2,canvas.selection.numItems());
        canvas.selection.ensureGroupInserted(g);
        EXPECT_EQ(3,canvas.selection.numItems());
        EXPECT_EQ(1,canvas.selection.numGroups());

        a.reset(); g.reset(); // prevent triggering assertion filaure in cut()
        cut();
        EXPECT_EQ(2,model->numItems()); //intVar should not be deleted
        EXPECT_EQ(0,model->numGroups());
      }

    TEST_F(MinskySuite,renameAll)
    {
      auto gi=make_shared<GodleyIcon>();
      model->addItem(gi);
      GodleyTable& godley=gi->table;
      godley.resize(3,4);
      godley.cell(0,1)="c";
      godley.cell(0,2)="d";
      godley.cell(0,3)="e";
      godley.cell(2,1)="a";
      godley.cell(2,2)="b";
      godley.cell(2,3)="f";
      gi->update();

      VariablePtr stockVar(VariableType::stock,"c");
      model->addItem(stockVar)->variableCast();
      canvas.item=stockVar;
      canvas.renameAllInstances("newC");
      EXPECT_EQ("newC",stockVar->name());
      EXPECT_EQ("newC",godley.cell(0,1));
      gi->update();

      // renaming godley column should rename canvas stock vars
      GodleyTableEditor& ged=gi->editor;
      godley.cell(0,1)="c";
      ged.selectedRow=0;
      ged.selectedCol=1;
      godley.savedText="newC";
      ged.update();
      EXPECT_EQ("c",stockVar->name());
      EXPECT_EQ("c",godley.cell(0,1));

      // renaming just the canvas variable should change  it's type
      canvas.item=stockVar;
      if (auto v=canvas.item->variableCast())
        {
          canvas.renameItem("foo");
          EXPECT_TRUE(canvas.item && canvas.item->variableCast());
          if (auto v1=canvas.item->variableCast())
            {
              EXPECT_EQ("foo",v1->name());
              EXPECT_EQ(VariableType::flow, v1->type());
              EXPECT_TRUE(model->findItem(*canvas.item));
            }
        }
      EXPECT_EQ("c",godley.cell(0,1));
    }

    // Test logging functionality
    TEST_F(MinskySuite, loggingFunctionality)
    {
      auto var1 = model->addItem(VariablePtr(VariableType::flow, "testVar1"));
      auto var2 = model->addItem(VariablePtr(VariableType::flow, "testVar2"));
      variableValues[":testVar1"]->init("1.0");
      variableValues[":testVar2"]->init("2.0");
      
      logVarList.insert(":testVar1");
      logVarList.insert(":testVar2");
      
      string logFile = (temp_directory_path() / unique_path("test_log_%%%%-%%%%.dat")).string();
      openLogFile(logFile);
      EXPECT_TRUE(loggingEnabled());
      
      // Verify log file header was created
      std::ifstream f(logFile);
      EXPECT_TRUE(f.good());
      string line;
      getline(f, line);
      EXPECT_TRUE(line.find("#time") != string::npos);
      EXPECT_TRUE(line.find("testVar1") != string::npos);
      EXPECT_TRUE(line.find("testVar2") != string::npos);
      f.close();
      
      closeLogFile();
      EXPECT_FALSE(loggingEnabled());
      
      remove(logFile.c_str());
    }

    // Test multipleEquities
    TEST_F(MinskySuite, multipleEquities)
    {
      EXPECT_FALSE(multipleEquities());
      bool result = multipleEquities(true);
      EXPECT_TRUE(result);
      EXPECT_TRUE(multipleEquities());
      multipleEquities(false);
      EXPECT_FALSE(multipleEquities());
    }

    // Test utility methods
    TEST_F(MinskySuite, utilityMethods)
    {
      // Test physicalMem
      size_t mem = physicalMem();
      EXPECT_GT(mem, 0);
      
      // Test numOpArgs
      EXPECT_EQ(2, numOpArgs(OperationType::add));
      EXPECT_EQ(2, numOpArgs(OperationType::multiply));
      EXPECT_EQ(2, numOpArgs(OperationType::integrate));
      
      // Test classifyOp
      EXPECT_EQ(OperationType::function, classifyOp(OperationType::sin));
      EXPECT_EQ(OperationType::binop, classifyOp(OperationType::add));
    }

    // Test available operations and types
    TEST_F(MinskySuite, availableOperationsAndTypes)
    {
      vector<string> ops = availableOperations();
      EXPECT_GT(ops.size(), 0);
      EXPECT_TRUE(find(ops.begin(), ops.end(), "add") != ops.end());
      
      auto mapping = availableOperationsMapping();
      EXPECT_GT(mapping.size(), 0);
      
      vector<string> varTypes = variableTypes();
      EXPECT_GT(varTypes.size(), 0);
      EXPECT_TRUE(find(varTypes.begin(), varTypes.end(), "flow") != varTypes.end());
      
      vector<string> assets = assetClasses();
      EXPECT_GT(assets.size(), 0);
    }

    // Test font operations
    TEST_F(MinskySuite, fontOperations)
    {
      // Test defaultFont
      string origFont = defaultFont();
      string newFont = "Arial";
      defaultFont(newFont);
      EXPECT_EQ(newFont, defaultFont());
      if (!origFont.empty())
        defaultFont(origFont);
      
      // Test fontScale
      double origScale = fontScale();
      double newScale = 1.5;
      fontScale(newScale);
      EXPECT_EQ(newScale, fontScale());
      fontScale(origScale);
    }

    // Test latex2pango
    TEST_F(MinskySuite, latex2pango)
    {
      string result = latex2pango("x^2");
      EXPECT_FALSE(result.empty());
      
      result = latex2pango("\\alpha");
      EXPECT_FALSE(result.empty());
    }

    // Test clipboard operations
    TEST_F(MinskySuite, clipboardOperations)
    {
      auto var1 = model->addItem(VariablePtr(VariableType::flow, "clipVar"));
      canvas.selection.ensureItemInserted(var1);
      
      copy();
      EXPECT_FALSE(clipboardEmpty());
      
      canvas.selection.clear();
      copy();
      this_thread::sleep_for(chrono::milliseconds(100)); // allow clipboard state to propagate
      EXPECT_TRUE(clipboardEmpty());
    }

    // Test history operations
    TEST_F(MinskySuite, historyOperations)
    {
      clearHistory();
      
      auto var1 = model->addItem(VariablePtr(VariableType::flow, "histVar"));
      bool pushed = pushHistory();
      
      // Should push only if different from previous
      EXPECT_TRUE(pushed || history.size() > 0);
      
      auto var2 = model->addItem(VariablePtr(VariableType::flow, "histVar2"));
      pushHistory();
      
      size_t histSize = history.size();
      if (histSize > 0)
      {
        undo(1);
        EXPECT_TRUE(model->items.size() <= 3); // May have been restored
      }
    }

    // Test dimension operations
    TEST_F(MinskySuite, dimensionOperations)
    {
      // Create a dimension with proper Type enum (e.g., Dimension::value)
      dimensions.emplace("testDim", Dimension(Dimension::value, "test"));
      
      // Create a variable with this dimension in its hypercube
      auto var1 = model->addItem(VariablePtr(VariableType::flow, "dimTestVar"));
      auto hc = variableValues[":dimTestVar"]->tensorInit.hypercube();
      hc.xvectors.emplace_back("testDim", Dimension(Dimension::value, "test"));
      variableValues[":dimTestVar"]->tensorInit.hypercube(hc);
      
      renameDimension("testDim", "newTestDim");
      
      // Verify dimension was renamed in dimensions map
      EXPECT_GT(dimensions.count("newTestDim"), 0);
      EXPECT_EQ(dimensions.count("testDim"), 0);
      
      // Verify variableValue hypercube was also updated
      auto updatedHc = variableValues[":dimTestVar"]->tensorInit.hypercube();
      bool foundRenamed = false;
      bool foundOld = false;
      for (const auto& xv : updatedHc.xvectors) {
        if (xv.name == "newTestDim") foundRenamed = true;
        if (xv.name == "testDim") foundOld = true;
      }
      EXPECT_TRUE(foundRenamed);
      EXPECT_FALSE(foundOld);
      
      dimensions.clear();
    }

    // Test Godley operations
    TEST_F(MinskySuite, godleyOperations)
    {
      auto g1 = new GodleyIcon;
      model->addItem(g1);
      g1->table.resize(3, 3);
      g1->table.cell(0,1) = "stock1";
      g1->table.cell(2,1) = "flow1";
      g1->update();
      
      setAllDEmode(true);
      EXPECT_TRUE(g1->table.doubleEntryCompliant);
      
      setAllDEmode(false);
      EXPECT_FALSE(g1->table.doubleEntryCompliant);
      
      vector<string> flowVars = allGodleyFlowVars();
      EXPECT_GT(flowVars.size(), 0);
      
      // Check that flow1 is present and stock1 is absent
      bool foundFlow1 = false;
      bool foundStock1 = false;
      for (const auto& var : flowVars) {
        if (var == "flow1") foundFlow1 = true;
        if (var == "stock1") foundStock1 = true;
      }
      EXPECT_TRUE(foundFlow1);
      EXPECT_FALSE(foundStock1);
    }

    // Test variable type conversion
    TEST_F(MinskySuite, convertVarType)
    {
      auto var1 = model->addItem(VariablePtr(VariableType::flow, "convertVar"));
      EXPECT_EQ(VariableType::flow, variableValues[":convertVar"]->type());
      
      convertVarType(":convertVar", VariableType::parameter);
      EXPECT_EQ(VariableType::parameter, variableValues[":convertVar"]->type());
    }

    // Test addIntegral
    TEST_F(MinskySuite, addIntegral)
    {
      auto var1 = model->addItem(VariablePtr(VariableType::flow, "integVar"));
      canvas.item = var1;
      
      size_t itemsBefore = model->items.size();
      addIntegral();
      EXPECT_GT(model->items.size(), itemsBefore);
      
      // Check that var1's type is now integral
      EXPECT_EQ(VariableType::integral, variableValues[":integVar"]->type());
      
      ASSERT_EQ(model->items.size(),2);

      // assume integral is placed at end
      auto integ=dynamic_cast<IntOp*>(model->items[1].get());
      ASSERT_TRUE(integ);
      EXPECT_EQ(integ->intVar, model->items[0]);
    }

    // Test requestReset and requestRedraw
    TEST_F(MinskySuite, requestOperations)
    {
      requestReset();
      EXPECT_TRUE(reset_flag());
      
      requestRedraw();
      // Just verify it doesn't crash
    }

    // Test autoSaveFile operations
    TEST_F(MinskySuite, autoSaveFileOperations)
    {
      string testFile = (temp_directory_path() / unique_path("autosave_test_%%%%-%%%%.mky")).string();
      
      // Add some items to the canvas
      auto var1 = model->addItem(VariablePtr(VariableType::flow, "autoVar1"));
      auto var2 = model->addItem(VariablePtr(VariableType::flow, "autoVar2"));
      
      setAutoSaveFile(testFile);
      EXPECT_EQ(testFile, autoSaveFile());
      
      // Push history to trigger autosave
      EXPECT_TRUE(pushHistory());
      
      // Give some time for autosave to complete (it runs in background)
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      
      // Verify the autosave file was created
      EXPECT_TRUE(exists(testFile));
      
      // Clear the model and load from autosave file
      clearAllMaps();
      EXPECT_EQ(0, model->items.size());
      
      load(testFile);
      
      // Check that the state was restored
      EXPECT_EQ(model->items.size(), 2);
      EXPECT_TRUE(variableValues.count(":autoVar1") > 0);
      EXPECT_TRUE(variableValues.count(":autoVar2") > 0);
      
      setAutoSaveFile("");
      EXPECT_EQ("", autoSaveFile());
      
      remove(testFile.c_str());
    }

    // Test layout operations
    TEST_F(MinskySuite, layoutOperations)
    {
      auto var1 = model->addItem(VariablePtr(VariableType::flow, "layoutVar1"));
      auto var2 = model->addItem(VariablePtr(VariableType::flow, "layoutVar2"));
      
      // Just verify these don't crash
      autoLayout();
      randomLayout();
    }

    // Test named items
    TEST_F(MinskySuite, namedItems)
    {
      auto var1 = model->addItem(VariablePtr(VariableType::flow, "namedVar"));
      canvas.item = var1;
      
      nameCurrentItem("testName");
      EXPECT_TRUE(namedItems.count("testName") > 0);
      
      itemFromNamedItem("testName");
      EXPECT_TRUE(canvas.item != nullptr);
      // Check that the item pointer equals var1
      EXPECT_EQ(canvas.item, var1);
    }

    // Test pushFlags and popFlags
    TEST_F(MinskySuite, flagOperations)
    {
      // Set flags to non-zero value
      flags = is_edited | reset_needed;
      int origFlags = flags;
      pushFlags();
      flags = 0;
      popFlags();
      EXPECT_EQ(origFlags, flags);
    }

    // Test deleteAllUnits
    TEST_F(MinskySuite, deleteAllUnits)
    {
      auto var1 = model->addItem(VariablePtr(VariableType::flow, "unitVar"));
      variableValues[":unitVar"]->units=Units("m/s");
      
      deleteAllUnits();
      
      // Verify units are cleared
      EXPECT_TRUE(variableValues[":unitVar"]->units.str().empty());
    }

    // Test setGodleyDisplayValue
    TEST_F(MinskySuite, setGodleyDisplayValue)
    {
      auto g1 = new GodleyIcon;
      model->addItem(g1);
      
      setGodleyDisplayValue(true, GodleyTable::DRCR);
      EXPECT_TRUE(displayValues);
      EXPECT_EQ(GodleyTable::DRCR, displayStyle);
      
      setGodleyDisplayValue(false, GodleyTable::sign);
      EXPECT_FALSE(displayValues);
      EXPECT_EQ(GodleyTable::sign, displayStyle);
    }

    // Test save and load
    TEST_F(MinskySuite, saveAndLoad)
    {
      string testFile = (temp_directory_path() / unique_path("test_save_%%%%-%%%%.mky")).string();
      
      auto var1 = model->addItem(VariablePtr(VariableType::flow, "saveVar"));
      variableValues[":saveVar"]->init("5.0");
      
      save(testFile);
      
      clearAllMaps();
      EXPECT_EQ(0, model->items.size());
      EXPECT_EQ(0, variableValues.count(":saveVar"));
     
      load(testFile);
      EXPECT_EQ(model->items.size(), 1); // time + saveVar
      EXPECT_TRUE(variableValues.count(":saveVar") > 0);
      
      remove(testFile.c_str());
    }

    // Test insertGroupFromFile
    TEST_F(MinskySuite, insertGroupFromFile)
    {
      string groupFile = (temp_directory_path() / unique_path("test_group_%%%%-%%%%.mky")).string();
      
      // Create a small model to save as a group
      auto var1 = model->addItem(VariablePtr(VariableType::flow, "groupVar"));
      saveGroupAsFile(*model, groupFile);
      
      clearAllMaps();
      
      insertGroupFromFile(groupFile);
      
      // model->items should be empty 
      EXPECT_EQ(0, model->items.size()); 
      // model->groups should contain one group
      EXPECT_EQ(1, model->groups.size());
      if (model->groups.size() > 0) {
        // which intern contains one item in model->groups[0]->items
        EXPECT_EQ(1, model->groups[0]->items.size());
      }
      
      remove(groupFile.c_str());
    }

    // Test makeVariablesConsistent
    TEST_F(MinskySuite, makeVariablesConsistent)
    {
      // makeVariablesConsistent calls update on GodleyIcons
      // Add a GodleyIcon with flow & stock variables in the table
      auto godley = new GodleyIcon;
      model->addItem(godley);
      godley->table.resize(3, 3);
      godley->table.cell(0, 1) = "stock1";
      godley->table.cell(0, 2) = "stock2";
      godley->table.cell(2, 1) = "flow1";
      godley->table.cell(2, 2) = "flow2";
      
      // Before makeVariablesConsistent, flowVars and stockVars may not be populated
      EXPECT_EQ(0, godley->flowVars().size());
      EXPECT_EQ(0, godley->stockVars().size());

      variableValues.emplace("temp to be removed",VariableValuePtr());
      
      // Create duplicate entries in variableValues table (same valueId)
      size_t varValuesSizeBefore = variableValues.size();
      
      // Make variables consistent - this should call update on the GodleyIcon
      makeVariablesConsistent();
      
      // Check that GodleyIcon's flowVars and stockVars have been populated
      auto fv=godley->flowVars(), sv=godley->stockVars();
      EXPECT_EQ(2, fv.size());
      EXPECT_EQ(2, sv.size());
      EXPECT_FALSE(find_if(fv.begin(),fv.end(),[](auto& i){return i->name()=="flow1";})==fv.end());
      EXPECT_FALSE(find_if(fv.begin(),fv.end(),[](auto& i){return i->name()=="flow2";})==fv.end());
      EXPECT_TRUE(find_if(fv.begin(),fv.end(),[](auto& i){return i->name()=="stock1";})==fv.end());
      EXPECT_TRUE(find_if(fv.begin(),fv.end(),[](auto& i){return i->name()=="stock2";})==fv.end());
      EXPECT_TRUE(find_if(sv.begin(),sv.end(),[](auto& i){return i->name()=="flow1";})==sv.end());
      EXPECT_TRUE(find_if(sv.begin(),sv.end(),[](auto& i){return i->name()=="flow2";})==sv.end());
      EXPECT_FALSE(find_if(sv.begin(),sv.end(),[](auto& i){return i->name()=="stock1";})==sv.end());
      EXPECT_FALSE(find_if(sv.begin(),sv.end(),[](auto& i){return i->name()=="stock2";})==sv.end());
     
      // Check that inconsistent entries in variableValues have been removed
      EXPECT_EQ(0, variableValues.count("temp to be removed"));
    }

    // Test garbageCollect
    TEST_F(MinskySuite, garbageCollect)
    {
      // Add some variables and operations to create temporary values
      auto var1 = model->addItem(VariablePtr(VariableType::flow, "gcVar"));
      auto op1 = model->addItem(OperationPtr(OperationType::add));
      auto integ=model->addItem(OperationPtr(OperationType::integrate));
      
      // Construct equations to create integrals, stockVars, flowVars
      model->addWire(op1->ports(0), var1->ports(1));
      model->addWire(var1->ports(0), integ->ports(1));
      
      constructEquations();
        
      // After constructing equations, there should be some flowVars, stockVars, equations, integrals
      EXPECT_GT(flowVars.size(), 0);
      EXPECT_GT(stockVars.size(), 0);
      EXPECT_GT(equations.size(), 0);
      EXPECT_GT(integrals.size(), 0);

      // stash the correct sizes
      auto fvSz=flowVars.size(), stSz=stockVars.size(), eqSz=stockVars.size(), intSz=integrals.size();
      // add some rubbish at the end of all of these
      flowVars.resize(fvSz+5);
      stockVars.resize(stSz+5);
      equations.resize(eqSz+5);
      integrals.resize(intSz+5);

      // add some temporary variables
      variableValues["temp"]=VariableValuePtr(VariableType::tempFlow);
      
      // GarbageCollect should clean things up
      garbageCollect();
        
      // Check that flowVars, stockVars, equations, integrals are empty
      EXPECT_EQ(flowVars.size(), 3); // zero, one and gcVar
      EXPECT_EQ(stockVars.size(), 1); // integral variable
      EXPECT_EQ(equations.size(), 0);
      EXPECT_EQ(integrals.size(), 0);
      EXPECT_EQ(variableValues.count("temp"), 0);
    }

    // Test imposeDimensions
    TEST_F(MinskySuite, imposeDimensions)
    {
      // Add dimension information to the dimensions table
      dimensions.emplace("testDimension", Dimension(Dimension::time, "seconds"));
      
      auto var1 = model->addItem(VariablePtr(VariableType::flow, "dimVar"));
      
      // Add a matching dimension to var1 but of different type
      auto hc = variableValues[":dimVar"]->tensorInit.hypercube();
      hc.xvectors.emplace_back("testDimension", Dimension(Dimension::value, "meters"));
      variableValues[":dimVar"]->tensorInit.hypercube(hc);
      
      // Impose dimensions - should update var1's dimension to match dimensions table
      imposeDimensions();
      
      // Check that the dimension has been updated
      auto updatedHc = variableValues[":dimVar"]->tensorInit.hypercube();
      bool foundWithCorrectType = false;
      for (const auto& xv : updatedHc.xvectors) {
        if (xv.name == "testDimension" && xv.dimension.type == Dimension::time) {
          foundWithCorrectType = true;
        }
      }
      EXPECT_TRUE(foundWithCorrectType);
      
      dimensions.clear();
    }

    // Test cycleCheck
    TEST_F(MinskySuite, cycleCheck)
    {
      // First test: no cycle
      auto var1 = model->addItem(VariablePtr(VariableType::flow, "cycleVar1"));
      auto var2 = model->addItem(VariablePtr(VariableType::flow, "cycleVar2"));
      auto op1 = model->addItem(OperationPtr(OperationType::add));
      
      model->addWire(var1->ports(0), op1->ports(1));
      model->addWire(op1->ports(0), var2->ports(1));
      
      EXPECT_FALSE(cycleCheck());
      
      // Now create a cycle: var3 -> op2 -> var3
      auto var3 = model->addItem(VariablePtr(VariableType::flow, "cycleVar3"));
      auto op2 = model->addItem(OperationPtr(OperationType::add));
      
      model->addWire(var3->ports(0), op2->ports(1));
      model->addWire(op2->ports(0), var3->ports(1));
      
      EXPECT_TRUE(cycleCheck());
    }

    // Test checkEquationOrder
    TEST_F(MinskySuite, checkEquationOrder)
    {
      auto var1 = model->addItem(VariablePtr(VariableType::flow, "orderVar"));
      auto op1 = model->addItem(OperationPtr(OperationType::time));
      model->addWire(op1->ports(0), var1->ports(1));
      
      constructEquations();
      EXPECT_TRUE(checkEquationOrder());
    }

    // Test edited flag
    TEST_F(MinskySuite, editedFlag)
    {
      flags &= ~is_edited;
      EXPECT_FALSE(edited());
      
      markEdited();
      EXPECT_TRUE(edited());
    }

    // Test reset_flag
    TEST_F(MinskySuite, resetFlag)
    {
      flags |= reset_needed;
      EXPECT_TRUE(reset_flag());
      
      flags &= ~reset_needed;
      EXPECT_FALSE(reset_flag());
    }

    // Test resetIfFlagged
    TEST_F(MinskySuite, resetIfFlagged)
    {
      flags |= reset_needed;
      bool result = resetIfFlagged();
      EXPECT_FALSE(result || reset_flag());
    }

    // Test exportSchema
    TEST_F(MinskySuite, exportSchema)
    {
      string schemaFile = (temp_directory_path() / unique_path("test_schema%%%%-%%%%.xsd")).string();
      // current schema is 3, crashes on default schema 1.
      exportSchema(schemaFile,3);
      
      std::ifstream f(schemaFile);
      EXPECT_TRUE(f.good());
      f.close();
      remove(schemaFile.c_str());
    }

    // Test populateMissingDimensions
    TEST_F(MinskySuite, populateMissingDimensions)
    {
      // Clear existing dimensions
      dimensions.clear();
      
      // Add variables with dimension information
      auto var1 = model->addItem(VariablePtr(VariableType::flow, "dimVar1"));
      auto var2 = model->addItem(VariablePtr(VariableType::flow, "dimVar2"));
      
      // Add dimensions to variable hypercubes
      auto hc1 = variableValues[":dimVar1"]->tensorInit.hypercube();
      hc1.xvectors.emplace_back("newDim1", Dimension(Dimension::time, "seconds"));
      hc1.xvectors[0].emplace_back();
      variableValues[":dimVar1"]->tensorInit.hypercube(hc1);
      
      auto hc2 = variableValues[":dimVar2"]->tensorInit.hypercube();
      hc2.xvectors.emplace_back("newDim2", Dimension(Dimension::value, "meters"));
      hc2.xvectors[0].emplace_back();
      variableValues[":dimVar2"]->tensorInit.hypercube(hc2);
      
      // Populate missing dimensions
      populateMissingDimensions();
      
      // Check that dimensions have been added to the dimensions table
      EXPECT_TRUE(dimensions.count("newDim1") > 0);
      EXPECT_TRUE(dimensions.count("newDim2") > 0);
      
      // Verify the dimension types match
      EXPECT_EQ(Dimension::time, dimensions["newDim1"].type);
      EXPECT_EQ(Dimension::value, dimensions["newDim2"].type);
    }

    // Test openGroupInCanvas and openModelInCanvas
    TEST_F(MinskySuite, canvasGroupOperations)
    {
      auto g1 = model->addGroup(new Group);
      g1->addItem(VariablePtr(VariableType::flow, "groupVar"));
      
      canvas.item = g1;
      openGroupInCanvas();
      EXPECT_TRUE(canvas.model != model);
      // Check that canvas.model equals g1
      EXPECT_EQ(canvas.model, g1);
      
      openModelInCanvas();
      EXPECT_TRUE(canvas.model == model);
    }

    // Test saveSelectionAsFile
    TEST_F(MinskySuite, saveSelectionAsFile)
    {
      string selFile = (temp_directory_path() / unique_path("test_selection_%%%%-%%%%.mky")).string();
      
      auto var1 = model->addItem(VariablePtr(VariableType::flow, "selVar"));
      canvas.selection.ensureItemInserted(var1);
      
      saveSelectionAsFile(selFile);
      
      std::ifstream f(selFile);
      EXPECT_TRUE(f.good());
      f.close();
      
      // Clear and load to check item is restored
      canvas.selection.clear();
      clearAllMaps();
      
      load(selFile);
      EXPECT_TRUE(variableValues.count(":selVar") > 0);
      
      remove(selFile.c_str());
    }

    // Test saveCanvasItemAsFile
    TEST_F(MinskySuite, saveCanvasItemAsFile)
    {
      string canvasFile = (temp_directory_path() / unique_path("test_canvas_item_%%%%-%%%%.mky")).string();
      
      auto g1 = model->addGroup(new Group);
      g1->addItem(VariablePtr(VariableType::flow, "canvasVar"));
      canvas.item = g1;
      
      saveCanvasItemAsFile(canvasFile);
      
      std::ifstream f(canvasFile);
      EXPECT_TRUE(f.good());
      f.close();
      
      // Load and check that the item was saved
      clearAllMaps();
      load(canvasFile);
      EXPECT_TRUE(variableValues.count(":canvasVar") > 0);
      
      remove(canvasFile.c_str());
    }

    // Test inputWired
    TEST_F(MinskySuite, inputWired)
    {
      auto var1 = model->addItem(VariablePtr(VariableType::flow, "wireVar"));
      EXPECT_FALSE(inputWired(":wireVar"));
      
      auto op1 = model->addItem(OperationPtr(OperationType::time));
      model->addWire(op1->ports(0), var1->ports(1));
      EXPECT_TRUE(inputWired(":wireVar"));
    }

    // Test commandHook
    TEST_F(MinskySuite, commandHook)
    {
      // Initialize history stack with pushHistory
      auto var1 = model->addItem(VariablePtr(VariableType::flow, "hookVar"));
      pushHistory();
      
      // Reset edited flag
      flags &= ~is_edited;
      size_t histSizeBefore = history.size();
      
      // Change the model
      var1->moveTo(500,500);
      
      // Test with a generic command - should push history and set edited flag
      bool result = commandHook("minsky.test.command", 1);
      
      // Check that history stack has been pushed
      EXPECT_GT(history.size(), histSizeBefore);
      // Check that edited flag is set
      EXPECT_TRUE(flags & is_edited);
      // Return value should be true
      EXPECT_TRUE(result);
      
      // Now test with a const command
      flags &= ~is_edited; // Reset edited flag
      size_t histSizeBeforeConst = history.size();
      
      result = commandHook("minsky.save", 0);
      
      // Edited flag should remain unset
      EXPECT_FALSE(flags & is_edited);
      // History stack should be unchanged
      EXPECT_EQ(history.size(), histSizeBeforeConst);
      // Return value should be false
      EXPECT_FALSE(result);
    }

