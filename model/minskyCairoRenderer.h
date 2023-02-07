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

#ifndef MINSKYCAIRORENDERER_H
#define MINSKYCAIRORENDERER_H
#include "cairoRenderer.h"
#include "pango.h"

namespace minsky
{
  struct CairoRenderer: public ravel::CairoRenderer
  {
    ecolab::Pango m_pango;
    
    static ecolab::Pango& pango(CAPIRenderer* r) {return static_cast<CairoRenderer*>(r)->m_pango;}

    static void s_showText(CAPIRenderer* c, const char* s)
    {
      pango(c).setText(s);
      pango(c).show();
    }
    static void s_setTextExtents(CAPIRenderer* c, const char* s)
    {pango(c).setText(s);}
    static double s_textWidth(CAPIRenderer* c) {return pango(c).width();}
    static double s_textHeight(CAPIRenderer* c) {return pango(c).height();}

    CairoRenderer(cairo_t* cairo): ravel::CairoRenderer(cairo), m_pango(cairo) {
      showText=s_showText;
      setTextExtents=s_setTextExtents;
      textWidth=s_textWidth;
      textHeight=s_textHeight;
    }
    
  };
}

#endif
