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
#include "minsky_epilogue.h"
#undef True
#include <gtest/gtest.h>
#include <exception>
using namespace minsky;


TEST(Units, set)
    {
      Units a("m");
      EXPECT_EQ(1,a.size());
      EXPECT_EQ(1,a["m"]);
      EXPECT_TRUE(Units(a.str())==a);

      a=Units("m/s");
      EXPECT_EQ(2,a.size());
      EXPECT_EQ(1,a["m"]);
      EXPECT_EQ(-1,a["s"]);
      EXPECT_TRUE(Units(a.str())==a);

      a=Units("ma^2so^-1/Fd^2g^-3");
      EXPECT_EQ(4,a.size());
      EXPECT_EQ(2,a["ma"]);
      EXPECT_EQ(-1,a["so"]);
      EXPECT_EQ(-2,a["Fd"]);
      EXPECT_EQ(3,a["g"]);
      EXPECT_TRUE(Units(a.str())==a);

      // a somewhat pathological case
      a=Units("m^0");
      EXPECT_TRUE(a.empty());
      
      EXPECT_THROW(Units("/w"),std::exception);
      EXPECT_THROW(Units("a/w/q"),std::exception);
      EXPECT_THROW(Units("^1"),std::exception);
      EXPECT_THROW(Units("a^b"),std::exception);
      EXPECT_THROW(Units("a^^1"),std::exception);

      a=Units("m/s s");
      EXPECT_EQ(2,a.size());
      EXPECT_EQ(1,a["m"]);
      EXPECT_EQ(-2,a["s"]);
      EXPECT_TRUE(Units(a.str())==a);
      
      a=Units("m m");
      EXPECT_EQ(1,a.size());
      EXPECT_EQ(2,a["m"]);
      EXPECT_TRUE(Units(a.str())==a);

      a=Units("1/s");
      EXPECT_EQ(1,a.size());
      EXPECT_EQ(-1,a["s"]);
      EXPECT_TRUE(Units(a.str())==a);
    }

  struct TestOp: public Minsky, public ::testing::Test
  {
    VariablePtr from1{VariableType::flow, "a"}, from2{VariableType::flow, "b"};
    OperationPtr opp;
    static bool finished; // check that all cases are implemented
    LocalMinsky lm{*this};
    TestOp() {
      timeUnit="s";
      model->addItem(from1);
      model->addItem(from2);
    }
    void init(OperationType::Type op) {
      from1->setUnits("m");
      from2->setUnits("s");
      opp.reset(OperationBase::create(op));
      model->addItem(opp);
      if (opp->portsSize()>1)
        model->addWire(from1->ports(0), opp->ports(1));
      if (opp->portsSize()>2)
        model->addWire(from2->ports(0), opp->ports(2));
    }
    
    template <OperationType::Type op> void impl();
    template <OperationType::Type op> void test();
    template <OperationType::Type op> void testCmp();
    template <OperationType::Type op> void testLogical();
  };
  
  bool TestOp::finished=false; 
  
  template <OperationType::Type op>
    void TestOp::impl() {
    // not sure what tensor ops should do yet
    if (OperationType::classify(op)==OperationType::tensor) return;
    if (OperationType::classify(op)==OperationType::statistics) return;
    init(op);
    // most single arg functions are dimensionless, and args must match for two args
    if (OperationTypeInfo::numArguments<op>()>0 && op!=OperationType::percent)
      EXPECT_THROW(opp->checkUnits(), std::exception);

    from2->setUnits(from1->unitsStr());
    if (OperationTypeInfo::numArguments<op>()==1)
       if (op!=OperationType::percent)
         EXPECT_THROW(opp->checkUnits(), std::exception);
       else {
		  // Add % sign to units from input to % operator. 
		  from1->setUnits("%"+from1->unitsStr());
		  EXPECT_TRUE(opp->checkUnits()==from1->units());  
	   }
    else 
      EXPECT_TRUE(opp->checkUnits()==from1->units());
  }

  // TODO - not sure what to do here
  template <> void TestOp::impl<OperationType::userFunction>() {}
  template <> void TestOp::impl<OperationType::data>() {}
  template <> void TestOp::impl<OperationType::differentiate>(){}
  template <> void TestOp::impl<OperationType::integrate>() {}
  template <> void TestOp::impl<OperationType::gather>() {/* nothing to check, arg2 should be dimensionless*/}


  template <OperationType::Type op> void TestOp::testCmp()
  {
    init(op);
    EXPECT_THROW(opp->checkUnits(),std::exception);
    from2->setUnits(from1->unitsStr());
    EXPECT_TRUE(opp->checkUnits().empty());
  }

  template <> void TestOp::impl<OperationType::lt>() {testCmp<OperationType::lt>();}
  template <> void TestOp::impl<OperationType::le>() {testCmp<OperationType::lt>();}
  template <> void TestOp::impl<OperationType::eq>() {testCmp<OperationType::lt>();}
  
  template <OperationType::Type op> void TestOp::testLogical()
  {
    init(op);
    EXPECT_THROW(opp->checkUnits(),std::exception);
    from2->setUnits(from1->unitsStr());
    EXPECT_THROW(opp->checkUnits(),std::exception);
    from1->setUnits("");
    from2->setUnits("");
    EXPECT_TRUE(opp->checkUnits().empty());
  }
  
  template <> void TestOp::impl<OperationType::and_>() {testLogical<OperationType::and_>();}
  template <> void TestOp::impl<OperationType::or_>() {testLogical<OperationType::or_>();}
  template <> void TestOp::impl<OperationType::not_>() {testLogical<OperationType::not_>();}

  template <> void TestOp::impl<OperationType::pow>()
  {
    init(OperationType::pow);
    // pow's arguments must be dimensionless
    EXPECT_THROW(opp->checkUnits(),std::exception);
    // but if second argument is an integer
    model->removeItem(*from2);
    from2=VariablePtr(VariableType::constant);
    model->addItem(from2);
    model->addWire(from2->ports(0),opp->ports(2));
    from2->init("2");
    EXPECT_EQ(1,opp->checkUnits().size());
    EXPECT_EQ(2,opp->checkUnits()["m"]);
    // check that it throws with a nonintegral exponent
    from2->init("2.5");
    EXPECT_THROW(opp->checkUnits(),std::exception);
  }

  template <> void TestOp::impl<OperationType::log>()
  {
    init(OperationType::log);
    // log's arguments must be dimensionless
    EXPECT_THROW(opp->checkUnits(),std::exception);
    from2->setUnits(from1->unitsStr());
    EXPECT_THROW(opp->checkUnits(),std::exception);
  }

  template <> void TestOp::impl<OperationType::polygamma>()
  {
    init(OperationType::polygamma);
    // polygamma's arguments must be dimensionless
    EXPECT_THROW(opp->checkUnits(),std::exception);
    from2->setUnits(from1->unitsStr());
    EXPECT_THROW(opp->checkUnits(),std::exception);
  }  
  
  template <> void TestOp::impl<OperationType::copy>()
  {
    init(OperationType::copy);
    EXPECT_TRUE(opp->checkUnits()==from1->units());
  }

  template <> void TestOp::impl<OperationType::divide>()
  {
    init(OperationType::divide);
    EXPECT_EQ(1,opp->units()["m"]);
    EXPECT_EQ(-1,opp->units()["s"]);
    from2->setUnits(from1->unitsStr());
    EXPECT_EQ(0,opp->units()["m"]);
  }

  template <> void TestOp::impl<OperationType::multiply>()
  {
    init(OperationType::multiply);
    EXPECT_EQ(1,opp->units()["m"]);
    EXPECT_EQ(1,opp->units()["s"]);
    from2->setUnits(from1->unitsStr());
    EXPECT_EQ(2,opp->units()["m"]);
    EXPECT_EQ(0,opp->units()["s"]);
  }

  template <> void TestOp::impl<OperationType::time>()
  {
    init(OperationType::time);
    EXPECT_EQ(1,opp->units().size());
    EXPECT_EQ(1,opp->units()[timeUnit]);
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
  
TEST(Units, opUnits)
  {
    TestOp().test<OperationType::add>();
    EXPECT_TRUE(TestOp::finished);
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
    class TestMinsky : public Minsky, public ::testing::Test
    {
    public:
      LocalMinsky lm;
      TestMinsky(): lm(*this) {}
    };
  }

  
  TEST_F(TestMinsky,dimensionalAnalysis)
  {
    timeUnit="s";
    auto t=model->addItem(OperationBase::create(OperationType::time));
    VariablePtr m(VariableType::parameter,"d");
    model->addItem(m);
    m->setUnits("m");
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

    EXPECT_THROW(dimensionalAnalysis(),std::exception);
    integ->intVar->setUnits("m s^2");
    dimensionalAnalysis();
    EXPECT_EQ(1,tm->units()["m"]);
    EXPECT_EQ(1,tm->units()["s"]);
    EXPECT_EQ(1,integ->intVar->units()["m"]);
    EXPECT_EQ(2,integ->intVar->units()["s"]);
    EXPECT_EQ(1,vd->units()["m"]);
    EXPECT_EQ(0,vd->units()["s"]);
  }

  TEST_F(TestMinsky,populateMissingDimensionsFromVariable)
  {
    civita::Hypercube hc({3,4});
    hc.xvectors[0].dimension=civita::Dimension(civita::Dimension::value,"m");
    hc.xvectors[0].name="length";
    hc.xvectors[1].dimension=civita::Dimension(civita::Dimension::time,"%Y-%m-%d");
    hc.xvectors[1].name="time";
    VariableValue v;
    v.hypercube(hc);
    populateMissingDimensionsFromVariable(v);
    EXPECT_EQ(1,dimensions.count("length"));
    EXPECT_EQ(1,dimensions.count("time"));
    EXPECT_EQ(civita::Dimension::value,dimensions["length"].type);
    EXPECT_EQ(civita::Dimension::time,dimensions["time"].type);
    EXPECT_EQ("m",dimensions["length"].units);
    EXPECT_EQ("%Y-%m-%d",dimensions["time"].units);
  }
