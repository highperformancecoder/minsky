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
#undef True
#include <gtest/gtest.h>

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
    double width() const override {return m_width;}
    double height() const override {return m_height;}
  };

  struct GridMock: public Grid<ICell>
  {
    unsigned numRows() const override {return 10;}
    unsigned numCols() const override {return 10;}
    vector<double> xpos, ypos; // capture moveTos
    void moveCursorTo(double x, double y) override {
      xpos.push_back(x);
      ypos.push_back(y);
    }
    Justification m_justification=left;
    Justification justification(unsigned col) const override {return m_justification;}
  };
  
  struct EvenHeightLeftGrid: public GridMock, public ::testing::Test
  {
    SizedCellMock theCell;
    SizedCellMock& cell(unsigned row, unsigned col) override {return theCell;}
  };

  struct UnevenHeightGridMock: public GridMock, public ::testing::Test
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

TEST_F(EvenHeightLeftGrid, draw)
  {
    draw();
    EXPECT_TRUE(numCols()>1 && numRows()>1);
    EXPECT_EQ(numCols(), rightColMargin.size());
    EXPECT_EQ(numRows(), bottomRowMargin.size());
    for (size_t i=0; i<rightColMargin.size(); ++i)
      EXPECT_EQ((i+1)*(cell(0,0).width()+padx), rightColMargin[i]);
    for (size_t i=0; i<bottomRowMargin.size(); ++i)
      EXPECT_EQ((i+1)*cell(0,0).height(), bottomRowMargin[i]);

    size_t m=0;
    for (size_t col=0; col<numCols(); ++col) 
      for (size_t row=0; row<numRows(); ++row, ++m)
       {
         EXPECT_EQ((col>0? rightColMargin[col-1]: 0)+0.5*padx, xpos[m]);
         EXPECT_EQ(row>0? bottomRowMargin[row-1]: 0, ypos[m]);
        }
  }

TEST_F(UnevenHeightGridMock, left)
  {
    draw();
    EXPECT_TRUE(numCols()>1 && numRows()>1);
    EXPECT_EQ(numCols(), rightColMargin.size());
    EXPECT_EQ(numRows(), bottomRowMargin.size());

    // maximum cell width is final row
    double x=0;
    for (size_t i=0; i<rightColMargin.size(); ++i)
      {
        x+=cell(numRows()-1,i).width()+padx;
        EXPECT_EQ(x, rightColMargin[i]);
      }
    // maximum cell height is final column
    double y=0;
    for (size_t i=0; i<bottomRowMargin.size(); ++i)
      {
        y+=cell(i,numCols()-1).height();
        EXPECT_EQ(y, bottomRowMargin[i]);
      }

    size_t m=0;
    for (size_t col=0; col<numCols(); ++col) 
      for (size_t row=0; row<numRows(); ++row, ++m)
       {
         EXPECT_EQ((col>0? rightColMargin[col-1]: 0)+0.5*padx, xpos[m]);
         EXPECT_EQ(row>0? bottomRowMargin[row-1]: 0, ypos[m]);
        }
  }

TEST_F(UnevenHeightGridMock, right)
  {
    m_justification=right;
    draw();
    EXPECT_TRUE(numCols()>1 && numRows()>1);
    EXPECT_EQ(numCols(), rightColMargin.size());
    EXPECT_EQ(numRows(), bottomRowMargin.size());

    // maximum cell width is final row
    double x=0;
    for (size_t i=0; i<rightColMargin.size(); ++i)
      {
        x+=cell(numRows()-1,i).width()+padx;
        EXPECT_EQ(x, rightColMargin[i]);
      }
    // maximum cell height is final column
    double y=0;
    for (size_t i=0; i<bottomRowMargin.size(); ++i)
      {
        y+=cell(i,numCols()-1).height();
        EXPECT_EQ(y, bottomRowMargin[i]);
      }

    size_t m=0;
    for (size_t col=0; col<numCols(); ++col) 
      for (size_t row=0; row<numRows(); ++row, ++m)
       {
         EXPECT_EQ((rightColMargin[col]-cell(row,col).width())+0.5*padx, xpos[m]);
         EXPECT_EQ((row>0? bottomRowMargin[row-1]: 0), ypos[m]);
        }
  }

TEST_F(UnevenHeightGridMock, centre)
  {
    m_justification=centre;
    draw();
    EXPECT_TRUE(numCols()>1 && numRows()>1);
    EXPECT_EQ(numCols(), rightColMargin.size());
    EXPECT_EQ(numRows(), bottomRowMargin.size());

    // maximum cell width is final row
    double x=0;
    for (size_t i=0; i<rightColMargin.size(); ++i)
      {
        x+=cell(numRows()-1,i).width()+padx;
        EXPECT_EQ(x, rightColMargin[i]);
      }
    // maximum cell height is final column
    double y=0;
    for (size_t i=0; i<bottomRowMargin.size(); ++i)
      {
        y+=cell(i,numCols()-1).height();
        EXPECT_EQ(y, bottomRowMargin[i]);
      }

    size_t m=0;
    for (size_t col=0; col<numCols(); ++col) 
      for (size_t row=0; row<numRows(); ++row, ++m)
       {
         EXPECT_EQ(0.5*((col>0? rightColMargin[col-1]: 0) + rightColMargin[col] - cell(row,col).width())+0.5*padx, xpos[m]);
         EXPECT_EQ((row>0? bottomRowMargin[row-1]: 0), ypos[m]);
        }
  }

TEST_F(UnevenHeightGridMock, colX)
  {
    draw();
    EXPECT_EQ(-1, colX(-1));
    EXPECT_EQ(-1, colX(rightColMargin.back()));
    for (size_t i=0; i<rightColMargin.size(); ++i)
      EXPECT_EQ(i, colX(rightColMargin[i]-0.5));
  }

TEST_F(UnevenHeightGridMock, rowY)
  {
    draw();
    EXPECT_EQ(-1, rowY(-1));
    EXPECT_EQ(-1, rowY(bottomRowMargin.back()));
    for (size_t i=0; i<bottomRowMargin.size(); ++i)
      EXPECT_EQ(i, rowY(bottomRowMargin[i]-0.5));
  }
