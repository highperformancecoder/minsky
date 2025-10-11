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
#include "lasso.h"
#include <pango.h>

#include "CSVDialog.rcd"
#include "CSVTools.xcd"
#include "dynamicRavelCAPI.h"
#include "dynamicRavelCAPI.rcd"
#include "dynamicRavelCAPI.xcd"
#include "minsky_epilogue.h"

#include "zStream.h"
#include "dimension.h"
#include <cstdlib>
#include <chrono>
#include <iostream>
#include <string>
#include <stdexcept>
#include <sstream>      
#include <regex>    

using namespace std;
using namespace minsky;
using namespace civita;
using ecolab::Pango;
using ecolab::cairo::CairoSave;

#include <filesystem>
using std::filesystem::file_size;

const unsigned CSVDialog::numInitialLines;

void CSVDialog::reportFromFile(const std::string& input, const std::string& output) const
{
  ifstream is(input);
  stripByteOrderingMarker(is);
  ofstream of(output);
  reportFromCSVFile(is,of,spec,file_size(input));
}

void CSVDialog::loadFile()
{
  loadFileFromName(url);
}

void CSVDialog::guessSpecAndLoadFile()
{
  spec=DataSpec();
  spec.guessFromFile(url);
  loadFileFromName(url);
  populateHeaders();
  classifyColumns();
}

void CSVDialog::loadFileFromName(const std::string& fname)
{  
  ifstream is(fname);
  stripByteOrderingMarker(is);
  initialLines.clear();
  for (size_t i=0; i<numInitialLines && is; ++i)
    {
      initialLines.emplace_back();
      getline(is, initialLines.back());
      // chomp any final '\r' character (DOS files)
      if (!initialLines.back().empty() && initialLines.back().back()=='\r')
        initialLines.back().erase(initialLines.back().end()-1);
    }
  // Ensure dimensions.size() is the same as nColAxes() upon first load of a CSV file. For ticket 974.
  if (spec.dimensions.size()<spec.nColAxes()) spec.setDataArea(spec.nRowAxes(),spec.nColAxes());    
}

template <class Parser>
vector<vector<string>> parseLines(const Parser& parser, const vector<string>& lines, size_t maxColumn)
{
  vector<vector<string>> r;
  for (const auto& line: lines)
    {
      r.emplace_back();
      try
        {
          const boost::tokenizer<Parser> tok(line.begin(), line.end(), parser);
          auto t=tok.begin();
          for (size_t i=0; i<maxColumn && t!=tok.end(); ++i, ++t)
            r.back().push_back(*t);
        }
      catch (...) // if not parseable, place entire line in first cell
        {
          r.back().push_back(line);
        }
    }
  return r;
}

std::vector<std::vector<std::string>> CSVDialog::parseLines(size_t maxColumn)
{
  vector<vector<string>> parsedLines;
  if (spec.mergeDelimiters)
    if (spec.separator==' ')
      parsedLines=::parseLines(boost::char_separator<char>(), initialLines, maxColumn);
    else
      {
        char separators[]={spec.separator,'\0'};
        parsedLines=::parseLines
          (boost::char_separator<char>(separators,""),initialLines, maxColumn);
      }
  else
    parsedLines=::parseLines
      (boost::escaped_list_separator<char>(spec.escape,spec.separator,spec.quote),
       initialLines, maxColumn);

  // update numCols iff maxColumn unrestricted
  if (maxColumn==numeric_limits<size_t>::max())
    {
      spec.numCols=0;
      for (auto& i: parsedLines)
        spec.numCols=std::max(spec.numCols, i.size());
    }
  return parsedLines;
}

void CSVDialog::populateHeaders()
{
  auto parsedLines=parseLines();
  if (spec.headerRow>=parsedLines.size()) return;
  auto& hr=parsedLines[spec.headerRow];
  spec.dimensionNames={hr.begin(), min(hr.end(), hr.begin()+spec.maxColumn)};
}

void CSVDialog::populateHeader(size_t col)
{
  auto parsedLines=parseLines();
  if (spec.headerRow>=parsedLines.size()) return;
  auto& headers=parsedLines[spec.headerRow];
  if (col<headers.size() && col<spec.maxColumn)
    spec.dimensionNames[col]=headers[col];
}

void CSVDialog::classifyColumns()
{
  auto parsedLines=parseLines();
  spec.dimensionCols.clear();
  spec.dataCols.clear();
  spec.dimensions.resize(min(spec.numCols,spec.maxColumn));
  for (size_t col=0; col<spec.numCols; ++col)
    {
      bool entryFound=false, timeFound=true, numberFound=true;
      for (size_t row=spec.nRowAxes(); row<parsedLines.size(); ++row)
        if (col<parsedLines[row].size() && !parsedLines[row][col].empty())
          {
            entryFound=true;
            if (numberFound && !isNumerical(parsedLines[row][col]))
              numberFound=false;
            static const AnyVal any(Dimension(Dimension::time,""));
            if (timeFound)
              try
                {any(parsedLines[row][col]);}
              catch (...)
                {timeFound=false;}
          }
      if (entryFound && col<spec.maxColumn)
        {
          if (numberFound)
            spec.dataCols.insert(col);
          else
            {
              spec.dimensionCols.insert(col);
              if (timeFound)
                spec.dimensions[col].type=Dimension::time;
              else
                spec.dimensions[col].type=Dimension::string;
              spec.dimensions[col].units.clear();
            }
        }
      else if (col>=spec.nColAxes() && col<spec.maxColumn)
        spec.dataCols.insert(col);
    }
}

std::vector<size_t> CSVDialog::correctedUniqueValues()
{
  auto r=spec.uniqueValues();
  // apply a correction by removing the values in the header rows
  vector<set<size_t>> correction(r.size());
  auto parsedLines=parseLines();
  const hash<string> h;
  for (size_t row=0; row<parsedLines.size() && row<spec.nRowAxes(); ++row)
    for (size_t col=0; col<correction.size() && col<parsedLines[row].size(); ++col)
      correction[col].insert(h(parsedLines[row][col]));
  for (size_t i=0; i<r.size(); ++i)
    if (r[i]>correction[i].size())
      r[i]-=correction[i].size();
    else
      r[i]=1;
  return r;
}

CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::CSVDialog);
