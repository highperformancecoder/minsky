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
#include <UnitTest++/UnitTest++.h>
#include <gsl/gsl_integration.h>
using namespace minsky;

namespace
{
  struct TestFixture: public Minsky
  {
    LocalMinsky lm;
    string savedMessage;
    TestFixture(): lm(*this)
    {
    }
    void message(const string& x) override {savedMessage=x;}
  };
}

SUITE(Minsky)
{
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

  TEST_FIXTURE(TestFixture,constructEquationsEx1)
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

      CHECK(var["a"]->lhs());
      CHECK(var["b"]->lhs());
      CHECK(!var["c"]->lhs());
      CHECK(!var["d"]->lhs());
      CHECK(!var["e"]->lhs());
      CHECK(var["f"]->lhs());

      auto addOp=model->addItem(OperationBase::create(OperationType::add)); 
      CHECK_EQUAL(3, addOp->portsSize());
      auto intOp=dynamic_cast<IntOp*>(OperationBase::create(OperationType::integrate));
      model->addItem(intOp); 
      CHECK_EQUAL(3, intOp->portsSize());
      auto mulOp=model->addItem(OperationBase::create(OperationType::multiply)); 
      CHECK_EQUAL(3, mulOp->portsSize());
 
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
          CHECK(!w->from()->input());
          CHECK(w->to()->input());
        }

      save("constructEquationsEx1.mky");

      CHECK(!cycleCheck());

      constructEquations();

      CHECK_EQUAL(4, integrals.size());
      // check that integral  stock vars aren't flowVars
      int nakedIntegral=-1;
      for (size_t i=0; i<integrals.size(); ++i)
        {
          if (integrals[i].stock->name=="int1")
            nakedIntegral=i;
          CHECK(!integrals[i].stock->isFlowVar());
        }
      CHECK(nakedIntegral>=0);

      var["c"]->value(0.1);
      var["d"]->value(0.2);
      var["e"]->value(0.3);

      order=1;
      implicit=false;
      step();

      CHECK_CLOSE(var["c"]->value()+var["d"]->value(), integrals[nakedIntegral].input().value(), 1e-4);
      CHECK_CLOSE(integrals[nakedIntegral].stock->value(), var["a"]->value(), 1e-4);
      CHECK_CLOSE(integrals[nakedIntegral].stock->value()*var["e"]->value(), var["b"]->value(), 1e-4);
      CHECK_CLOSE(var["e"]->value(), var["f"]->value(), 1e-4);
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

  TEST_FIXTURE(TestFixture,constructEquationsEx2)
    {
      model->addItem(VariablePtr(VariableType::flow,"g"));
      model->addItem(VariablePtr(VariableType::flow,"h"));

      // build a table of variables - names will be unique at this stage
      map<string, VariablePtr> var;
      for (ItemPtr& i: model->items)
        if (auto v=dynamic_pointer_cast<VariableBase>(i))
            var[v->name()]=v;

      auto op4=model->addItem(new VarConstant);
      CHECK_EQUAL(1, op4->portsSize());
      auto op5=model->addItem(new VarConstant);
      CHECK_EQUAL(1, op5->portsSize());
      auto op6=model->addItem(OperationBase::create(OperationType::add));
      CHECK_EQUAL(3, op6->portsSize());

      ecolab::array<float> coords(4,0);
 
      auto wire8=model->addWire(*op4, *var["g"], 1);
      auto wire9=model->addWire(*op4, *op6, 1);
      auto wire10=model->addWire(*op5, *op6, 2);
      auto wire11=model->addWire(*op6, *var["h"], 1);
 
      for (auto& w: model->wires)
        {
          CHECK(!w->from()->input());
          CHECK(w->to()->input());
        }
 
      CHECK(dynamic_cast<VarConstant*>(op4.get()));
      if (VarConstant* c=dynamic_cast<VarConstant*>(op4.get()))
        c->init("0.1");
      CHECK(dynamic_cast<VarConstant*>(op5.get()));
      if (VarConstant* c=dynamic_cast<VarConstant*>(op5.get()))
        c->init("0.2");

      reset();

      CHECK_CLOSE(0.1, var["g"]->value(), 1e-4);
      CHECK_CLOSE(0.3, var["h"]->value(), 1e-4);

    }

  TEST_FIXTURE(TestFixture,godleyEval)
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
 
      variableValues[":a"]->init="5";
 
      garbageCollect();
      reset();
      CHECK_EQUAL(10,variableValues[":c"]->value());
      CHECK_EQUAL(20,variableValues[":d"]->value());
      CHECK_EQUAL(30,variableValues[":e"]->value());
      CHECK_EQUAL(5,variableValues[":a"]->value());
      for (size_t i=0; i<stockVars.size(); ++i)
        stockVars[i]=0;
     
      evalGodley.eval(stockVars.data(), flowVars.data());
      CHECK_EQUAL(5,variableValues[":c"]->value());
      CHECK_EQUAL(-5,variableValues[":d"]->value());
      CHECK_EQUAL(0,variableValues[":e"]->value());
      CHECK_EQUAL(5,variableValues[":a"]->value());
    
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
  TEST_FIXTURE(TestFixture,derivative)
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
 
      CHECK(!cycleCheck());
      reset();
      vector<double> j(stockVars.size()*stockVars.size());
      Matrix jac(stockVars.size(),j.data());
 
      auto& c=*variableValues[":c"];   c=100;
      auto& d=*variableValues[":d"];   d=200;
      auto& e=*variableValues[":e"];   e=300;
      double& x=stockVars.back();   x=0; // temporary variable storing \int c+d
 
      CHECK_EQUAL(4, stockVars.size());
 
      save("derivative.mky");
      evalJacobian(jac,t,stockVars.data());
   
      CHECK_EQUAL(0, jac(0,0));
      CHECK_EQUAL(0, jac(0,1));  
      CHECK_EQUAL(0, jac(0,2));
      CHECK_EQUAL(1, jac(0,3));
      CHECK_EQUAL(0, jac(1,0));
      CHECK_EQUAL(0, jac(1,1));
      CHECK_EQUAL(x, jac(1,2));
      CHECK_EQUAL(e.value(), jac(1,3));
      CHECK_EQUAL(0,jac(2,0));
      CHECK_EQUAL(0,jac(2,1));
      CHECK_EQUAL(1,jac(2,2));
      CHECK_EQUAL(0,jac(2,3));
      CHECK_EQUAL(1,jac(3,0));
      CHECK_EQUAL(1,jac(3,1));
      CHECK_EQUAL(0,jac(3,2));
      CHECK_EQUAL(0,jac(3,3));
    }

  TEST_FIXTURE(TestFixture,integrals)
    {
      // First, integrate a constant
      auto op1=model->addItem(new VarConstant);
      auto op2=model->addItem(OperationPtr(OperationBase::integrate));
      
      IntOp* intOp=dynamic_cast<IntOp*>(op2.get());
      CHECK(intOp);
      intOp->description("output");
      model->addWire(*op1,*op2,1);

      constructEquations();
      CHECK(variableValues.validEntries());
      
      double value = 10;
      dynamic_cast<VariableBase*>(op1.get())->init(to_string(value));
      nSteps=1;
      running=true;
      step();
      // for now, constructEquations doesn work
      CHECK_CLOSE(value*t, integrals[0].stock->value(), 1e-5);
      CHECK_CLOSE(integrals[0].stock->value(), variableValues[":output"]->value(), 1e-5);
 
      // now integrate the linear function
      auto op3=model->addItem(OperationPtr(OperationBase::integrate));
      model->addWire(*intOp->intVar, *op3, 1);
      reset();
      running=true;
      step();
      //      CHECK_CLOSE(0.5*value*t*t, integrals[1].stock.value(), 1e-5);
      intOp=dynamic_cast<IntOp*>(op3.get());
      CHECK(intOp);
      CHECK_CLOSE(0.5*value*t*t, intOp->intVar->value(), 1e-5);
    }

  /*
    check that cyclic networks throw an exception

    a
    \ 
    + - a
    /
    b
  */

  TEST_FIXTURE(TestFixture,cyclicThrows)
    {
      // First, integrate a constant
      auto op1=model->addItem(OperationPtr(OperationType::add));
      auto w=model->addItem(VariablePtr(VariableType::flow,"w"));
      auto a=model->addItem(VariablePtr(VariableType::flow,"a"));
      CHECK(model->addWire(new Wire(op1->ports(0), w->ports(1))));
      CHECK(model->addWire(new Wire(w->ports(0), op1->ports(1))));
      model->addWire(new Wire(op1->ports(0), w->ports(1)));
      model->addWire(new Wire(w->ports(0), op1->ports(1)));
      model->addWire(new Wire(a->ports(0), op1->ports(2)));

      CHECK(cycleCheck());
      CHECK_THROW(constructEquations(), ecolab::error);
    }

  /*
    but integration is allowed to cycle

    +--------+ 
    \        \
    *- int---+
    /
    b
  */

  TEST_FIXTURE(TestFixture,cyclicIntegrateDoesntThrow)
    {
      // First, integrate a constant
      auto op1=model->addItem(OperationPtr(OperationType::integrate));
      auto op2=model->addItem(OperationPtr(OperationType::multiply));
      auto a=model->addItem(VariablePtr(VariableType::flow,"a"));
      CHECK(model->addWire(new Wire(op1->ports(0), op2->ports(1))));
      CHECK(model->addWire(new Wire(op2->ports(0), op1->ports(1))));
      CHECK(model->addWire(new Wire(a->ports(0), op2->ports(2))));

      model->addWire(new Wire(op1->ports(0), op2->ports(1)));
      model->addWire(new Wire(op2->ports(0), op1->ports(1)));
      model->addWire(new Wire(a->ports(0), op2->ports(2)));

      CHECK(!cycleCheck());
      constructEquations();
    }

  TEST_FIXTURE(TestFixture,godleyIconVariableOrder)
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
          CHECK_EQUAL(g.table.cell(0,i), g.stockVars()[i-1]->name());
          CHECK_EQUAL(g.table.cell(2,i), g.flowVars()[i-1]->name());
        }
    }

  /*
    a --
    b / \
    + -- c
  */
   

  TEST_FIXTURE(TestFixture,multiVariableInputsAdd)
    {
      auto varA = model->addItem(VariablePtr(VariableType::flow, "a"));
      auto varB = model->addItem(VariablePtr(VariableType::flow, "b"));
      auto varC = model->addItem(VariablePtr(VariableType::flow, "c"));
      variableValues[":a"]->init="0.1";
      variableValues[":b"]->init="0.2";

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
      CHECK_CLOSE(0.3, variableValues[":c"]->value(), 1e-5);
    }

  TEST_FIXTURE(TestFixture,multiVariableInputsSubtract)
    {
      auto varA = model->addItem(VariablePtr(VariableType::flow, "a"));
      auto varB = model->addItem(VariablePtr(VariableType::flow, "b"));
      auto varC = model->addItem(VariablePtr(VariableType::flow, "c"));
      variableValues[":a"]->init="0.1";
      variableValues[":b"]->init="0.2";

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
      CHECK_CLOSE(-0.3, variableValues[":c"]->value(), 1e-5);
    }

  TEST_FIXTURE(TestFixture,multiVariableInputsMultiply)
    {
      auto varA = model->addItem(VariablePtr(VariableType::flow, "a"));
      auto varB = model->addItem(VariablePtr(VariableType::flow, "b"));
      auto varC = model->addItem(VariablePtr(VariableType::flow, "c"));
      variableValues[":a"]->init="0.1";
      variableValues[":b"]->init="0.2";

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
      CHECK_CLOSE(0.02, variableValues[":c"]->value(), 1e-5);
    }

  TEST_FIXTURE(TestFixture,multiVariableInputsDivide)
    {
      auto varA = model->addItem(VariablePtr(VariableType::flow, "a"));
      auto varB = model->addItem(VariablePtr(VariableType::flow, "b"));
      auto varC = model->addItem(VariablePtr(VariableType::flow, "c"));
      variableValues[":a"]->init="0.1";
      variableValues[":b"]->init="0.2";

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
      CHECK_CLOSE(50, variableValues[":c"]->value(), 1e-5);
    }

  // instantiate all operations and variables to ensure that definitions
  // have been provided for all virtual methods
  TEST(checkAllOpsDefined)
  {
    using namespace MathDAG;
    for (int o=0; o<OperationType::sum; ++o)
      {
        if (OperationType::Type(o)==OperationType::constant) continue; // constant deprecated
        OperationType::Type op=OperationType::Type(o);
        OperationPtr po(op);
        CHECK_EQUAL(OperationType::typeName(op), OperationType::typeName(po->type()));
        if (o!=OperationType::differentiate) // no evalop etc for differentiate needed.
          {
            EvalOpPtr ev(op);
            CHECK_EQUAL(OperationType::typeName(op), OperationType::typeName(ev->type()));

            shared_ptr<OperationDAGBase> opDAG
              (OperationDAGBase::create(OperationType::Type(op), ""));
            CHECK_EQUAL(OperationType::typeName(op), OperationType::typeName(opDAG->type()));
          }
      }

    for (int v=0; v<VariableType::numVarTypes; ++v)
      {
        VariableType::Type vt=VariableType::Type(v);
        VariablePtr pv(vt);
        CHECK_EQUAL(vt, pv->type());
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
      CHECK_CLOSE(evalOp->evaluate(b,0)-evalOp->evaluate(a,0), result, 2*abserr); \
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
  TEST(checkDerivativesThrow)
  {
    CHECK_THROW(EvalOp<OperationType::and_>().d1(0,0),error); 
    CHECK_THROW(EvalOp<OperationType::or_>().d1(0,0),error); 
    CHECK_THROW(EvalOp<OperationType::not_>().d1(0,0),error); 
    CHECK_THROW(EvalOp<OperationType::lt>().d1(0,0),error); 
    CHECK_THROW(EvalOp<OperationType::le>().d1(0,0),error); 
    CHECK_THROW(EvalOp<OperationType::eq>().d1(0,0),error); 
    CHECK_THROW(EvalOp<OperationType::floor>().d1(0,0),error); 
    CHECK_THROW(EvalOp<OperationType::frac>().d1(0,0),error); 
 }
  
  // Tests derivative definitions of one argument functions by
  // integration over [0.5,1], using fundamental theorem of calculus.
  // Hopefully, this range will avoid any singularities in the derivative
  TEST(checkDerivatives)
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

  TEST(evalOpEvaluate)
  {
    EvalOp<OperationType::floor> floor;
    CHECK_EQUAL(3,floor.evaluate(3.2,0));
    EvalOp<OperationType::frac> frac;
    CHECK_CLOSE(0.2,frac.evaluate(3.2,0),1e-6);
  }
  
  TEST_FIXTURE(TestFixture,multiGodleyRules)
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
      godley1.cell(0,1)=":hello"; godley1._assetClass(1, GodleyAssetClass::asset);
      godley1.cell(0,2)=":bar"; godley1._assetClass(2, GodleyAssetClass::liability);
      godley1.cell(0,3)=":foo"; godley1._assetClass(3, GodleyAssetClass::equity);

      // should still be no problem
      initGodleys();

      godley2.resize(4,4);
      godley2.cell(0,1)=":hello2"; godley2._assetClass(1, GodleyAssetClass::asset);
      godley2.cell(0,2)=":bar2"; godley2._assetClass(2, GodleyAssetClass::liability);
      godley2.cell(0,3)=":foo2"; godley2._assetClass(3, GodleyAssetClass::equity);

      godley3.resize(4,4);
      godley3.cell(0,1)=":hello3"; godley3._assetClass(1, GodleyAssetClass::asset);
      godley3.cell(0,2)=":bar3"; godley3._assetClass(2, GodleyAssetClass::liability);
      godley3.cell(0,3)=":foo3"; godley3._assetClass(3, GodleyAssetClass::equity);

      // should be no problem - all columns are different
      initGodleys();

      godley3.cell(0,2)=":foo2"; 

      // two incompatible columns
      CHECK_THROW(initGodleys(), ecolab::error);

  
      godley3.cell(0,2)=":foo3"; 
      godley3.cell(0,1)=":hello2"; 

      // two incompatible columns asset columns
      CHECK_THROW(initGodleys(), ecolab::error);

      // should now be a compatible asset/liability pair
      godley3._assetClass(1, GodleyAssetClass::liability);
      initGodleys();
  

      // add in another asset/liability pair
      godley1.cell(0,3)=":bar3"; godley1._assetClass(3, GodleyAssetClass::asset);
      initGodleys();

      // now conflict that pair
      godley2.cell(0,3)=":bar3";godley2._assetClass(3, GodleyAssetClass::asset);
      CHECK_THROW(initGodleys(), ecolab::error);
      godley2.cell(0,3)=":bar2";
  
      // now add some flow variables and check those
      godley2.cell(2,1)="2:a";
      godley3.cell(2,1)=":a";
      godley3.cell(3,1)=":a";
      (variableValues[":a"]=VariableValuePtr(VariableType::flow))->allocValue();
      (variableValues[":hello2"]=VariableValuePtr(VariableType::stock))->allocValue();

      initGodleys();

      godley3.cell(3,1)="";
      CHECK_THROW(initGodleys(), ecolab::error);
 

    }

  TEST_FIXTURE(TestFixture,matchingTableColumns)
    {
      auto g1=new GodleyIcon; model->addItem(g1);
      auto g2=new GodleyIcon; model->addItem(g2);
      auto g3=new GodleyIcon; model->addItem(g3);
      GodleyTable& godley1=g1->table;
      GodleyTable& godley2=g2->table;
      godley1.resize(6,4);
      godley2.resize(3,5);

      godley1._assetClass(1,GodleyAssetClass::asset);  
      godley1._assetClass(2,GodleyAssetClass::liability);  
      godley1._assetClass(3,GodleyAssetClass::equity);  
      godley2._assetClass(1,GodleyAssetClass::asset);  
      godley2._assetClass(2,GodleyAssetClass::liability);
      godley2._assetClass(3,GodleyAssetClass::liability);
      godley2._assetClass(4,GodleyAssetClass::equity);

      godley1.cell(0,1)="a1";  
      godley1.cell(0,2)="l1";
      godley1.cell(0,3)="e1";
      g1->update();
      
      set<string> cols=matchingTableColumns(*g2,GodleyAssetClass::asset);
      CHECK_EQUAL(1, cols.size());
      CHECK_EQUAL("l1", *cols.begin());

      godley2.cell(0,1)="l1";  
      godley2.cell(0,2)="l2";  
      godley2.cell(0,4)="e2";
      g2->update();
      
      cols=matchingTableColumns(*g1,GodleyAssetClass::asset);
      CHECK_EQUAL(1, cols.size());
      CHECK_EQUAL("l2", *cols.begin());

      cols=matchingTableColumns(*g1,GodleyAssetClass::liability);
      CHECK_EQUAL(0, cols.size());

      cols=matchingTableColumns(*g1,GodleyAssetClass::equity);
      CHECK_EQUAL(1, cols.size());
      CHECK_EQUAL("a1", *cols.begin());

      cols=matchingTableColumns(*g2,GodleyAssetClass::equity);
      CHECK_EQUAL(0, cols.size());

      cols=matchingTableColumns(*g3,GodleyAssetClass::asset);
      CHECK_EQUAL(1, cols.size());
      CHECK_EQUAL("l2", *cols.begin());

      cols=matchingTableColumns(*g3,GodleyAssetClass::liability);
      CHECK_EQUAL(1, cols.size());
      CHECK_EQUAL("a1", *cols.begin());

      

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
      CHECK_EQUAL(3,godley2.rows());
      balanceDuplicateColumns(*g1, 2); 
      godley2.exportToCSV("after.csv");
      // two rows should have been added, and one deleted
      CHECK_EQUAL(4,godley2.rows());
      CHECK_EQUAL("a",trimWS(godley2.cell(1,1)));
      CHECK_EQUAL("b",godley2.cell(2,1));
      CHECK_EQUAL("c",godley2.cell(3,1));
      CHECK_EQUAL("row3",godley2.cell(3,0)); // check label transferred
    }

  TEST_FIXTURE(TestFixture,bug1157)
    {
      auto g1=new GodleyIcon; model->addItem(g1);
      auto g2=new GodleyIcon; model->addItem(g2);
      GodleyTable& godley1=g1->table;
      GodleyTable& godley2=g2->table;
      godley1.resize(3,2);
      godley2.resize(3,2);

      godley1._assetClass(1,GodleyAssetClass::asset);  
      godley2._assetClass(1,GodleyAssetClass::liability);  

      godley1.cell(0,1)="a";  
      godley1.cell(2,0)="xx";
      godley1.cell(2,1)="-b";
      g1->update();
      
      godley2.cell(0,1)="a";  
      godley2.cell(2,0)="yy";  
      godley2.cell(2,1)="-b";
      g2->update();

      CHECK_EQUAL(3,godley2.rows());
      CHECK_EQUAL("yy",godley2.cell(2,0));
      CHECK_EQUAL("-b",godley2.cell(2,1));
      godley2.exportToCSV("before.csv");

      godley1.cell(2,1)="b";
      balanceDuplicateColumns(*g1, 1); 
      godley2.exportToCSV("after.csv");
      CHECK_EQUAL(4,godley2.rows());
      CHECK_EQUAL("yy",godley2.cell(2,0)); // row label should be updated
      CHECK_EQUAL("-b",godley2.cell(2,1));  // sign should be transferred
      CHECK_EQUAL("xx",godley2.cell(3,0)); // row label should be updated
      CHECK_EQUAL("2b",godley2.cell(3,1));  // sign should be transferred
    }

  TEST_FIXTURE(TestFixture,importDuplicateColumn)
    {
      auto g1=new GodleyIcon; model->addItem(g1);
      auto g2=new GodleyIcon; model->addItem(g2);
      GodleyTable& godley1=g1->table;
      GodleyTable& godley2=g2->table;
      godley1.resize(4,3);
      godley2.resize(2,2);

      godley1._assetClass(1,GodleyAssetClass::asset);  
      godley2._assetClass(1,GodleyAssetClass::liability);

      godley1.cell(0,1)="a1";  
      godley1.cell(2,0)="row1";
      godley1.cell(2,1)="a";
      godley1.cell(3,0)="row2";
      godley1.cell(3,1)="b";

      godley2.cell(0,1)="a1";
      importDuplicateColumn(godley2, 1);
      CHECK_EQUAL("row1",godley2.cell(2,0));
      CHECK_EQUAL("a",godley2.cell(2,1));
      CHECK_EQUAL("row2",godley2.cell(3,0));
      CHECK_EQUAL("b",godley2.cell(3,1));

      // move cell between columns
      g1->moveCell({2,1,2,2});
      CHECK_EQUAL("a",godley1.cell(2,2));
      CHECK_EQUAL("",godley1.cell(2,1));
      CHECK_EQUAL("",godley2.cell(2,1));

      g1->moveCell({2,2,3,2});
      CHECK_EQUAL("a",godley1.cell(3,2));
      CHECK_EQUAL("",godley1.cell(2,2));
    }

  TEST_FIXTURE(TestFixture,godleyRowSums)
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
      CHECK_EQUAL("a-2b+c",godley1.rowSum(1));
    }

  TEST_FIXTURE(TestFixture,godleyMoveRowCol)
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
      godley1._assetClass(1,GodleyAssetClass::asset);
      godley1._assetClass(2,GodleyAssetClass::asset);
      godley1.moveRow(1,1);
      CHECK_EQUAL("a",godley1.cell(2,1));
      CHECK_EQUAL("2b",godley1.cell(2,2));
      CHECK_EQUAL("a1",godley1.cell(1,1));
      CHECK_EQUAL("2b1",godley1.cell(1,2));
      godley1.moveCol(1,1);
      CHECK_EQUAL("a",godley1.cell(2,2));
      CHECK_EQUAL("2b",godley1.cell(2,1));
      CHECK_EQUAL("a1",godley1.cell(1,2));
      CHECK_EQUAL("2b1",godley1.cell(1,1));
    }

  TEST_FIXTURE(TestFixture,godleyNameUnique)
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
      CHECK(godley1.title!=godley2.title);
      CHECK(godley2.title!=godley3.title);
      CHECK(godley1.title!=godley3.title);
    }

    TEST_FIXTURE(TestFixture,clearInitCond)
    {
      auto g1=new GodleyIcon; model->addItem(g1);
      CHECK(g1->table.initialConditionRow(1));
      // to clear a cell, it first needs to contain something
      g1->setCell(1,1,"1");
      g1->setCell(1,1,"");
      CHECK_EQUAL("0",g1->table.cell(1,1));
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
    
    TEST_FIXTURE(TestFixture,varNameScoping)
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
      CHECK(a0->valueId()!=a1->valueId());
      CHECK(a0->valueId()!=a3->valueId());
      CHECK_EQUAL(a1->valueId(),a2->valueId());
      CHECK(a1->valueId()!=a3->valueId());

      CHECK_EQUAL(b0->valueId(),b1->valueId());
      CHECK_EQUAL(b0->valueId(),b3->valueId());
      CHECK(b2->valueId()!=b3->valueId());

      // check display values:
      CHECK_EQUAL("a",a0->name());
      CHECK_EQUAL("a",a1->name());
      CHECK_EQUAL("a",a2->name());
      CHECK_EQUAL("a",a3->name());
      CHECK_EQUAL("b",b0->name());
      CHECK_EQUAL("b",b1->name());
      CHECK_EQUAL("b",b2->name());
      CHECK_EQUAL("b",b3->name());
      CHECK_EQUAL("c",c->name());

      // now check move rules
      g0->addItem(a1);
      CHECK_EQUAL("a",a1->name());

      g2->addItem(a1);
      CHECK_EQUAL("a",a1->name());

      g1->addItem(a1);
      CHECK_EQUAL("a",a1->name());

      g1->addItem(c);
      CHECK_EQUAL("c",c->name());

    }

    TEST_FIXTURE(TestFixture,cantMultiplyDefineVars)
    {
      VariablePtr f1(VariableType::flow,"foo");
      VariablePtr f2(VariableType::flow,"foo");
      OperationPtr op(OperationType::time);
      model->addItem(f1);
      model->addItem(f2);
      model->addItem(op);
      model->addWire(op->ports(0),f1->ports(1));
      model->addWire(op->ports(0),f2->ports(1));
      CHECK_EQUAL(1, f1->ports(1).lock()->wires().size());
      CHECK_EQUAL(0, f2->ports(1).lock()->wires().size());
    }

    TEST_FIXTURE(TestFixture,MultiplyDefinedVarsThrowsOnReset)
    {
      VariablePtr f1(VariableType::flow,"foo");
      VariablePtr f2(VariableType::flow,"foo");
      OperationPtr op(OperationType::time);
      model->addItem(f1);
      model->addItem(f2);
      model->addItem(op);
      model->addWire(op->ports(0),f1->ports(1));
      model->addWire(std::make_shared<Wire>(op->ports(0),f2->ports(1)));
      CHECK_EQUAL(1, f1->ports(1).lock()->wires().size());
      // We've tricked the system into having a multiply defined variable
      CHECK_EQUAL(1, f2->ports(1).lock()->wires().size());
      CHECK_THROW(reset(), std::exception);
    }

    TEST_FIXTURE(TestFixture, RemoveDefinitionsFromPastedVars)
      {
        VariablePtr a(VariableType::flow,"a");
        VariablePtr b(VariableType::flow,"b");
        model->addItem(a); model->addItem(b);
        model->addWire(a->ports(0), b->ports(1));
        canvas.selection.ensureItemInserted(a);
        canvas.selection.ensureItemInserted(b);
        CHECK_EQUAL(1,canvas.selection.numWires());
        copy();
        paste();
        CHECK_EQUAL(4, model->items.size());
        // ensure extra wire is not copied
        CHECK_EQUAL(1, model->wires.size());
        // check that b's definition remains as before
        CHECK(definingVar(":b")==b);
      }

    TEST_FIXTURE(TestFixture, DefinitionPasted)
      {
        VariablePtr a(VariableType::flow,"a");
        VariablePtr b(VariableType::flow,"b");
        model->addItem(a); model->addItem(b);
        model->addWire(a->ports(0), b->ports(1));
        canvas.selection.ensureItemInserted(a);
        canvas.selection.ensureItemInserted(b);
        copy();
        model->deleteItem(*b);
        CHECK_EQUAL(0, model->wires.size());
        paste();
        // ensure extra wire is not copied
        CHECK_EQUAL(1, model->wires.size());
        // check that b's definition is now the copied var
        CHECK(definingVar(":b")!=b);
      }
    
    TEST_FIXTURE(TestFixture, PastedIntOpShowsMessage)
      {
        auto intOp=make_shared<IntOp>();
        intOp->description("foo");
        model->addItem(intOp);
        CHECK_EQUAL(2,model->items.size());
        canvas.selection.ensureItemInserted(intOp);
        copy();
        paste();
        CHECK(savedMessage.size()); // check that pop message is written
      }

    TEST_FIXTURE(TestFixture, RetypePastedIntegralVariable)
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
        CHECK_EQUAL(2,model->items.size());
        CHECK(model->items[1]->variableCast());
        CHECK_EQUAL(VariableType::flow, model->items[1]->variableCast()->type());
        CHECK_EQUAL(clonedIntVar->name(), model->items[1]->variableCast()->name());
      }
    
    TEST_FIXTURE(TestFixture, cut)
      {
        auto a=model->addItem(new Variable<VariableType::flow>("a"));
        auto integ=new IntOp;
        model->addItem(integ);
        auto g=model->addGroup(new Group);
        g->addItem(new Variable<VariableType::flow>("a1"));
        CHECK_EQUAL(4,model->numItems());
        CHECK_EQUAL(1,model->numGroups());

        canvas.selection.ensureItemInserted(a);
        CHECK_EQUAL(1,canvas.selection.numItems());
        canvas.selection.toggleItemMembership(integ->intVar);
        CHECK_EQUAL(3,canvas.selection.numItems()); // both integral and intVar must be inserted
        canvas.selection.toggleItemMembership(model->findItem(*integ));
        CHECK_EQUAL(1,canvas.selection.numItems());
        canvas.selection.items.push_back(integ->intVar);
        CHECK_EQUAL(2,canvas.selection.numItems());
        canvas.selection.ensureGroupInserted(g);
        CHECK_EQUAL(3,canvas.selection.numItems());
        CHECK_EQUAL(1,canvas.selection.numGroups());

        a.reset(); g.reset(); // prevent triggering assertion filaure in cut()
        cut();
        CHECK_EQUAL(2,model->numItems()); //intVar should not be deleted
        CHECK_EQUAL(0,model->numGroups());
      }

    TEST_FIXTURE(TestFixture,renameAll)
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
      CHECK_EQUAL("newC",stockVar->name());
      CHECK_EQUAL("newC",godley.cell(0,1));
      gi->update();

      // renaming godley column should rename canvas stock vars
      GodleyTableEditor& ged=gi->editor;
      godley.cell(0,1)="c";
      ged.selectedRow=0;
      ged.selectedCol=1;
      godley.savedText="newC";
      ged.update();
      CHECK_EQUAL("c",stockVar->name());
      CHECK_EQUAL("c",godley.cell(0,1));

      // renaming just the canvas variable should change  it's type
      canvas.item=stockVar;
      if (auto v=canvas.item->variableCast())
        {
          canvas.renameItem("foo");
          CHECK(canvas.item && canvas.item->variableCast());
          if (auto v1=canvas.item->variableCast())
            {
              CHECK_EQUAL("foo",v1->name());
              CHECK_EQUAL(VariableType::flow, v1->type());
              CHECK(model->findItem(*canvas.item));
            }
        }
      CHECK_EQUAL("c",godley.cell(0,1));
    }

}
