/*
  @copyright Steve Keen 2017
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

#ifndef FONTDISPLAY_H
#define FONTDISPLAY_H
#include "renderNativeWindow.h"
#include <pango.h>

namespace minsky
{
  // display a sample of the current selected font
  struct FontDisplay: public RenderNativeWindow
  {
    bool redraw(int, int, int width, int height) override {
      cairo_move_to(surface->cairo(),0,0);
      ecolab::Pango pango(surface->cairo());
      pango.setFontSize(10);
      pango.setText("←→↑↓—▼αΣ∫√⊗≤");
      pango.show();
      return true;
    }
    void requestRedraw() {if (surface.get()) surface->requestRedraw();}
    FontDisplay()=default;
    FontDisplay(const FontDisplay&)=default;
    FontDisplay& operator=(const FontDisplay&) {return *this;}
  };
}

#include "fontDisplay.cd"
#include "fontDisplay.xcd"
#endif
