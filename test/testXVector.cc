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

#include "cairoItems.h"
#include "variableType.h"
#include "evalOp.h"
#include "selection.h"
#include "lasso.h"
#include "xvector.h"
#include "minsky_epilogue.h"
#undef True
#include <gtest/gtest.h>
using namespace minsky;

#include <exception>
using namespace std;

#include <boost/date_time.hpp>
using namespace boost;
using namespace boost::posix_time;
using namespace boost::gregorian;
typedef vector<XVector> XV;

class XVectorTest : public XVector, public ::testing::Test
{
};

TEST_F(XVectorTest, push_back)
  {
    // firstly check the simple string case
    push_back("foo");
    EXPECT_TRUE(front().type==Dimension::string);
    EXPECT_EQ("foo",str(front()));
    clear();
    // now check values
    dimension.type=Dimension::value;
    push_back("0.5");
    EXPECT_TRUE(front().type==Dimension::value);
    EXPECT_EQ(0.5,stod(str(front())));
    EXPECT_THROW(push_back("foo"),std::exception);
    clear();
    // now check dates and times

    dimension.type=Dimension::time;
    dimension.units="%Y-Q%Q";
    push_back("2018-Q2");
    
    EXPECT_EQ(ptime(date(2018,Apr,1)), back().time);
    EXPECT_THROW(push_back("2-2018"),std::exception);
    
    dimension.units="Q%Q-%Y";
    push_back("Q2-2018");
    EXPECT_EQ(ptime(date(2018,Apr,1)), back().time);
    EXPECT_THROW(push_back("2-2018"),std::exception);
    
    dimension.units="Q%Q";
    EXPECT_THROW(push_back("Q1"),std::exception);

    dimension.units="%Y-%m-%d";
    push_back("2018-04-01");
    EXPECT_EQ(ptime(date(2018,Apr,1)), back().time);
    EXPECT_THROW(push_back("2-2018"),std::exception);

    // test some wonky dates and times
    dimension.units="%d/%m/%Y";
    push_back("1/4/2018");
    EXPECT_EQ(ptime(date(2018,Apr,1)), back().time);
    EXPECT_THROW(push_back("2-2018"),std::exception);

    dimension.units="%d/%m/%Y %H:%M:%S";
    push_back("1/4/2018 12:52:13");
    EXPECT_EQ(ptime(date(2018,Apr,1),time_duration(12,52,13)), back().time);
    EXPECT_THROW(push_back("2-2018"),std::exception);
    
    dimension.units.clear();
    push_back("2018-04-01");
    EXPECT_EQ(ptime(date(2018,Apr,1)), back().time);
    EXPECT_THROW(push_back("foo"),std::exception);

  }

class ConversionsTest : public Conversions, public ::testing::Test
{
};

TEST_F(ConversionsTest, convert)
  {
    emplace("day:hour",24);
    EXPECT_EQ(48,convert(2,"day","hour"));
    EXPECT_EQ(2,convert(48,"hour","day"));
    EXPECT_THROW(convert(1,"metre","second"), std::exception);
  }

TEST(XVector, str)
  {
    EXPECT_EQ("0.3",str(civita::any("0.3"),""));
    EXPECT_EQ("0.300000",str(civita::any(0.3),""));
    civita::any t(boost::posix_time::time_from_string("2002-04-20 23:59:59.000"));
    EXPECT_EQ("2002-04-20T23:59:59", str(t,""));
    EXPECT_EQ("2002/04/20", str(t,"%Y/%m/%d"));
    EXPECT_EQ("2002-Q2", str(t,"%Y-Q%Q"));
    EXPECT_EQ("Q2-2002", str(t,"Q%Q-%Y"));
    EXPECT_THROW(str(t,"Q%Q"), std::exception);
  }

TEST(XVector, compareInvalidStoredType)
{
  XVector a("a",{Dimension::string,""},{"foo","bar","foobar"}), b=a;
  b[0]=string("1.0"); // ensure we have mixed types
  b[1]="2.0";
  EXPECT_FALSE(a==b);
}

TEST(XVector, anyValSanityChecking)
{
  EXPECT_THROW(anyVal({Dimension::time,"%Y-%Q"}, "2001-5"), std::exception);
  EXPECT_THROW(anyVal({Dimension::time,"%y:%d:%m"}, "foobar"), std::exception);
  EXPECT_THROW(anyVal({Dimension::time,"%y:%d:%m"}, "100:1:12"), std::exception);
  EXPECT_THROW(anyVal({Dimension::time,"%y:%d:%m"}, "foo:1:bar"), std::exception);
  EXPECT_EQ("1999-12-01",str(anyVal({Dimension::time,"%y:%d:%m"}, "99:1:12"), "%Y-%m-%d"));
  EXPECT_EQ("2009-12-01",str(anyVal({Dimension::time,"%y:%d:%m"}, "09:1:12"), "%Y-%m-%d"));
  EXPECT_THROW(anyVal({Dimension::time,"%Y-%b-%d"}, "foobar"), std::exception);

  //screwy dates
  EXPECT_EQ("2009-09-01",str(anyVal({Dimension::time,"%m/%d/%Y"}, "9/1/2009"), "%Y-%m-%d"));
  EXPECT_EQ("2009-09-01",str(anyVal({Dimension::time,"%Y%m%d"}, "20090901"), "%Y-%m-%d"));
 

}

TEST(XVector, incompatibleDiff)
{
  civita::any x, y;
  x=0.5; y="hello";
  EXPECT_THROW(diff(x,y), std::exception); // incompatible type
}

TEST(XVector, timeFormat)
{
  XVector x("hello",{Dimension::time,""});
  EXPECT_EQ("",x.timeFormat());
  x.push_back("2000-01-01T01:01:00");
  x.push_back("2000-01-01T01:01:01");
  EXPECT_EQ("%s",x.timeFormat());
  x.push_back("2000-01-01T01:05:01");
  EXPECT_EQ("%M:%S",x.timeFormat());
  x.push_back("2000-01-01T03:05:01");
  EXPECT_EQ("%H:%M",x.timeFormat());
  x.push_back("2000-01-03T03:05:01");
  EXPECT_EQ("%d %H:%M",x.timeFormat());
  x.push_back("2000-02-03T03:05:01");
  EXPECT_EQ("%d %b",x.timeFormat());
  x.push_back("2000-09-03T03:05:01");
  EXPECT_EQ("%b",x.timeFormat());
  x.push_back("2003-09-03T03:05:01");
  EXPECT_EQ("%b %Y",x.timeFormat());
  x.push_back("2020-09-03T03:05:01");
  EXPECT_EQ("%Y",x.timeFormat());
}
