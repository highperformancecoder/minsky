/*
  @copyright Steve Keen 2015
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
#include "switchIcon.h"
#include "minsky.h"
#include "init.h"
#include "cairoItems.h"
#include <ecolab_epilogue.h>
using namespace ecolab::cairo;
using namespace ecolab;
using namespace std;

namespace minsky
{
  namespace
  {

    static Tk_CustomOption tagsOption = {
      (Tk_OptionParseProc *) Tk_CanvasTagsParseProc,
      Tk_CanvasTagsPrintProc, (ClientData) NULL
    };

    struct SwitchIconItem: public CairoImage
    {
      static Tk_ConfigSpec configSpecs[];
      ItemPtr sw;
      void draw()
      {
        if (cairoSurface && sw)
          {
            cairoSurface->clear();
            sw->draw(cairoSurface->cairo());
          }
      }

    };
    
    // we need some extra fields to handle the additional options
    struct TkXGLItem: public ImageItem
    {
      int id; // C++ object identifier
    };

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
    Tk_ConfigSpec SwitchIconItem::configSpecs[] =
    {
      {TK_CONFIG_INT, "-id", NULL, NULL,
       NULL, Tk_Offset(TkXGLItem, id), 0},
      {TK_CONFIG_CUSTOM, "-tags", NULL, NULL,
       NULL, 0, TK_CONFIG_NULL_OK, &tagsOption},
      {TK_CONFIG_END}
    };
#pragma GCC diagnostic pop

    int creatProc(Tcl_Interp *interp, Tk_Canvas canvas, 
                  Tk_Item *itemPtr, int objc,Tcl_Obj *CONST objv[])
    {
      TkXGLItem* tkXGLItem=(TkXGLItem*)(itemPtr);
      tkXGLItem->id=-1;
      int r=createImage<SwitchIconItem>(interp,canvas,itemPtr,objc,objv);
      if (r==TCL_OK)
        {
          SwitchIconItem* xglItem=(SwitchIconItem*)(tkXGLItem->cairoItem);
          if (xglItem) 
            {
              xglItem->sw = minsky::minsky().model->findItem(tkXGLItem->id);
              TkImageCode::ComputeImageBbox(canvas, tkXGLItem);
            }
        }
      return r;
    }

  // overrride cairoItem's configureProc to process the extra config options
  int configureProc(Tcl_Interp *interp,Tk_Canvas canvas,Tk_Item *itemPtr,
                    int objc,Tcl_Obj *CONST objv[],int flags)
  {
    return TkImageCode::configureCairoItem
      (interp,canvas,itemPtr,objc,objv,flags, SwitchIconItem::configSpecs);
  }

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif
    // register GodleyItem with Tk for use in canvases.
    int registerItem()
    {
      static Tk_ItemType switchIconType = cairoItemType();
      switchIconType.name="switch";
      switchIconType.itemSize=sizeof(TkXGLItem);
      switchIconType.createProc=creatProc;
      switchIconType.configProc=configureProc;
      switchIconType.configSpecs=SwitchIconItem::configSpecs;
      Tk_CreateItemType(&switchIconType);
      return 0;
    }
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

    int dum=(initVec().push_back(registerItem), 0);
  }
 
  SwitchIcon::SwitchIcon()
  {
    ports.emplace_back(new Port(*this,Port::noFlags));
    ports.emplace_back(new Port(*this, Port::inputPort));
    setNumCases(2); ///<default to if/then
  }

  void SwitchIcon::setNumCases(unsigned n)
  {
    if (n<2) throw error("switches need at least two cases");
    for (unsigned i=ports.size(); i<n+2; ++i)
      ports.emplace_back(new Port(*this, Port::inputPort));
    ports.resize(n+2); // in case ports was larger than n+2
  }

  unsigned SwitchIcon::value() const
  {
    double x=ports[1]->value();
    if (x<1)
      return 0;
    else if (x>=numCases()-1)
      return numCases()-1;
    else
      return unsigned(x);
  }

  void SwitchIcon::draw(cairo_t* cairo) const
  {
    cairo_set_line_width(cairo,1);
    // square icon
    float width=8*zoomFactor*numCases();
    cairo_rectangle(cairo,-0.5*width,-0.5*width,width,width);
    cairo_stroke(cairo);


    float w=flipped? -width: width;
    float o=flipped? -8: 8;
    // output port
    drawTriangle(cairo, 0.5*w, 0, palette[0], flipped? M_PI: 0);
    ports[0]->moveTo(x()+0.5*w, y());
    // control port
    drawTriangle(cairo, 0, -0.5*width-8, palette[0], M_PI/2);
    ports[1]->moveTo(x(), y()-0.5*width-8);
    float dy=width/numCases();
    float y1=-0.5*width+0.5*dy;
    // case ports
    for (size_t i=2; i<ports.size(); ++i, y1+=dy)
      {
        drawTriangle(cairo, -0.5*w-o, y1, palette[(i-2)%paletteSz], flipped? M_PI: 0);
        ports[i]->moveTo(x()+-0.5*w-o, y()+y1);
      }
    // draw indicating arrow
    cairo_move_to(cairo,0.5*w, 0);
    y1=-0.5*width+0.5*dy+value()*dy;
    cairo_line_to(cairo,-0.45*w,0.9*y1);
    cairo_stroke(cairo);

    if (mouseFocus)
      drawPorts(cairo);

    // add 8 pt margin to allow for ports
    cairo_rectangle(cairo,-0.5*width-8,-0.5*width-8,width+16,width+8);
    cairo_clip(cairo);
   
  }


}
