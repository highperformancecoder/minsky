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
#include <pango.h>
#include <ecolab_epilogue.h>
using namespace std;
using namespace minsky;
using ecolab::Pango;
using ecolab::cairo::CairoSave;

void CSVDialog::loadFile(const string& fname)
{
  spec.guessFromFile(fname);
  ifstream is(fname);
  initialLines.clear();
  for (size_t i=0; i<numInitialLines && is; ++i)
    {
      initialLines.emplace_back();
      getline(is, initialLines.back());
    }
}

template <class Parser>
vector<vector<string>> parseLines(const Parser& parser, const vector<string>& lines)
{
  vector<vector<string>> r;
  for (auto& line: lines)
    {
      r.emplace_back();
      boost::tokenizer<Parser> tok(line.begin(), line.end(), parser);
      for (auto& t: tok)
        r.back().push_back(t);
    }
  return r;
}

void CSVDialog::redraw(int, int, int width, int height)
{
  Pango pango(surface->cairo());
  for (auto& line: initialLines)
    {
      pango.setText(line);
      rowHeight=max(rowHeight, pango.height());
    }
  
  vector<vector<string>> parsedLines;
  if (spec.mergeDelimiters)
    if (spec.separator==' ')
      parsedLines=parseLines(boost::char_separator<char>(), initialLines);
    else
      {
        char separators[]={spec.separator,'\0'};
        parsedLines=parseLines
          (boost::char_separator<char>(separators,""),initialLines);
      }
  else
    parsedLines=parseLines
      (boost::escaped_list_separator<char>(spec.escape,spec.separator,spec.quote),
       initialLines);

  set<size_t> done;
  double x=xoffs, y=0;
  for (size_t col=0; done.size()<parsedLines.size(); ++col)
    {
      double colWidth=0;
      for (size_t row=0; row<parsedLines.size(); ++row)
        {
          auto& line=parsedLines[row];
          if (col<line.size())
            {
              CairoSave cs(surface->cairo());
              pango.setText(line[col]);
              if (line.size()>1 && !spec.commentRows.count(row)) // do not column adjust for header comment lines
                colWidth=max(colWidth, pango.width());
              cairo_move_to(surface->cairo(), x, y);
              if (spec.commentRows.count(row) || spec.commentCols.count(col))
                cairo_set_source_rgb(surface->cairo(),1,0,0);
              else if (col<spec.nColAxes)
                if (row<spec.nRowAxes)
                  cairo_set_source_rgb(surface->cairo(),0,0.7,0);
                else
                  cairo_set_source_rgb(surface->cairo(),0,0,1);
              else if (row<spec.nRowAxes)
                cairo_set_source_rgb(surface->cairo(),0,0,1);
              pango.show();
            }
          else
            done.insert(row);
          y+=rowHeight;
        }
      x+=colWidth+5;
      y=0;
      colOffsets.push_back(x);
    }
  
}

size_t CSVDialog::columnOver(double x)
{
  for (size_t i=0; i<colOffsets.size(); ++i)
    if (x-xoffs<colOffsets[i])
      return i;
  return colOffsets.size();
}

size_t CSVDialog::rowOver(double y)
{
  return size_t(y/rowHeight);
}
