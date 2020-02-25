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
#include "CSVParser.h"
#include "group.h"
#include "selection.h"
#include "minsky_epilogue.h"
#include <UnitTest++/UnitTest++.h>
using namespace minsky;
using namespace std;

SUITE(CSVParser)
{
  TEST_FIXTURE(DataSpec,guess)
    {
      string input="A comment\n"
        ";;foobar\n" // horizontal dim name
        "foo;bar;A;B;C\n"
        "A;A;1.2;1.3;1.4\n"
        "A;B;1;2;3\n"
        "B;A;3;2;1\n";

      istringstream is(input);
      guessFromStream(is);

      CHECK_EQUAL(';',separator);
      CHECK_EQUAL(3,nRowAxes());
      CHECK_EQUAL(2,nColAxes());
      CHECK_EQUAL(2,headerRow);
      CHECK((set<unsigned>{0,1}==dimensionCols));
    }
  
  TEST_FIXTURE(DataSpec,guessColumnar)
    {
      string input="Country,Time_Period,value$\n"
        "Australia,1967-Q4,1.8\n"
        "Australia,1968-Q1,1.9\n";

      istringstream is(input);
      guessFromStream(is);

      CHECK_EQUAL(',',separator);
      CHECK_EQUAL(1,nRowAxes());
      CHECK_EQUAL(2,nColAxes());
      CHECK_EQUAL(0,headerRow);
      CHECK_EQUAL(Dimension::string,dimensions[0].type);
      CHECK_EQUAL(Dimension::time,dimensions[1].type);
      CHECK_EQUAL("%Y-Q%Q",dimensions[1].units);
      CHECK((set<unsigned>{0,1}==dimensionCols));
    }
    
  TEST_FIXTURE(DataSpec,reportFromCSV)
    {
      string input="A comment\n"
        ";;foobar\n"
        "foo;bar;A;B;C\n"
        "A;A;1.2;1.3;1.4\n"
        "A;B;1;2;3\n"
        "B;A;3;2;1\n";
      string output="";  
      istringstream is(input);
      ostringstream os(output);
            
      reportFromCSVFile(is,os,*this);
      
      CHECK(os.str().find("error") != std::string::npos);
      CHECK(os.str().find("invalid numerical data") != std::string::npos);
      CHECK(os.str().find("duplicate key") != std::string::npos);
      
      string in="Country,value$,\n"
        "Australia\n"
        "Brazil,1.1,\n"
        "China,1.5,\n"
        "\n";
      string out="";  
      istringstream isn(in);
      ostringstream osn(out);
            
      reportFromCSVFile(isn,osn,*this);
      
      CHECK(osn.str().find("missing numerical data") != std::string::npos);
    }    
  
  TEST_FIXTURE(DataSpec,loadVar)
    {
      string input="A comment\n"
        ";;foobar\n" // horizontal dim name
        "foo;bar;A;B;C\n"
        "A;A;1.2;1.3;1.4\n"
        "A;B;1;2;3\n"
        "B;A;3;2;1\n";
      istringstream is(input);
      
      separator=';';
      setDataArea(3,2);
      missingValue=-1;
      headerRow=2;
      dimensionNames={"foo","bar"};
      dimensionCols={0,1};
      horizontalDimName="foobar";
      
      VariableValue v;
      loadValueFromCSVFile(v,is,*this);

      CHECK_EQUAL(3, v.rank());
      CHECK_ARRAY_EQUAL(vector<unsigned>({2,2,3}),v.hypercube().dims(),3);
      CHECK_EQUAL("foo", v.hypercube().xvectors[0].name);
      CHECK_EQUAL("A", str(v.hypercube().xvectors[0][0]));
      CHECK_EQUAL("B", str(v.hypercube().xvectors[0][1]));
      CHECK_EQUAL("bar", v.hypercube().xvectors[1].name);
      CHECK_EQUAL("A", str(v.hypercube().xvectors[1][0]));
      CHECK_EQUAL("B", str(v.hypercube().xvectors[1][1]));
      CHECK_EQUAL("foobar", v.hypercube().xvectors[2].name);
      CHECK_EQUAL("A", str(v.hypercube().xvectors[2][0]));
      CHECK_EQUAL("B", str(v.hypercube().xvectors[2][1]));
      CHECK_EQUAL("C", str(v.hypercube().xvectors[2][2]));
      CHECK(v.hypercube().dims()==v.tensorInit.hypercube().dims());
      CHECK_EQUAL(12, v.tensorInit.size());
      CHECK_ARRAY_CLOSE(vector<double>({1.2,3,1,-1,1.3,2,2,-1,1.4,1,3,-1}),
                        v.tensorInit, 12, 1e-4);
    }
  
}
