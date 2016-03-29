/*
  @copyright Steve Keen 2016
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

/// definition of cairoItem code supporting rendering of a
/// minsky::Item on a Tk canvas

#include "init.h"
#include "minskyTCL.h"
#include <cairo_base.h>
#include <ecolab_epilogue.h>

using namespace ecolab::cairo;
using namespace ecolab;
using namespace std;
using namespace minsky;

namespace 
{
  // we need some extra fields to handle the additional options
  struct TkMinskyItem: public ImageItem
  {
    int id; // identifier of the C++ object this item represents
  };

  static Tk_CustomOption tagsOption = {
    (Tk_OptionParseProc *) Tk_CanvasTagsParseProc,
    Tk_CanvasTagsPrintProc, (ClientData) NULL
  };

  struct MinskyItemImage: public CairoImage
  {
    static Tk_ConfigSpec configSpecs[];
  };

  struct MinskyCairoItem: public MinskyItemImage
  {
    ItemPtr op;    
    cairo_t * cairo;


    void draw()
    {
      if (cairoSurface && op)
        {
          op->setCairoSurface(cairoSurface);
          cairo=cairoSurface->cairo();
          string label;
          cairo_reset_clip(cairo);
          xScale=yScale=1;
          initMatrix();
          cairo_select_font_face(cairo, "sans-serif", CAIRO_FONT_SLANT_ITALIC,
                                 CAIRO_FONT_WEIGHT_NORMAL);
          cairo_set_font_size(cairo,12);
          cairo_set_line_width(cairo,1);
          op->draw(cairo);
        }
    }
  };

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Winvalid-offsetof"

  // causes problems on MacOSX
  //#undef Tk_Offset
  //#define Tk_Offset(type, field) ((int) ((char *) &((type *) 1)->field)-1)

  Tk_ConfigSpec MinskyItemImage::configSpecs[] =
    {
      {TK_CONFIG_STRING, "-image", NULL, NULL,
       NULL, Tk_Offset(ImageItem, imageString), 0},
      {TK_CONFIG_DOUBLE, "-scale", NULL, NULL,
       "1.0", Tk_Offset(ImageItem, scale), TK_CONFIG_NULL_OK},
      {TK_CONFIG_DOUBLE, "-rotation", NULL, NULL,
       "0.0", Tk_Offset(ImageItem, rotation), TK_CONFIG_NULL_OK},
      {TK_CONFIG_INT, "-id", NULL, NULL,
       NULL, Tk_Offset(TkMinskyItem, id), 0},
      {TK_CONFIG_CUSTOM, "-tags", NULL, NULL,
       NULL, 0, TK_CONFIG_NULL_OK, &tagsOption},
      {TK_CONFIG_END}
    };
#pragma GCC diagnostic pop

  int creatProc(Tcl_Interp *interp, Tk_Canvas canvas, 
                         Tk_Item *itemPtr, int objc,Tcl_Obj *CONST objv[])
  {
    TkMinskyItem* tkMinskyItem=(TkMinskyItem*)(itemPtr);
    tkMinskyItem->id=-1;
    int r=createImage<MinskyCairoItem>(interp,canvas,itemPtr,objc,objv);
    if (r==TCL_OK && tkMinskyItem->id>=0)
      {
        if (MinskyCairoItem* i=(MinskyCairoItem*)(tkMinskyItem->cairoItem))
          if (auto m=dynamic_cast<const MinskyTCL*>(&minsky::cminsky()))
          {
            i->op=m->items[tkMinskyItem->id];
            if (!i->op)
              {
                Tcl_AppendResult
                  (interp,("invalid item id="+str(tkMinskyItem->id)).c_str(),NULL);
                return TCL_ERROR;
              }
            TkImageCode::ComputeImageBbox(canvas, tkMinskyItem);
          }
      }
    return r;
  }

  // overrride cairoItem's configureProc to process the extra config options
  int configureProc(Tcl_Interp *interp,Tk_Canvas canvas,Tk_Item *itemPtr,
                    int objc,Tcl_Obj *CONST objv[],int flags)
  {
    return TkImageCode::configureCairoItem
      (interp,canvas,itemPtr,objc,objv,flags, MinskyItemImage::configSpecs);
  }

  // register OperatorItem with Tk for use in canvases.
  int registerItems()
  {
    static Tk_ItemType itemType = cairoItemType();
    itemType.name=const_cast<char*>("item");
    itemType.itemSize=sizeof(TkMinskyItem);
    itemType.createProc=creatProc;
    itemType.configProc=configureProc;
    itemType.configSpecs=MinskyItemImage::configSpecs;
    Tk_CreateItemType(&itemType);

    return 0;
  }
}


static int dum=(initVec().push_back(registerItems), 0);
