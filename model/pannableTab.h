/*
  @copyright Steve Keen 2021
  @author Russell Standish
  @author Wynand Dednam
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

#ifndef PANNABLE_TAB_H
#define PANNABLE_TAB_H
#include "classdesc_access.h"
#include <vector>

namespace minsky
{
  /// Mixin implementing common panning functionality in tabs
  template <class Base>
  class PannableTab: public Base
  {
    bool mousePressed=false;
    float deltaX, deltaY;
    CLASSDESC_ACCESS(PannableTab);
  public:
    PannableTab() {}
    template <class A> PannableTab(A& arg): Base(arg) {}
    void mouseDown(float x, float y) override
    {
      deltaX=x-Base::offsx;
      deltaY=y-Base::offsy;
      mousePressed=true;
    }
    void mouseUp(float x, float y) override
    {
      mouseMove(x,y);
      mousePressed=false;
    }
    void mouseMove(float x, float y) override
    {
      if (mousePressed)
        {
          Base::offsx=x-deltaX;
          Base::offsy=y-deltaY;
          Base::requestRedraw();
        }
    }
    void moveTo(float x, float y) override
    {
      Base::offsx=x;
      Base::offsy=y;
      Base::requestRedraw();
    }
    std::vector<float> position() const override
    {
      return {Base::offsx, Base::offsy};
    }
  };
}

#include "pannableTab.cd"
#endif
