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

/**
   Support the CSV import dialog
*/

#ifndef CSVDIALOG_H
#define CSVDIALOG_H
#include "CSVParser.h"
#include "renderNativeWindow.h"
#include <cairoSurfaceImage.h>

#include <vector>
#include <string>

namespace minsky
{
  class CSVDialog: public RenderNativeWindow
  {
    std::vector<std::string> initialLines; ///< initial lines of file
    double rowHeight=0;
    double m_tableWidth;
    CLASSDESC_ACCESS(DataSpec);
    bool redraw(int, int, int width, int height) override;
    
  public:
    static const unsigned numInitialLines=100;
    double xoffs=80;
    double colWidth=50;
    bool flashNameRow=false;
    DataSpec spec;
    /// filename, or web url
    std::string url;
    /// width of table (in pixels)
    double tableWidth() const {return m_tableWidth;}

    /// loads an initial sequence of lines from \a url. If fname
    /// contains "://", is is treated as a URL, and downloaded from
    /// the web.
    void loadFile(); 
    /// guess the spec, then load an initial sequence of like loadFile()
    void guessSpecAndLoadFile();
    /// common implementation of loading the initial sequence of lines
    void loadFileFromName(const std::string& fname);
    void reportFromFile(const std::string& input, const std::string& output) const;
    void requestRedraw() {if (surface.get()) surface->requestRedraw();}
    /// return column mouse is over
    std::size_t columnOver(double x) const;
    /// return row mouse is over
    std::size_t rowOver(double y) const;
    std::vector<std::vector<std::string> > parseLines(size_t maxColumn=std::numeric_limits<size_t>::max());
    /// populate all column names from the headers row
    void populateHeaders();
    /// populate the name of column \a col with the data from the header row
    void populateHeader(size_t col);
    /// try to classify axis,data,ignore columns based on read in data
    void classifyColumns();
    /// unique values in each column, ignoring initial header lines
    /// could slightly underestimate the value, and is never less than
    /// 1, even for empty columns
    std::vector<size_t> correctedUniqueValues();
  };

  bool isNumerical(const std::string& s);
}

#include "CSVDialog.cd"
#include "CSVDialog.xcd"
#endif
