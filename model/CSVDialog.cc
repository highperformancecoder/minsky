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

#include "CSVDialog.h"
#include "group.h"
#include "selection.h"
#include "lasso.h"
#include <pango.h>

#include "CSVDialog.rcd"
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

#include <boost/filesystem.hpp>
using boost::filesystem::file_size;

const unsigned CSVDialog::numInitialLines;

void CSVDialog::reportFromFile(const std::string& input, const std::string& output) const
{
  ifstream is(input);
  stripByteOrderingMarker(is);
  ofstream of(output);
  reportFromCSVFile(is,of,spec,file_size(input));
}

namespace
{
  // manage temporary files
  struct CacheEntry
  {
    chrono::time_point<chrono::system_clock> timestamp;
    string url, filename;
    CacheEntry(const string& url): timestamp(chrono::system_clock::now()), url(url),
                            filename(boost::filesystem::unique_path().string()) {}
    ~CacheEntry() {boost::filesystem::remove(filename);}
    bool operator<(const CacheEntry& x) const {return url<x.url;}
  };

  // note: this cache will leak disk storage if Minsky is killed, not shut down cleanly
  struct Cache: private set<CacheEntry> 
  {
    using set<CacheEntry>::find;
    using set<CacheEntry>::end;
    using set<CacheEntry>::erase;
    iterator emplace(const string& url)
    {
      if (size()>=10)
        {
          // find oldest element and erase
          auto entryToErase=begin();
          auto ts=entryToErase->timestamp;
          for (auto i=begin(); i!=end(); ++i)
            if (i->timestamp<ts)
              {
                ts=i->timestamp;
                entryToErase=i;
              }
          erase(entryToErase);
        }
      return set<CacheEntry>::emplace(url).first;
    }
  };
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

namespace
{
  struct CroppedPango: public Pango
  {
    cairo_t* cairo;
    double w, x=0, y=0;
    CroppedPango(cairo_t* cairo, double width): Pango(cairo), cairo(cairo), w(width) {}
    void setxy(double xx, double yy) {x=xx; y=yy;}
    void show() {
      const CairoSave cs(cairo);
      cairo_rectangle(cairo,x,y,w,height());
      cairo_clip(cairo);
      cairo_move_to(cairo,x,y);
      Pango::show();
    }
  };
}

bool CSVDialog::redraw(int, int, int, int)
{
  cairo_t* cairo=surface->cairo();
  rowHeight=15;
  vector<vector<string>> parsedLines=parseLines();

  // LHS row labels
  {
    Pango pango(cairo);
    pango.setText("Dimension");
    cairo_move_to(cairo,xoffs-pango.width()-5,0);
    pango.show();
    pango.setText("Type");
    cairo_move_to(cairo,xoffs-pango.width()-5,rowHeight);
    pango.show();
    pango.setText("Format");
    cairo_move_to(cairo,xoffs-pango.width()-5,2*rowHeight);
    pango.show();
    if (flashNameRow)
      pango.setMarkup("<b>Name</b>");
    else
      pango.setText("Name");
    cairo_move_to(cairo,xoffs-pango.width()-5,3*rowHeight);
    pango.show();
    pango.setText("Header");
    cairo_move_to(cairo,xoffs-pango.width()-5,(4+spec.headerRow)*rowHeight);
    pango.show();
    
  }	
  
  CroppedPango pango(cairo, colWidth);
  pango.setFontSize(0.8*rowHeight);
  
  set<size_t> done;
  double x=xoffs, y=0;
  size_t col=0;
  for (; done.size()<parsedLines.size(); ++col)
    {
      if (col<spec.nColAxes())
        {// dimension check boxes
          const CairoSave cs(cairo);
          const double cbsz=5;
          cairo_set_line_width(cairo,1);
          cairo_translate(cairo,x+0.5*colWidth,y+0.5*rowHeight);
          cairo_rectangle(cairo,-cbsz,-cbsz,2*cbsz,2*cbsz);
          if (spec.dimensionCols.contains(col))
            {
              cairo_move_to(cairo,-cbsz,-cbsz);
              cairo_line_to(cairo,cbsz,cbsz);
              cairo_move_to(cairo,cbsz,-cbsz);
              cairo_line_to(cairo,-cbsz,cbsz);
            }
          cairo_stroke(cairo);
        }
      y+=rowHeight;
      // type
      if (spec.dimensionCols.contains(col) && col<spec.dimensions.size() && col<spec.nColAxes())
        {
          pango.setText(classdesc::enumKey<Dimension::Type>(spec.dimensions[col].type));
          pango.setxy(x,y);
          pango.show();
        }
      y+=rowHeight;
      if (spec.dimensionCols.contains(col) && col<spec.dimensions.size() && col<spec.nColAxes())
        {
          pango.setText(spec.dimensions[col].units);
          pango.setxy(x,y);
          pango.show();
        }
      y+=rowHeight;
      if (spec.dimensionCols.contains(col) && col<spec.dimensionNames.size() && col<spec.nColAxes())
        {
          pango.setText(spec.dimensionNames[col]);
          pango.setxy(x,y);
          pango.show();
        }
      y+=rowHeight;
      for (size_t row=0; row<parsedLines.size(); ++row)
        {
          auto& line=parsedLines[row];
          if (col<line.size())
            {
              const CairoSave cs(cairo);
              pango.setText(line[col]);
              pango.setxy(x, y);
              if (row==spec.headerRow)
                if (col<spec.nColAxes())
                  cairo_set_source_rgb(surface->cairo(),0,0.7,0);
                else
                  cairo_set_source_rgb(surface->cairo(),0,0,1);
              else if (row<spec.nRowAxes() || (col<spec.nColAxes() && !spec.dimensionCols.contains(col)))
                cairo_set_source_rgb(surface->cairo(),1,0,0);
              else if (col<spec.nColAxes())
                cairo_set_source_rgb(surface->cairo(),0,0,1);
              pango.show();
            }
          else
            done.insert(row);
          y+=rowHeight;
        }
      {
        const CairoSave cs(cairo);
        cairo_set_source_rgb(cairo,.5,.5,.5);
        cairo_move_to(cairo,x-2.5,0);
        cairo_rel_line_to(cairo,0,(parsedLines.size()+4)*rowHeight);
        cairo_stroke(cairo);
      }
      x+=colWidth+5;
      y=0;
    }
  m_tableWidth=(col-1)*(colWidth+5);
  for (size_t row=0; row<parsedLines.size()+5; ++row)
    {
      const CairoSave cs(cairo);
      cairo_set_source_rgb(cairo,.5,.5,.5);
      cairo_move_to(cairo,xoffs-2.5,row*rowHeight);
      cairo_rel_line_to(cairo,m_tableWidth,0);
      cairo_stroke(cairo);
    }
  return true;
}

size_t CSVDialog::columnOver(double x) const
{
  return size_t((x-xoffs)/(colWidth+5));
}

size_t CSVDialog::rowOver(double y) const
{
  return size_t(y/rowHeight);
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
  std::vector<std::set<std::string>> correction(r.size());
  auto parsedLines=parseLines();
  for (size_t row=0; row<spec.nRowAxes(); ++row)
    for (size_t col=0; col<correction.size() && col<parsedLines.size(); ++col)
      correction[col].insert(parsedLines[row][col]);
  for (size_t i=0; i<r.size(); ++i)
    if (r[i]>correction[i].size())
      r[i]-=correction[i].size();
    else
      r[i]=1;
  return r;
}

CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::CSVDialog);
