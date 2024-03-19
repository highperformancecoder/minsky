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

#ifndef EQUATION_DISPLAY_H
#define EQUATION_DISPLAY_H
#include "renderNativeWindow.h"
#include "classdesc_access.h"

namespace minsky
{
  class Minsky;
  // handle the display of rendered equations on the screen
  class EquationDisplay: public RenderNativeWindow
  {
    Minsky& m;
    double m_width=0, m_height=0;
    bool redraw(int x0, int y0, int width, int height) override;
    CLASSDESC_ACCESS(EquationDisplay);
  public:
    float offsx=0, offsy=0, m_zoomFactor=1 ; // pan & zooming controls
    double width() const {return m_width;}
    double height() const {return m_height;}
    EquationDisplay(Minsky& m): m(m) {}
    EquationDisplay& operator=(const EquationDisplay& x) {RenderNativeWindow::operator=(x); return *this;}
    EquationDisplay(const EquationDisplay&)=default;
    void requestRedraw() {if (surface.get()) surface->requestRedraw();}
  };

}

#include "equationDisplay.cd"
#include "equationDisplay.rcd"
#include "equationDisplay.xcd"
#endif
