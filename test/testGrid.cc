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
#include "minsky_epilogue.h"
#include <UnitTest++/UnitTest++.h>

using namespace minsky;
using namespace std;

namespace
{
  struct SizedCellMock: public ICell
  {
    void show() override {}
    SizedCellMock(double width=10, double height=5):
      m_width(width), m_height(height) {}
    double m_width, m_height;
    double width() const {return m_width;}
    double height() const {return m_height;}
  };

  struct GridMock: public Grid<ICell>
  {
    unsigned numRows() const override {return 10;}
    unsigned numCols() const override {return 10;}
    vector<double> xpos, ypos; // capture moveTos
    void moveCursorTo(double x, double y) {
      xpos.push_back(x);
      ypos.push_back(y);
    }
    Justification m_justification=left;
    Justification justification(unsigned col) const override {return m_justification;}
  };
  
  struct EvenHeightLeftGrid: public GridMock
  {
    SizedCellMock theCell;
    SizedCellMock& cell(unsigned row, unsigned col) override {return theCell;}
  };

  struct UnevenHeightGridMock: public GridMock
  {
    vector<vector<SizedCellMock>> cells;
    UnevenHeightGridMock() {
      for (unsigned i=0; i<numRows(); ++i)
        {
          cells.emplace_back();
          for (unsigned j=0; j<numCols(); ++j)
            cells.back().emplace_back(10*(i+j+1),10*(i+j+1));
        }
    }
    SizedCellMock& cell(unsigned row, unsigned col) override {return cells[row][col];}
    bool evenHeight() const override {return false;}
  };

}

const double padx=2;

SUITE(Grid)
{
  TEST_FIXTURE(EvenHeightLeftGrid, draw)
    {
      draw();
      CHECK(numCols()>1 && numRows()>1);
      CHECK_EQUAL(numCols(), rightColMargin.size());
      CHECK_EQUAL(numRows(), bottomRowMargin.size());
      for (size_t i=0; i<rightColMargin.size(); ++i)
        CHECK_EQUAL((i+1)*(cell(0,0).width()+padx), rightColMargin[i]);
      for (size_t i=0; i<bottomRowMargin.size(); ++i)
        CHECK_EQUAL((i+1)*cell(0,0).height(), bottomRowMargin[i]);

      size_t m=0;
      for (size_t col=0; col<numCols(); ++col) 
        for (size_t row=0; row<numRows(); ++row, ++m)
         {
           CHECK_EQUAL((col>0? rightColMargin[col-1]: 0)+0.5*padx, xpos[m]);
           CHECK_EQUAL(row>0? bottomRowMargin[row-1]: 0, ypos[m]);
          }
    }
  
  TEST_FIXTURE(UnevenHeightGridMock, left)
    {
      draw();
      CHECK(numCols()>1 && numRows()>1);
      CHECK_EQUAL(numCols(), rightColMargin.size());
      CHECK_EQUAL(numRows(), bottomRowMargin.size());

      // maximum cell width is final row
      double x=0;
      for (size_t i=0; i<rightColMargin.size(); ++i)
        {
          x+=cell(numRows()-1,i).width()+padx;
          CHECK_EQUAL(x, rightColMargin[i]);
        }
      // maximum cell height is final column
      double y=0;
      for (size_t i=0; i<bottomRowMargin.size(); ++i)
        {
          y+=cell(i,numCols()-1).height();
          CHECK_EQUAL(y, bottomRowMargin[i]);
        }

      size_t m=0;
      for (size_t col=0; col<numCols(); ++col) 
        for (size_t row=0; row<numRows(); ++row, ++m)
         {
           CHECK_EQUAL((col>0? rightColMargin[col-1]: 0)+0.5*padx, xpos[m]);
           CHECK_EQUAL(row>0? bottomRowMargin[row-1]: 0, ypos[m]);
          }
    }

  TEST_FIXTURE(UnevenHeightGridMock, right)
    {
      m_justification=right;
      draw();
      CHECK(numCols()>1 && numRows()>1);
      CHECK_EQUAL(numCols(), rightColMargin.size());
      CHECK_EQUAL(numRows(), bottomRowMargin.size());

      // maximum cell width is final row
      double x=0;
      for (size_t i=0; i<rightColMargin.size(); ++i)
        {
          x+=cell(numRows()-1,i).width()+padx;
          CHECK_EQUAL(x, rightColMargin[i]);
        }
      // maximum cell height is final column
      double y=0;
      for (size_t i=0; i<bottomRowMargin.size(); ++i)
        {
          y+=cell(i,numCols()-1).height();
          CHECK_EQUAL(y, bottomRowMargin[i]);
        }

      size_t m=0;
      for (size_t col=0; col<numCols(); ++col) 
        for (size_t row=0; row<numRows(); ++row, ++m)
         {
           CHECK_EQUAL((rightColMargin[col]-cell(row,col).width())+0.5*padx, xpos[m]);
           CHECK_EQUAL((row>0? bottomRowMargin[row-1]: 0), ypos[m]);
          }
    }

  TEST_FIXTURE(UnevenHeightGridMock, centre)
    {
      m_justification=centre;
      draw();
      CHECK(numCols()>1 && numRows()>1);
      CHECK_EQUAL(numCols(), rightColMargin.size());
      CHECK_EQUAL(numRows(), bottomRowMargin.size());

      // maximum cell width is final row
      double x=0;
      for (size_t i=0; i<rightColMargin.size(); ++i)
        {
          x+=cell(numRows()-1,i).width()+padx;
          CHECK_EQUAL(x, rightColMargin[i]);
        }
      // maximum cell height is final column
      double y=0;
      for (size_t i=0; i<bottomRowMargin.size(); ++i)
        {
          y+=cell(i,numCols()-1).height();
          CHECK_EQUAL(y, bottomRowMargin[i]);
        }

      size_t m=0;
      for (size_t col=0; col<numCols(); ++col) 
        for (size_t row=0; row<numRows(); ++row, ++m)
         {
           CHECK_EQUAL(0.5*((col>0? rightColMargin[col-1]: 0) + rightColMargin[col] - cell(row,col).width())+0.5*padx, xpos[m]);
           CHECK_EQUAL((row>0? bottomRowMargin[row-1]: 0), ypos[m]);
          }
    }

  TEST_FIXTURE(UnevenHeightGridMock, colX)
    {
      draw();
      CHECK_EQUAL(-1, colX(-1));
      CHECK_EQUAL(-1, colX(rightColMargin.back()));
      for (size_t i=0; i<rightColMargin.size(); ++i)
        CHECK_EQUAL(i, colX(rightColMargin[i]-0.5));
    }
  
  TEST_FIXTURE(UnevenHeightGridMock, rowY)
    {
      draw();
      CHECK_EQUAL(-1, rowY(-1));
      CHECK_EQUAL(-1, rowY(bottomRowMargin.back()));
      for (size_t i=0; i<bottomRowMargin.size(); ++i)
        CHECK_EQUAL(i, rowY(bottomRowMargin[i]-0.5));
    }
}
