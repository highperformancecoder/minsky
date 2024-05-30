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
   A simple spreadsheet view
 */

#ifndef SHEET_H
#define SHEET_H
#include <itemT.h>
#include "dynamicRavelCAPI.h"
#include "showSlice.h"

namespace minsky
{
  class Sheet: public ItemT<Sheet>
  {
    
    CLASSDESC_ACCESS(Sheet);
    ravel::Ravel inputRavel; ///< ravel for controlling input
    /// value used for populating sheet. Potentially a tensor expression representing ravel manipulation
    civita::TensorPtr value;
    /// size of ravel in screen coordinates
    double ravelSize() const;
    /// @{ ravel coordinate from screen coordinate
    double ravelX(double xx) const;
    double ravelY(double yy) const;

    /// offset for scrolling through higher ranked inputs
    size_t scrollOffset=0, scrollMax=1;
    size_t scrollDelta=0;
    std::string sliceIndicator;
  public:
    Sheet();

    // copy operations needed for clone, but not really used for now
    // define them as empty operations to prevent double frees if accidentally used
    void operator=(const Sheet&){}
    Sheet(const Sheet&) {}
    
    bool onResizeHandle(float x, float y) const override;
    void drawResizeHandles(cairo_t* cairo) const override;

    bool onRavelButton(float, float) const;
    bool inRavel(float, float) const;
    bool inItem(float, float) const override;
//    void onMouseDown(float x, float y) override;
//    void onMouseUp(float x, float y) override;
//    bool onMouseMotion(float x, float y) override;
//    bool onMouseOver(float x, float y) override;
//    void onMouseLeave() override;
    ClickType::Type clickType(float x, float y) const override;
    std::vector<Point> corners() const override;
    bool contains(float x, float y) const override;

    bool scrollUp();
    bool scrollDown();
    bool onKeyPress(int keySym, const std::string& utf8, int state) override;
    void setSliceIndicator();
    
    void draw(cairo_t* cairo) const override;
    
    /// calculates the input value
    void computeValue();
    
    /// export the plotted data as a CSV file
    /// @param tabular - if true, the longest dimension is split across columns as a horizontal dimension
    void exportAsCSV(const std::string& filename, bool tabular) const;

    bool showRavel=false;
    ShowSlice showRowSlice=ShowSlice::head; ///< whether to elide rows from beginning, end or middle
    ShowSlice showColSlice=ShowSlice::head; ///< whether to elide columnss from beginning, end or middle
  };
}

#include "sheet.cd"
#include "sheet.xcd"
#endif
