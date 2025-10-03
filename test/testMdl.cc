/*
  @copyright Steve Keen 2020
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

#include "cairoItems.h"
#include "group.h"
#include "selection.h"
#include "lasso.h"
#include "userFunction.h"
#include "mdlReader.h"
#include "minsky_epilogue.h"
#include <gtest/gtest.h>

#include <sstream>
using namespace minsky;
using namespace std;


// All tests are commented out in original
//TEST(MDL, lookupPairs)
//  {
//    istringstream input("foo([(0,0)-(1,1)],(0,0),(0.1,0.2),(0.5,0.5),(0.9,0.8))~~|");
//    Group g;
//    RungeKutta simParms;
//    readMdl(g,simParms,input);
//    // Needs to be a user function foo that interpolates the above data
//    int count=0;
//    for (auto& i: g.items)
//      if (auto f=dynamic_cast<UserFunction*>(i.get()))
//        if (f->name()=="foo")
//          {
//            ++count;
//            f->compile();
//            // TODO current hangs
//            //              EXPECT_EQ(0.2,f->evaluate(0.1,0));
////              EXPECT_EQ(0.35,f->evaluate(0.3,0));
////              EXPECT_EQ(0.5,f->evaluate(0.5,0));
////              EXPECT_EQ(0.65,f->evaluate(0.7,0));
////              EXPECT_EQ(0.8,f->evaluate(0.9,0));
//        }
//    EXPECT_EQ(1, count);
//  }
//
//TEST(MDL, lookupBasic)
//  {
//    istringstream input("foo(0,0.1,0.5,0.9,0,0.2,0.5,0.8)~~|");
//    Group g;
//    RungeKutta simParms;
//    readMdl(g,simParms,input);
//    // Needs to be a user function foo that interpolates the above data
//    int count=0;
//    for (auto& i: g.items)
//      if (auto f=dynamic_cast<UserFunction*>(i.get()))
//        if (f->name()=="foo")
//          {
//            ++count;
////              EXPECT_EQ(0.2,f->evaluate(0.1,0));
////              EXPECT_EQ(0.35,f->evaluate(0.3,0));
////              EXPECT_EQ(0.5,f->evaluate(0.5,0));
////              EXPECT_EQ(0.65,f->evaluate(0.7,0));
////              EXPECT_EQ(0.8,f->evaluate(0.9,0));
//        }
//    EXPECT_EQ(1, count);
//  }
//
//TEST(MDL, lookupInvalid)
//  {
//    istringstream input1("foo(0,0.1,0.5,0.9,0,0.2,0.5)~~|");
//    istringstream input2("foo([aa],xx)~~|");
//    Group g;
//    RungeKutta simParms;
//    // invalid, odd amount of data
//    EXPECT_THROW(readMdl(g,simParms,input1), std::exception);
//    EXPECT_THROW(readMdl(g,simParms,input2), std::exception);
//  }

