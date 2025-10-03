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
using namespace minsky;

namespace
{
  struct TestFixture: public Minsky, testing::Test
  {
    LocalMinsky lm;
    string savedMessage;
    TestFixture(): lm(*this)
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

  TEST_F(TestFixture,constructEquationsEx1)
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

  TEST_F(TestFixture,constructEquationsEx2)
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

  TEST_F(TestFixture,godleyEval)
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
  TEST_F(TestFixture,derivative)
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

  TEST_F(TestFixture,integrals)
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

  TEST_F(TestFixture,cyclicThrows)
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

  TEST_F(TestFixture,cyclicIntegrateDoesntThrow)
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

  TEST_F(TestFixture,godleyIconVariableOrder)
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
   

  TEST_F(TestFixture,multiVariableInputsAdd)
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

  TEST_F(TestFixture,multiVariableInputsSubtract)
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

  TEST_F(TestFixture,multiVariableInputsMultiply)
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

  TEST_F(TestFixture,multiVariableInputsDivide)
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
  
  TEST_F(TestFixture,multiGodleyRules)
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

  TEST_F(TestFixture,matchingTableColumns)
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

  TEST_F(TestFixture,bug1157)
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

  TEST_F(TestFixture,importDuplicateColumn)
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

  TEST_F(TestFixture,godleyRowSums)
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

  TEST_F(TestFixture,godleyMoveRowCol)
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

  TEST_F(TestFixture,godleyNameUnique)
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

    TEST_F(TestFixture,clearInitCond)
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
    
    TEST_F(TestFixture,varNameScoping)
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

    TEST_F(TestFixture,cantMultiplyDefineVars)
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

    TEST_F(TestFixture,MultiplyDefinedVarsThrowsOnReset)
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

    TEST_F(TestFixture, RemoveDefinitionsFromPastedVars)
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

    TEST_F(TestFixture, DefinitionPasted)
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
    
    TEST_F(TestFixture, PastedIntOpShowsMessage)
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

    TEST_F(TestFixture, RetypePastedIntegralVariable)
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
    
    TEST_F(TestFixture, cut)
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

    TEST_F(TestFixture,renameAll)
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

