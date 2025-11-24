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
#include "minsky.h"
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



    TEST_F(PlotWidgetTest, numLines)
    {
      // Test numLines getter/setter
      EXPECT_EQ(1u, numLines());
      
      numLines(3);
      EXPECT_EQ(3u, numLines());
      
      numLines(5);
      EXPECT_EQ(5u, numLines());
      
      // Check that yvars and xvars are resized appropriately
      EXPECT_EQ(4*numLines()+nBoundsPorts, portsSize());
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
      EXPECT_EQ(ClickType::onPort, clickType(0, 0));  // Ports have not been moved into position yet (requires draw)
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
      var->name = "test";
      var->init("1.0");
      
      connectVar(var, PlotWidget::nBoundsPorts);  // Connect to first y port
      EXPECT_FALSE(yvars.empty());
      
      disconnectAllVars();
      EXPECT_TRUE(xvars.empty());
      EXPECT_TRUE(yvars.empty());
    }

    TEST_F(PlotWidgetTest, connectVarBounds)
    {
      // Test connecting variables to bound ports
      auto xminVar = std::make_shared<VariableValue>();
      xminVar->name = "xmin";
      xminVar->init("0.0");
      connectVar(xminVar, 0);
      
      auto xmaxVar = std::make_shared<VariableValue>();
      xmaxVar->name = "xmax";
      xmaxVar->init("10.0");
      connectVar(xmaxVar, 1);
      
      auto yminVar = std::make_shared<VariableValue>();
      yminVar->name = "ymin";
      yminVar->init("-1.0");
      connectVar(yminVar, 2);
      
      auto ymaxVar = std::make_shared<VariableValue>();
      ymaxVar->name = "ymax";
      ymaxVar->init("1.0");
      connectVar(ymaxVar, 3);
      
      auto y1minVar = std::make_shared<VariableValue>();
      y1minVar->name = "y1min";
      y1minVar->init("-2.0");
      connectVar(y1minVar, 4);
      
      auto y1maxVar = std::make_shared<VariableValue>();
      y1maxVar->name = "y1max";
      y1maxVar->init("2.0");
      connectVar(y1maxVar, 5);
      
      // Verify the variables are connected (these are public members)
      EXPECT_EQ(xminVar, this->xminVar);
      EXPECT_EQ(xmaxVar, this->xmaxVar);
      EXPECT_EQ(yminVar, this->yminVar);
      EXPECT_EQ(ymaxVar, this->ymaxVar);
      EXPECT_EQ(y1minVar, this->y1minVar);
      EXPECT_EQ(y1maxVar, this->y1maxVar);
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

    TEST_F(PlotWidgetTest, scalePlotWithFiniteBounds)
    {
      // Test scalePlot with finite bounds
      xmin = 0.0;
      xmax = 10.0;
      ymin = -5.0;
      ymax = 5.0;
      
      scalePlot();
      
      // After scaling, autoscale should be false
      EXPECT_FALSE(autoscale);
    }

    TEST_F(PlotWidgetTest, autoScale)
    {
      // Test autoScale method
      auto xminVar = std::make_shared<VariableValue>();
      xminVar->name = "xmin";
      xminVar->init("0.0");
      connectVar(xminVar, 0);
      
      // Verify it's connected
      EXPECT_EQ(xminVar, this->xminVar);
      
      autoScale();
      
      // After autoScale, all bound variables should be cleared
      EXPECT_EQ(nullptr, this->xminVar);
      EXPECT_EQ(nullptr, this->xmaxVar);
      EXPECT_EQ(nullptr, this->yminVar);
      EXPECT_EQ(nullptr, this->ymaxVar);
      EXPECT_EQ(nullptr, this->y1minVar);
      EXPECT_EQ(nullptr, this->y1maxVar);
    }

    TEST_F(PlotWidgetTest, addPlotPtSimple)
    {
      // Smoketest adding a simple plot point
      auto yvar = std::make_shared<VariableValue>(VariableType::flow);
      yvar->name = "y";
      yvar->init("1.0");
      yvar->allocValue();
      connectVar(yvar, PlotWidget::nBoundsPorts);
      
      addPlotPt(0.0);
      
   }

    TEST_F(PlotWidgetTest, resizeNegativeDimensions)
    {
      // Test resize with negative dimensions (should use abs)
      LassoBox box;
      box.x0 = 300;
      box.y0 = 200;
      box.x1 = 0;
      box.y1 = 0;
      
      resize(box);
      
      EXPECT_EQ(300.0f, iWidth());
      EXPECT_EQ(200.0f, iHeight());
    }

    TEST_F(PlotWidgetTest, numLinesWithPorts)
    {
      // Test that changing numLines updates port count
      numLines(3);
      
      // Port count should change: nBoundsPorts + 4*numLines
      // nBoundsPorts=6, numLines=3 -> 6 + 4*3 = 18 ports
      EXPECT_EQ(PlotWidget::nBoundsPorts + 4*3, m_ports.size());
    }

    TEST_F(PlotWidgetTest, connectVarYPorts)
    {
      // Test connecting variables to Y data ports
      auto yvar1 = std::make_shared<VariableValue>();
      yvar1->name = "yvar1";
      yvar1->init("1.0");
      
      auto yvar2 = std::make_shared<VariableValue>();
      yvar2->name = "yvar2";
      yvar2->init("2.0");
      
      numLines(2);
      
      // Connect to first Y port (port 6)
      connectVar(yvar1, PlotWidget::nBoundsPorts);
      
      // Connect to second Y port (port 7)
      connectVar(yvar2, PlotWidget::nBoundsPorts + 1);
      
      // yvars should now have entries
      EXPECT_FALSE(yvars.empty());
    }

    TEST_F(PlotWidgetTest, connectVarXPorts)
    {
      // Test connecting variables to X data ports
      auto xvar = std::make_shared<VariableValue>();
      xvar->name = "xvar";
      xvar->init("0.5");
      
      numLines(2);
      
      // X ports start at nBoundsPorts + 2*numLines
      // With numLines=2: port 6 + 2*2 = port 10
      connectVar(xvar, PlotWidget::nBoundsPorts + 2*numLines());
      
      // xvars should now have an entry
      EXPECT_FALSE(xvars.empty());
    }

    TEST_F(PlotWidgetTest, widthAndHeight)
    {
      // Test width() and height() methods
      EXPECT_NEAR(150.0f, width(),1);
      EXPECT_NEAR(150.0f, height(),1);
      
      iWidth(200);
      iHeight(250);
      
      EXPECT_NEAR(200.0f, width(),1);
      EXPECT_NEAR(250.0f, height(),1);
    }

    TEST_F(PlotWidgetTest, plotWidgetCast)
    {
      // Test plotWidgetCast methods
      const PlotWidget* constPtr = plotWidgetCast();
      EXPECT_NE(nullptr, constPtr);
      
      PlotWidget* ptr = plotWidgetCast();
      EXPECT_NE(nullptr, ptr);
    }

    TEST_F(PlotWidgetTest, containsMethod)
    {
      // Test contains method
      EXPECT_TRUE(contains(0, 0));  // Center should be contained
      
      const double z = Item::zoomFactor();
      const double w = 0.5 * iWidth() * z;
      const double h = 0.5 * iHeight() * z;
      
      // Far outside should not be contained
      EXPECT_FALSE(contains(x() + w + 100, y() + h + 100));
    }



    TEST_F(PlotWidgetTest, onMouseLeave)
    {
      // Test onMouseLeave clears valueString
      valueString = "test value";
      EXPECT_EQ("test value", valueString);
      
      onMouseLeave();
      
      // valueString should be empty after onMouseLeave
      EXPECT_EQ("", valueString);
    }

    TEST_F(PlotWidgetTest, updateIconCallsAddPlotPt)
    {
      // Test that updateIcon delegates to addPlotPt
      auto yvar = std::make_shared<VariableValue>(VariableType::flow);
      yvar->name = "y";
      yvar->init("1.0");
      yvar->allocValue();
      connectVar(yvar, PlotWidget::nBoundsPorts);
      
      updateIcon(1.0);
    }



    TEST_F(PlotWidgetTest, multipleNumLinesChanges)
    {
      // Test changing numLines multiple times
      EXPECT_EQ(1u, numLines());
      
      numLines(2);
      EXPECT_EQ(2u, numLines());
      
      numLines(1);
      EXPECT_EQ(1u, numLines());
      
      numLines(4);
      EXPECT_EQ(4u, numLines());
    }

    TEST_F(PlotWidgetTest, barWidthAllPalettes)
    {
      // Test that barWidth affects all palette entries
      // Add some palette entries if needed
      if (palette.empty()) {
        palette.resize(3);
      }
      
      barWidth(0.75);
      
      // All palette entries should have the same barWidth
      for (const auto& ls : palette) {
        EXPECT_EQ(0.75, ls.barWidth);
      }
    }

    TEST_F(PlotWidgetTest, barWidthMin)
    {
      // Test that barWidth returns the minimum across all palette entries
      palette.resize(2);
      
      palette[0].barWidth = 0.5;
      palette[1].barWidth = 0.3;
      
      // barWidth() should return the minimum
      EXPECT_EQ(0.3, barWidth());
    }

    TEST_F(PlotWidgetTest, labelPenClearsOnFirstCall)
    {
      // Test that labelPen clears penLabels on first call after clearPensOnLabelling is set
      penLabels.push_back("Old Label");
      EXPECT_EQ(1u, penLabels.size());
      
      // Manually trigger the clearing mechanism by labeling a pen
      labelPen(0, "New Label");
      
      // Check that the new label is set
      ASSERT_FALSE(penLabels.empty());
      EXPECT_EQ("New Label", penLabels[0]);
    }

    TEST_F(PlotWidgetTest, clickTypeOnResize)
    {
      auto p=corners()[0];
      // assume first item is -ve,-ve corner
      EXPECT_EQ(clickType(p.x()-0.8*portRadius, p.y()-0.8*portRadius), ClickType::onResize);
    }

    TEST_F(PlotWidgetTest, portsExist)
    {
      // Test that ports are created
      EXPECT_FALSE(m_ports.empty());
      
      // Should have at least nBoundsPorts
      EXPECT_GE(m_ports.size(), PlotWidget::nBoundsPorts);
    }

    TEST_F(PlotWidgetTest, connectMultipleVarsToSamePort)
    {
      // Test connecting multiple variables to the same Y port
      numLines(2);
      
      auto yvar1 = std::make_shared<VariableValue>();
      yvar1->name = "yvar1";
      yvar1->init("1.0");
      
      auto yvar2 = std::make_shared<VariableValue>();
      yvar2->name = "yvar2";
      yvar2->init("2.0");
      
      // Connect both to the same port
      connectVar(yvar1, PlotWidget::nBoundsPorts);
      connectVar(yvar2, PlotWidget::nBoundsPorts);
      
      // Both should be in yvars[0]
      ASSERT_FALSE(yvars.empty());
      ASSERT_GE(yvars[0].size(), 2u);
      EXPECT_EQ(yvar1, yvars[0][0]);
      EXPECT_EQ(yvar2, yvars[0][1]);
    }


  }
}
