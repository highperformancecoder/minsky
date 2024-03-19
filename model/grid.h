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

#ifndef GRID_H
#define GRID_H
#include <vector>
#include "classdesc_access.h"

namespace minsky
{
  /// Cell template parameter duck types this interface
  class ICell
  {
  public:
    virtual ~ICell()=default;
    virtual void show()=0;           ///< render onto cairo surface
    virtual double width() const=0;  ///< width of cell on canvas
    virtual double height() const=0; ///< height of cell
  };

  struct JustificationStruct
  {
    enum Justification {left, right, centre};
  };
  
  template <class Cell>
  class Grid: public JustificationStruct
  {
    /// move current cursor to \a x,y
    virtual void moveCursorTo(double x, double y)=0;
    CLASSDESC_ACCESS(Grid);
  public:
    virtual ~Grid()=default;
    /// return reference to cell \a row, \a col. Reference is valid until next call to cell()
    virtual Cell& cell(unsigned row, unsigned col)=0;
    virtual unsigned numRows() const=0;
    virtual unsigned numCols() const=0;
   /// justification of \a col
    virtual JustificationStruct::Justification justification(unsigned col) const {return left;};
    /// whether cells all have the same height or not
    virtual bool evenHeight() const {return true;}
    /// coordinates of right and bottom margins of cells. Valid after draw()
    std::vector<double> rightColMargin, bottomRowMargin;
    /// draw the grid
    void draw();
    /// column at \a x in unzoomed coordinates
    int colX(double x) const;
    /// row at \a y in unzoomed coordinates
    int rowY(double y) const;

  };
  
}

#include "grid.cd"
#include "grid.xcd"
#include "grid.rcd"
#endif
