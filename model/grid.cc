/*
  @copyright Steve Keen 2021
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

#include "grid.h"
#include <pango.h>
#include "minsky_epilogue.h"
#include <algorithm>

namespace minsky
{
  template <class Cell>
  void Grid<Cell>::draw()
  {
    bottomRowMargin.clear();
    if (evenHeight())
      {
        double rowHeight=cell(0,0).height();
        for (unsigned i=0; i<numRows(); ++i)
          bottomRowMargin.push_back((i+1)*rowHeight);
      }
    else
      {
        double y=0;
        for (unsigned i=0; i<numRows(); ++i)
          {
            double rowHeight=0;
            for (unsigned j=0; j<numCols(); ++j)
              rowHeight=std::max(rowHeight, cell(i,j).height());
            y+=rowHeight;
            bottomRowMargin.push_back(y);
          }
      }
    assert(bottomRowMargin.size()==numRows());

    rightColMargin.clear();
    double x=0;
    for (unsigned col=0; col<numCols(); ++col)
      {
        double colWidth=0, y=0;
        switch (justification(col))
          {
          case right: case centre:
            // work out column width
            for (unsigned row=0; row<numRows(); ++row)
              colWidth=std::max(colWidth,cell(row,col).width());
            break;
          default:
            break;
          }

        
        for (unsigned row=0; row<numRows(); ++row)
          {
            auto& currentCell=cell(row,col);
            double offset=0;
            switch (justification(col))
              {
              case left: break;
              case right: offset+=colWidth-currentCell.width(); break;
              case centre: offset+=0.5*(colWidth-currentCell.width()); break;
              }
            moveTo(x+offset,y);
            currentCell.show();
            colWidth=std::max(colWidth, currentCell.width());
            y=bottomRowMargin[row];
          }
        x+=colWidth;
        rightColMargin.push_back(x);
      }
  }
  
  template <class Cell>
  int Grid<Cell>::colX(double x) const
  {
    if (rightColMargin.empty() || x<0 || x>=rightColMargin.back())
      return -1;
    auto p=std::upper_bound(rightColMargin.begin(), rightColMargin.end(), x);
    return p-rightColMargin.begin();
  }
    
  template <class Cell>
  int Grid<Cell>::rowY(double y) const
  {
    if (bottomRowMargin.empty() || y<0 || y>=bottomRowMargin.back())
      return -1;
    auto p=std::upper_bound(bottomRowMargin.begin(), bottomRowMargin.end(), y);
    return p-bottomRowMargin.begin();
  }

  template class Grid<ICell>;
  template class Grid<ecolab::Pango>;
  
}
