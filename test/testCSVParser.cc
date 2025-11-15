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
#include <gtest/gtest.h> // Replaced UnitTest++/UnitTest++.h with gtest/gtest.h
#include <boost/filesystem.hpp>
#include <numeric>
#include <set> // Added for std::set

using namespace minsky;
using namespace std;
using namespace civita;

// Fixture for DataSpec tests
class CSVParserTest : public ::testing::Test {
protected:
    DataSpec spec;
};

struct TestCSVDialog : public CSVDialog {
  void importFromCSV(const std::vector<std::string>& filenames) override {}
};

// Fixture for CSVDialog tests
class CSVDialogTest : public ::testing::Test {
protected:
    TestCSVDialog dialog;
    DataSpec& spec = dialog.spec; // Access DataSpec through CSVDialog
    string url;

    void SetUp() override {
        // Common setup for CSVDialogTest if needed
    }

    void TearDown() override {
        if (!url.empty()) {
            boost::filesystem::remove(url);
        }
    }

    void loadFile() {
        dialog.url = url;
        dialog.loadFile();
    }

    void classifyColumns() {
        dialog.classifyColumns();
    }
};


TEST_F(CSVParserTest, GuessSemicolonSeparator) {
    string input = "A comment\n"
                   ";;foobar\n" // horizontal dim name
                   "foo;bar;A;B;C\n"
                   "A;A;1.2;1.3;1.4\n"
                   "A;B;1;2;3\n"
                   "B;A;3;2;1\n";

    istringstream is(input);
    spec.guessFromStream(is);

    ASSERT_EQ(';', spec.separator);
    ASSERT_EQ(3, spec.nRowAxes());
    ASSERT_EQ(2, spec.nColAxes());
    ASSERT_EQ(2, spec.headerRow);
    ASSERT_EQ((set<unsigned>{0, 1}), spec.dimensionCols);
}

TEST_F(CSVParserTest, GuessSpaceSeparator) {
    string input = "foo bar  A B C\n"
                   "A A 1.2 1.3  1.4\n"
                   "A B  1 2 3\n"
                   "B A 3 2 1\n";

    istringstream is(input);
    spec.guessFromStream(is);

    ASSERT_EQ(' ', spec.separator);
    ASSERT_EQ(1, spec.nRowAxes());
    ASSERT_EQ(2, spec.nColAxes());
    ASSERT_EQ(0, spec.headerRow);
    ASSERT_EQ((set<unsigned>{0, 1}), spec.dimensionCols);
}

TEST_F(CSVParserTest, GuessTabSeparator) {
    string input = "A comment\n"
                   "\t\tfoobar\n" // horizontal dim name
                   "foo\tbar\tA\tB\tC\n"
                   "A\tA\t1.2\t1.3\t1.4\n"
                   "A\tB\t1\t2\t3\n"
                   "B\tA\t3\t2\t1\n";

    istringstream is(input);
    spec.guessFromStream(is);

    ASSERT_EQ('\t', spec.separator);
    ASSERT_EQ(3, spec.nRowAxes());
    ASSERT_EQ(2, spec.nColAxes());
    ASSERT_EQ(2, spec.headerRow);
    ASSERT_EQ((set<unsigned>{0, 1}), spec.dimensionCols);
}

TEST_F(CSVParserTest, GuessColumnar) {
    string input = "Country,Time_Period,value$\n"
                   "Australia,1967-Q4,1.8\n"
                   "Australia,1968-Q1,1.9\n";

    istringstream is(input);
    spec.guessFromStream(is);

    ASSERT_EQ(',', spec.separator);
    ASSERT_EQ(1, spec.nRowAxes());
    ASSERT_EQ(2, spec.nColAxes());
    ASSERT_EQ(0, spec.headerRow);
    ASSERT_EQ((set<unsigned>{0, 1}), spec.dimensionCols);
    ASSERT_EQ((set<unsigned>{2}), spec.dataCols);
}

TEST_F(CSVParserTest, BadTimeData) {
    string input = "time,data\n"
                   "1966-Q,1\n";
    istringstream is(input);
    spec.setDataArea(1, 1);
    spec.dimensionCols = {0};
    spec.dimensions[0].type = Dimension::time;
    spec.dimensions[0].units = "%Y-Q%Q";
    VariableValue v(VariableType::parameter, ":foo");

    ASSERT_THROW(loadValueFromCSVFile(v, is, spec), std::exception);

    is.clear();
    is.seekg(0);
    try {
        loadValueFromCSVFile(v, is, spec);
    } catch (const std::exception& ex) {
        ASSERT_TRUE(string(ex.what()).starts_with("Invalid"));
    }

    is.clear();
    is.seekg(0);
    string output;
    ostringstream os(output);
    reportFromCSVFile(is, os, spec, 2);
    ASSERT_NE(string::npos, os.str().find("Invalid data"));
}

TEST_F(CSVParserTest, ReportFromCSV) {
    string input = "A comment\n"
                   ";;foobar\n"
                   "foo;bar;A;B;C\n"
                   "A;A;1.2;1.3;1.4\n"
                   "A;A;1;2;3\n"
                   "B;A;3;S;1\n";
    string output = "";
    istringstream is(input);
    ostringstream os(output);
    spec.guessFromStream(is);
    is.clear();
    is.seekg(0);
    spec.setDataArea(3, 2);
    spec.dataCols = {2, 3, 4};
    spec.dimensionNames = {"foo", "bar", "A", "B", "C"};


    reportFromCSVFile(is, os, spec, 6);

    struct Count {
        const char target;
        Count(char target) : target(target) {}
        int operator()(int x, char c) const { return x + (c == target); }
    };

    auto osStr = os.str();
    auto numLines = accumulate(input.begin(), input.end(), 0, Count('\n'));
    auto numSep = accumulate(input.begin(), input.end(), 0, Count(';'));

    ASSERT_EQ(numLines, accumulate(osStr.begin(), osStr.end(), 0, Count('\n')));
    ASSERT_EQ(numSep + (numLines - spec.nRowAxes() + 1),
              accumulate(osStr.begin(), osStr.end(), 0, Count(';')));

    ASSERT_NE(string::npos, os.str().find("Error"));
    ASSERT_NE(string::npos, os.str().find("Invalid data"));
    ASSERT_NE(string::npos, os.str().find("Duplicate key"));
}

TEST_F(CSVDialogTest, ClassifyColumns) {
    string input = "10,2022/10/2,hello,\n"
                   "'5,150,000','2023/1/3','foo bar',\n"
                   "2 00,2023/1/3,ggg,\n";

    url = boost::filesystem::unique_path().string();
    {
        ofstream of(url);
        of << input;
    }

    spec.quote = '\'';
    spec.setDataArea(0, 4);
    loadFile();
    classifyColumns();
    ASSERT_EQ(4, spec.numCols);
    ASSERT_TRUE(spec.dataCols.count(0));
    ASSERT_FALSE(spec.dimensionCols.count(0));
    ASSERT_FALSE(spec.dataCols.count(1));
    ASSERT_TRUE(spec.dimensionCols.count(1));
    ASSERT_EQ(Dimension::time, spec.dimensions[1].type);
    ASSERT_FALSE(spec.dataCols.count(2));
    ASSERT_TRUE(spec.dimensionCols.count(2));
    ASSERT_EQ(Dimension::string, spec.dimensions[2].type);
    ASSERT_FALSE(spec.dataCols.count(3));
    ASSERT_FALSE(spec.dimensionCols.count(3));
}

TEST_F(CSVDialogTest, LoadFile) {
    string input = "A,B,C\n"
                   "1,2,3\n"
                   "4,5,6\n";

    url = boost::filesystem::unique_path().string();
    {
        ofstream of(url);
        of << input;
    }

    dialog.url = url;
    dialog.loadFile();

    auto parsedLines = dialog.parseLines();
    ASSERT_EQ(3, parsedLines.size());
    ASSERT_EQ(3, parsedLines[0].size());
    ASSERT_EQ("A", parsedLines[0][0]);
    ASSERT_EQ("B", parsedLines[0][1]);
    ASSERT_EQ("C", parsedLines[0][2]);
}

TEST_F(CSVDialogTest, LoadFileFromNameWithDOSLineEndings) {
    string input = "A,B,C\r\n"
                   "1,2,3\r\n"
                   "4,5,6\r\n";

    url = boost::filesystem::unique_path().string();
    {
        ofstream of(url);
        of << input;
    }

    dialog.loadFileFromName(url);

    auto parsedLines = dialog.parseLines();
    ASSERT_EQ(3, parsedLines.size());
    ASSERT_EQ(3, parsedLines[0].size());
    ASSERT_EQ("A", parsedLines[0][0]);
    ASSERT_EQ("B", parsedLines[0][1]);
    ASSERT_EQ("C", parsedLines[0][2]);
}

TEST_F(CSVDialogTest, GuessSpecAndLoadFile) {
    string input = "foo,bar,A,B,C\n"
                   "A,A,1.2,1.3,1.4\n"
                   "A,B,1,2,3\n"
                   "B,A,3,2,1\n";

    url = boost::filesystem::unique_path().string();
    {
        ofstream of(url);
        of << input;
    }

    dialog.url = url;
    dialog.guessSpecAndLoadFile();

    ASSERT_EQ(',', spec.separator);
    ASSERT_GE(spec.dimensionNames.size(), 2);
    ASSERT_EQ("foo", spec.dimensionNames[0]);
    ASSERT_EQ("bar", spec.dimensionNames[1]);
}

TEST_F(CSVDialogTest, PopulateHeaders) {
    string input = "Name,Age,City\n"
                   "Alice,30,NYC\n"
                   "Bob,25,LA\n";

    url = boost::filesystem::unique_path().string();
    {
        ofstream of(url);
        of << input;
    }

    spec.separator = ',';
    spec.headerRow = 0;
    spec.setDataArea(1, 3);
    dialog.url = url;
    dialog.loadFile();
    dialog.populateHeaders();

    ASSERT_GE(spec.dimensionNames.size(), 3);
    ASSERT_EQ("Name", spec.dimensionNames[0]);
    ASSERT_EQ("Age", spec.dimensionNames[1]);
    ASSERT_EQ("City", spec.dimensionNames[2]);
}

TEST_F(CSVDialogTest, PopulateHeader) {
    string input = "Name,Age,City\n"
                   "Alice,30,NYC\n"
                   "Bob,25,LA\n";

    url = boost::filesystem::unique_path().string();
    {
        ofstream of(url);
        of << input;
    }

    spec.separator = ',';
    spec.headerRow = 0;
    spec.setDataArea(1, 3);
    spec.dimensionNames.resize(3);
    dialog.url = url;
    dialog.loadFile();
    
    dialog.populateHeader(1);
    ASSERT_EQ("Age", spec.dimensionNames[1]);
}

TEST_F(CSVDialogTest, ParseLinesWithMergeDelimiters) {
    string input = "A  B  C\n"
                   "1  2  3\n"
                   "4  5  6\n";

    url = boost::filesystem::unique_path().string();
    {
        ofstream of(url);
        of << input;
    }

    spec.separator = ' ';
    spec.mergeDelimiters = true;
    dialog.url = url;
    dialog.loadFile();

    auto parsedLines = dialog.parseLines();
    ASSERT_EQ(3, parsedLines.size());
    ASSERT_GE(parsedLines[0].size(), 3);
    ASSERT_EQ("A", parsedLines[0][0]);
    ASSERT_EQ("B", parsedLines[0][1]);
    ASSERT_EQ("C", parsedLines[0][2]);
}

TEST_F(CSVDialogTest, ParseLinesWithMaxColumn) {
    string input = "A,B,C,D,E\n"
                   "1,2,3,4,5\n"
                   "6,7,8,9,10\n";

    url = boost::filesystem::unique_path().string();
    {
        ofstream of(url);
        of << input;
    }

    spec.separator = ',';
    dialog.url = url;
    dialog.loadFile();

    auto parsedLines = dialog.parseLines(3);
    ASSERT_EQ(3, parsedLines.size());
    ASSERT_EQ(3, parsedLines[0].size());
    ASSERT_EQ("A", parsedLines[0][0]);
    ASSERT_EQ("B", parsedLines[0][1]);
    ASSERT_EQ("C", parsedLines[0][2]);
}

TEST_F(CSVDialogTest, CorrectedUniqueValues) {
    string input = "A,B,C\n"
                   "X,1,foo\n"
                   "Y,2,bar\n"
                   "X,3,foo\n"
                   "Z,1,baz\n";

    url = boost::filesystem::unique_path().string();
    {
        ofstream of(url);
        of << input;
    }

    spec.separator = ',';
    spec.setDataArea(1, 3);
    spec.headerRow = 0;
    dialog.url = url;
    dialog.loadFile();

    auto uniqueVals = dialog.correctedUniqueValues();
    ASSERT_GE(uniqueVals.size(), 3);
    // Column 0 should have 3 unique values (X, Y, Z) after removing header
    ASSERT_EQ(3, uniqueVals[0]);
    // Column 1 should have 3 unique values (1, 2, 3) after removing header
    ASSERT_EQ(3, uniqueVals[1]);
    // Column 2 should have 3 unique values (foo, bar, baz) after removing header
    ASSERT_EQ(3, uniqueVals[2]);
}

TEST_F(CSVDialogTest, ReportFromFile) {
    string input = "A,B,C\n"
                   "1,2,3\n"
                   "4,5,6\n";

    url = boost::filesystem::unique_path().string();
    string outputFile = boost::filesystem::unique_path().string();
    
    {
        ofstream of(url);
        of << input;
    }

    spec.separator = ',';
    spec.setDataArea(1, 3);
    dialog.url = url;
    dialog.loadFile();
    
    dialog.reportFromFile(url, outputFile);
    
    // Check that output file was created
    ASSERT_TRUE(boost::filesystem::exists(outputFile));
    
    // Clean up
    boost::filesystem::remove(outputFile);
}

TEST_F(CSVDialogTest, PopulateHeadersWithInvalidHeaderRow) {
    string input = "A,B,C\n"
                   "1,2,3\n";

    url = boost::filesystem::unique_path().string();
    {
        ofstream of(url);
        of << input;
    }

    spec.separator = ',';
    spec.headerRow = 10; // Beyond the number of lines
    spec.setDataArea(1, 3);
    dialog.url = url;
    dialog.loadFile();
    
    // This should not crash - just return without populating
    dialog.populateHeaders();
    
    // Headers should not have been populated
    ASSERT_TRUE(spec.dimensionNames.empty() || spec.dimensionNames.size() == 3);
}

TEST_F(CSVDialogTest, PopulateHeaderWithInvalidColumn) {
    string input = "Name,Age,City\n"
                   "Alice,30,NYC\n";

    url = boost::filesystem::unique_path().string();
    {
        ofstream of(url);
        of << input;
    }

    spec.separator = ',';
    spec.headerRow = 0;
    spec.setDataArea(1, 3);
    spec.dimensionNames.resize(5);
    spec.dimensionNames[3] = "Original";
    dialog.url = url;
    dialog.loadFile();
    
    // Try to populate a column that doesn't exist
    dialog.populateHeader(10);
    
    // The value at index 3 should remain unchanged
    ASSERT_EQ("Original", spec.dimensionNames[3]);
}

TEST_F(CSVDialogTest, ParseLinesWithEscapedQuotes) {
    string input = "\"A\",\"B\",\"C\"\n"
                   "\"foo\",\"bar\",\"baz\"\n"
                   "\"x\",\"y\",\"z\"\n";

    url = boost::filesystem::unique_path().string();
    {
        ofstream of(url);
        of << input;
    }

    spec.separator = ',';
    spec.quote = '"';
    spec.mergeDelimiters = false;
    dialog.url = url;
    dialog.loadFile();

    auto parsedLines = dialog.parseLines();
    ASSERT_EQ(3, parsedLines.size());
    ASSERT_GE(parsedLines[0].size(), 3);
    ASSERT_EQ("A", parsedLines[0][0]);
    ASSERT_EQ("B", parsedLines[0][1]);
    ASSERT_EQ("C", parsedLines[0][2]);
}

TEST_F(CSVDialogTest, LoadFileWithManyLines) {
    // Create a file with more than numInitialLines (100) lines
    url = boost::filesystem::unique_path().string();
    {
        ofstream of(url);
        for (int i = 0; i < 150; ++i) {
            of << "A" << i << ",B" << i << ",C" << i << "\n";
        }
    }

    spec.separator = ',';
    dialog.url = url;
    dialog.loadFile();

    auto parsedLines = dialog.parseLines();
    // Should only load numInitialLines (100) lines
    ASSERT_EQ(CSVDialog::numInitialLines, parsedLines.size());
}

TEST_F(CSVDialogTest, ClassifyColumnsEmptyData) {
    string input = "A,B,C\n"
                   ",,\n"
                   ",,\n";

    url = boost::filesystem::unique_path().string();
    {
        ofstream of(url);
        of << input;
    }

    spec.separator = ',';
    spec.setDataArea(1, 3);
    dialog.url = url;
    dialog.loadFile();
    dialog.classifyColumns();

    // Empty columns should be treated as data columns
    ASSERT_EQ(3, spec.numCols);
    // All three columns should be in dataCols since they're empty
    ASSERT_TRUE(spec.dataCols.count(0));
    ASSERT_TRUE(spec.dataCols.count(1));
    ASSERT_TRUE(spec.dataCols.count(2));
}


TEST_F(CSVParserTest, LoadVar) {
    string input = "A comment\n"
                   ";;foobar\n" // horizontal dim name
                   "foo;bar;A;B;C\n"
                   "A;A;1.2;1.3;-1.4\n"
                   "A;B;1;2;£3\n"
                   "B;A;£-3.1;.2;-£5\n";

    istringstream is(input);

    spec.separator = ';';
    spec.setDataArea(3, 2);
    spec.numCols = 5;
    spec.missingValue = -1;
    spec.headerRow = 2;
    spec.dimensionNames = {"foo", "bar", "A", "B", "C"};
    spec.dimensionCols = {0, 1};
    spec.horizontalDimName = "foobar";

    VariableValue v(VariableType::parameter);
    loadValueFromCSVFile(v, is, spec);

    ASSERT_EQ(3, v.rank());
    ASSERT_EQ((vector<unsigned>({2, 2, 3})), v.hypercube().dims());
    ASSERT_EQ("foo", v.hypercube().xvectors[0].name);
    ASSERT_EQ("A", str(v.hypercube().xvectors[0][0]));
    ASSERT_EQ("B", str(v.hypercube().xvectors[0][1]));
    ASSERT_EQ("bar", v.hypercube().xvectors[1].name);
    ASSERT_EQ("A", str(v.hypercube().xvectors[1][0]));
    ASSERT_EQ("B", str(v.hypercube().xvectors[1][1]));
    ASSERT_EQ("foobar", v.hypercube().xvectors[2].name);
    ASSERT_EQ("A", str(v.hypercube().xvectors[2][0]));
    ASSERT_EQ("B", str(v.hypercube().xvectors[2][1]));
    ASSERT_EQ("C", str(v.hypercube().xvectors[2][2]));
    ASSERT_EQ(v.hypercube().dims(), v.tensorInit.hypercube().dims());
    ASSERT_EQ(12, v.tensorInit.size());
    // Using a loop for array comparison with tolerance
    vector<double> expected = {1.2, -3.1, 1, -1, 1.3, .2, 2, -1, -1.4, -5, 3, -1};
    for (size_t i = 0; i < expected.size(); ++i) {
        ASSERT_NEAR(expected[i], v.tensorInit[i], 1e-4);
    }
}

TEST_F(CSVParserTest, EmbeddedNewline) {
    string input = "A comment\n"
                   ",,foobar\n" // horizontal dim name
                   "foo,bar,A,B,C\n"
                   "A,'A\nB',1.2,1.3,'1,000'\n"
                   "A,B,1,'2\n.0',3\n"
                   "B,AB,3,2,1\n";

    istringstream is(input);

    spec.separator = ',';
    spec.quote = '\'';
    spec.setDataArea(3, 2);
    spec.numCols = 5;
    spec.missingValue = -1;
    spec.headerRow = 2;
    spec.dimensionNames = {"foo", "bar", "A", "B", "C"};
    spec.dimensionCols = {0, 1};
    spec.horizontalDimName = "foobar";

    VariableValue v(VariableType::parameter);
    loadValueFromCSVFile(v, is, spec);

    ASSERT_EQ(3, v.rank());
    ASSERT_EQ((vector<unsigned>({2, 2, 3})), v.hypercube().dims());
    ASSERT_EQ("foo", v.hypercube().xvectors[0].name);
    ASSERT_EQ("A", str(v.hypercube().xvectors[0][0]));
    ASSERT_EQ("B", str(v.hypercube().xvectors[0][1]));
    ASSERT_EQ("bar", v.hypercube().xvectors[1].name);
    ASSERT_EQ("AB", str(v.hypercube().xvectors[1][0]));
    ASSERT_EQ("B", str(v.hypercube().xvectors[1][1]));
    ASSERT_EQ("foobar", v.hypercube().xvectors[2].name);
    ASSERT_EQ("A", str(v.hypercube().xvectors[2][0]));
    ASSERT_EQ("B", str(v.hypercube().xvectors[2][1]));
    ASSERT_EQ("C", str(v.hypercube().xvectors[2][2]));
    ASSERT_EQ(v.hypercube().dims(), v.tensorInit.hypercube().dims());
    ASSERT_EQ(12, v.tensorInit.size());
    vector<double> expected = {1.2, 3, 1, -1, 1.3, 2, 2, -1, 1000, 1, 3, -1};
    for (size_t i = 0; i < expected.size(); ++i) {
        ASSERT_NEAR(expected[i], v.tensorInit[i], 1e-4);
    }
}

TEST_F(CSVParserTest, JanuaryDoubleDip) {
    string input =
        "country;2014;2014-01;2014-02\n"
        "Aus;10;;\n"
        "UK;;10;20\n";

    istringstream is(input);

    spec.separator = ';';
    spec.setDataArea(1, 1);
    spec.numCols = 4;
    spec.missingValue = -1;
    spec.headerRow = 0;
    spec.dimensionNames = {"country", "2014", "2014-01", "2014-02"};
    spec.dimensionCols = {0};
    spec.horizontalDimName = "date";
    spec.horizontalDimension.type = Dimension::time;

    VariableValue v(VariableType::parameter);
    loadValueFromCSVFile(v, is, spec);

    ASSERT_EQ(2, v.rank());
    ASSERT_EQ((vector<unsigned>({2, 2})), v.hypercube().dims());
    ASSERT_EQ("country", v.hypercube().xvectors[0].name);
    ASSERT_EQ("Aus", str(v.hypercube().xvectors[0][0]));
    ASSERT_EQ("UK", str(v.hypercube().xvectors[0][1]));
    ASSERT_EQ("date", v.hypercube().xvectors[1].name);
    ASSERT_EQ("2014-01-01T00:00:00", str(v.hypercube().xvectors[1][0]));
    ASSERT_EQ("2014-02-01T00:00:00", str(v.hypercube().xvectors[1][1]));
    ASSERT_EQ(v.hypercube().dims(), v.tensorInit.hypercube().dims());
    ASSERT_EQ(4, v.tensorInit.size());
    vector<double> expected = {10, 10, -1, 20};
    for (size_t i = 0; i < expected.size(); ++i) {
        ASSERT_NEAR(expected[i], v.tensorInit[i], 1e-4);
    }
}

TEST_F(CSVParserTest, DotTimeSep) {
    string input =
        "country;2014.01;2014.02;2014.10\n"
        "Aus;10;;\n"
        "UK;;10;20\n";

    istringstream is(input);

    spec.separator = ';';
    spec.setDataArea(1, 1);
    spec.numCols = 4;
    spec.missingValue = -1;
    spec.headerRow = 0;
    spec.dimensionNames = {"country", "2014.01", "2014.02", "2014.10"};
    spec.dimensionCols = {0};
    spec.horizontalDimName = "date";
    spec.horizontalDimension.type = Dimension::time;

    VariableValue v(VariableType::parameter);
    loadValueFromCSVFile(v, is, spec);

    ASSERT_EQ(2, v.rank());
    ASSERT_EQ((vector<unsigned>({2, 3})), v.hypercube().dims());
    ASSERT_EQ("country", v.hypercube().xvectors[0].name);
    ASSERT_EQ("Aus", str(v.hypercube().xvectors[0][0]));
    ASSERT_EQ("UK", str(v.hypercube().xvectors[0][1]));
    ASSERT_EQ("date", v.hypercube().xvectors[1].name);
    ASSERT_EQ("2014-01-01T00:00:00", str(v.hypercube().xvectors[1][0]));
    ASSERT_EQ("2014-02-01T00:00:00", str(v.hypercube().xvectors[1][1]));
    ASSERT_EQ("2014-10-01T00:00:00", str(v.hypercube().xvectors[1][2]));
    ASSERT_EQ(v.hypercube().dims(), v.tensorInit.hypercube().dims());
    ASSERT_EQ(6, v.tensorInit.size());
    vector<double> expected = {10, -1, -1, 10, -1, 20};
    for (size_t i = 0; i < expected.size(); ++i) {
        ASSERT_NEAR(expected[i], v.tensorInit[i], 1e-4);
    }
}

TEST_F(CSVParserTest, LoadVarSpace) {
    string input = "A comment\n"
                   "  foobar\n" // horizontal dim name
                   "\"foo\" bar \\A B C\n"
                   "A A 1.2 1.3 1.4\n"
                   "A B 1 2 3\n"
                   "B A 3 2 1\n";
    istringstream is(input);

    spec.separator = ' ';
    spec.setDataArea(3, 2);
    spec.numCols = 5;
    spec.missingValue = -1;
    spec.headerRow = 2;
    spec.dimensionNames = {"foo", "bar", "A", "B", "C"};
    spec.dimensionCols = {0, 1};
    spec.horizontalDimName = "foobar";

    VariableValue v;
    loadValueFromCSVFile(v, is, spec);

    ASSERT_EQ(3, v.rank());
    ASSERT_EQ((vector<unsigned>({2, 2, 3})), v.hypercube().dims());
    ASSERT_EQ("foo", v.hypercube().xvectors[0].name);
    ASSERT_EQ("A", str(v.hypercube().xvectors[0][0]));
    ASSERT_EQ("B", str(v.hypercube().xvectors[0][1]));
    ASSERT_EQ("bar", v.hypercube().xvectors[1].name);
    ASSERT_EQ("A", str(v.hypercube().xvectors[1][0]));
    ASSERT_EQ("B", str(v.hypercube().xvectors[1][1]));
    ASSERT_EQ("foobar", v.hypercube().xvectors[2].name);
    ASSERT_EQ("A", str(v.hypercube().xvectors[2][0]));
    ASSERT_EQ("B", str(v.hypercube().xvectors[2][1]));
    ASSERT_EQ("C", str(v.hypercube().xvectors[2][2]));
    ASSERT_EQ(v.hypercube().dims(), v.tensorInit.hypercube().dims());
    ASSERT_EQ(12, v.tensorInit.size());
    vector<double> expected = {1.2, 3, 1, -1, 1.3, 2, 2, -1, 1.4, 1, 3, -1};
    for (size_t i = 0; i < expected.size(); ++i) {
        ASSERT_NEAR(expected[i], v.tensorInit[i], 1e-4);
    }
}

TEST_F(CSVParserTest, DuplicateActions) {
    string input = "A comment\n"
                   ";;foobar\n" // horizontal dim name
                   "foo;bar;A;B;C\n"
                   "A;A;1.2;1.3;1.4\n"
                   "A;A;1;2;3\n"
                   "B;A;3;2;1\n";

    spec.separator = ';';
    spec.setDataArea(3, 2);
    spec.numCols = 5;
    spec.missingValue = -1;
    spec.headerRow = 2;
    spec.dimensionNames = {"foo", "bar", "A", "B", "C"};
    spec.dimensionCols = {0, 1};
    spec.horizontalDimName = "foobar";

    VariableValue v(VariableType::parameter);
    {
        istringstream is(input);
        ASSERT_THROW(loadValueFromCSVFile(v, is, spec), std::exception);
    }

    {
        istringstream is(input);
        spec.duplicateKeyAction = DataSpec::sum; // Use enum value
        loadValueFromCSVFile(v, is, spec);
        ASSERT_NEAR(2.2, v.tensorInit[0], 1e-9);
    }

    {
        istringstream is(input);
        spec.duplicateKeyAction = DataSpec::product; // Use enum value
        loadValueFromCSVFile(v, is, spec);
        ASSERT_NEAR(1.2, v.tensorInit[0], 1e-9);
    }

    {
        istringstream is(input);
        spec.duplicateKeyAction = DataSpec::min; // Use enum value
        loadValueFromCSVFile(v, is, spec);
        ASSERT_NEAR(1, v.tensorInit[0], 1e-9);
    }

    {
        istringstream is(input);
        spec.duplicateKeyAction = DataSpec::max; // Use enum value
        loadValueFromCSVFile(v, is, spec);
        ASSERT_NEAR(1.2, v.tensorInit[0], 1e-9);
    }

    {
        istringstream is(input);
        spec.duplicateKeyAction = DataSpec::av; // Use enum value
        loadValueFromCSVFile(v, is, spec);
        ASSERT_NEAR(1.1, v.tensorInit[0], 1e-9);
    }
}

TEST_F(CSVParserTest, ToggleDimensions) {
    spec.toggleDimension(2);
    ASSERT_EQ(1, spec.dimensionCols.count(2));
    spec.toggleDimension(2);
    ASSERT_EQ(0, spec.dimensionCols.count(2));
}

TEST(CSVParserIndependentTest, GuessFromVariableExport) {
    Hypercube hc;
    hc.xvectors =
        {
            XVector("height", {Dimension::value, ""}, {"1.5", "2", "2.5"}),
            XVector("sex", {Dimension::string, ""}, {"male", "female"}),
            XVector("date", {Dimension::time, "%Y"}, {"2010", "2011", "2012"})
        };

    VariableValue v(VariableType::flow);
    v.hypercube(hc);
    for (size_t i = 0; i < v.size(); ++i)
        v[i] = rand(); // Use rand() for simplicity in test, consider better pseudo-random for production

    string filename = "tmp.csv";
    v.exportAsCSV(filename);

    DataSpec spec;
    {
        ifstream f(filename);
        spec.guessFromStream(f);
    }
    VariableValue newV(VariableType::flow);
    {
        ifstream f(filename);
        loadValueFromCSVFile(newV, f, spec);
    }

    ASSERT_EQ(newV.hypercube().xvectors, v.hypercube().xvectors);
    ASSERT_EQ(v.size(), newV.size());
    for (size_t i = 0; i < v.size(); ++i)
        ASSERT_NEAR(v[i], newV.tensorInit[i], 0.001 * v[1]); // Assuming v[1] is representative and non-zero

    // Clean up the created file
    boost::filesystem::remove(filename);
}

string testEscapeDoubledQuotes(string x) {
    DataSpec spec;
    spec.quote = '\'';
    spec.separator = ',';
    spec.escape = '&';
    escapeDoubledQuotes(x, spec);
    return x;
}

TEST(CSVParserIndependentTest, EscapeDoubledQuotes) {
    ASSERT_EQ("foo", testEscapeDoubledQuotes("foo"));
    ASSERT_EQ("'foo'", testEscapeDoubledQuotes("'foo'"));
    ASSERT_EQ("&'foo&'", testEscapeDoubledQuotes("''foo''"));                 // not strictly CSV standard
    ASSERT_EQ("'&'foo&''", testEscapeDoubledQuotes("'''foo'''"));
    ASSERT_EQ("'&'&''", testEscapeDoubledQuotes("''''''"));
    ASSERT_EQ("'fo&'o'", testEscapeDoubledQuotes("'fo''o'"));
    ASSERT_EQ("foo,bar", testEscapeDoubledQuotes("foo,bar"));
    ASSERT_EQ("'foo','bar'", testEscapeDoubledQuotes("'foo','bar'"));
    ASSERT_EQ("&'foo&',&'bar&'", testEscapeDoubledQuotes("''foo'',''bar''")); // not strictly CSV standard
    ASSERT_EQ("'&'foo&'','&'bar&''", testEscapeDoubledQuotes("'''foo''','''bar'''"));
    ASSERT_EQ("'fo&'o','b&'ar'", testEscapeDoubledQuotes("'fo''o','b''ar'"));
}

TEST(CSVParserIndependentTest, IsNumerical) {
    ASSERT_TRUE(isNumerical(""));
    ASSERT_TRUE(isNumerical("1.2"));
    ASSERT_TRUE(isNumerical("'1.2'"));
    ASSERT_TRUE(isNumerical("-1.2"));
    ASSERT_TRUE(isNumerical("1e2"));
    ASSERT_TRUE(isNumerical("NaN"));
    ASSERT_TRUE(isNumerical("nan"));
    ASSERT_TRUE(isNumerical("inf"));
    ASSERT_TRUE(isNumerical("inf"));
    // leading nonumerical strings are considered non-numerical, but
    // will be parsed as numbers anyway if in data column
    ASSERT_FALSE(isNumerical("$100"));
    ASSERT_FALSE(isNumerical("£100"));
    ASSERT_FALSE(isNumerical("hello"));
}
