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
#include "CSVDialog.h"
#include "group.h"
#include "selection.h"
#include "dimension.h"
#include "lasso.h"
#include "variableValue.h"
#include "minsky_epilogue.h"
#include <UnitTest++/UnitTest++.h>
#include <boost/filesystem.hpp>
#include <numeric>

using namespace minsky;
using namespace std;
using namespace civita;

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
   TEST_FIXTURE(DataSpec,guessSpace)
    {
      string input="foo bar  A B C\n"
        "A A 1.2 1.3  1.4\n"
        "A B  1 2 3\n"
        "B A 3 2 1\n";

      istringstream is(input);
      guessFromStream(is);

      CHECK_EQUAL(' ',separator);
      CHECK_EQUAL(1,nRowAxes());
      CHECK_EQUAL(2,nColAxes());
      CHECK_EQUAL(0,headerRow);
      CHECK((set<unsigned>{0,1}==dimensionCols));
    }
    TEST_FIXTURE(DataSpec,guessTab)
    {
      string input="A comment\n"
        "\t\tfoobar\n" // horizontal dim name
        "foo\tbar\tA\tB\tC\n"
        "A\tA\t1.2\t1.3\t1.4\n"
        "A\tB\t1\t2\t3\n"
        "B\tA\t3\t2\t1\n";

      istringstream is(input);
      guessFromStream(is);

      CHECK_EQUAL('\t',separator);
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
      CHECK((set<unsigned>{0,1}==dimensionCols));
      CHECK((set<unsigned>{2}==dataCols));
    }

  TEST_FIXTURE(DataSpec,badTimeData)
    {
      string input="time,data\n"
        "1966-Q,1\n";
      istringstream is(input);
      setDataArea(1,1);
      dimensionCols={0};
      dimensions[0].type=Dimension::time;
      dimensions[0].units="%Y-Q%Q";
      VariableValue v(VariableType::parameter,":foo");
      CHECK_THROW(loadValueFromCSVFile(v,is,*this),std::exception);
      is.clear(); is.seekg(0);
      try {loadValueFromCSVFile(v,is,*this);}
      catch (const std::exception& ex) {
        CHECK(string(ex.what()).starts_with("Invalid"));
      }

      is.clear(); is.seekg(0);
      string output;  
      ostringstream os(output);
      reportFromCSVFile(is,os,*this,2);
      CHECK(os.str().find("Invalid data") != std::string::npos);
    }
  
  // disable temporarily until this is fixed.
  TEST_FIXTURE(DataSpec,reportFromCSV)
    {
      string input="A comment\n"
        ";;foobar\n"
        "foo;bar;A;B;C\n"
        "A;A;1.2;1.3;1.4\n"
        "A;A;1;2;3\n"
        "B;A;3;S;1\n";
      string output="";  
      istringstream is(input);
      ostringstream os(output);
      guessFromStream(is);
      is.clear();
      is.seekg(0);
      setDataArea(3,2);
      dataCols={2,3,4};
      dimensionNames={"foo","bar","A","B","C"};
      
      
      reportFromCSVFile(is,os,*this,6);

      struct Count
      {
        const char target;
        Count(char target): target(target) {}
        int operator()(int x,char c) const {return x+(c==target);}
      };
      
      // output should have same number of lines as input, and 1 extra ; for each data line
      auto osStr=os.str();
      auto numLines=accumulate(input.begin(), input.end(), 0, Count('\n'));
      auto numSep=accumulate(input.begin(), input.end(), 0, Count(';'));
      
      CHECK_EQUAL(numLines, accumulate(osStr.begin(), osStr.end(), 0, Count('\n')));
      CHECK_EQUAL(numSep+(numLines-nRowAxes()+1),
                  accumulate(osStr.begin(), osStr.end(), 0, Count(';')));
      
      CHECK(os.str().find("Error") != std::string::npos);
      CHECK(os.str().find("Invalid data") != std::string::npos);
      CHECK(os.str().find("Duplicate key") != std::string::npos);

      // TODO - this doesn't exercise all the value checking paths
    }

  // disabled, because I don't think this test makes much sense
//   TEST_FIXTURE(CSVDialog,guessSpaceFile)
//    {
//      spec=DataSpec();
//      string fname="testEqGodley.csv";
//      spec.guessFromFile(fname);                                  
//      ifstream is(fname);
//
//      spec.separator=',';
//      spec.headerRow=1;
//      spec.setDataArea(3,2);     
//      spec.dimensionNames={"asset","liability","equity"};          
//      spec.dimensionCols={1,2,3};      
//          
//      VariableValue v(VariableType::parameter,":foo");
//      loadValueFromCSVFile(v,is,spec);
//
//      CHECK_EQUAL(1, v.rank()); 
//      spec.toggleDimension(2);
//      CHECK_EQUAL(2,spec.dimensionCols.size());  // equity column is empty
//         
//    }      

  TEST_FIXTURE(CSVDialog,classifyColumns)
    {
      string input="10,2022/10/2,hello,\n"
        "'5,150,000','2023/1/3','foo bar',\n"
        "2 00,2023/1/3,ggg,\n";

      url=boost::filesystem::unique_path().string();
      {
        ofstream of(url);
        of<<input;
      }

      spec.quote='\'';
      spec.setDataArea(0,4);
      loadFile();
      classifyColumns();
      CHECK_EQUAL(4,spec.numCols);
      CHECK(spec.dataCols.count(0));
      CHECK(!spec.dimensionCols.count(0));
      CHECK(!spec.dataCols.count(1));
      CHECK(spec.dimensionCols.count(1));
      CHECK(spec.dimensions[1].type==Dimension::time);
      CHECK(!spec.dataCols.count(2));
      CHECK(spec.dimensionCols.count(2));
      CHECK(spec.dimensions[2].type==Dimension::string);
      CHECK(!spec.dataCols.count(3));
      CHECK(!spec.dimensionCols.count(3));
    }

  
  TEST_FIXTURE(DataSpec,loadVar)
    {
      string input="A comment\n"
        ";;foobar\n" // horizontal dim name
        "foo;bar;A;B;C\n"
        "A;A;1.2;1.3;-1.4\n"
        "A;B;1;2;£3\n"
        "B;A;£-3.1;.2;-£5\n";

      istringstream is(input);
      
      separator=';';
      setDataArea(3,2);
      numCols=5;
      missingValue=-1;
      headerRow=2;
      dimensionNames={"foo","bar","A","B","C"};
      dimensionCols={0,1};
      horizontalDimName="foobar";
      
      VariableValue v(VariableType::parameter);
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
      CHECK_ARRAY_CLOSE(vector<double>({1.2,-3.1,1,-1,1.3,.2,2,-1,-1.4,-5,3,-1}),
                        v.tensorInit, 12, 1e-4);
    }
  
   TEST_FIXTURE(DataSpec,embeddedNewline)
    {
      string input="A comment\n"
        ",,foobar\n" // horizontal dim name
        "foo,bar,A,B,C\n"
        "A,'A\nB',1.2,1.3,'1,000'\n"
        "A,B,1,'2\n.0',3\n"
        "B,AB,3,2,1\n";

      istringstream is(input);
      
      separator=',';
      quote='\'';
      setDataArea(3,2);
      numCols=5;
      missingValue=-1;
      headerRow=2;
      dimensionNames={"foo","bar","A","B","C"};
      dimensionCols={0,1};
      horizontalDimName="foobar";
      
      VariableValue v(VariableType::parameter);
      loadValueFromCSVFile(v,is,*this);

      CHECK_EQUAL(3, v.rank());
      CHECK_ARRAY_EQUAL(vector<unsigned>({2,2,3}),v.hypercube().dims(),3);
      CHECK_EQUAL("foo", v.hypercube().xvectors[0].name);
      CHECK_EQUAL("A", str(v.hypercube().xvectors[0][0]));
      CHECK_EQUAL("B", str(v.hypercube().xvectors[0][1]));
      CHECK_EQUAL("bar", v.hypercube().xvectors[1].name);
      CHECK_EQUAL("AB", str(v.hypercube().xvectors[1][0]));
      CHECK_EQUAL("B", str(v.hypercube().xvectors[1][1]));
      CHECK_EQUAL("foobar", v.hypercube().xvectors[2].name);
      CHECK_EQUAL("A", str(v.hypercube().xvectors[2][0]));
      CHECK_EQUAL("B", str(v.hypercube().xvectors[2][1]));
      CHECK_EQUAL("C", str(v.hypercube().xvectors[2][2]));
      CHECK(v.hypercube().dims()==v.tensorInit.hypercube().dims());
      CHECK_EQUAL(12, v.tensorInit.size());
      CHECK_ARRAY_CLOSE(vector<double>({1.2,3,1,-1,1.3,2,2,-1,1000,1,3,-1}),
                        v.tensorInit, 12, 1e-4);
    }
 
  TEST_FIXTURE(DataSpec,januaryDoubleDip)
    {
      string input=
        "country;2014;2014-01;2014-02\n" 
        "Aus;10;;\n"
        "UK;;10;20\n";

      istringstream is(input);
      
      separator=';';
      setDataArea(1,1);
      numCols=4;
      missingValue=-1;
      headerRow=0;
      dimensionNames={"country","2014","2014-01","2014-02"};
      dimensionCols={0};
      horizontalDimName="date";
      horizontalDimension.type=Dimension::time;
      
      VariableValue v(VariableType::parameter);
      loadValueFromCSVFile(v,is,*this);

      CHECK_EQUAL(2, v.rank());
      CHECK_ARRAY_EQUAL(vector<unsigned>({2,2}),v.hypercube().dims(),2);
      CHECK_EQUAL("country", v.hypercube().xvectors[0].name);
      CHECK_EQUAL("Aus", str(v.hypercube().xvectors[0][0]));
      CHECK_EQUAL("UK", str(v.hypercube().xvectors[0][1]));
      CHECK_EQUAL("date", v.hypercube().xvectors[1].name);
      CHECK_EQUAL("2014-01-01T00:00:00", str(v.hypercube().xvectors[1][0]));
      CHECK_EQUAL("2014-02-01T00:00:00", str(v.hypercube().xvectors[1][1]));
      CHECK(v.hypercube().dims()==v.tensorInit.hypercube().dims());
      CHECK_EQUAL(4, v.tensorInit.size());
      CHECK_ARRAY_CLOSE(vector<double>({10,10,-1,20}),
                        v.tensorInit, 4, 1e-4);
    }

   TEST_FIXTURE(DataSpec,dotTimeSep)
    {
      string input=
        "country;2014.01;2014.02;2014.10\n" 
        "Aus;10;;\n"
        "UK;;10;20\n";

      istringstream is(input);
      
      separator=';';
      setDataArea(1,1);
      numCols=4;
      missingValue=-1;
      headerRow=0;
      dimensionNames={"country","2014.01","2014.02","2014.10"};
      dimensionCols={0};
      horizontalDimName="date";
      horizontalDimension.type=Dimension::time;
      
      VariableValue v(VariableType::parameter);
      loadValueFromCSVFile(v,is,*this);

      CHECK_EQUAL(2, v.rank());
      CHECK_ARRAY_EQUAL(vector<unsigned>({2,3}),v.hypercube().dims(),2);
      CHECK_EQUAL("country", v.hypercube().xvectors[0].name);
      CHECK_EQUAL("Aus", str(v.hypercube().xvectors[0][0]));
      CHECK_EQUAL("UK", str(v.hypercube().xvectors[0][1]));
      CHECK_EQUAL("date", v.hypercube().xvectors[1].name);
      CHECK_EQUAL("2014-01-01T00:00:00", str(v.hypercube().xvectors[1][0]));
      CHECK_EQUAL("2014-02-01T00:00:00", str(v.hypercube().xvectors[1][1]));
      CHECK_EQUAL("2014-10-01T00:00:00", str(v.hypercube().xvectors[1][2]));
      CHECK(v.hypercube().dims()==v.tensorInit.hypercube().dims());
      CHECK_EQUAL(6, v.tensorInit.size());
      CHECK_ARRAY_CLOSE(vector<double>({10,-1,-1,10,-1,20}),
                        v.tensorInit, 6, 1e-4);
    }

   TEST_FIXTURE(DataSpec,loadVarSpace)
    {
      string input="A comment\n"
        "  foobar\n" // horizontal dim name
        "\"foo\" bar \\A B C\n"
        "A A 1.2 1.3 1.4\n"
        "A B 1 2 3\n"
        "B A 3 2 1\n";
      istringstream is(input);
      
      separator=' ';
      setDataArea(3,2);
      numCols=5;
      missingValue=-1;
      headerRow=2;
      dimensionNames={"foo","bar","A","B","C"};
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

#if 0
  // disabled because of temporary change to CSVParser.cc:502
  TEST_FIXTURE(DataSpec,loadVarSparse)
    {
      string input="A comment\n"
        ";;foobar\n" // horizontal dim name
        "foo;bar;A;B;C\n"
        "A;A;;1.3;1.4\n"
        "A;B;1;;\n"
        "B;A;;2;\n";
      istringstream is(input);
      
      separator=';';
      setDataArea(3,2);
      numCols=5;
      headerRow=2;
      dimensionNames={"foo","bar","A","B","C"};
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
      CHECK_EQUAL(4, v.tensorInit.size());
      CHECK_ARRAY_EQUAL(vector<unsigned>({2,4,5,8}),v.tensorInit.index(), 4);
      CHECK_ARRAY_CLOSE(vector<double>({1,1.3,2,1.4}),v.tensorInit, 4, 1e-4);
    }
#endif

  TEST_FIXTURE(DataSpec, duplicateActions)
    {
      string input="A comment\n"
        ";;foobar\n" // horizontal dim name
        "foo;bar;A;B;C\n"
        "A;A;1.2;1.3;1.4\n"
        "A;A;1;2;3\n"
        "B;A;3;2;1\n";
      
      separator=';';
      setDataArea(3,2);
      numCols=5;
      missingValue=-1;
      headerRow=2;
      dimensionNames={"foo","bar","A","B","C"};
      dimensionCols={0,1};
      horizontalDimName="foobar";

      VariableValue v(VariableType::parameter);
      {
        istringstream is(input);
        CHECK_THROW(loadValueFromCSVFile(v,is,*this), std::exception);
      }
      
      {
        istringstream is(input);
        duplicateKeyAction=sum;
        loadValueFromCSVFile(v,is,*this);
        CHECK_EQUAL(2.2, v.tensorInit[0]);
      }
      
      {
        istringstream is(input);
        duplicateKeyAction=product;
        loadValueFromCSVFile(v,is,*this);
        CHECK_EQUAL(1.2, v.tensorInit[0]);
      }
      
      {
        istringstream is(input);
        duplicateKeyAction=min;
        loadValueFromCSVFile(v,is,*this);
        CHECK_EQUAL(1, v.tensorInit[0]);
      }
      
      {
        istringstream is(input);
        duplicateKeyAction=max;
        loadValueFromCSVFile(v,is,*this);
        CHECK_EQUAL(1.2, v.tensorInit[0]);
      }
      
      {
        istringstream is(input);
        duplicateKeyAction=av;
        loadValueFromCSVFile(v,is,*this);
        CHECK_EQUAL(1.1, v.tensorInit[0]);
      }
    }

  TEST_FIXTURE(DataSpec, toggleDimensions)
    {
      toggleDimension(2);
      CHECK_EQUAL(1,dimensionCols.count(2));
      toggleDimension(2);
      CHECK_EQUAL(0,dimensionCols.count(2));
    }
  
  TEST(guessFromVariableExport)
    {
      Hypercube hc;
      hc.xvectors=
        {
          XVector("height",{Dimension::value,""},{"1.5","2","2.5"}),
          XVector("sex",{Dimension::string,""},{"male","female"}),
          XVector("date",{Dimension::time,"%Y"},{"2010","2011","2012"})
        };
      
      VariableValue v(VariableType::flow);
      v.hypercube(hc);
      for (size_t i=0; i<v.size(); ++i)
        v[i]=rand();
      v.exportAsCSV("tmp.csv");

      DataSpec spec;
      {
        ifstream f("tmp.csv");
        spec.guessFromStream(f);
      }
      VariableValue newV(VariableType::flow);
      loadValueFromCSVFile(newV,{"tmp.csv"},spec);

      CHECK(newV.hypercube().xvectors==v.hypercube().xvectors);
      CHECK_EQUAL(v.size(), newV.size());
      for (size_t i=0; i<v.size(); ++i)
        CHECK_CLOSE(v[i], newV.tensorInit[i], 0.001*v[1]);
    }

  string testEscapeDoubledQuotes(string x)
  {
    DataSpec spec;
    spec.quote='\'';
    spec.separator=',';
    spec.escape='&';
    escapeDoubledQuotes(x,spec);
    return x;
  }
  
  TEST(escapeDoubledQuotes)
    {
      CHECK_EQUAL("foo",testEscapeDoubledQuotes("foo"));
      CHECK_EQUAL("'foo'",testEscapeDoubledQuotes("'foo'"));
      CHECK_EQUAL("&'foo&'",testEscapeDoubledQuotes("''foo''"));                 // not strictly CSV standard
      CHECK_EQUAL("'&'foo&''",testEscapeDoubledQuotes("'''foo'''"));
      CHECK_EQUAL("'&'&''",testEscapeDoubledQuotes("''''''"));
      CHECK_EQUAL("'fo&'o'",testEscapeDoubledQuotes("'fo''o'"));
      CHECK_EQUAL("foo,bar",testEscapeDoubledQuotes("foo,bar"));
      CHECK_EQUAL("'foo','bar'",testEscapeDoubledQuotes("'foo','bar'"));
      CHECK_EQUAL("&'foo&',&'bar&'",testEscapeDoubledQuotes("''foo'',''bar''")); // not strictly CSV standard
      CHECK_EQUAL("'&'foo&'','&'bar&''",testEscapeDoubledQuotes("'''foo''','''bar'''"));
      CHECK_EQUAL("'fo&'o','b&'ar'",testEscapeDoubledQuotes("'fo''o','b''ar'"));
    }
  
  TEST(isNumerical)
    {
      CHECK(isNumerical(""));
      CHECK(isNumerical("1.2"));
      CHECK(isNumerical("'1.2'"));
      CHECK(isNumerical("-1.2"));
      CHECK(isNumerical("1e2"));
      CHECK(isNumerical("NaN"));
      CHECK(isNumerical("nan"));
      CHECK(isNumerical("inf"));
      CHECK(isNumerical("inf"));
      // leading nonumerical strings are considered non-numerical, but
      // will be parsed as numbers anyway if in data column
      CHECK(!isNumerical("$100"));
      CHECK(!isNumerical("£100"));
      CHECK(!isNumerical("hello"));
    }
}
