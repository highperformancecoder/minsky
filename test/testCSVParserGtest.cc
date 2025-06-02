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
#undef True
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <numeric>

using namespace minsky;
using namespace std;
using namespace civita;

// Test fixture for DataSpec tests
class DataSpecTest : public ::testing::Test
{
protected:
    DataSpec spec;
    
    void SetUp() override
    {
        // Initialize spec if needed
    }
};

// Test fixture for CSVDialog tests
class CSVDialogTest : public ::testing::Test
{
protected:
    CSVDialog dialog;
    
    void SetUp() override
    {
        // Initialize dialog if needed
    }
};

TEST_F(DataSpecTest, Guess)
{
    string input="A comment\n"
        ";;foobar\n" // horizontal dim name
        "foo;bar;A;B;C\n"
        "A;A;1.2;1.3;1.4\n"
        "A;B;1;2;3\n"
        "B;A;3;2;1\n";

    istringstream is(input);
    spec.guessFromStream(is);

    EXPECT_EQ(';', spec.separator);
    EXPECT_EQ(3, spec.nRowAxes());
    EXPECT_EQ(2, spec.nColAxes());
    EXPECT_EQ(2, spec.headerRow);
    EXPECT_EQ((set<unsigned>{0,1}), spec.dimensionCols);
}

TEST_F(DataSpecTest, GuessSpace)
{
    string input="foo bar  A B C\n"
        "A A 1.2 1.3  1.4\n"
        "A B  1 2 3\n"
        "B A 3 2 1\n";

    istringstream is(input);
    spec.guessFromStream(is);

    EXPECT_EQ(' ', spec.separator);
    EXPECT_EQ(1, spec.nRowAxes());
    EXPECT_EQ(2, spec.nColAxes());
    EXPECT_EQ(0, spec.headerRow);
    EXPECT_EQ((set<unsigned>{0,1}), spec.dimensionCols);
}

TEST_F(DataSpecTest, GuessTab)
{
    string input="A comment\n"
        "\t\tfoobar\n" // horizontal dim name
        "foo\tbar\tA\tB\tC\n"
        "A\tA\t1.2\t1.3\t1.4\n"
        "A\tB\t1\t2\t3\n"
        "B\tA\t3\t2\t1\n";

    istringstream is(input);
    spec.guessFromStream(is);

    EXPECT_EQ('\t', spec.separator);
    EXPECT_EQ(3, spec.nRowAxes());
    EXPECT_EQ(2, spec.nColAxes());
    EXPECT_EQ(2, spec.headerRow);
    EXPECT_EQ((set<unsigned>{0,1}), spec.dimensionCols);
}

TEST_F(DataSpecTest, GuessColumnar)
{
    string input="Country,Time_Period,value$\n"
        "Australia,1967-Q4,1.8\n"
        "Australia,1968-Q1,1.9\n";

    istringstream is(input);
    spec.guessFromStream(is);

    EXPECT_EQ(',', spec.separator);
    EXPECT_EQ(1, spec.nRowAxes());
    EXPECT_EQ(2, spec.nColAxes());
    EXPECT_EQ(0, spec.headerRow);
    EXPECT_EQ((set<unsigned>{0,1}), spec.dimensionCols);
    EXPECT_EQ((set<unsigned>{2}), spec.dataCols);
}

TEST_F(DataSpecTest, BadTimeData)
{
    string input="time,data\n"
        "1966-Q,1\n";
    istringstream is(input);
    spec.setDataArea(1,1);
    spec.dimensionCols={0};
    spec.dimensions[0].type=Dimension::time;
    spec.dimensions[0].units="%Y-Q%Q";
    VariableValue v(VariableType::parameter,":foo");
    EXPECT_THROW(loadValueFromCSVFile(v,is,spec), std::exception);
    
    is.clear(); 
    is.seekg(0);
    try {
        loadValueFromCSVFile(v,is,spec);
    }
    catch (const std::exception& ex) {
        EXPECT_TRUE(string(ex.what()).starts_with("Invalid"));
    }

    is.clear(); 
    is.seekg(0);
    string output;  
    ostringstream os(output);
    reportFromCSVFile(is,os,spec,2);
    EXPECT_NE(string::npos, os.str().find("Invalid data"));
}

TEST_F(DataSpecTest, ReportFromCSV)
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
    spec.guessFromStream(is);
    is.clear();
    is.seekg(0);
    spec.setDataArea(3,2);
    spec.dataCols={2,3,4};
    spec.dimensionNames={"foo","bar","A","B","C"};
    
    reportFromCSVFile(is,os,spec,6);

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
    
    EXPECT_EQ(numLines, accumulate(osStr.begin(), osStr.end(), 0, Count('\n')));
    EXPECT_EQ(numSep+(numLines-spec.nRowAxes()+1),
              accumulate(osStr.begin(), osStr.end(), 0, Count(';')));
    
    EXPECT_NE(string::npos, os.str().find("Error"));
    EXPECT_NE(string::npos, os.str().find("Invalid data"));
    EXPECT_NE(string::npos, os.str().find("Duplicate key"));
}

TEST_F(CSVDialogTest, ClassifyColumns)
{
    string input="10,2022/10/2,hello,\n"
        "'5,150,000','2023/1/3','foo bar',\n"
        "2 00,2023/1/3,ggg,\n";

    dialog.url=boost::filesystem::unique_path().string();
    {
        ofstream of(dialog.url);
        of<<input;
    }

    dialog.spec.quote='\'';
    dialog.spec.setDataArea(0,4);
    dialog.loadFile();
    dialog.classifyColumns();
    EXPECT_EQ(4, dialog.spec.numCols);
    EXPECT_TRUE(dialog.spec.dataCols.count(0));
    EXPECT_FALSE(dialog.spec.dimensionCols.count(0));
    EXPECT_FALSE(dialog.spec.dataCols.count(1));
    EXPECT_TRUE(dialog.spec.dimensionCols.count(1));
    EXPECT_EQ(Dimension::time, dialog.spec.dimensions[1].type);
    EXPECT_FALSE(dialog.spec.dataCols.count(2));
    EXPECT_TRUE(dialog.spec.dimensionCols.count(2));
    EXPECT_EQ(Dimension::string, dialog.spec.dimensions[2].type);
    EXPECT_FALSE(dialog.spec.dataCols.count(3));
    EXPECT_FALSE(dialog.spec.dimensionCols.count(3));
}

TEST_F(DataSpecTest, LoadVar)
{
    string input="A comment\n"
        ";;foobar\n" // horizontal dim name
        "foo;bar;A;B;C\n"
        "A;A;1.2;1.3;-1.4\n"
        "A;B;1;2;£3\n"
        "B;A;£-3.1;.2;-£5\n";

    istringstream is(input);
    
    spec.separator=';';
    spec.setDataArea(3,2);
    spec.numCols=5;
    spec.missingValue=-1;
    spec.headerRow=2;
    spec.dimensionNames={"foo","bar","A","B","C"};
    spec.dimensionCols={0,1};
    spec.horizontalDimName="foobar";
    
    VariableValue v(VariableType::parameter);
    loadValueFromCSVFile(v,is,spec);

    EXPECT_EQ(3, v.rank());
    vector<unsigned> expectedDims = {2,2,3};
    auto dims=v.hypercube().dims();
    EXPECT_EQ(expectedDims, dims);
    EXPECT_EQ("foo", v.hypercube().xvectors[0].name);
    EXPECT_EQ("A", str(v.hypercube().xvectors[0][0]));
    EXPECT_EQ("B", str(v.hypercube().xvectors[0][1]));
    EXPECT_EQ("bar", v.hypercube().xvectors[1].name);
    EXPECT_EQ("A", str(v.hypercube().xvectors[1][0]));
    EXPECT_EQ("B", str(v.hypercube().xvectors[1][1]));
    EXPECT_EQ("foobar", v.hypercube().xvectors[2].name);
    EXPECT_EQ("A", str(v.hypercube().xvectors[2][0]));
    EXPECT_EQ("B", str(v.hypercube().xvectors[2][1]));
    EXPECT_EQ("C", str(v.hypercube().xvectors[2][2]));
    EXPECT_TRUE(v.hypercube().dims()==v.tensorInit.hypercube().dims());
    EXPECT_EQ(12, v.tensorInit.size());
    
    vector<double> expected = {1.2,-3.1,1,-1,1.3,.2,2,-1,-1.4,-5,3,-1};
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_NEAR(expected[i], v.tensorInit[i], 1e-4);
    }
}

TEST_F(DataSpecTest, EmbeddedNewline)
{
    string input="A comment\n"
        ",,foobar\n" // horizontal dim name
        "foo,bar,A,B,C\n"
        "A,'A\nB',1.2,1.3,'1,000'\n"
        "A,B,1,'2\n.0',3\n"
        "B,AB,3,2,1\n";

    istringstream is(input);
    
    spec.separator=',';
    spec.quote='\'';
    spec.setDataArea(3,2);
    spec.numCols=5;
    spec.missingValue=-1;
    spec.headerRow=2;
    spec.dimensionNames={"foo","bar","A","B","C"};
    spec.dimensionCols={0,1};
    spec.horizontalDimName="foobar";
    
    VariableValue v(VariableType::parameter);
    loadValueFromCSVFile(v,is,spec);

    EXPECT_EQ(3, v.rank());
    vector<unsigned> expectedDims = {2,2,3};
    auto dims=v.hypercube().dims();
    EXPECT_EQ(expectedDims, dims);
    EXPECT_EQ("foo", v.hypercube().xvectors[0].name);
    EXPECT_EQ("A", str(v.hypercube().xvectors[0][0]));
    EXPECT_EQ("B", str(v.hypercube().xvectors[0][1]));
    EXPECT_EQ("bar", v.hypercube().xvectors[1].name);
    EXPECT_EQ("AB", str(v.hypercube().xvectors[1][0]));
    EXPECT_EQ("B", str(v.hypercube().xvectors[1][1]));
    EXPECT_EQ("foobar", v.hypercube().xvectors[2].name);
    EXPECT_EQ("A", str(v.hypercube().xvectors[2][0]));
    EXPECT_EQ("B", str(v.hypercube().xvectors[2][1]));
    EXPECT_EQ("C", str(v.hypercube().xvectors[2][2]));
    EXPECT_TRUE(v.hypercube().dims()==v.tensorInit.hypercube().dims());
    EXPECT_EQ(12, v.tensorInit.size());
    
    vector<double> expected = {1.2,3,1,-1,1.3,2,2,-1,1000,1,3,-1};
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_NEAR(expected[i], v.tensorInit[i], 1e-4);
    }
}

TEST_F(DataSpecTest, JanuaryDoubleDip)
{
    string input=
        "country;2014;2014-01;2014-02\n" 
        "Aus;10;;\n"
        "UK;;10;20\n";

    istringstream is(input);
    
    spec.separator=';';
    spec.setDataArea(1,1);
    spec.numCols=4;
    spec.missingValue=-1;
    spec.headerRow=0;
    spec.dimensionNames={"country","2014","2014-01","2014-02"};
    spec.dimensionCols={0};
    spec.horizontalDimName="date";
    spec.horizontalDimension.type=Dimension::time;
    
    VariableValue v(VariableType::parameter);
    loadValueFromCSVFile(v,is,spec);

    EXPECT_EQ(2, v.rank());
    vector<unsigned> expectedDims = {2,2};
    auto dims=v.hypercube().dims();
    EXPECT_EQ(expectedDims, dims);
    EXPECT_EQ("country", v.hypercube().xvectors[0].name);
    EXPECT_EQ("Aus", str(v.hypercube().xvectors[0][0]));
    EXPECT_EQ("UK", str(v.hypercube().xvectors[0][1]));
    EXPECT_EQ("date", v.hypercube().xvectors[1].name);
    EXPECT_EQ("2014-01-01T00:00:00", str(v.hypercube().xvectors[1][0]));
    EXPECT_EQ("2014-02-01T00:00:00", str(v.hypercube().xvectors[1][1]));
    EXPECT_TRUE(v.hypercube().dims()==v.tensorInit.hypercube().dims());
    EXPECT_EQ(4, v.tensorInit.size());
    
    vector<double> expected = {10,10,-1,20};
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_NEAR(expected[i], v.tensorInit[i], 1e-4);
    }
}

TEST_F(DataSpecTest, DotTimeSep)
{
    string input=
        "country;2014.01;2014.02;2014.10\n" 
        "Aus;10;;\n"
        "UK;;10;20\n";

    istringstream is(input);
    
    spec.separator=';';
    spec.setDataArea(1,1);
    spec.numCols=4;
    spec.missingValue=-1;
    spec.headerRow=0;
    spec.dimensionNames={"country","2014.01","2014.02","2014.10"};
    spec.dimensionCols={0};
    spec.horizontalDimName="date";
    spec.horizontalDimension.type=Dimension::time;
    
    VariableValue v(VariableType::parameter);
    loadValueFromCSVFile(v,is,spec);

    EXPECT_EQ(2, v.rank());
    vector<unsigned> expectedDims = {2,3};
    auto dims=v.hypercube().dims();
    EXPECT_EQ(expectedDims, dims);
    EXPECT_EQ("country", v.hypercube().xvectors[0].name);
    EXPECT_EQ("Aus", str(v.hypercube().xvectors[0][0]));
    EXPECT_EQ("UK", str(v.hypercube().xvectors[0][1]));
    EXPECT_EQ("date", v.hypercube().xvectors[1].name);
    EXPECT_EQ("2014-01-01T00:00:00", str(v.hypercube().xvectors[1][0]));
    EXPECT_EQ("2014-02-01T00:00:00", str(v.hypercube().xvectors[1][1]));
    EXPECT_EQ("2014-10-01T00:00:00", str(v.hypercube().xvectors[1][2]));
    EXPECT_TRUE(v.hypercube().dims()==v.tensorInit.hypercube().dims());
    EXPECT_EQ(6, v.tensorInit.size());
    
    vector<double> expected = {10,-1,-1,10,-1,20};
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_NEAR(expected[i], v.tensorInit[i], 1e-4);
    }
}

TEST_F(DataSpecTest, LoadVarSpace)
{
    string input="A comment\n"
        "  foobar\n" // horizontal dim name
        "\"foo\" bar \\A B C\n"
        "A A 1.2 1.3 1.4\n"
        "A B 1 2 3\n"
        "B A 3 2 1\n";
    istringstream is(input);
    
    spec.separator=' ';
    spec.setDataArea(3,2);
    spec.numCols=5;
    spec.missingValue=-1;
    spec.headerRow=2;
    spec.dimensionNames={"foo","bar","A","B","C"};
    spec.dimensionCols={0,1};
    spec.horizontalDimName="foobar";
    
    VariableValue v;
    loadValueFromCSVFile(v,is,spec);

    EXPECT_EQ(3, v.rank());
    vector<unsigned> expectedDims = {2,2,3};
    auto dims=v.hypercube().dims();
    EXPECT_EQ(expectedDims, dims);
    EXPECT_EQ("foo", v.hypercube().xvectors[0].name);
    EXPECT_EQ("A", str(v.hypercube().xvectors[0][0]));
    EXPECT_EQ("B", str(v.hypercube().xvectors[0][1]));
    EXPECT_EQ("bar", v.hypercube().xvectors[1].name);
    EXPECT_EQ("A", str(v.hypercube().xvectors[1][0]));
    EXPECT_EQ("B", str(v.hypercube().xvectors[1][1]));
    EXPECT_EQ("foobar", v.hypercube().xvectors[2].name);
    EXPECT_EQ("A", str(v.hypercube().xvectors[2][0]));
    EXPECT_EQ("B", str(v.hypercube().xvectors[2][1]));
    EXPECT_EQ("C", str(v.hypercube().xvectors[2][2]));
    EXPECT_TRUE(v.hypercube().dims()==v.tensorInit.hypercube().dims());
    EXPECT_EQ(12, v.tensorInit.size());
    
    vector<double> expected = {1.2,3,1,-1,1.3,2,2,-1,1.4,1,3,-1};
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_NEAR(expected[i], v.tensorInit[i], 1e-4);
    }
}

TEST_F(DataSpecTest, DuplicateActions)
{
    string input="A comment\n"
        ";;foobar\n" // horizontal dim name
        "foo;bar;A;B;C\n"
        "A;A;1.2;1.3;1.4\n"
        "A;A;1;2;3\n"
        "B;A;3;2;1\n";
    
    spec.separator=';';
    spec.setDataArea(3,2);
    spec.numCols=5;
    spec.missingValue=-1;
    spec.headerRow=2;
    spec.dimensionNames={"foo","bar","A","B","C"};
    spec.dimensionCols={0,1};
    spec.horizontalDimName="foobar";

    VariableValue v(VariableType::parameter);
    {
        istringstream is(input);
        EXPECT_THROW(loadValueFromCSVFile(v,is,spec), std::exception);
    }
    
    {
        istringstream is(input);
        spec.duplicateKeyAction=DataSpec::sum;
        loadValueFromCSVFile(v,is,spec);
        EXPECT_EQ(2.2, v.tensorInit[0]);
    }
    
    {
        istringstream is(input);
        spec.duplicateKeyAction=DataSpec::product;
        loadValueFromCSVFile(v,is,spec);
        EXPECT_EQ(1.2, v.tensorInit[0]);
    }
    
    {
        istringstream is(input);
        spec.duplicateKeyAction=DataSpec::min;
        loadValueFromCSVFile(v,is,spec);
        EXPECT_EQ(1, v.tensorInit[0]);
    }
    
    {
        istringstream is(input);
        spec.duplicateKeyAction=DataSpec::max;
        loadValueFromCSVFile(v,is,spec);
        EXPECT_EQ(1.2, v.tensorInit[0]);
    }
    
    {
        istringstream is(input);
        spec.duplicateKeyAction=DataSpec::av;
        loadValueFromCSVFile(v,is,spec);
        EXPECT_EQ(1.1, v.tensorInit[0]);
    }
}

TEST_F(DataSpecTest, ToggleDimensions)
{
    spec.toggleDimension(2);
    EXPECT_EQ(1, spec.dimensionCols.count(2));
    spec.toggleDimension(2);
    EXPECT_EQ(0, spec.dimensionCols.count(2));
}

TEST(CSVParserTest, GuessFromVariableExport)
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
    {
        ifstream f("tmp.csv");
        loadValueFromCSVFile(newV,f,spec);
    }

    EXPECT_TRUE(newV.hypercube().xvectors==v.hypercube().xvectors);
    EXPECT_EQ(v.size(), newV.size());
    for (size_t i=0; i<v.size(); ++i)
        EXPECT_NEAR(v[i], newV.tensorInit[i], 0.001*v[1]);
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

TEST(CSVParserTest, EscapeDoubledQuotes)
{
    EXPECT_EQ("foo", testEscapeDoubledQuotes("foo"));
    EXPECT_EQ("'foo'", testEscapeDoubledQuotes("'foo'"));
    EXPECT_EQ("&'foo&'", testEscapeDoubledQuotes("''foo''"));                 // not strictly CSV standard
    EXPECT_EQ("'&'foo&''", testEscapeDoubledQuotes("'''foo'''"));
    EXPECT_EQ("'&'&''", testEscapeDoubledQuotes("''''''"));
    EXPECT_EQ("'fo&'o'", testEscapeDoubledQuotes("'fo''o'"));
    EXPECT_EQ("foo,bar", testEscapeDoubledQuotes("foo,bar"));
    EXPECT_EQ("'foo','bar'", testEscapeDoubledQuotes("'foo','bar'"));
    EXPECT_EQ("&'foo&',&'bar&'", testEscapeDoubledQuotes("''foo'',''bar''")); // not strictly CSV standard
    EXPECT_EQ("'&'foo&'','&'bar&''", testEscapeDoubledQuotes("'''foo''','''bar'''"));
    EXPECT_EQ("'fo&'o','b&'ar'", testEscapeDoubledQuotes("'fo''o','b''ar'"));
}

TEST(CSVParserTest, IsNumerical)
{
    EXPECT_TRUE(isNumerical(""));
    EXPECT_TRUE(isNumerical("1.2"));
    EXPECT_TRUE(isNumerical("'1.2'"));
    EXPECT_TRUE(isNumerical("-1.2"));
    EXPECT_TRUE(isNumerical("1e2"));
    EXPECT_TRUE(isNumerical("NaN"));
    EXPECT_TRUE(isNumerical("nan"));
    EXPECT_TRUE(isNumerical("inf"));
    EXPECT_TRUE(isNumerical("inf"));
    // leading nonumerical strings are considered non-numerical, but
    // will be parsed as numbers anyway if in data column
    EXPECT_FALSE(isNumerical("$100"));
    EXPECT_FALSE(isNumerical("£100"));
    EXPECT_FALSE(isNumerical("hello"));
}

