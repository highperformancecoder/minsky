/*
  @copyright Steve Keen 2024
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
#include "sheet.h"
#include "minsky.h"
#include "minsky_epilogue.h"
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>

using namespace minsky;
using namespace std;

namespace
{
  class SheetTest : public Sheet, public ::testing::Test
  {
  protected:
    void SetUp() override
    {
      // Set default dimensions for tests
      iWidth(200);
      iHeight(150);
      moveTo(100, 100);
    }
    
    void TearDown() override
    {
      // Clean up global minsky state after each test
      minsky::minsky().clearAllMaps();
    }
  };

  TEST_F(SheetTest, construction)
  {
    Sheet sheet;
    EXPECT_EQ(1, sheet.portsSize());
    EXPECT_EQ(100, sheet.iWidth());
    EXPECT_EQ(100, sheet.iHeight());
  }

  TEST_F(SheetTest, onResizeHandle)
  {
    moveTo(100, 100);
    iWidth(200);
    iHeight(150);
    double z = zoomFactor();
    double w = 0.5 * 200 * z;
    double h = 0.5 * 150 * z;
    
    // Test bottom-right resize handle
    EXPECT_TRUE(onResizeHandle(100 + w, 100 + h));
    
    // Test top-right resize handle
    EXPECT_TRUE(onResizeHandle(100 + w, 100 - h));
    
    // Test bottom-left resize handle (when showRavel is false)
    showRavel = false;
    EXPECT_TRUE(onResizeHandle(100 - w, 100 + h));
    
    // Test points not on resize handle
    EXPECT_FALSE(onResizeHandle(100, 100));
  }

  TEST_F(SheetTest, inItem)
  {
    moveTo(100, 100);
    iWidth(200);
    iHeight(150);
    
    // Test point inside item
    EXPECT_TRUE(inItem(100, 100));
    
    // Test point outside item
    EXPECT_FALSE(inItem(0, 0));
    EXPECT_FALSE(inItem(300, 300));
  }

  TEST_F(SheetTest, clickType)
  {
    moveTo(100, 100);
    iWidth(200);
    iHeight(150);
    double z = zoomFactor();
    double w = 0.5 * 200 * z;
    double h = 0.5 * 150 * z;
    
    // Test resize handle
    EXPECT_EQ(ClickType::onResize, clickType(100 + w, 100 + h));
    
    // Test inside item
    EXPECT_EQ(ClickType::inItem, clickType(100, 100));
    
    // Test on item border
    EXPECT_EQ(ClickType::onItem, clickType(100 + w * 0.9, 100 + h * 0.9));
    
    // Test outside item
    EXPECT_EQ(ClickType::outside, clickType(0, 0));
  }

  TEST_F(SheetTest, contains)
  {
    moveTo(100, 100);
    iWidth(200);
    iHeight(150);
    
    // Test point inside item
    EXPECT_TRUE(contains(100, 100));
    
    // Test point outside item
    EXPECT_FALSE(contains(0, 0));
  }

  TEST_F(SheetTest, scrollUpDown)
  {
    // Initial state - no scrolling possible without data
    EXPECT_FALSE(scrollUp());
    EXPECT_FALSE(scrollDown());
    // add a rank 3 tensor, which is "scrollable"
    minsky::minsky().canvas.addVariable("3dTensor",VariableType::parameter);
    auto& param=*minsky::minsky().canvas.itemFocus->variableCast();
    param.init("rand(3,3,3)");
    minsky::minsky().canvas.addSheet();
    auto& sheet=dynamic_cast<Sheet&>(*minsky::minsky().canvas.itemFocus);
    minsky::minsky().model->addWire(param, sheet, 0);
    minsky::minsky().reset();
    EXPECT_TRUE(sheet.scrollUp());
    EXPECT_TRUE(sheet.scrollDown());
  }

  TEST_F(SheetTest, onKeyPress)
  {
    // Test arrow key handling with rank 3 tensor
    minsky::minsky().canvas.addVariable("3dTensor",VariableType::parameter);
    auto& param=*minsky::minsky().canvas.itemFocus->variableCast();
    param.init("rand(3,3,3)");
    minsky::minsky().canvas.addSheet();
    auto& sheet=dynamic_cast<Sheet&>(*minsky::minsky().canvas.itemFocus);
    minsky::minsky().model->addWire(param, sheet, 0);
    minsky::minsky().reset();
    
    // Up arrow (0xff52) - should scroll up
    EXPECT_TRUE(sheet.onKeyPress(0xff52, "", 0));
    
    // Down arrow (0xff54) - should scroll down
    EXPECT_TRUE(sheet.onKeyPress(0xff54, "", 0));
    
    // Right arrow (0xff53) - should scroll up
    EXPECT_TRUE(sheet.onKeyPress(0xff53, "", 0));
    
    // Left arrow (0xff51) - should scroll down
    EXPECT_TRUE(sheet.onKeyPress(0xff51, "", 0));
    
    // Other key - should return false
    EXPECT_FALSE(sheet.onKeyPress(0x0041, "A", 0));
  }

  TEST_F(SheetTest, exportAsCSVWithoutValue)
  {
    // Attempting to export without a value should throw an error
    string tmpFile = boost::filesystem::unique_path().string();
    EXPECT_THROW(exportAsCSV(tmpFile, false), std::exception);
  }

  TEST_F(SheetTest, setSliceIndicator)
  {
    // Add a rank 3 tensor to test slice indicator
    minsky::minsky().canvas.addVariable("3dTensor",VariableType::parameter);
    auto& param=*minsky::minsky().canvas.itemFocus->variableCast();
    param.init("rand(3,3,3)");
    minsky::minsky().canvas.addSheet();
    auto& sheet=dynamic_cast<Sheet&>(*minsky::minsky().canvas.itemFocus);
    minsky::minsky().model->addWire(param, sheet, 0);
    minsky::minsky().reset();
    
    // Check that sliceIndicator is set to something useful
    EXPECT_FALSE(sheet.setSliceIndicator().empty());
  }
}
