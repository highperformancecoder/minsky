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
#include <ecolab_epilogue.h>
#include <UnitTest++/UnitTest++.h>
#include <gsl/gsl_integration.h>
using namespace minsky;

namespace
{
  struct TestFixture: public Minsky
  {
    LocalMinsky lm;
    TestFixture(): lm(*this)
    {
    }
    using Minsky::addWire;
    void addWire(const Wire& w)
    {
      CHECK(ports.count(w.from));
      CHECK(ports.count(w.to));
      CHECK(!ports[w.from].input() && ports[w.to].input());
      CHECK(variables.addWire(w.from, w.to));
      PortManager::addWire(w);
    }
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
      GodleyTable& godley=godleyItems[0].table;
      godley.resize(3,4);
      godley.cell(0,1)="c";
      godley.cell(0,2)="d";
      godley.cell(0,3)="e";
      godley.cell(2,1)="a";
      godley.cell(2,2)="b";
      godley.cell(2,3)="f";
      godleyItems[0].update();

      // build a table of variables - names will be unique at this stage
      map<string, VariablePtr> var;
      for (VariablePtr& v: variables)
        {
          var[v->name()]=v;
        }

      CHECK(var["a"]->lhs());
      CHECK(var["b"]->lhs());
      CHECK(!var["c"]->lhs());
      CHECK(!var["d"]->lhs());
      CHECK(!var["e"]->lhs());
      CHECK(var["f"]->lhs());

      int addOp=addOperation("add"); 
      CHECK_EQUAL(3, operations[addOp]->numPorts());
      int intOp=addOperation("integrate"); 
      CHECK_EQUAL(2, operations[intOp]->numPorts());
      int mulOp=addOperation("multiply"); 
      CHECK_EQUAL(3, operations[mulOp]->numPorts());
 
      addWire(Wire(var["e"]->outPort(), var["f"]->inPort()));
      addWire(Wire(var["c"]->outPort(), operations[addOp]->ports()[1]));
      addWire(Wire(var["d"]->outPort(), operations[addOp]->ports()[2]));;
      addWire(Wire(operations[addOp]->ports()[0], operations[intOp]->ports()[1]));
      addWire(Wire(operations[intOp]->ports()[0], var["a"]->inPort()));
      addWire(Wire(operations[intOp]->ports()[0], operations[mulOp]->ports()[1]));
      addWire(Wire(var["e"]->outPort(), operations[mulOp]->ports()[2]));
      addWire(Wire(operations[mulOp]->ports()[0], var["b"]->inPort()));

      for (PortManager::Wires::const_iterator w=wires.begin(); w!=wires.end(); ++w)
        {
          CHECK(!ports[w->from].input());
          CHECK(ports[w->to].input());
        }

      save("constructEquationsEx1.xml");

      CHECK(!cycleCheck());

      constructEquations();

      CHECK_EQUAL(4, integrals.size());
      // check that integral  stock vars aren't flowVars
      int nakedIntegral=-1;
      for (size_t i=0; i<integrals.size(); ++i)
        {
          if (integrals[i].stock.name==":int1")
            nakedIntegral=i;
          CHECK(!integrals[i].stock.isFlowVar());
        }
      CHECK(nakedIntegral>=0);

      var["c"]->value(0.1);
      var["d"]->value(0.2);
      var["e"]->value(0.3);

      step();

      CHECK_CLOSE(var["c"]->value()+var["d"]->value(), integrals[nakedIntegral].input.value(), 1e-4);
      CHECK_CLOSE(integrals[nakedIntegral].stock.value(), var["a"]->value(), 1e-4);
      CHECK_CLOSE(integrals[nakedIntegral].stock.value()*var["e"]->value(), var["b"]->value(), 1e-4);
      CHECK_CLOSE(var["e"]->value(), var["f"]->value(), 1e-4);

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
      newVariable("g");
      newVariable("h");

      // build a table of variables - names will be unique at this stage
      map<string, VariablePtr> var;
      for (VariablePtr& v: variables)
        {
          var[v->name()]=v;
        }
 
      int op4=addOperation("constant");
      CHECK_EQUAL(1, operations[op4]->numPorts());
      int op5=addOperation("constant");
      CHECK_EQUAL(1, operations[op5]->numPorts());
      int op6=addOperation("add");
      CHECK_EQUAL(3, operations[op6]->numPorts());

      ecolab::array<float> coords(4,0);
 
      int wire8=addWire(operations[op4]->ports()[0], var["g"]->inPort(), coords);
      int wire9=addWire(operations[op4]->ports()[0], operations[op6]->ports()[1], coords);
      int wire10=addWire(operations[op5]->ports()[0], operations[op6]->ports()[2], coords);
      int wire11=addWire(operations[op6]->ports()[0], var["h"]->inPort(), coords);
 
      for (PortManager::Wires::const_iterator w=wires.begin(); w!=wires.end(); ++w)
        {
          CHECK(!ports[w->from].input());
          CHECK(ports[w->to].input());
        }
 
      CHECK(dynamic_cast<Constant*>(operations[op4].get()));
      if (Constant* c=dynamic_cast<Constant*>(operations[op4].get()))
        {
          c->value=0.1;
          c->description="0.1";
        }
      CHECK(dynamic_cast<Constant*>(operations[op5].get()));
      if (Constant* c=dynamic_cast<Constant*>(operations[op5].get()))
        {
          c->value=0.2;
          c->description="0.2";
        }

      constructEquations();
      step();

      CHECK_CLOSE(0.1, var["g"]->value(), 1e-4);
      CHECK_CLOSE(0.3, var["h"]->value(), 1e-4);

    }

  TEST_FIXTURE(TestFixture,godleyEval)
    {
      GodleyTable& godley=godleyItems[0].table;
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
      godleyItems[0].update();
 
      variables.values[":a"].init="5";
 
      garbageCollect();
      reset();
      CHECK_EQUAL(10,variables.values[":c"].value());
      CHECK_EQUAL(20,variables.values[":d"].value());
      CHECK_EQUAL(30,variables.values[":e"].value());
      CHECK_EQUAL(5,variables.values[":a"].value());
      for (size_t i=0; i<stockVars.size(); ++i)
        stockVars[i]=0;
     
      evalGodley.eval(&stockVars[0], &flowVars[0]);
      CHECK_EQUAL(5,variables.values[":c"].value());
      CHECK_EQUAL(-5,variables.values[":d"].value());
      CHECK_EQUAL(0,variables.values[":e"].value());
      CHECK_EQUAL(5,variables.values[":a"].value());
    
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
      GodleyTable& godley=godleyItems[0].table;
      garbageCollect();
 
      godley.resize(3,4);
      godley.cell(0,1)="c";
      godley.cell(0,2)="d";
      godley.cell(0,3)="e";
      godley.cell(2,1)="a";
      godley.cell(2,2)="b";
      godley.cell(2,3)="f";
      godleyItems[0].update();
 
      // build a table of variables - names will be unique at this stage
      map<string, VariablePtr> var;
      for (VariablePtr& v: variables)
        {
          var[v->name()]=v;
        }
 
      operations[1]=OperationPtr(OperationType::add);
      CHECK_EQUAL(3, operations[1]->numPorts());
      operations[2]=OperationPtr(OperationType::integrate);
      CHECK_EQUAL(2, operations[2]->numPorts());
      operations[3]=OperationPtr(OperationType::multiply);
      CHECK_EQUAL(3, operations[3]->numPorts());
  
      addWire(Wire(var["e"]->outPort(), var["f"]->inPort()));
      addWire(Wire(var["c"]->outPort(), operations[1]->ports()[1]));
      addWire(Wire(var["d"]->outPort(), operations[1]->ports()[2]));
      addWire(Wire(operations[1]->ports()[0], operations[2]->ports()[1]));
      addWire(Wire(operations[2]->ports()[0], var["a"]->inPort()));
      addWire(Wire(operations[2]->ports()[0], operations[3]->ports()[1]));
      addWire(Wire(var["e"]->outPort(), operations[3]->ports()[2]));
      addWire(Wire(operations[3]->ports()[0], var["b"]->inPort()));
 
      CHECK(!cycleCheck());
      reset();
      vector<double> j(stockVars.size()*stockVars.size());
      Matrix jac(stockVars.size(),&j[0]);
 
      VariableValue& c=variables.values[":c"];   c=100;
      VariableValue& d=variables.values[":d"];   d=200;
      VariableValue& e=variables.values[":e"];   e=300;
      double& x=stockVars.back();   x=0; // temporary variable storing \int c+d
 
      CHECK_EQUAL(4, stockVars.size());
 
      save("derivative.mky");
      jacobian(jac,t,&stockVars[0]);
   
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
      operations[1]=OperationPtr(OperationType::constant);
      operations[2]=OperationPtr(OperationType::integrate);
      IntOp* intOp=dynamic_cast<IntOp*>(operations[2].get());
      CHECK(intOp);
      intOp->description("output");
      int var=intOp->intVarID();
      addWire(Wire(operations[1]->ports()[0], operations[2]->ports()[1]));
      //   addWire(Wire(operations[2]->ports()[0], variables[var]->inPort()));
 
      variables.makeConsistent();
      constructEquations();
      double& value = dynamic_cast<Constant*>(operations[1].get())->value;
      value=10;
      nSteps=1;
      step();
      CHECK_CLOSE(value*t, integrals[0].stock.value(), 1e-5);
      CHECK_CLOSE(integrals[0].stock.value(), variables.values[":output"].value(), 1e-5);
 
      // now integrate the linear function
      operations[3]=OperationPtr(OperationType::integrate);
      addWire(Wire(operations[2]->ports()[0], operations[3]->ports()[1]));
      reset();
      step();
      //   CHECK_CLOSE(0.5*value*t*t, integrals[1].stock.value(), 1e-5);
      intOp=dynamic_cast<IntOp*>(operations[3].get());
      CHECK(intOp);
      CHECK_CLOSE(0.5*value*t*t, intOp->getIntVar()->value(), 1e-5);
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
      operations[1]=OperationPtr(OperationType::add);
      int w=variables.addVariable(VariablePtr(VariableType::flow,"w"));
      int a=variables.addVariable(VariablePtr(VariableType::flow,"a"));
      CHECK(variables.addWire(operations[1]->ports()[0], variables[w]->inPort()));
      CHECK(variables.addWire(variables[w]->outPort(), operations[1]->ports()[1]));
      PortManager::addWire(Wire(operations[1]->ports()[0], variables[w]->inPort()));
      PortManager::addWire(Wire(variables[w]->outPort(), operations[1]->ports()[1]));
      PortManager::addWire(Wire(variables[a]->outPort(), operations[1]->ports()[2]));

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
      operations[1]=OperationPtr(OperationType::integrate);
      operations[2]=OperationPtr(OperationType::multiply);
      int a=variables.addVariable(VariablePtr(VariableType::flow,"a"));
      CHECK(variables.addWire(operations[1]->ports()[0], operations[2]->ports()[1]));
      CHECK(variables.addWire(operations[2]->ports()[0], operations[1]->ports()[1]));
      CHECK(variables.addWire(variables[a]->outPort(), operations[2]->ports()[2]));

      PortManager::addWire(Wire(operations[1]->ports()[0], operations[2]->ports()[1]));
      PortManager::addWire(Wire(operations[2]->ports()[0], operations[1]->ports()[1]));
      PortManager::addWire(Wire(variables[a]->outPort(), operations[2]->ports()[2]));

      CHECK(!cycleCheck());
      constructEquations();
    }

  TEST_FIXTURE(TestFixture,godleyIconVariableOrder)
    {
      GodleyIcon& g=godleyItems[0];
      g.table.dimension(3,4);
      g.table.cell(0,1)="a1";
      g.table.cell(0,2)="z2";
      g.table.cell(0,3)="d1";
      g.table.cell(2,1)="b3";
      g.table.cell(2,2)="x1";
      g.table.cell(2,3)="h2";
      g.update();
  
      assert(g.stockVars.size()==g.table.cols()-1 && g.flowVars.size()==g.table.cols()-1);
      for (size_t i=1; i<g.table.cols(); ++i)
        {
          CHECK_EQUAL(g.table.cell(0,i), g.stockVars[i-1]->name());
          CHECK_EQUAL(g.table.cell(2,i), g.flowVars[i-1]->name());
        }
    }

  /*
    a --
    b / \
    + -- c
  */
   

  TEST_FIXTURE(TestFixture,multiVariableInputsAdd)
    {
      VariablePtr varA = variables[variables.newVariable("a", VariableType::flow)];
      VariablePtr varB = variables[variables.newVariable("b", VariableType::flow)];
      VariablePtr varC = variables[variables.newVariable("c", VariableType::flow)];
      variables.values[":a"].init="0.1";
      variables.values[":b"].init="0.2";

      OperationPtr& intOp = operations[0]=OperationPtr(OperationType::integrate); //enables equations to step
  

      OperationPtr& op=operations[1]=OperationPtr(OperationType::add);

      addWire(Wire(varA->outPort(), op->ports()[1]));
      addWire(Wire(varB->outPort(), op->ports()[1]));
      addWire(Wire(op->ports()[0], varC->inPort()));
      addWire(Wire(varC->outPort(), intOp->ports()[1]));

      // move stuff around to make layout a bit better
      varA->moveTo(10,100);
      varB->moveTo(10,200);
      varC->moveTo(100,150);
      op->moveTo(50,150);
      intOp->moveTo(150,150);

      save("multiVariableInputs.mky");

      constructEquations();
      step();
      CHECK_CLOSE(0.3, variables.values[":c"].value(), 1e-5);
    }

  TEST_FIXTURE(TestFixture,multiVariableInputsSubtract)
    {
      VariablePtr varA = variables[variables.newVariable("a", VariableType::flow)];
      VariablePtr varB = variables[variables.newVariable("b", VariableType::flow)];
      VariablePtr varC = variables[variables.newVariable("c", VariableType::flow)];
      variables.values[":a"].init="0.1";
      variables.values[":b"].init="0.2";

      OperationPtr& intOp = operations[0]=OperationPtr(OperationType::integrate); //enables equations to step
  

      OperationPtr& op=operations[1]=OperationPtr(OperationType::subtract);

      addWire(Wire(varA->outPort(), op->ports()[2]));
      addWire(Wire(varB->outPort(), op->ports()[2]));
      addWire(Wire(op->ports()[0], varC->inPort()));
      addWire(Wire(varC->outPort(), intOp->ports()[1]));

      // move stuff around to make layout a bit better
      varA->moveTo(10,100);
      varB->moveTo(10,200);
      varC->moveTo(100,150);
      op->moveTo(50,150);
      intOp->moveTo(150,150);

      //Save("multiVariableInputs.mky");

      constructEquations();
      step();
      CHECK_CLOSE(-0.3, variables.values[":c"].value(), 1e-5);
    }

  TEST_FIXTURE(TestFixture,multiVariableInputsMultiply)
    {
      VariablePtr varA = variables[variables.newVariable("a", VariableType::flow)];
      VariablePtr varB = variables[variables.newVariable("b", VariableType::flow)];
      VariablePtr varC = variables[variables.newVariable("c", VariableType::flow)];
      variables.values[":a"].init="0.1";
      variables.values[":b"].init="0.2";

      OperationPtr& intOp = operations[0]=OperationPtr(OperationType::integrate); //enables equations to step
  

      OperationPtr& op=operations[1]=OperationPtr(OperationType::multiply);

      addWire(Wire(varA->outPort(), op->ports()[1]));
      addWire(Wire(varB->outPort(), op->ports()[1]));
      addWire(Wire(op->ports()[0], varC->inPort()));
      addWire(Wire(varC->outPort(), intOp->ports()[1]));

      // move stuff around to make layout a bit better
      varA->moveTo(10,100);
      varB->moveTo(10,200);
      varC->moveTo(100,150);
      op->moveTo(50,150);
      intOp->moveTo(150,150);

      //Save("multiVariableInputs.mky");

      constructEquations();
      step();
      CHECK_CLOSE(0.02, variables.values[":c"].value(), 1e-5);
    }

  TEST_FIXTURE(TestFixture,multiVariableInputsDivide)
    {
      VariablePtr varA = variables[variables.newVariable("a", VariableType::flow)];
      VariablePtr varB = variables[variables.newVariable("b", VariableType::flow)];
      VariablePtr varC = variables[variables.newVariable("c", VariableType::flow)];
      variables.values[":a"].init="0.1";
      variables.values[":b"].init="0.2";

      OperationPtr& intOp = operations[0]=OperationPtr(OperationType::integrate); //enables equations to step

      OperationPtr& op=operations[1]=OperationPtr(OperationType::divide);

      addWire(Wire(varA->outPort(), op->ports()[2]));
      addWire(Wire(varB->outPort(), op->ports()[2]));
      addWire(Wire(op->ports()[0], varC->inPort()));
      addWire(Wire(varC->outPort(), intOp->ports()[1]));

      // move stuff around to make layout a bit better
      varA->moveTo(10,100);
      varB->moveTo(10,200);
      varC->moveTo(100,150);
      op->moveTo(50,150);
      intOp->moveTo(150,150);

      //Save("multiVariableInputs.mky");

      constructEquations();
      step();
      CHECK_CLOSE(50, variables.values[":c"].value(), 1e-5);
    }

  // instantiate all operations and variables to ensure that definitions
  // have been provided for all virtual methods
  TEST(checkAllOpsDefined)
  {
    using namespace MathDAG;
    for (int o=0; o<OperationType::numOps; ++o)
      {
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
      EvalOpPtr& e=*static_cast<EvalOpPtr*>(params);
      return e->d1(x,0);
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
    struct FixedArg1: public EvalOpBase
    {
      EvalOpPtr op;
      double arg1;
      FixedArg1(OperationType::Type op, double arg): op(op), arg1(arg) {}

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

    struct FixedArg2: public EvalOpBase
    {
      EvalOpPtr op;
      double arg2;
      FixedArg2(OperationType::Type op, double arg): op(op), arg2(arg) {}

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

  // Tests derivative definitions of one argument functions by
  // integration over [0.5,1], using fundamental theorem of calculus.
  // Hopefully, this range will avoid any singularities in the derivative
  TEST(checkDerivatives)
  {
    gsl_integration_workspace* ws=gsl_integration_workspace_alloc(1000);
    for (int op=0; op<OperationType::numOps; ++op)
      {
        // derivatives should not have EvalOps instantiated
        // other operation cannot be tested this way
        switch (op)
          {
          case OperationType::differentiate:
          case OperationType::and_:
          case OperationType::or_:
          case OperationType::not_:
            continue;
          default:
            break;
          }
        cout << "checking "<<OperationType::typeName(op)<<endl;
        EvalOpPtr evalOp=EvalOpPtr(OperationType::Type(op));
        if (evalOp->numArgs()==1 && evalOp->type()!=OperationType::integrate)
          {
            testUnOp(evalOp, ws);
          }
        else if (evalOp->numArgs()==2)
          {
            evalOp.reset(new FixedArg1(OperationType::Type(op), 2.0));
            testUnOp(evalOp, ws);
            evalOp.reset(new FixedArg2(OperationType::Type(op), 2.0));
            testUnOp(evalOp, ws);
          }
      }

    gsl_integration_workspace_free(ws);
  }

  
  TEST_FIXTURE(TestFixture,multiGodleyRules)
    {
      GodleyTable& godley1=godleyItems[1].table;
      GodleyTable& godley2=godleyItems[2].table;
      GodleyTable& godley3=godleyItems[3].table;
      godley1.resize(2,1);
      godley2.resize(2,1);
      godley3.resize(2,1);

      godley1.doubleEntryCompliant=true;
      godley2.doubleEntryCompliant=true;
      godley3.doubleEntryCompliant=true;

      // empty godleys should be fine
      evalGodley.initialiseGodleys(makeGodleyIt(godleyItems.begin()), 
                                   makeGodleyIt(godleyItems.end()), variables.values);

      godley1.resize(4,4);
      godley1.cell(0,1)=":hello"; godley1._assetClass(1, GodleyAssetClass::asset);
      godley1.cell(0,2)=":foo"; godley1._assetClass(2, GodleyAssetClass::equity);
      godley1.cell(0,3)=":bar"; godley1._assetClass(3, GodleyAssetClass::liability);

      // should still be no problem
      evalGodley.initialiseGodleys(makeGodleyIt(godleyItems.begin()), 
                                   makeGodleyIt(godleyItems.end()), variables.values);

      godley2.resize(4,4);
      godley2.cell(0,1)=":hello2"; godley2._assetClass(1, GodleyAssetClass::asset);
      godley2.cell(0,2)=":foo2"; godley2._assetClass(2, GodleyAssetClass::equity);
      godley2.cell(0,3)=":bar2"; godley2._assetClass(3, GodleyAssetClass::liability);

      godley3.resize(4,4);
      godley3.cell(0,1)=":hello3"; godley3._assetClass(1, GodleyAssetClass::asset);
      godley3.cell(0,2)=":foo3"; godley3._assetClass(2, GodleyAssetClass::equity);
      godley3.cell(0,3)=":bar3"; godley3._assetClass(3, GodleyAssetClass::liability);
 
      // should be no problem - all columns are different
      evalGodley.initialiseGodleys(makeGodleyIt(godleyItems.begin()), 
                                   makeGodleyIt(godleyItems.end()), variables.values);

      godley3.cell(0,2)=":foo2"; 

      // two incompatible columns
      CHECK_THROW(evalGodley.initialiseGodleys(makeGodleyIt(godleyItems.begin()), 
                                               makeGodleyIt(godleyItems.end()), variables.values), ecolab::error);

  
      godley3.cell(0,2)=":foo3"; 
      godley3.cell(0,1)=":hello2"; 

      // two incompatible columns asset columns
      CHECK_THROW(evalGodley.initialiseGodleys(makeGodleyIt(godleyItems.begin()), 
                                               makeGodleyIt(godleyItems.end()), variables.values), ecolab::error);

      // should now be a compatible asset/liability pair
      godley3._assetClass(1, GodleyAssetClass::liability);
      evalGodley.initialiseGodleys(makeGodleyIt(godleyItems.begin()), 
                                   makeGodleyIt(godleyItems.end()), variables.values);
  

      // add in another asset/liability pair
      godley1.cell(0,3)=":bar3"; godley1._assetClass(3, GodleyAssetClass::asset);
      evalGodley.initialiseGodleys(makeGodleyIt(godleyItems.begin()), 
                                   makeGodleyIt(godleyItems.end()), variables.values);

      // now conflict that pair
      godley2.cell(0,3)=":bar3";
      CHECK_THROW(evalGodley.initialiseGodleys(makeGodleyIt(godleyItems.begin()), 
                                               makeGodleyIt(godleyItems.end()), variables.values), ecolab::error);
      godley2.cell(0,3)=":bar2";
  
      // now add some flow variables and check those
      godley2.cell(2,1)="2:a";
      godley3.cell(2,1)="-:a";
      godley3.cell(3,1)="-:a";
      variables.values[":a"]=VariableValue(VariableType::flow).allocValue();
      variables.values[":hello2"]=VariableValue(VariableType::stock).allocValue();

      evalGodley.initialiseGodleys(makeGodleyIt(godleyItems.begin()), 
                                   makeGodleyIt(godleyItems.end()), variables.values);

      godley3.cell(3,1)="";
      CHECK_THROW(evalGodley.initialiseGodleys(makeGodleyIt(godleyItems.begin()), 
                                               makeGodleyIt(godleyItems.end()), variables.values), ecolab::error);
 

    }

  TEST_FIXTURE(TestFixture,matchingTableColumns)
    {
      GodleyTable& godley1=godleyItems[1].table;
      GodleyTable& godley2=godleyItems[2].table;
      GodleyTable& godley3=godleyItems[3].table;
      godley1.resize(6,3);
      godley2.resize(3,3);
      godley3.resize(2,3);

      godley1._assetClass(1,GodleyAssetClass::asset);  
      godley1._assetClass(2,GodleyAssetClass::liability);  
      godley2._assetClass(1,GodleyAssetClass::asset);  
      godley2._assetClass(2,GodleyAssetClass::liability);
      godley3._assetClass(1,GodleyAssetClass::asset);  
      godley3._assetClass(2,GodleyAssetClass::liability);

      godley1.cell(0,1)="a1";  
      godley1.cell(0,2)="l1";

      set<string> cols=matchingTableColumns(2,GodleyAssetClass::asset);
      CHECK_EQUAL(1, cols.size());
      CHECK_EQUAL("l1", *cols.begin());

      godley2.cell(0,1)="l1";  
      godley2.cell(0,2)="l2";  

      cols=matchingTableColumns(1,GodleyAssetClass::asset);
      CHECK_EQUAL(1, cols.size());
      CHECK_EQUAL("l2", *cols.begin());

      cols=matchingTableColumns(1,GodleyAssetClass::liability);
      CHECK_EQUAL(0, cols.size());

      cols=matchingTableColumns(3,GodleyAssetClass::asset);
      CHECK_EQUAL(1, cols.size());
      CHECK_EQUAL("l2", *cols.begin());

      cols=matchingTableColumns(3,GodleyAssetClass::liability);
      CHECK_EQUAL(1, cols.size());
      CHECK_EQUAL("a1", *cols.begin());

      variables.newVariable(":a",VariableType::flow);
      variables.newVariable(":b",VariableType::flow);
      variables.newVariable(":c",VariableType::flow);
      variables.newVariable(":d",VariableType::flow);
      variables.newVariable(":e",VariableType::flow);

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
      balanceDuplicateColumns(godleyItems[1], 2);
      // two rows should have been added
      CHECK_EQUAL(5,godley2.rows());
      CHECK_EQUAL("-a",trimWS(godley2.cell(1,1)));
      CHECK(trimWS(godley2.cell(2,1)).empty()); // "d" is deleted
      CHECK_EQUAL("-b",godley2.cell(3,1));
      CHECK_EQUAL("-c",godley2.cell(4,1));
      CHECK_EQUAL("row3",godley2.cell(4,0)); // check label transferred
    }

  TEST_FIXTURE(TestFixture,importDuplicateColumn)
    {
      GodleyTable& godley1=godleyItems[1].table;
      GodleyTable& godley2=godleyItems[2].table;
      godley1.resize(4,2);
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
      CHECK_EQUAL("-a",godley2.cell(2,1));
      CHECK_EQUAL("row2",godley2.cell(3,0));
      CHECK_EQUAL("-b",godley2.cell(3,1));

      // move cell between columns
      godleyItems[1].moveCell(2,1,2,2);
      CHECK_EQUAL("a",godley1.cell(2,2));
      CHECK_EQUAL("",godley1.cell(2,1));
      CHECK_EQUAL("",godley2.cell(2,1));

      godleyItems[1].moveCell(2,2,3,2);
      CHECK_EQUAL("a",godley1.cell(3,2));
      CHECK_EQUAL("",godley1.cell(2,2));
    }

  TEST_FIXTURE(TestFixture,godleyRowSums)
    {
      GodleyTable& godley1=godleyItems[1].table;
      godley1.resize(6,3);
      godley1.cell(1,1)="a";
      godley1.cell(1,2)="2b";
      godley1.cell(1,3)="-c";
      CHECK_EQUAL("a+2b-c",godley1.rowSum(1));
    }

  TEST_FIXTURE(TestFixture,godleyMoveRowCol)
    {
      GodleyTable& godley1=godleyItems[1].table;
      godley1.resize(6,3);
      godley1.cell(1,1)="a";
      godley1.cell(1,2)="2b";
      godley1.cell(2,1)="a1";
      godley1.cell(2,2)="2b1";
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
      GodleyTable& godley1=godleyItems[1].table;
      GodleyTable& godley2=godleyItems[2].table;
      GodleyTable& godley3=godleyItems[3].table;
      godley1.nameUnique();
      godley2.nameUnique();
      godley3.nameUnique();
      CHECK(godley1.title!=godley2.title);
      CHECK(godley2.title!=godley3.title);
      CHECK(godley1.title!=godley3.title);
    }

    TEST_FIXTURE(TestFixture,clearInitCond)
    {
      GodleyIcon& godley=godleyItems[1];
      CHECK(godley.table.initialConditionRow(1));
      // to clear a cell, it first needs to contain something
      godley.setCell(1,1,"1");
      godley.setCell(1,1,"");
      CHECK_EQUAL("0",godley.table.cell(1,1));
    }
}
