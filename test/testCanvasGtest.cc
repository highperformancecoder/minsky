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
#include "canvas.h"
#include "minsky_epilogue.h"
#undef True
#include <gtest/gtest.h>
using namespace minsky;
using namespace std;

// Test fixture for Canvas tests
class CanvasTest : public ::testing::Test, public minsky::Canvas
{
protected:
    CanvasTest() : minsky::Canvas(make_shared<Group>()) {}
    
    void SetUp() override
    {
        // Initialize canvas if needed
    }
};

TEST_F(CanvasTest, defaultPlotOptions)
{
    addPlot();
    auto originalItem=itemFocus;
    auto originalPlot=dynamic_cast<PlotWidget*>(itemFocus.get());
    ASSERT_TRUE(originalPlot);
    originalPlot->subgrid=true;
    originalPlot->title="hello";
    originalPlot->xlabel("x");
    originalPlot->ylabel("y");
    originalPlot->y1label("y1");
    originalPlot->palette.emplace_back();
    originalPlot->palette.back().width=2;
    originalPlot->palette.back().dashStyle=ecolab::Plot::LineStyle::dash;
    
    addPlot();
    auto firstItem=itemFocus;
    auto firstPlot=dynamic_cast<PlotWidget*>(itemFocus.get());
    ASSERT_TRUE(firstPlot);
    EXPECT_EQ("",firstPlot->title);
    EXPECT_EQ("",firstPlot->xlabel());
    EXPECT_EQ("",firstPlot->ylabel());
    EXPECT_EQ("",firstPlot->y1label());
    EXPECT_EQ(originalPlot->palette.size()-1,firstPlot->palette.size());
    
    item=originalItem;
    setDefaultPlotOptions();

    // check we can apply the default options
    item=firstItem;
    applyDefaultPlotOptions();
    EXPECT_EQ("",firstPlot->title);
    EXPECT_EQ("",firstPlot->xlabel());
    EXPECT_EQ("",firstPlot->ylabel());
    EXPECT_EQ("",firstPlot->y1label());
    EXPECT_TRUE(firstPlot->subgrid);
    EXPECT_EQ(originalPlot->palette.size(),firstPlot->palette.size());
    EXPECT_EQ(originalPlot->palette.back().width, firstPlot->palette.back().width);
    EXPECT_EQ(originalPlot->palette.back().dashStyle, firstPlot->palette.back().dashStyle);

    addPlot();
    auto secondPlot=dynamic_cast<PlotWidget*>(itemFocus.get());
    ASSERT_TRUE(secondPlot);
    EXPECT_EQ(originalPlot->title,secondPlot->title);
    EXPECT_EQ(originalPlot->xlabel(),secondPlot->xlabel());
    EXPECT_EQ(originalPlot->ylabel(),secondPlot->ylabel());
    EXPECT_EQ(originalPlot->y1label(),secondPlot->y1label());
    EXPECT_TRUE(secondPlot->subgrid);
    EXPECT_EQ(originalPlot->palette.size(),secondPlot->palette.size());
    EXPECT_EQ(originalPlot->palette.back().width, secondPlot->palette.back().width);
    EXPECT_EQ(originalPlot->palette.back().dashStyle, secondPlot->palette.back().dashStyle);
}