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
#include <cairoSurfaceImage.h>

#include <vector>
#include <string>

namespace minsky
{
  class CSVDialog: public ecolab::CairoSurface
  {
    std::vector<std::string> initialLines; ///< initial lines of file
    double rowHeight=0;
    CLASSDESC_ACCESS(DataSpec);
  public:
    const unsigned numInitialLines=30;
    double xoffs=80;
    double colWidth=50;
    bool flashNameRow=false;
    DataSpec spec;
    void redraw(int, int, int width, int height) override;
    void loadFile(const std::string& fname);
    // Return file name after downloading a CSV file from the web. 
    std::string loadWebFile(const std::string& url); 
    void reportFromFile(const std::string& input, const std::string& output);
    void requestRedraw() {if (surface.get()) surface->requestRedraw();}
    /// return column mouse is over
    size_t columnOver(double x);
    /// return row mouse is over
    size_t rowOver(double x);
    void copyHeaderRowToDimNames(size_t row);
    std::string headerForCol(size_t col) const;
    std::vector<std::vector<std::string>> parseLines() const;
  };
}

#include "CSVDialog.cd"
#endif
