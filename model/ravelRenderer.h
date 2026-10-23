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

#ifndef RAVELRENDERER_H
#define RAVELRENDERER_H
#include "capiRenderer.h"
#include "IMansoura.h"
#include <memory>

namespace minsky
{
  /// adapts a mansoura::IMansoura drawing context to the Ravel
  /// CAPIRenderer interface
  struct RavelRenderer: public CAPIRenderer
  {
    const mansoura::IMansoura& shim;
    std::unique_ptr<mansoura::ICacheRender> text;

    static const mansoura::IMansoura& s(CAPIRenderer* r)
    {return static_cast<RavelRenderer*>(r)->shim;}
    static mansoura::ICacheRender& txt(CAPIRenderer* r)
    {return *static_cast<RavelRenderer*>(r)->text;}

    static void s_showText(CAPIRenderer* c, const char* str)
    {
      double x,y;
      s(c).getCurrentPoint(x,y);
      txt(c).setText(str);
      // paint a partially obscure white rectange to make the text pop
      {
        const mansoura::PreserveContext pc(s(c));
        auto e=txt(c).extents();
        s(c).rectangle(x,y,e.width,e.height);
        s(c).setSourceRGBA(1,1,1,0.3);
        s(c).fill();
      }
      s(c).moveTo(x,y);
      txt(c).show();
    }
    static void s_setTextExtents(CAPIRenderer* c, const char* str)
    {txt(c).setText(str);}
    static double s_textWidth(CAPIRenderer* c) {return txt(c).extents().width;}
    static double s_textHeight(CAPIRenderer* c) {return txt(c).extents().height;}

    RavelRenderer(const mansoura::IMansoura& shim):
      shim(shim), text(shim.cachedRender(mansoura::TextProperties()))
    {
      moveTo=[](CAPIRenderer* c, double x, double y){s(c).moveTo(x,y);};
      lineTo=[](CAPIRenderer* c, double x, double y){s(c).lineTo(x,y);};
      relMoveTo=[](CAPIRenderer* c, double x, double y){s(c).relMoveTo(x,y);};
      relLineTo=[](CAPIRenderer* c, double x, double y){s(c).relLineTo(x,y);};
      arc=[](CAPIRenderer* c, double x, double y, double r, double a0, double a1){s(c).arc(x,y,r,a0,a1);};
      setLineWidth=[](CAPIRenderer* c, double w){s(c).setLineWidth(w);};
      newPath=[](CAPIRenderer* c){s(c).newPath();};
      closePath=[](CAPIRenderer* c){s(c).closePath();};
      fill=[](CAPIRenderer* c){s(c).fill();};
      clip=[](CAPIRenderer* c){s(c).clip();};
      stroke=[](CAPIRenderer* c){s(c).stroke();};
      strokePreserve=[](CAPIRenderer* c){s(c).strokePreserve();};
      setSourceRGB=[](CAPIRenderer* c, double r, double g, double b){s(c).setSourceRGB(r,g,b);};
      setSourceRGBA=[](CAPIRenderer* c, double r, double g, double b, double a){s(c).setSourceRGBA(r,g,b,a);};
      showText=s_showText;
      setTextExtents=s_setTextExtents;
      textWidth=s_textWidth;
      textHeight=s_textHeight;
      identityMatrix=[](CAPIRenderer* c){s(c).identityMatrix();};
      translate=[](CAPIRenderer* c, double x, double y){s(c).translate(x,y);};
      scale=[](CAPIRenderer* c, double x, double y){s(c).scale(x,y);};
      rotate=[](CAPIRenderer* c, double a){s(c).rotate(a);};
      save=[](CAPIRenderer* c){s(c).save();};
      restore=[](CAPIRenderer* c){s(c).restore();};
    }
  };
}

#endif
