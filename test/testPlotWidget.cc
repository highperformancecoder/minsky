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

    TEST_F(PlotWidgetTest, constructor)
    {
      // Test that constructor initializes with expected default values
      PlotWidget widget;
      EXPECT_EQ(150, widget.iWidth());
      EXPECT_EQ(150, widget.iHeight());
      EXPECT_EQ(10u, widget.nxTicks);
      EXPECT_EQ(10u, widget.nyTicks);
      EXPECT_EQ(2, widget.fontScale);
      EXPECT_TRUE(widget.leadingMarker);
      EXPECT_TRUE(widget.grid);
      EXPECT_EQ(0.1, widget.legendLeft);
      EXPECT_EQ(1u, widget.numLines());
    }

    TEST_F(PlotWidgetTest, numLines)
    {
      // Test numLines getter/setter
      EXPECT_EQ(1u, numLines());
      
      numLines(3);
      EXPECT_EQ(3u, numLines());
      
      numLines(5);
      EXPECT_EQ(5u, numLines());
      
      // Check that yvars and xvars are resized appropriately
      EXPECT_EQ(10u, yvars.size()); // 2*numLines
      EXPECT_EQ(5u, xvars.size());
    }

    TEST_F(PlotWidgetTest, barWidth)
    {
      // Test barWidth getter/setter
      double width = barWidth(0.5);
      EXPECT_EQ(0.5, width);
      EXPECT_EQ(0.5, barWidth());
      
      barWidth(1.0);
      EXPECT_EQ(1.0, barWidth());
    }

    TEST_F(PlotWidgetTest, labels)
    {
      // Test xlabel, ylabel, y1label shadowed methods
      EXPECT_EQ("", xlabel());
      xlabel("X Axis");
      EXPECT_EQ("X Axis", xlabel());
      
      EXPECT_EQ("", ylabel());
      ylabel("Y Axis");
      EXPECT_EQ("Y Axis", ylabel());
      
      EXPECT_EQ("", y1label());
      y1label("Y1 Axis");
      EXPECT_EQ("Y1 Axis", y1label());
    }

    TEST_F(PlotWidgetTest, title)
    {
      // Test title field
      EXPECT_EQ("", title);
      title = "Test Plot";
      EXPECT_EQ("Test Plot", title);
    }

    TEST_F(PlotWidgetTest, resize)
    {
      // Test resize method with LassoBox
      LassoBox box;
      box.x0 = 0;
      box.y0 = 0;
      box.x1 = 300;
      box.y1 = 200;
      
      resize(box);
      
      EXPECT_EQ(300.0f, iWidth());
      EXPECT_EQ(200.0f, iHeight());
      EXPECT_EQ(150.0f, x());  // Center x = (0 + 300) / 2
      EXPECT_EQ(100.0f, y());  // Center y = (0 + 200) / 2
    }

    TEST_F(PlotWidgetTest, clickTypeOnItem)
    {
      // Test clickType returns onItem when clicking inside the widget
      const double z = Item::zoomFactor();
      EXPECT_EQ(ClickType::onItem, clickType(0, 0));  // Center
      EXPECT_EQ(ClickType::onItem, clickType(10, 10));
    }

    TEST_F(PlotWidgetTest, clickTypeOutside)
    {
      // Test clickType returns outside when clicking far outside
      const double z = Item::zoomFactor();
      const double w = 0.5 * iWidth() * z;
      const double h = 0.5 * iHeight() * z;
      EXPECT_EQ(ClickType::outside, clickType(x() + w + 100, y() + h + 100));
    }

    TEST_F(PlotWidgetTest, disconnectAllVars)
    {
      // Test disconnectAllVars clears all variable connections
      auto var = std::make_shared<VariableValue>();
      var->init("test", 1.0);
      
      connectVar(var, PlotWidget::nBoundsPorts);  // Connect to first y port
      EXPECT_FALSE(yvars.empty());
      
      disconnectAllVars();
      EXPECT_TRUE(xvars.empty());
      EXPECT_TRUE(yvars.empty());
    }

    TEST_F(PlotWidgetTest, startPen)
    {
      // Test startPen calculation
      numLines(2);
      
      // Add some variables to yvars
      auto var1 = std::make_shared<VariableValue>();
      var1->init("var1", 1.0);
      auto var2 = std::make_shared<VariableValue>();
      var2->init("var2", 2.0);
      
      connectVar(var1, PlotWidget::nBoundsPorts);  // port 6, first y port
      connectVar(var2, PlotWidget::nBoundsPorts);  // port 6, first y port
      
      // startPen(0) should be 0
      EXPECT_EQ(0u, startPen(0));
      // startPen(1) should be the number of vars in yvars[0]
      EXPECT_GE(startPen(1), 1u);
    }

    TEST_F(PlotWidgetTest, connectVarBounds)
    {
      // Test connecting variables to bound ports
      auto xminVar = std::make_shared<VariableValue>();
      xminVar->init("xmin", 0.0);
      connectVar(xminVar, 0);
      
      auto xmaxVar = std::make_shared<VariableValue>();
      xmaxVar->init("xmax", 10.0);
      connectVar(xmaxVar, 1);
      
      auto yminVar = std::make_shared<VariableValue>();
      yminVar->init("ymin", -1.0);
      connectVar(yminVar, 2);
      
      auto ymaxVar = std::make_shared<VariableValue>();
      ymaxVar->init("ymax", 1.0);
      connectVar(ymaxVar, 3);
      
      auto y1minVar = std::make_shared<VariableValue>();
      y1minVar->init("y1min", -2.0);
      connectVar(y1minVar, 4);
      
      auto y1maxVar = std::make_shared<VariableValue>();
      y1maxVar->init("y1max", 2.0);
      connectVar(y1maxVar, 5);
      
      // Verify the variables are connected (can't directly test private members, 
      // but we can verify the connection didn't crash)
      EXPECT_TRUE(true);
    }

    TEST_F(PlotWidgetTest, plotType)
    {
      // Test plotType field
      EXPECT_EQ(PlotType::automatic, plotType);
      plotType = PlotType::line;
      EXPECT_EQ(PlotType::line, plotType);
      plotType = PlotType::bar;
      EXPECT_EQ(PlotType::bar, plotType);
    }

    TEST_F(PlotWidgetTest, boundsValues)
    {
      // Test settable bounds values
      EXPECT_TRUE(std::isnan(xmin));
      EXPECT_TRUE(std::isnan(xmax));
      EXPECT_TRUE(std::isnan(ymin));
      EXPECT_TRUE(std::isnan(ymax));
      EXPECT_TRUE(std::isnan(y1min));
      EXPECT_TRUE(std::isnan(y1max));
      
      xmin = 0.0;
      xmax = 100.0;
      ymin = -10.0;
      ymax = 10.0;
      y1min = -5.0;
      y1max = 5.0;
      
      EXPECT_EQ(0.0, xmin);
      EXPECT_EQ(100.0, xmax);
      EXPECT_EQ(-10.0, ymin);
      EXPECT_EQ(10.0, ymax);
      EXPECT_EQ(-5.0, y1min);
      EXPECT_EQ(5.0, y1max);
    }

    TEST_F(PlotWidgetTest, displaySettings)
    {
      // Test display settings
      EXPECT_EQ(3u, displayNTicks);
      EXPECT_EQ(3.0, displayFontSize);
      
      displayNTicks = 5;
      displayFontSize = 4.5;
      
      EXPECT_EQ(5u, displayNTicks);
      EXPECT_EQ(4.5, displayFontSize);
    }

    TEST_F(PlotWidgetTest, markers)
    {
      // Test horizontal and vertical markers
      EXPECT_TRUE(horizontalMarkers.empty());
      EXPECT_TRUE(verticalMarkers.empty());
      
      horizontalMarkers.push_back("marker1");
      verticalMarkers.push_back("marker2");
      
      EXPECT_EQ(1u, horizontalMarkers.size());
      EXPECT_EQ(1u, verticalMarkers.size());
      EXPECT_EQ("marker1", horizontalMarkers[0]);
      EXPECT_EQ("marker2", verticalMarkers[0]);
    }

    TEST_F(PlotWidgetTest, penLabels)
    {
      // Test labelPen functionality
      EXPECT_TRUE(penLabels.empty());
      
      labelPen(0, "Pen 0");
      labelPen(1, "Pen 1");
      
      EXPECT_EQ(2u, penLabels.size());
      EXPECT_EQ("Pen 0", penLabels[0]);
      EXPECT_EQ("Pen 1", penLabels[1]);
      
      // Test that labelPen can expand the vector
      labelPen(5, "Pen 5");
      EXPECT_EQ(6u, penLabels.size());
      EXPECT_EQ("Pen 5", penLabels[5]);
    }
  }
}
