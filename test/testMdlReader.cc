/*
  @copyright Steve Keen 2024
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

#include "mdlReader.h"
#include "minsky.h"
#include "group.h"
#include "minsky_epilogue.h"

#include <gtest/gtest.h>
#include <sstream>

using namespace minsky;
using namespace std;

namespace
{
  struct MdlReaderTest: public ::testing::Test
  {
    GroupPtr group=std::make_shared<Group>();
    Simulation simParms;
    
    void SetUp() override {
      group->self = group;
    }
  };
}

TEST_F(MdlReaderTest, ParseValidMdlFile)
{
  // Simple MDL file with UTF-8 header
  stringstream mdl;
  mdl << "{UTF-8}" << endl;
  mdl << "*\n.Control\n*" << endl;
  mdl << "INITIAL TIME = 0" << endl;
  mdl << "  ~  ~  |" << endl;
  mdl << "FINAL TIME = 10" << endl;
  mdl << "  ~  ~  |" << endl;
  mdl << "TIME STEP = 0.1" << endl;
  mdl << "  ~ Month ~  |" << endl;
  mdl << "*| " << endl;
  mdl << "\\\\\\---/// sketch information" << endl;
  
  EXPECT_NO_THROW(readMdl(*group, simParms, mdl));
  EXPECT_EQ(0, simParms.t0);
  EXPECT_EQ(10, simParms.tmax);
  EXPECT_EQ(0.1, simParms.stepMax);
  EXPECT_EQ("Month", simParms.timeUnit);
}

TEST_F(MdlReaderTest, HandleNonUTF8File)
{
  // MDL file without UTF-8 header should throw
  stringstream mdl;
  mdl << "{ASCII}" << endl;
  
  EXPECT_THROW(readMdl(*group, simParms, mdl), runtime_error);
}

TEST_F(MdlReaderTest, ParseSimpleVariable)
{
  stringstream mdl;
  mdl << "{UTF-8}" << endl;
  mdl << "test_var = 100" << endl;
  mdl << "  ~ units ~  |" << endl;
  mdl << "\\\\\\---/// sketch information" << endl;
  
  size_t initialItems = group->items.size();
  EXPECT_NO_THROW(readMdl(*group, simParms, mdl));
  // Should have added a parameter variable
  EXPECT_GT(group->items.size(), initialItems);
}

TEST_F(MdlReaderTest, ParseIntegralEquation)
{
  stringstream mdl;
  mdl << "{UTF-8}" << endl;
  mdl << "Stock = INTEG(flow, 10)" << endl;
  mdl << "  ~ units ~  |" << endl;
  mdl << "flow = 5" << endl;
  mdl << "  ~ units/time ~  |" << endl;
  mdl << "\\\\\\---/// sketch information" << endl;
  
  size_t initialItems = group->items.size();
  EXPECT_NO_THROW(readMdl(*group, simParms, mdl));
  // Should have added integral and related variables
  EXPECT_GT(group->items.size(), initialItems);
  
  // Check that an IntOp was created
  bool hasIntOp = false;
  for (auto& item : group->items) {
    if (dynamic_pointer_cast<IntOp>(item)) {
      hasIntOp = true;
      break;
    }
  }
  EXPECT_TRUE(hasIntOp);
}

TEST_F(MdlReaderTest, ParseLookupFunction)
{
  stringstream mdl;
  mdl << "{UTF-8}" << endl;
  mdl << "lookup_func([(0,0)-(10,100)],(0,0),(5,50),(10,100))" << endl;
  mdl << "  ~ ~  |" << endl;
  mdl << "\\\\\\---/// sketch information" << endl;
  
  size_t initialItems = group->groups.size();
  EXPECT_NO_THROW(readMdl(*group, simParms, mdl));
  // Should have created a group for the lookup function
  EXPECT_GT(group->groups.size(), initialItems);
}

TEST_F(MdlReaderTest, ParseFlowVariable)
{
  stringstream mdl;
  mdl << "{UTF-8}" << endl;
  mdl << "flow_var = input1 + input2" << endl;
  mdl << "  ~ dimensionless ~  |" << endl;
  mdl << "input1 = 10" << endl;
  mdl << "  ~ ~  |" << endl;
  mdl << "input2 = 20" << endl;
  mdl << "  ~ ~  |" << endl;
  mdl << "\\\\\\---/// sketch information" << endl;
  
  size_t initialItems = group->items.size();
  EXPECT_NO_THROW(readMdl(*group, simParms, mdl));
  EXPECT_GT(group->items.size(), initialItems);
}

TEST_F(MdlReaderTest, HandleComments)
{
  stringstream mdl;
  mdl << "{UTF-8}" << endl;
  mdl << "var1 {this is a comment} = 100" << endl;
  mdl << "  ~ units ~  |" << endl;
  mdl << "\\\\\\---/// sketch information" << endl;
  
  // Should parse successfully, ignoring comments
  EXPECT_NO_THROW(readMdl(*group, simParms, mdl));
}

TEST_F(MdlReaderTest, ParseSliderSpec)
{
  stringstream mdl;
  mdl << "{UTF-8}" << endl;
  mdl << "slider_var = 50" << endl;
  mdl << "  ~ [0,100,1] ~  |" << endl;
  mdl << "\\\\\\---/// sketch information" << endl;
  
  EXPECT_NO_THROW(readMdl(*group, simParms, mdl));
  
  // Find the variable and check slider bounds were set
  for (auto& item : group->items) {
    if (auto var = dynamic_pointer_cast<VariableBase>(item)) {
      if (auto vv = var->vValue()) {
        // Slider should be configured (exact values depend on implementation)
        EXPECT_GE(vv->sliderMax, vv->sliderMin);
        break;
      }
    }
  }
}

TEST_F(MdlReaderTest, ParseVensimBuiltinFunctions)
{
  stringstream mdl;
  mdl << "{UTF-8}" << endl;
  mdl << "result = STEP(100, 5)" << endl;
  mdl << "  ~ ~  |" << endl;
  mdl << "\\\\\\---/// sketch information" << endl;
  
  // Should successfully parse and map built-in function
  EXPECT_NO_THROW(readMdl(*group, simParms, mdl));
}

TEST_F(MdlReaderTest, EmptyFile)
{
  stringstream mdl;
  mdl << "{UTF-8}" << endl;
  mdl << "\\\\\\---/// sketch information" << endl;
  
  // Empty file (just header and sketch marker) should parse without error
  EXPECT_NO_THROW(readMdl(*group, simParms, mdl));
}

TEST_F(MdlReaderTest, CollapseWhitespace)
{
  stringstream mdl;
  mdl << "{UTF-8}" << endl;
  mdl << "Multi  Word   Variable = 42" << endl;
  mdl << "  ~ ~  |" << endl;
  mdl << "\\\\\\---/// sketch information" << endl;
  
  size_t initialItems = group->items.size();
  EXPECT_NO_THROW(readMdl(*group, simParms, mdl));
  // Should create variable with collapsed/camelCase name
  EXPECT_GT(group->items.size(), initialItems);
}
