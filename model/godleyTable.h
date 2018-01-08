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
   Provide support for the Godley Table spreadsheet window
 */

#ifndef GODLEYTABLE_H
#define GODLEYTABLE_H
#include "godleyIcon.h"
#include <cairoSurfaceImage.h>
#include <memory>
#include <vector>

namespace minsky
{
  class GodleyTableWindow: public ecolab::CairoSurface
  {
  public:
    /// offset of the table within the window
    static constexpr double leftTableOffset=0, topTableOffset=20;
    std::shared_ptr<GodleyIcon> godleyIcon;
    /// starting row/col number of the scrolling region
    unsigned scrollRowStart=1, scrollColStart=1;
    /// which cell is active
    int selectedRow=-1, selectedCol=-1;
    /// computed positions of the table columns
    std::vector<double> colLeftMargin;
    /// computed height of each row
    double rowHeight=0;
    /// location of insertion pointer in selected cell, as well as
    /// other end of selection (if mouse-swiped)
    unsigned insertIdx=0, selectIdx=0; 

    void redraw(int, int, int width, int height) override;
    void requestRedraw() {if (surface.get()) surface->requestRedraw();}
    /// event handling 
    void mouseDown(double x, double y);
    void mouseUp(double x, double y);
    void mouseMove(double x, double y);
    void keyPress(int keySym);
    void keyRelease(int keySym);

    enum ClickType {background, row0, col0, internal};
    ClickType clickType(double x, double y) const;

    /// add/delete rows/columns at x,y
    void addStockVar(double x);
    void importStockVar(const string& name, double x);
    void deleteStockVar(double x);
    void addFlow(double y);
    void deleteFlow(double y);
    
    int colX(double x) const;
    int rowY(double y) const;

    void highlightColumn(cairo_t* cairo,unsigned col);
    void highlightRow(cairo_t* cairo,unsigned row);
    void highlightCell(cairo_t* cairo,unsigned row, unsigned col);

    // support cut/copy/paste operations
    void delSelection();
    void cut() {copy(); delSelection();requestRedraw();}
    void copy();
    void paste();

    
    int textIdx(double x) const;

  private:
    int motionRow=-1, motionCol=-1; ///< current cell under mouse motion
    bool controlChar=false; ///< control pressed
  };
}

#include "godleyTable.cd"
#endif
