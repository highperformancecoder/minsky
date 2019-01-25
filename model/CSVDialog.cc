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

namespace
{
  struct CroppedPango: public Pango
  {
    cairo_t* cairo;
    double w, x, y;
    CroppedPango(cairo_t* cairo, double width): Pango(cairo), cairo(cairo), w(width) {}
    void setxy(double xx, double yy) {x=xx; y=yy;}
    void show() {
      CairoSave cs(cairo);
      cairo_rectangle(cairo,x,y,w,height());
      cairo_clip(cairo);
      cairo_move_to(cairo,x,y);
      Pango::show();
    }
  };
}

void CSVDialog::redraw(int, int, int width, int height)
{
  cairo_t* cairo=surface->cairo();
  CroppedPango pango(cairo, colWidth);
  rowHeight=15;
  pango.setFontSize(0.8*rowHeight);
//  for (auto& line: initialLines)
//    {
//      pango.setText(line);
//      rowHeight=max(rowHeight, pango.height());
//    }
  
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
  size_t col=0;
  for (; done.size()<parsedLines.size(); ++col)
    {
      for (size_t row=0; row<parsedLines.size(); ++row)
        {
          auto& line=parsedLines[row];
          if (col<line.size())
            {
              CairoSave cs(cairo);
              pango.setText(line[col]);
              pango.setxy(x, y);
              if (row==spec.headerRow)
                if (col<spec.nColAxes)
                  cairo_set_source_rgb(surface->cairo(),0,0.7,0);
                else
                  cairo_set_source_rgb(surface->cairo(),0,0,1);
              else if (row<spec.nRowAxes || col<spec.nColAxes && !spec.dimensionCols.count(col))
                cairo_set_source_rgb(surface->cairo(),1,0,0);
              else if (col<spec.nColAxes)
                cairo_set_source_rgb(surface->cairo(),0,0,1);
              pango.show();
            }
          else
            done.insert(row);
          y+=rowHeight;
        }
      {
        CairoSave cs(cairo);
        cairo_set_source_rgb(cairo,.5,.5,.5);
        cairo_move_to(cairo,x-2.5,0);
        cairo_rel_line_to(cairo,0,parsedLines.size()*rowHeight);
        cairo_stroke(cairo);
      }
      x+=colWidth+5;
      y=0;
    }
  for (size_t row=0; row<parsedLines.size(); ++row)
    {
      CairoSave cs(cairo);
      cairo_set_source_rgb(cairo,.5,.5,.5);
      cairo_move_to(cairo,xoffs-2.5,row*rowHeight);
      cairo_rel_line_to(cairo,(col-1)*(colWidth+5),0);
      cairo_stroke(cairo);
    }
}

size_t CSVDialog::columnOver(double x)
{
  return size_t((x-xoffs)/(colWidth+5));
}

size_t CSVDialog::rowOver(double y)
{
  return size_t(y/rowHeight);
}
