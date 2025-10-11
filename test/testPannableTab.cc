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
#include "pannableTab.h"
#include "minsky_epilogue.h"
#undef True
#include <gtest/gtest.h>

namespace minsky
{
  namespace
  {
    struct Base
    {
      virtual void mouseDown(float, float)=0;
      virtual void mouseUp(float, float)=0;
      virtual void mouseMove(float x, float y)=0;
      virtual void moveTo(float x, float y)=0;
      virtual std::vector<float> position() const=0;
      virtual void zoom(double x, double y, double z)=0;
      virtual double zoomFactor() const=0;
      void requestRedraw() {}
      float offsx=0, offsy=0;
      double m_zoomFactor=1;
    };

    class PannableTabFixture : public PannableTab<Base>, public ::testing::Test
    {
    };

    TEST_F(PannableTabFixture, mouseDownUp)
    {
      moveTo(100,150);
      auto x1=position();
      EXPECT_EQ(2,x1.size());
      EXPECT_EQ(100,x1[0]);
      EXPECT_EQ(150,x1[1]);
      mouseDown(10,10);
      mouseUp(20,20);
      auto x2=position();
      EXPECT_EQ(2,x2.size());
      EXPECT_EQ(110,x2[0]);
      EXPECT_EQ(160,x2[1]);
    }
  }
}
