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
#include "operationType.h"
#include "variableValue.h"
#include "evalOp.h"
#include "selection.h"
#include "minsky.h"
#include <ecolab_epilogue.h>
#include <UnitTest++/UnitTest++.h>
#include <exception>
using namespace minsky;

SUITE(Units)
{
  TEST(set)
    {
      Units a("m");
      CHECK_EQUAL(1,a["m"]);
      CHECK(Units(a.str())==a);

      a=Units("m/s");
      CHECK_EQUAL(1,a["m"]);
      CHECK_EQUAL(-1,a["s"]);
      CHECK(Units(a.str())==a);

      a=Units("ma^2so^-1/Fd^2g^-3");
      CHECK_EQUAL(2,a["ma"]);
      CHECK_EQUAL(-1,a["so"]);
      CHECK_EQUAL(-2,a["Fd"]);
      CHECK_EQUAL(3,a["g"]);
      CHECK(Units(a.str())==a);

      // a somewhat pathological case
      a=Units("m^0");
      CHECK(a.empty());
      
      CHECK_THROW(Units("/w"),std::exception);
      CHECK_THROW(Units("a/w/q"),std::exception);
      CHECK_THROW(Units("^1"),std::exception);
      CHECK_THROW(Units("a^b"),std::exception);
      CHECK_THROW(Units("a^^1"),std::exception);

    }

  struct TestOp
  {
    VariableValue from1{VariableType::tempFlow},
      from2{VariableType::tempFlow}, to{VariableType::tempFlow};
    static bool finished; // check that all cases are implemented
    TestOp() {
      from1.units=Units("m");
      from2.units=Units("s");
      from1.allocValue();
      from2.allocValue();
    }

    template <OperationType::Type op> void impl();
    template <OperationType::Type op> void test();
    template <OperationType::Type op> void testCmp();
    template <OperationType::Type op> void testLogical();
  };
  
  bool TestOp::finished=false; 
  
  template <OperationType::Type op>
    void TestOp::impl() {
    // most single arg functions are dimensionless, and args must match for two args
    if (OperationTypeInfo::numArguments<op>()>0)
      CHECK_THROW(EvalOpPtr(op,nullptr,to,from1,from2), std::exception);

    from2.units=from1.units;
    if (OperationTypeInfo::numArguments<op>()==1)
      CHECK_THROW(EvalOpPtr(op,nullptr,to,from1,from2), std::exception);
    else
      {
        EvalOpPtr(op,nullptr,to,from1,from2);
        CHECK(to.units==from1.units);
      }
  }

  // TODO - not sure what to do here
  template <> void TestOp::impl<OperationType::data>() {}
  template <> void TestOp::impl<OperationType::differentiate>(){}
  template <> void TestOp::impl<OperationType::integrate>() {}
  template <> void TestOp::impl<OperationType::gather>() {/* nothing to check, arg2 should be dimensionless*/}


  template <OperationType::Type op> void TestOp::testCmp()
  {
    CHECK_THROW(EvalOpPtr(op,nullptr, to,from1,from2),std::exception);
    from2.units=from1.units;
    EvalOpPtr(op,nullptr, to,from1,from2);
    CHECK(to.units.empty());
  }

  template <> void TestOp::impl<OperationType::lt>() {testCmp<OperationType::lt>();}
  template <> void TestOp::impl<OperationType::le>() {testCmp<OperationType::lt>();}
  template <> void TestOp::impl<OperationType::eq>() {testCmp<OperationType::lt>();}
  
  template <OperationType::Type op> void TestOp::testLogical()
  {
    CHECK_THROW(EvalOpPtr(op,nullptr, to,from1,from2),std::exception);
    from2.units=from1.units;
    CHECK_THROW(EvalOpPtr(op,nullptr, to,from1,from2),std::exception);
    from1.units.clear();
    from2.units.clear();
    EvalOpPtr(op,nullptr, to,from1,from2);
    CHECK(to.units.empty());
  }
  
  template <> void TestOp::impl<OperationType::and_>() {testLogical<OperationType::and_>();}
  template <> void TestOp::impl<OperationType::or_>() {testLogical<OperationType::or_>();}
  template <> void TestOp::impl<OperationType::not_>() {testLogical<OperationType::not_>();}

  template <> void TestOp::impl<OperationType::pow>()
  {
    // pow's arguments must be dimensionless
    CHECK_THROW(EvalOpPtr(OperationType::pow,nullptr, to,from1,from2),std::exception);
    // but if second argument is an integer
    from2=VariableValue(VariableType::constant,"","2");
    from2.allocValue();
    EvalOpPtr(OperationType::pow,nullptr, to,from1,from2);
    CHECK_EQUAL(1,to.units.size());
    CHECK_EQUAL(2,to.units["m"]);
    // check that it throws with a nonintegral exponent
    from2.init=2.5;
    CHECK_THROW(EvalOpPtr(OperationType::pow,nullptr, to,from1,from2),std::exception);
  }

  template <> void TestOp::impl<OperationType::log>()
  {
    // log's arguments must be dimensionless
    CHECK_THROW(EvalOpPtr(OperationType::log,nullptr, to,from1,from2),std::exception);
  }
  
  template <> void TestOp::impl<OperationType::copy>()
  {
    EvalOpPtr(OperationType::copy,nullptr, to,from1,from2);
    CHECK(to.units==from1.units);
  }
  


  template <> void TestOp::impl<OperationType::divide>()
  {
    EvalOpPtr(OperationType::divide,nullptr, to,from1,from2);
    CHECK_EQUAL(1,to.units["m"]);
    CHECK_EQUAL(-1,to.units["s"]);
    from2.units=from1.units;
    EvalOpPtr(OperationType::divide,nullptr, to,from1,from2);
    CHECK(to.units.empty());
  }

  template <> void TestOp::impl<OperationType::multiply>()
  {
    EvalOpPtr(OperationType::multiply,nullptr, to,from1,from2);
    CHECK_EQUAL(1,to.units["m"]);
    CHECK_EQUAL(1,to.units["s"]);
    from2.units=from1.units;
    EvalOpPtr(OperationType::multiply,nullptr, to,from1,from2);
    CHECK_EQUAL(2,to.units["m"]);
    CHECK_EQUAL(0,to.units["s"]);
  }

  template <> void TestOp::impl<OperationType::time>()
  {
    EvalOpPtr(OperationType::time,nullptr, to,from1,from2);
    CHECK_EQUAL(1,to.units[EvalOpBase::timeUnit]);
  }

  // specialising, terminating the static recursion
  template <>
    void TestOp::test<OperationType::numOps>() {finished=true;}
  
  template <OperationType::Type op>
    void TestOp::test()
  {
    cout << "Testing: "<<op<<endl;
    impl<op>();
    TestOp().test<OperationType::Type(op+1)>();
  }
  
  TEST(evalOpPtr)
  {
    EvalOpBase::timeUnit="s";
    TestOp().test<OperationType::add>();
    CHECK(TestOp::finished);
  }

  // test that units are correctly inferred in a simple model
  /*
    t-     - ∫
      \   /
        * 
      /   \
    d-     - d/dt - vd
  */
  namespace
  {
    struct TestMinsky: public Minsky
    {
      LocalMinsky lm;
      TestMinsky(): lm(*this) {}
    };
  }

  
  TEST_FIXTURE(TestMinsky,constructEquations)
  {
    timeUnit="s";
    auto t=model->addItem(OperationBase::create(OperationType::time));
    VariablePtr m(VariableType::parameter,"d");
    model->addItem(m);
    m->units("m");
    auto mult=model->addItem(OperationBase::create(OperationType::multiply));
    auto integ=make_shared<IntOp>();
    model->addItem(integ);
    auto diff=model->addItem(OperationBase::create(OperationType::differentiate));
    VariablePtr vd(VariableType::flow,"vd");
    model->addItem(vd);
    VariablePtr tm(VariableType::flow,"tm");
    model->addItem(tm);
    
    // wires
    model->addWire(*t,*mult,1);
    model->addWire(*m,*mult,2);
    model->addWire(*mult,*tm,1);
    model->addWire(*mult,*integ,1);
    model->addWire(*mult,*diff,1);
    model->addWire(*diff,*vd,1);

    constructEquations();
    CHECK_EQUAL(1,tm->_units()["m"]);
    CHECK_EQUAL(1,tm->_units()["s"]);
    CHECK_EQUAL(1,integ->intVar->_units()["m"]);
    CHECK_EQUAL(2,integ->intVar->_units()["s"]);
    CHECK_EQUAL(1,vd->_units()["m"]);
    CHECK_EQUAL(0,vd->_units()["s"]);
  }
}
