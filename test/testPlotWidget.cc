/*
  @copyright Steve Keen 2023
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
#include "cairoItems.h"
#include "lasso.h"
#include "plotWidget.h"
#include "plot.xcd"
#include "CSVTools.xcd"
#include "tensorInterface.xcd"
#include "tensorVal.xcd"
#include "minsky_epilogue.h"
#undef True
#include <gtest/gtest.h>

namespace minsky
{
  namespace
  {
    class PlotWidgetTest : public PlotWidget, public ::testing::Test
    {
    };

    TEST_F(PlotWidgetTest, legendMove)
    {
      legend=true;
      addPt(0,0.0,1.0);
      addPt(0,1.0,0.0);
      addPt(1,1.0,0.0);
      addPt(1,0.0,1.0);
      labelPen(0,"hello");
      labelPen(1,"hello");
      double xoffs, yoffs, legendWidth, legendHeight;
      legendSize(xoffs, yoffs, legendWidth, legendHeight, iWidth(), iHeight()-10);
      double x=(legendLeft-0.5)*iWidth() +0.5*legendWidth;
      double y=(0.5-legendTop)*iHeight() + 0.5*legendHeight;
      EXPECT_EQ(ClickType::legendMove, clickType(x,y));
      mouseDown(x,y);
      double oldLegendLeft=legendLeft, oldLegendTop=legendTop;
      mouseUp(x+10,y+20);
      EXPECT_NEAR(oldLegendLeft+10/(iWidth()-20),legendLeft, 0.01);
      EXPECT_NEAR(oldLegendTop-20/(iHeight()-10),legendTop,0.01);
      // legend should not change size
      double newLegendWidth, newLegendHeight;
      legendSize(xoffs, yoffs, newLegendWidth, newLegendHeight, iWidth(), iHeight()-10);
      EXPECT_NEAR(legendWidth,newLegendWidth, 0.01);
      EXPECT_NEAR(legendHeight,newLegendHeight,0.01);
     }
  
    TEST_F(PlotWidgetTest, legendResize)
    {
      legend=true;
      addPt(0,0.0,1.0);
      addPt(0,1.0,0.0);
      addPt(1,1.0,0.0);
      addPt(1,0.0,1.0);
      labelPen(0,"hello");
      labelPen(1,"hello");
      double xoffs, yoffs, legendWidth, legendHeight;
      legendSize(xoffs, yoffs, legendWidth, legendHeight, iWidth(), iHeight()-10);
      double x=(legendLeft-0.5)*iWidth() +0.5*legendWidth;
      double y=(0.5-legendTop)*iHeight() + 0.95*legendHeight;
      EXPECT_EQ(ClickType::legendResize, clickType(x,y));
      mouseDown(x,y);
      double oldLegendLeft=legendLeft, oldLegendTop=legendTop;
      mouseUp(x+10,y+20);
      // corner should not move
      EXPECT_NEAR(oldLegendLeft,legendLeft, 0.01);
      EXPECT_NEAR(oldLegendTop,legendTop,0.01);
      double newLegendWidth, newLegendHeight;
      legendSize(xoffs, yoffs, newLegendWidth, newLegendHeight, iWidth(), iHeight()-10);
       // width change will be proportionate - just check the new height is about right
      EXPECT_NEAR(legendHeight+20,newLegendHeight,2);
    }
  }
}
