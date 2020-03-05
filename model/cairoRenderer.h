/*
  Ravel C API. © Ravelation Pty Ltd 2018
*/

#ifndef CAIRORENDERER_H
#define CAIRORENDERER_H
#include "capiRenderer.h"
#include <cairo/cairo.h>
namespace ravel
{
  struct CairoRenderer: public CAPIRenderer
  {
    cairo_t* cr;
    static cairo_t* cairo(CAPIRenderer* r)
    {return static_cast<CairoRenderer*>(r)->cr;}
    
    cairo_text_extents_t ext{0,0,0,0,0,0};
    static cairo_text_extents_t& extents(CAPIRenderer* r)
    {return static_cast<CairoRenderer*>(r)->ext;}
    
    CairoRenderer(cairo_t* cairo): cr(cairo)
    {
      moveTo=s_moveTo;
      lineTo=s_lineTo;
      relMoveTo=s_relMoveTo;
      relLineTo=s_relLineTo;
      arc=s_arc;
      setLineWidth=s_setLineWidth;
      newPath=s_newPath;
      closePath=s_closePath;
      fill=s_fill;
      clip=s_clip;
      stroke=s_stroke;
      strokePreserve=s_strokePreserve;
      setSourceRGB=s_setSourceRGB;
      showText=s_showText;
      setTextExtents=s_setTextExtents;
      textWidth=s_textWidth;
      textHeight=s_textHeight;
      identityMatrix=s_identityMatrix;
      translate=s_translate;
      scale=s_scale;
      rotate=s_rotate;
      save=s_save;
      restore=s_restore;
    }
    
    static void s_moveTo(CAPIRenderer* c, double x, double y)
    {cairo_move_to(cairo(c),x,y);}
    static void s_lineTo(CAPIRenderer* c, double x, double y)
    {cairo_line_to(cairo(c),x,y);}
    static void s_relMoveTo(CAPIRenderer* c, double x, double y)
    {cairo_rel_move_to(cairo(c),x,y);}
    static void s_relLineTo(CAPIRenderer* c, double x, double y)
    {cairo_rel_line_to(cairo(c),x,y);}
    static void s_arc(CAPIRenderer* c, double x, double y, double radius, double start, double end)
    {cairo_arc(cairo(c),x,y,radius,start,end);}
    static void s_setLineWidth(CAPIRenderer* c, double w)
    {cairo_set_line_width(cairo(c), w);}
    static void s_newPath(CAPIRenderer* c) {cairo_new_path(cairo(c));}
    static void s_closePath(CAPIRenderer* c) {cairo_close_path(cairo(c));}
    static void s_fill(CAPIRenderer* c)  {cairo_fill(cairo(c));}
    static void s_clip(CAPIRenderer* c)  {cairo_clip(cairo(c));}
    static void s_stroke(CAPIRenderer* c) {cairo_stroke(cairo(c));}
    static void s_strokePreserve(CAPIRenderer* c) {cairo_stroke_preserve(cairo(c));}
    static void s_setSourceRGB(CAPIRenderer* c, double r, double g, double b)
    {cairo_set_source_rgb(cairo(c),r,g,b);}
    static void s_showText(CAPIRenderer* c, const char* s)
    {cairo_show_text(cairo(c),s);}
    static void s_setTextExtents(CAPIRenderer* c, const char* s)
    {cairo_text_extents(cairo(c),s,&extents(c));}
    static double s_textWidth(CAPIRenderer* c) {return extents(c).width;}
    static double s_textHeight(CAPIRenderer* c) {return extents(c).height;}
    static void s_identityMatrix(CAPIRenderer* c)
    {cairo_identity_matrix(cairo(c));}
    static void s_translate(CAPIRenderer* c, double x, double y)
    {cairo_translate(cairo(c),x,y);}
    static void s_scale(CAPIRenderer* c, double sx, double sy)
    {cairo_scale(cairo(c),sx,sy);}
    static void s_rotate(CAPIRenderer* c, double angle)
    {cairo_rotate(cairo(c),angle);}
    static void s_save(CAPIRenderer* c)
    {cairo_save(cairo(c));}
    static void s_restore(CAPIRenderer* c)
    {cairo_restore(cairo(c));}
  };
}
#endif
