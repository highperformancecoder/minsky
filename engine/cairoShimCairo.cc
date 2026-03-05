#include "cairoShim.h"
#define CAIRO_WIN32_STATIC_BUILD
#include <cairo.h>
#undef CAIRO_WIN32_STATIC_BUILD

using namespace std;

namespace ravel
{
  void CairoShimCairo::moveTo(double x, double y) 
  {cairo_move_to(cairo,x,y);}

  void CairoShimCairo::lineTo(double x, double y)
  {cairo_line_to(cairo,x,y);}

  void CairoShimCairo::relMoveTo(double x, double y)
  {cairo_rel_move_to(cairo,x,y);}

  void CairoShimCairo::relLineTo(double x, double y)
  {cairo_rel_line_to(cairo,x,y);}

  void CairoShimCairo::arc
  (double x, double y, double radius, double start, double end)
  {cairo_arc(cairo,x,y,radius,start,end);}

    // paths
  void CairoShimCairo::newPath()
  {cairo_new_path(cairo);}

  void CairoShimCairo::closePath()
  {cairo_close_path(cairo);}

  void CairoShimCairo::fill()
  {cairo_fill(cairo);}
  
  void CairoShimCairo::clip()
  {cairo_clip(cairo);}

  void CairoShimCairo::stroke()
  {cairo_stroke(cairo);}
  
   void CairoShimCairo::strokePreserve()
  {cairo_stroke_preserve(cairo);}

   void CairoShimCairo::setLineWidth(double w)
  {cairo_set_line_width(cairo, w);}

  // sources
   void CairoShimCairo::setSourceRGB
  (double r, double g, double b)
  {cairo_set_source_rgb(cairo,r,g,b);}

   void CairoShimCairo::setSourceRGBA
  (double r, double g, double b, double a)
  {cairo_set_source_rgba(cairo,r,g,b,a);}

  // text. Argument is in UTF8 encoding
   void CairoShimCairo::showText(const std::string& text)
  {cairo_show_text(cairo,text.c_str());}

   void CairoShimCairo::setTextExtents(const std::string& text)
  {cairo_text_extents(cairo,text.c_str(),&extents);}

   double CairoShimCairo::textWidth() const
  {return extents.width;}

   double CairoShimCairo::textHeight() const
  {return extents.height;}

  // matrix transformation
   void CairoShimCairo::identityMatrix()
  {cairo_identity_matrix(cairo);}
  
   void CairoShimCairo::translate(double x, double y)
  {cairo_translate(cairo,x,y);}

   void CairoShimCairo::scale(double sx, double sy)
  {cairo_scale(cairo,sx,sy);}

   void CairoShimCairo::rotate(double angle)
  {cairo_rotate(cairo,angle);}

    // context manipulation
   void CairoShimCairo::save()
  {cairo_save(cairo);}

   void CairoShimCairo::restore()
  {cairo_restore(cairo);}


}
