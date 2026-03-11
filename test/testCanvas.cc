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
    auto plotBeforeDefaultsApplied=dynamic_cast<PlotWidget*>(itemFocus.get());
    ASSERT_TRUE(plotBeforeDefaultsApplied);
    EXPECT_EQ("",plotBeforeDefaultsApplied->title);
    EXPECT_EQ("",plotBeforeDefaultsApplied->xlabel());
    EXPECT_EQ("",plotBeforeDefaultsApplied->ylabel());
    EXPECT_EQ("",plotBeforeDefaultsApplied->y1label());
    EXPECT_EQ(originalPlot->palette.size()-1,plotBeforeDefaultsApplied->palette.size());
    
    item=originalItem;
    setDefaultPlotOptions();

    // check we can apply the default options
    item=firstItem;
    applyDefaultPlotOptions();
    EXPECT_EQ("",plotBeforeDefaultsApplied->title);
    EXPECT_EQ("",plotBeforeDefaultsApplied->xlabel());
    EXPECT_EQ("",plotBeforeDefaultsApplied->ylabel());
    EXPECT_EQ("",plotBeforeDefaultsApplied->y1label());
    EXPECT_TRUE(plotBeforeDefaultsApplied->subgrid);
    EXPECT_EQ(originalPlot->palette.size(),plotBeforeDefaultsApplied->palette.size());
    EXPECT_EQ(originalPlot->palette.back().width, plotBeforeDefaultsApplied->palette.back().width);
    EXPECT_EQ(originalPlot->palette.back().dashStyle, plotBeforeDefaultsApplied->palette.back().dashStyle);

    addPlot();
    auto plotAfterDefaultsApplied=dynamic_cast<PlotWidget*>(itemFocus.get());
    ASSERT_TRUE(plotAfterDefaultsApplied);
    EXPECT_EQ(originalPlot->title,plotAfterDefaultsApplied->title);
    EXPECT_EQ(originalPlot->xlabel(),plotAfterDefaultsApplied->xlabel());
    EXPECT_EQ(originalPlot->ylabel(),plotAfterDefaultsApplied->ylabel());
    EXPECT_EQ(originalPlot->y1label(),plotAfterDefaultsApplied->y1label());
    EXPECT_TRUE(plotAfterDefaultsApplied->subgrid);
    EXPECT_EQ(originalPlot->palette.size(),plotAfterDefaultsApplied->palette.size());
    EXPECT_EQ(originalPlot->palette.back().width, plotAfterDefaultsApplied->palette.back().width);
    EXPECT_EQ(originalPlot->palette.back().dashStyle, plotAfterDefaultsApplied->palette.back().dashStyle);
}

/// Regression test for issue #610: unable to pull wire from group output.
/// Verifies that Group::select() correctly finds edge I/O variables even
/// without a preceding draw() call, by using positionEdgeVariables().
TEST(GroupIOTest, groupIOVarSelect)
{
    // Build a standalone group with an output variable.
    auto grp = make_shared<Group>();
    grp->self = grp;
    grp->moveTo(200, 200);
    grp->iWidth(100);
    grp->iHeight(100);

    // Add an output variable to the group (populates outVariables).
    grp->addOutputVar();
    ASSERT_EQ(1u, grp->outVariables.size());

    // Update bounding box for the output variable so margins() works.
    auto& outVar = *grp->outVariables[0];
    outVar.bb.update(outVar);

    // Compute the expected output-variable x position (right edge midpoint):
    // This replicates the draw1edge() math used by positionEdgeVariables().
    float leftMargin, rightMargin;
    grp->margins(leftMargin, rightMargin);
    const float z     = grp->zoomFactor();
    const float xEdge = 0.5f * (grp->iWidth() * z - rightMargin);
    // First variable (i==0): Rotate is identity for zero rotation, yOff == 0.
    const float expectedX = grp->x() + xEdge;
    const float expectedY = grp->y();

    // Before the fix, select() used stale positions (0,0) so this would return
    // nullptr. After the fix, positionEdgeVariables() is called first.
    auto found = grp->select(expectedX, expectedY);
    EXPECT_NE(nullptr, found)
        << "select() should find the output variable without a prior draw() call (issue #610)";
    if (found)
        EXPECT_EQ(grp->outVariables[0].get(), found.get())
            << "select() should return the output variable, not some other item";
}

/// Regression test to verify the zoomed wire-pull path works.
TEST(GroupIOTest, groupIOVarSelectZoomed)
{
    // Build a standalone group with an output variable.
    auto grp = make_shared<Group>();
    grp->self = grp;
    grp->moveTo(200, 200);
    grp->iWidth(100);
    grp->iHeight(100);

    // Set a non-default zoom
    grp->setZoom(2.5f);

    grp->addOutputVar();
    ASSERT_EQ(1u, grp->outVariables.size());

    // Update bounding box for the output variable so margins() works.
    auto& outVar = *grp->outVariables[0];
    outVar.bb.update(outVar);

    float leftMargin, rightMargin;
    grp->margins(leftMargin, rightMargin);
    const float z     = grp->zoomFactor();
    const float xEdge = 0.5f * (grp->iWidth() * z - rightMargin);
    const float expectedX = grp->x() + xEdge;
    const float expectedY = grp->y();

    auto found = grp->select(expectedX, expectedY);
    EXPECT_NE(nullptr, found)
        << "select() should find the output variable at a non-default zoom";

    // Since `draw()` hasn't positioned the ports dynamically, we check the exact 
    // underlying static port coordinates of the found variable after positions are set.
    auto outPort = outVar.ports(0).lock();
    EXPECT_EQ(ClickType::onPort, grp->clickType(outPort->x(), outPort->y()))
        << "clickType() on group should yield onPort at a non-default zoom for wire-pulling";
}

