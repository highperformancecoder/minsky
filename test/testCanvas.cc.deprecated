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
#include <UnitTest++/UnitTest++.h>
using namespace minsky;
using namespace std;

namespace {
  struct CanvasFixture: public minsky::Canvas
  {
    CanvasFixture(): minsky::Canvas(make_shared<Group>()) {}
  };
}

SUITE(Canvas)
{
  TEST_FIXTURE(CanvasFixture,defaultPlotOptions)
    {
      addPlot();
      auto originalItem=itemFocus;
      auto originalPlot=dynamic_cast<PlotWidget*>(itemFocus.get());
      CHECK(originalPlot);
      if (!originalPlot) return;
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
      CHECK(firstPlot);
      if (!firstPlot) return;
      CHECK_EQUAL("",firstPlot->title);
      CHECK_EQUAL("",firstPlot->xlabel());
      CHECK_EQUAL("",firstPlot->ylabel());
      CHECK_EQUAL("",firstPlot->y1label());
      CHECK_EQUAL(originalPlot->palette.size()-1,firstPlot->palette.size());
      
      item=originalItem;
      setDefaultPlotOptions();

      // check we can apply the default options
      item=firstItem;
      applyDefaultPlotOptions();
      CHECK_EQUAL("",firstPlot->title);
      CHECK_EQUAL("",firstPlot->xlabel());
      CHECK_EQUAL("",firstPlot->ylabel());
      CHECK_EQUAL("",firstPlot->y1label());
      CHECK(firstPlot->subgrid);
      CHECK_EQUAL(originalPlot->palette.size(),firstPlot->palette.size());
      CHECK_EQUAL(originalPlot->palette.back().width, firstPlot->palette.back().width);
      CHECK_EQUAL(originalPlot->palette.back().dashStyle, firstPlot->palette.back().dashStyle);

      addPlot();
      auto secondPlot=dynamic_cast<PlotWidget*>(itemFocus.get());
      CHECK_EQUAL(originalPlot->title,secondPlot->title);
      CHECK_EQUAL(originalPlot->xlabel(),secondPlot->xlabel());
      CHECK_EQUAL(originalPlot->ylabel(),secondPlot->ylabel());
      CHECK_EQUAL(originalPlot->y1label(),secondPlot->y1label());
      CHECK(secondPlot->subgrid);
      CHECK_EQUAL(originalPlot->palette.size(),secondPlot->palette.size());
      CHECK_EQUAL(originalPlot->palette.back().width, secondPlot->palette.back().width);
      CHECK_EQUAL(originalPlot->palette.back().dashStyle, secondPlot->palette.back().dashStyle);
      
      
    }
}
