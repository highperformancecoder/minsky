/*
  @copyright Steve Keen 2012
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
// Implementations of canvas items representing operations and variables.

// invert the display of the power operator so that y is on top, and x
// below, for ticket #327
#define DISPLAY_POW_UPSIDE_DOWN


#include "operation.h"
#include "minsky.h"
#include "init.h"
#include "cairoItems.h"
#include "latexMarkup.h"
#include <cairo_base.h>
#include <arrays.h>
#include <pango.h>
#include <ecolab_epilogue.h>
#include <boost/geometry/geometry.hpp>

#if defined(MXE) && CAIRO_VERSION >= 11100
#error "Please use Cairo 1.10.x. See ticket #435."
#endif

using namespace ecolab::cairo;
using namespace ecolab;
using namespace std;
using namespace minsky;
using namespace boost::geometry;

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

  struct OperationItem: public MinskyItemImage
  {
    OperationPtr op;    
    cairo_t * cairo;


    void draw()
    {
      if (cairoSurface && op)
        {
          cairo=cairoSurface->cairo();
          string label;
          cairo_reset_clip(cairo);
          xScale=yScale=1;
          initMatrix();
          cairo_select_font_face(cairo, "sans-serif", CAIRO_FONT_SLANT_ITALIC,
                                 CAIRO_FONT_WEIGHT_NORMAL);
          cairo_set_font_size(cairo,12);
          cairo_set_line_width(cairo,1);


          RenderOperation(*op,cairo).draw();
        }
    }
  };

  struct VariableItem: public MinskyItemImage
  {
    VariablePtr var;
    void draw()
    {
      if (cairoSurface && var)
        {
          cairo_t *cairo=cairoSurface->cairo();
          xScale=yScale=1;

          cairo_reset_clip(cairo);
          initMatrix();
          cairo_set_line_width(cairo,1);

          RenderVariable rv(*var, cairo);
          rv.draw();
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

  int operationCreatProc(Tcl_Interp *interp, Tk_Canvas canvas, 
                         Tk_Item *itemPtr, int objc,Tcl_Obj *CONST objv[])
  {
    TkMinskyItem* tkMinskyItem=(TkMinskyItem*)(itemPtr);
    tkMinskyItem->id=-1;
    int r=createImage<OperationItem>(interp,canvas,itemPtr,objc,objv);
    if (r==TCL_OK && tkMinskyItem->id>=0)
      {
        OperationItem* opItem=(OperationItem*)(tkMinskyItem->cairoItem);
        if (opItem) 
          {
            opItem->op=minsky::minsky().operations[tkMinskyItem->id];
            TkImageCode::ComputeImageBbox(canvas, tkMinskyItem);
          }
      }
    return r;
  }

  int varCreatProc(Tcl_Interp *interp, Tk_Canvas canvas, 
                         Tk_Item *itemPtr, int objc,Tcl_Obj *CONST objv[])
  {
    TkMinskyItem* tkMinskyItem=(TkMinskyItem*)(itemPtr);
    tkMinskyItem->id=-1;
    int r=createImage<VariableItem>(interp,canvas,itemPtr,objc,objv);
    if (r==TCL_OK && tkMinskyItem->id>=0)
      {
        VariableItem* varItem=dynamic_cast<VariableItem*>(tkMinskyItem->cairoItem);
        if (varItem) 
          {
            varItem->var=minsky::minsky().variables[tkMinskyItem->id];
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
    static Tk_ItemType operationItemType = cairoItemType();
    operationItemType.name=const_cast<char*>("operation");
    operationItemType.itemSize=sizeof(TkMinskyItem);
    operationItemType.createProc=operationCreatProc;
    operationItemType.configProc=configureProc;
    operationItemType.configSpecs=MinskyItemImage::configSpecs;
    Tk_CreateItemType(&operationItemType);

    static Tk_ItemType varItemType = operationItemType;
    varItemType.name=const_cast<char*>("variable");
    varItemType.createProc=varCreatProc;
    Tk_CreateItemType(&varItemType);
    return 0;
  }

}


static int dum=(initVec().push_back(registerItems), 0);

void minsky::drawSelected(cairo_t* cairo)
{
  // implemented by filling the clip region with a transparent grey
  cairo_save(cairo);
  cairo_set_source_rgba(cairo, 0.5,0.5,0.5,0.25);
  cairo_paint(cairo);
  cairo_restore(cairo);
}

RenderOperation::RenderOperation(const OperationBase& op, cairo_t* cairo):
  op(op), cairo(cairo), hoffs(0)
{
  cairo_t *lcairo=cairo;
  cairo_surface_t* surf=NULL;
  if (!lcairo)
    {
      surf = cairo_image_surface_create(CAIRO_FORMAT_A1, 100,100);
      lcairo = cairo_create(surf);
    }

  const float l=op.l, r=op.r;
  w=0.5*(-l+r);
  h=op.h;

  switch (op.type())
    {
    case OperationType::constant:
    case OperationType::data:
      {
        cairo_text_extents_t bbox;
        const NamedOp& c=dynamic_cast<const NamedOp&>(op);

        Pango pango(lcairo);
        pango.setFontSize(10);
        pango.setMarkup(latexToPango(c.description));
        w=0.5*pango.width()+2; 
        h=0.5*pango.height()+4;
        hoffs=pango.top();
        break;
      }
    case OperationType::integrate:
      {
        const IntOp& i=dynamic_cast<const IntOp&>(op);
        if (i.coupled())
          {
            RenderVariable rv(*i.getIntVar(),cairo);
            w+=i.intVarOffset+rv.width(); 
            h=max(h, rv.height());
          }
        break;
      }
    default: break;
    }
 if (surf) //cleanup temporary surface
    {
      cairo_destroy(lcairo);
      cairo_surface_destroy(surf);
    }
}

Polygon RenderOperation::geom() const
{
  Rotate rotate(op.rotation, op.x(), op.y());
  float zl=op.l*op.zoomFactor, zh=op.h*op.zoomFactor, 
    zr=op.r*op.zoomFactor;
  Polygon r;
  // TODO: handle bound integration variables, and constants
  r+= rotate(op.x()+zl, op.y()-zh), rotate(op.x()+zl, op.y()+zh), 
    rotate(op.x()+zr, op.y());
  correct(r);
  return r;
}

namespace
{
  struct DrawBinOp
  {
    cairo_t *cairo;
    DrawBinOp(cairo_t *cairo): cairo(cairo) {}

    void drawPlus() const
    {
      cairo_move_to(cairo,0,-5);
      cairo_line_to(cairo,0,5);
      cairo_move_to(cairo,-5,0);
      cairo_line_to(cairo,5,0);
      cairo_stroke(cairo);
    }

    void drawMinus() const
    {
      cairo_move_to(cairo,-5,0);
      cairo_line_to(cairo,5,0);
      cairo_stroke(cairo);
    }

    void drawMultiply() const
    {
      cairo_move_to(cairo,-5,-5);
      cairo_line_to(cairo,5,5);
      cairo_move_to(cairo,-5,5);
      cairo_line_to(cairo,5,-5);
      cairo_stroke(cairo);
    }

    void drawDivide() const
    {
      cairo_move_to(cairo,-5,0);
      cairo_line_to(cairo,5,0);
      cairo_new_sub_path(cairo);
      cairo_arc(cairo,0,3,1,0,2*M_PI);
      cairo_new_sub_path(cairo);
      cairo_arc(cairo,0,-3,1,0,2*M_PI);
      cairo_stroke(cairo);
    }

    // puts a small symbol to identify port
    // x, y = position of symbol
    void drawPort(void (DrawBinOp::*symbol)() const, float x, float y, float rotation)  const
    {
      cairo_save(cairo);
      
      double angle=rotation * M_PI / 180.0;
      double fm=std::fmod(rotation,360);
      if (!((fm>-90 && fm<90) || fm>270 || fm<-270))
        y=-y;
      cairo_rotate(cairo, angle);
      
      cairo_translate(cairo,0.7*x,0.6*y);
      cairo_scale(cairo,0.5,0.5);
      
      // and counter-rotate
      cairo_rotate(cairo, -angle);
      (this->*symbol)();
      cairo_restore(cairo);
    }
  };
}

namespace minsky
{

  // virtual draw methods for operations - defined here rather than
  // operations.cc because it is more related to the functionality in
  // this file.

  template <> void Operation<OperationType::constant>::iconDraw(cairo_t* cairo) const
  {
    assert(false); //shouldn't be here
  }

  template <> void Operation<OperationType::data>::iconDraw(cairo_t* cairo) const
  {
    cairo_translate(cairo,-1,0);
    cairo_scale(cairo,1.5,0.75);
    cairo_arc(cairo,0,-3,3,0,2*M_PI);
    cairo_arc(cairo,0,3,3,0,M_PI);
    cairo_move_to(cairo,-3,3);
    cairo_line_to(cairo,-3,-3);
    cairo_move_to(cairo,3,3);
    cairo_line_to(cairo,3,-3);
    cairo_identity_matrix(cairo);
    cairo_set_line_width(cairo,1);
    cairo_stroke(cairo);
  }

  template <> void Operation<OperationType::time>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-4,2);
    cairo_show_text(cairo,"t");
  }

  template <> void Operation<OperationType::copy>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-4,2);
    cairo_show_text(cairo,"=");
  }

  template <> void Operation<OperationType::integrate>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-7,4.5);
    cairo_show_text(cairo,"\xE2\x88\xAB");
    cairo_show_text(cairo,"dt");
  }

  template <> void Operation<OperationType::differentiate>::iconDraw(cairo_t* cairo) const
  {
    cairo_save(cairo);
    cairo_move_to(cairo,-7,-1);
    cairo_set_font_size(cairo,8);
    cairo_show_text(cairo,"d");
    cairo_move_to(cairo,-7,0);cairo_line_to(cairo,2,0);
    cairo_set_line_width(cairo,0.5);cairo_stroke(cairo);
    cairo_move_to(cairo,-7,7);
    cairo_show_text(cairo,"dt");
    cairo_restore(cairo);
  }

  template <> void Operation<OperationType::sqrt>::iconDraw(cairo_t* cairo) const
  {
    cairo_save(cairo);
    cairo_set_font_size(cairo,10);   
    cairo_move_to(cairo,-7,6);
    cairo_show_text(cairo,"\xE2\x88\x9a");
    cairo_set_line_width(cairo,0.5);
    cairo_rel_move_to(cairo,0,-9);
    cairo_rel_line_to(cairo,5,0);
    cairo_set_source_rgb(cairo,0,0,0);
    cairo_stroke(cairo);
    cairo_restore(cairo);
    //    cairo_show_text(cairo,"sqrt");
  }

  template <> void Operation<OperationType::exp>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-7,3);
    cairo_show_text(cairo,"e");
    cairo_rel_move_to(cairo,0,-4);
    cairo_set_font_size(cairo,7);
    cairo_show_text(cairo,"x");
  }

  template <> void Operation<OperationType::pow>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-6,3);
    cairo_show_text(cairo,"x");
    cairo_rel_move_to(cairo,0,-4);
    cairo_set_font_size(cairo,7);
    cairo_show_text(cairo,"y");
    cairo_set_font_size(cairo,5);
    cairo_move_to(cairo, l+1, -h+6);
#ifdef DISPLAY_POW_UPSIDE_DOWN
    cairo_show_text(cairo,"y");
#else
    cairo_show_text(cairo,"x");
#endif
    cairo_move_to(cairo, l+1, h-3);
#ifdef DISPLAY_POW_UPSIDE_DOWN
    cairo_show_text(cairo,"x");
#else
    cairo_show_text(cairo,"y");
#endif
  }

  template <> void Operation<OperationType::le>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"≤");
  }

  template <> void Operation<OperationType::lt>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"<");
  }

  template <> void Operation<OperationType::eq>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"=");
  }

  template <> void Operation<OperationType::min>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"min");
  }

  template <> void Operation<OperationType::max>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"max");
  }

  template <> void Operation<OperationType::and_>::iconDraw(cairo_t* cairo) const
  {
    cairo_save(cairo);
    cairo_set_source_rgb(cairo,0,0,0);
    cairo_move_to(cairo,-4,3);
    cairo_line_to(cairo,-1,-3);
    cairo_line_to(cairo,2,3);
    cairo_restore(cairo);
  }

  template <> void Operation<OperationType::or_>::iconDraw(cairo_t* cairo) const
  {
    cairo_save(cairo);
    cairo_set_source_rgb(cairo,0,0,0);
    cairo_move_to(cairo,-4,-3);
    cairo_line_to(cairo,-1,3);
    cairo_line_to(cairo,2,-3);
    cairo_restore(cairo);
  }

  template <> void Operation<OperationType::not_>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"¬");
  }

  template <> void Operation<OperationType::ln>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo," ln");
  }

  template <> void Operation<OperationType::log>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,10);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"log");
    cairo_rel_move_to(cairo,0,3);
    cairo_set_font_size(cairo,7);
    cairo_show_text(cairo,"b");
    cairo_set_font_size(cairo,5);
    cairo_move_to(cairo, l+1, -h+6);
    cairo_show_text(cairo,"x");
    cairo_move_to(cairo, l+1, h-3);
    cairo_show_text(cairo,"b");
  
  }

  template <> void Operation<OperationType::sin>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,10);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"sin");
  }

  template <> void Operation<OperationType::cos>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,10);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"cos");
  }

  template <> void Operation<OperationType::tan>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,10);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"tan");
  }

  template <> void Operation<OperationType::asin>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,9);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"sin");
    cairo_rel_move_to(cairo,0,-3);
    cairo_set_font_size(cairo,7);
    cairo_show_text(cairo,"-1");
    cairo_rel_move_to(cairo,0,-2);
  }

  template <> void Operation<OperationType::acos>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,9);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"cos");
    cairo_rel_move_to(cairo,0,-3);
    cairo_set_font_size(cairo,7);
    cairo_show_text(cairo,"-1");
    cairo_rel_move_to(cairo,0,-2);
  }

  template <> void Operation<OperationType::atan>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,9);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"tan");
    cairo_rel_move_to(cairo,0,-3);
    cairo_set_font_size(cairo,7);
    cairo_show_text(cairo,"-1");
    cairo_rel_move_to(cairo,0,-2);
  }

  template <> void Operation<OperationType::sinh>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,8);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"sinh");
  }

  template <> void Operation<OperationType::cosh>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,8);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"cosh");
  }

  template <> void Operation<OperationType::tanh>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,8);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"tanh");
  }

  template <> void Operation<OperationType::abs>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,8);
    cairo_move_to(cairo,-6,3);
    cairo_show_text(cairo,"|x|");
  }
  template <> void Operation<OperationType::add>::iconDraw(cairo_t* cairo) const
  {
    DrawBinOp d(cairo);
    d.drawPlus();
    d.drawPort(&DrawBinOp::drawPlus, l, h, rotation);
    d.drawPort(&DrawBinOp::drawPlus, l, -h, rotation);
  }

  template <> void Operation<OperationType::subtract>::iconDraw(cairo_t* cairo) const
  {
    DrawBinOp d(cairo);
    d.drawMinus();
    d.drawPort(&DrawBinOp::drawPlus, l, -h, rotation);
    d.drawPort(&DrawBinOp::drawMinus, l, h, rotation);
  }

  template <> void Operation<OperationType::multiply>::iconDraw(cairo_t* cairo) const
  {
    DrawBinOp d(cairo);
    d.drawMultiply();
    d.drawPort(&DrawBinOp::drawMultiply, l, h, rotation);
    d.drawPort(&DrawBinOp::drawMultiply, l, -h, rotation);
  }

  template <> void Operation<OperationType::divide>::iconDraw(cairo_t* cairo) const
  {
    DrawBinOp d(cairo);
    d.drawDivide();
    d.drawPort(&DrawBinOp::drawMultiply, l, -h, rotation);
    d.drawPort(&DrawBinOp::drawDivide, l, h, rotation);
  }

  template <> void Operation<OperationType::numOps>::iconDraw(cairo_t* cairo) const
  {/* needs to be here, and is actually called */}

}

void RenderOperation::draw()
{
  op.draw(cairo);
}

void OperationBase::draw(cairo_t* cairo) const
{
  // if rotation is in 1st or 3rd quadrant, rotate as
  // normal, otherwise flip the text so it reads L->R
  double angle=rotation * M_PI / 180.0;
  double fm=std::fmod(rotation,360);
  bool textFlipped=!((fm>-90 && fm<90) || fm>270 || fm<-270);
  double coupledIntTranslation=0;

  auto t=type();
  // call the iconDraw method if data description is empty
  if (t==OperationType::data && dynamic_cast<const NamedOp&>(*this).description.empty())
    t=OperationType::numOps;

  switch (t)
    {
      // at the moment its too tricky to get all the information
      // together for rendering constants
    case OperationType::constant:
    case OperationType::data:
      {
        
        const NamedOp& c=dynamic_cast<const NamedOp&>(*this);
        cairo_save(cairo);
        
        Pango pango(cairo);
        pango.setFontSize(10*zoomFactor);
        pango.setMarkup(latexToPango(c.description));
        pango.angle=angle + (textFlipped? M_PI: 0);
        Rotate r(rotation+ (textFlipped? 180: 0),0,0);

        // parameters of icon in userspace (unscaled) coordinates
        float w, h, hoffs;
        w=0.5*pango.width()+2*zoomFactor; 
        h=0.5*pango.height()+4*zoomFactor;
        hoffs=pango.top()/zoomFactor;
    
        cairo_move_to(cairo,r.x(-w+1,-h-hoffs+2*zoomFactor), r.y(-w+1,-h-hoffs+2*zoomFactor));
        pango.show();
        cairo_restore(cairo);
        cairo_rotate(cairo, angle);
               
        cairo_set_source_rgb(cairo,0,0,1);
        cairo_move_to(cairo,-w,-h);
        cairo_line_to(cairo,-w,h);
        cairo_line_to(cairo,w,h);

        cairo_line_to(cairo,w+2*zoomFactor,0);
        cairo_line_to(cairo,w,-h);
        cairo_close_path(cairo);
        cairo_clip_preserve(cairo);
        cairo_stroke(cairo);

        // set the output ports coordinates
        // compute port coordinates relative to the icon's
        // point of reference
        double xx=w+2, yy=0;
        cairo_user_to_device(cairo, &xx, &yy);

        minsky().movePortTo
          (ports()[0], x()+xx, y()+yy);
        if (numPorts()>1)
          {
            xx=-w, yy=0;
            cairo_user_to_device(cairo, &xx, &yy);
            minsky().movePortTo
              (ports()[1], x()+xx, y()+yy);
          }
        if (mouseFocus)
          {
            cairo_save(cairo);
            cairo_identity_matrix(cairo);
            drawPorts(*this, ports(), cairo);
            cairo_restore(cairo);
          }
        if (selected) drawSelected(cairo);
        return;
    }
    case OperationType::integrate:
      if (const IntOp* i=dynamic_cast<const IntOp*>(this))
        if (i->coupled())
          {
            // we need to add some translation if the variable is bound
            RenderVariable rv(*i->getIntVar(),cairo);
            cairo_rotate(cairo,rotation*M_PI/180.0);
            coupledIntTranslation=-0.5*(i->intVarOffset+2*rv.width()+2+r)*zoomFactor;
            cairo_translate(cairo, coupledIntTranslation, 0);
            cairo_rotate(cairo,-rotation*M_PI/180.0);
          }
      cairo_save(cairo);
      cairo_scale(cairo,zoomFactor,zoomFactor);
      iconDraw(cairo);
      cairo_restore(cairo);
      break;
    default:
      cairo_save(cairo);
      cairo_scale(cairo,zoomFactor,zoomFactor);
      iconDraw(cairo);
      cairo_restore(cairo);
      break;
    }

  float l=OperationBase::l*zoomFactor, r=OperationBase::r*zoomFactor, 
    h=OperationBase::h*zoomFactor;
  int intVarWidth=0;
  cairo_rotate(cairo, angle);
  cairo_move_to(cairo,l,h);
  cairo_line_to(cairo,l,-h);
  cairo_line_to(cairo,r,0);
              
  cairo_close_path(cairo);

  //  cairo_save(cairo);
  cairo_set_source_rgb(cairo,0,0,1);
  cairo_stroke_preserve(cairo);
  //  cairo_restore(cairo);

  if (const IntOp* i=dynamic_cast<const IntOp*>(this))
    if (i->coupled())
      {
        float ivo=i->intVarOffset*zoomFactor;
        cairo_new_path(cairo);
        cairo_move_to(cairo,r,0);
        cairo_line_to(cairo,r+ivo,0);
        cairo_set_source_rgb(cairo,0,0,0);
        cairo_stroke(cairo);
        
        VariablePtr intVar=i->getIntVar();
        // display an integration variable next to it
        RenderVariable rv(*intVar, cairo);
        // save the render width for later use in setting the clip
        intVarWidth=rv.width(); 
        // set the port location...
        intVar->moveTo(i->x()+ivo+intVarWidth, i->y());
            
        cairo_save(cairo);
        cairo_translate(cairo,r+ivo+intVarWidth,0);
        // to get text to render correctly, we need to set
        // the var's rotation, then antirotate it
        i->getIntVar()->rotation=i->rotation;
        cairo_rotate(cairo, -M_PI*i->rotation/180.0);
        rv.draw();
        //i->getIntVar()->draw(cairo);
        cairo_restore(cairo);

        // build clip path the hard way grr...
        cairo_move_to(cairo,l,h);
        cairo_line_to(cairo,l,-h);
        cairo_line_to(cairo,r,0);
        cairo_line_to(cairo,r+ivo,0);
        float rvw=rv.width()*zoomFactor, rvh=rv.height()*zoomFactor;
        cairo_line_to(cairo,r+ivo,rvh);
        cairo_line_to(cairo,r+ivo+2*rvw,rvh);
        cairo_line_to(cairo,r+ivo+2*rvw+2*zoomFactor,0);
        cairo_line_to(cairo,r+ivo+2*rvw,-rvh);
        cairo_line_to(cairo,r+ivo,-rvh);
        cairo_line_to(cairo,r+ivo,0);
        cairo_close_path(cairo);
      }

  cairo_clip(cairo);

  // compute port coordinates relative to the icon's
  // point of reference
  double x0=r-2, y0=0, x1=l, y1=numPorts() > 2? -h+3: 0, 
    x2=l, y2=numPorts() > 2? h-3: 0;
                  
  if (textFlipped) swap(y1,y2);

  // adjust for integration variable
  if (const IntOp* i=dynamic_cast<const IntOp*>(this))
    if (i->coupled())
      x0+=i->intVarOffset+2*intVarWidth+2;
  // adjust by the translation  used for coupled integrals
  x0+=coupledIntTranslation;
  x1+=coupledIntTranslation;
  x2+=coupledIntTranslation;


  cairo_save(cairo);
  cairo_identity_matrix(cairo);
  cairo_translate(cairo, x(), y());
  //cairo_scale(cairo,zoomFactor,zoomFactor);
  cairo_rotate(cairo, angle);
  cairo_user_to_device(cairo, &x0, &y0);
  cairo_user_to_device(cairo, &x1, &y1);
  cairo_user_to_device(cairo, &x2, &y2);
  cairo_restore(cairo);

  if (numPorts()>0) 
    minsky().movePortTo(ports()[0], x0, y0);
  if (numPorts()>1) 
    {
#ifdef DISPLAY_POW_UPSIDE_DOWN
    if (type()==OperationType::pow)
      minsky().movePortTo(ports()[1], x2, y2);
    else
#endif
      minsky().movePortTo(ports()[1], x1, y1);
    }

  if (numPorts()>2)
    {
#ifdef DISPLAY_POW_UPSIDE_DOWN
      if (type()==OperationType::pow)
        minsky().movePortTo(ports()[2], x1, y1);
      else
#endif
        minsky().movePortTo(ports()[2], x2, y2);
    }

  cairo_translate(cairo,-coupledIntTranslation,0);
 
  if (mouseFocus)
    {
      cairo_save(cairo);
      cairo_identity_matrix(cairo);
      drawPorts(*this, ports(), cairo);
      cairo_restore(cairo);
    }
  if (selected) drawSelected(cairo);

}


RenderVariable::RenderVariable(const VariableBase& var, cairo_t* cairo):
  var(var), cairo(cairo)
{
  cairo_t *lcairo=cairo;
  cairo_surface_t* surf=NULL;
  if (!lcairo)
    {
      surf = cairo_image_surface_create(CAIRO_FORMAT_A1, 100,100);
      lcairo = cairo_create(surf);
    }

  Pango pango(lcairo);
  pango.setFontSize(12);
  pango.setMarkup(latexToPango(var.name()));
  w=0.5*pango.width()+2; 
  h=0.5*pango.height()+4;
  hoffs=pango.top();


  if (surf) //cleanup temporary surface
    {
      cairo_destroy(lcairo);
      cairo_surface_destroy(surf);
    }
}

Polygon RenderVariable::geom() const
{
  float x=var.x(), y=var.y();
  float wz=w*var.zoomFactor, hz=h*var.zoomFactor;
  Rotate rotate(var.rotation, x, y);

  Polygon r;
  r+= rotate(x-wz, y-hz), rotate(x-wz, y+hz), 
    rotate(x+wz, y+hz), rotate(x+wz, y-hz);
  correct(r);
  return r;
}

void RenderVariable::draw()
{
  updatePortLocs();
  var.draw(cairo);
}

void VariableBase::draw(cairo_t *cairo) const
{
  double angle=rotation * M_PI / 180.0;
  double fm=std::fmod(rotation,360);

  //  cairo_save(cairo);
  //  cairo_save(cairo);

  //cairo_scale(cairo,zoomFactor,zoomFactor);

  Pango pango(cairo);
  pango.setFontSize(12*zoomFactor);
  pango.setMarkup(latexToPango(name()));
  // if rotation is in 1st or 3rd quadrant, rotate as
  // normal, otherwise flip the text so it reads L->R
  bool notflipped=(fm>-90 && fm<90) || fm>270 || fm<-270;
  Rotate r(rotation + (notflipped? 0: 180),0,0);
  pango.angle=angle+(notflipped? 0: M_PI);
      
  // parameters of icon in userspace (unscaled) coordinates
  float w, h, hoffs;
  w=0.5*pango.width()+2*zoomFactor; 
  h=0.5*pango.height()+4*zoomFactor;
  hoffs=pango.top()/zoomFactor;

  cairo_move_to(cairo,r.x(-w+1,-h-hoffs+2), r.y(-w+1,-h-hoffs+2)/*h-2*/);
  pango.show();
  //  cairo_restore(cairo);

  cairo_rotate(cairo, angle);
  // constants and parameters should be rendered in blue, all others in red
  switch (type())
    {
    case constant: case parameter:
      cairo_set_source_rgb(cairo,0,0,1);
      break;
    default:
      cairo_set_source_rgb(cairo,1,0,0);
      break;
    }
  cairo_move_to(cairo,-w,-h);
  if (lhs())
    cairo_line_to(cairo,-w+2*zoomFactor,0);
  cairo_line_to(cairo,-w,h);
  cairo_line_to(cairo,w,h);
  cairo_line_to(cairo,w+2*zoomFactor,0);
  cairo_line_to(cairo,w,-h);
  cairo_close_path(cairo);
  cairo_clip_preserve(cairo);
  cairo_stroke(cairo);
  
  //  cairo_restore(cairo);
  if (mouseFocus)
    {
      cairo_save(cairo);
      cairo_identity_matrix(cairo);
      drawPorts(*this, ports(), cairo);
      cairo_restore(cairo);
    }
  if (selected) drawSelected(cairo);
}

void RenderVariable::updatePortLocs()
{
  double angle=var.rotation * M_PI / 180.0;
  double x0=w, y0=0, x1=-w+2, y1=0;
  double sa=sin(angle), ca=cos(angle);
  minsky().movePortTo(var.outPort(), 
                           var.x()+var.zoomFactor*(x0*ca-y0*sa), 
                           var.y()+var.zoomFactor*(y0*ca+x0*sa));
  minsky().movePortTo(var.inPort(), 
                           var.x()+var.zoomFactor*(x1*ca-y1*sa), 
                           var.y()+var.zoomFactor*(y1*ca+x1*sa));
}

bool RenderVariable::inImage(float x, float y)
{
  float dx=x-var.x(), dy=y-var.y();
  float rx=dx*cos(var.rotation*M_PI/180)-dy*sin(var.rotation*M_PI/180);
  float ry=dy*cos(var.rotation*M_PI/180)+dx*sin(var.rotation*M_PI/180);
  return rx>=-w && rx<=w && ry>=-h && ry <= h;
}


void minsky::drawTriangle
(cairo_t* cairo, double x, double y, cairo::Colour& col, double angle)
{
  cairo_save(cairo);
  cairo_new_path(cairo);
  cairo_set_source_rgba(cairo,col.r,col.g,col.b,col.a);
  cairo_translate(cairo,x,y);
  cairo_rotate(cairo, angle);
  cairo_move_to(cairo,10,0);
  cairo_line_to(cairo,0,-3);
  cairo_line_to(cairo,0,3);
  cairo_fill(cairo);
  cairo_restore(cairo);
}

