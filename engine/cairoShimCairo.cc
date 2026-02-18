#include "cairoShimCairo.h"
#define CAIRO_WIN32_STATIC_BUILD
#include <cairo.h>
#undef CAIRO_WIN32_STATIC_BUILD

using namespace std;

namespace minsky
{
  CairoShimCairo::CairoShimCairo(cairo_t* c) : cairo(c) {}
  
  CairoShimCairo::~CairoShimCairo() = default;

  // Drawing operations
  void CairoShimCairo::moveTo(double x, double y) 
  {cairo_move_to(cairo,x,y);}

  void CairoShimCairo::lineTo(double x, double y)
  {cairo_line_to(cairo,x,y);}

  void CairoShimCairo::relMoveTo(double x, double y)
  {cairo_rel_move_to(cairo,x,y);}

  void CairoShimCairo::relLineTo(double x, double y)
  {cairo_rel_line_to(cairo,x,y);}

  void CairoShimCairo::arc(double x, double y, double radius, double start, double end)
  {cairo_arc(cairo,x,y,radius,start,end);}

  void CairoShimCairo::curveTo(double x1, double y1, double x2, double y2, double x3, double y3)
  {cairo_curve_to(cairo,x1,y1,x2,y2,x3,y3);}

  void CairoShimCairo::rectangle(double x, double y, double width, double height)
  {cairo_rectangle(cairo,x,y,width,height);}

  // Path operations
  void CairoShimCairo::newPath()
  {cairo_new_path(cairo);}

  void CairoShimCairo::newSubPath()
  {cairo_new_sub_path(cairo);}

  void CairoShimCairo::closePath()
  {cairo_close_path(cairo);}

  void CairoShimCairo::getCurrentPoint(double& x, double& y)
  {cairo_get_current_point(cairo, &x, &y);}

  // Fill and stroke operations
  void CairoShimCairo::fill()
  {cairo_fill(cairo);}

  void CairoShimCairo::fillPreserve()
  {cairo_fill_preserve(cairo);}
  
  void CairoShimCairo::clip()
  {cairo_clip(cairo);}

  void CairoShimCairo::resetClip()
  {cairo_reset_clip(cairo);}

  void CairoShimCairo::stroke()
  {cairo_stroke(cairo);}
  
  void CairoShimCairo::strokePreserve()
  {cairo_stroke_preserve(cairo);}

  void CairoShimCairo::paint()
  {cairo_paint(cairo);}

  // Line properties
  void CairoShimCairo::setLineWidth(double w)
  {cairo_set_line_width(cairo, w);}

  double CairoShimCairo::getLineWidth()
  {return cairo_get_line_width(cairo);}

  void CairoShimCairo::setDash(const double* dashes, int num_dashes, double offset)
  {cairo_set_dash(cairo, dashes, num_dashes, offset);}

  void CairoShimCairo::setFillRule(cairo_fill_rule_t fill_rule)
  {cairo_set_fill_rule(cairo, fill_rule);}

  // Color operations
  void CairoShimCairo::setSourceRGB(double r, double g, double b)
  {cairo_set_source_rgb(cairo,r,g,b);}

  void CairoShimCairo::setSourceRGBA(double r, double g, double b, double a)
  {cairo_set_source_rgba(cairo,r,g,b,a);}

  // Text operations
  void CairoShimCairo::showText(const std::string& text)
  {cairo_show_text(cairo,text.c_str());}

  void CairoShimCairo::setFontSize(double size)
  {cairo_set_font_size(cairo, size);}

  void CairoShimCairo::textExtents(const std::string& text, cairo_text_extents_t& extents)
  {cairo_text_extents(cairo,text.c_str(),&extents);}

  // Transformation operations
  void CairoShimCairo::identityMatrix()
  {cairo_identity_matrix(cairo);}
  
  void CairoShimCairo::translate(double x, double y)
  {cairo_translate(cairo,x,y);}

  void CairoShimCairo::scale(double sx, double sy)
  {cairo_scale(cairo,sx,sy);}

  void CairoShimCairo::rotate(double angle)
  {cairo_rotate(cairo,angle);}

  void CairoShimCairo::userToDevice(double& x, double& y)
  {cairo_user_to_device(cairo, &x, &y);}

  // Context state operations
  void CairoShimCairo::save()
  {cairo_save(cairo);}

  void CairoShimCairo::restore()
  {cairo_restore(cairo);}

  // Tolerance
  void CairoShimCairo::setTolerance(double tolerance)
  {cairo_set_tolerance(cairo, tolerance);}

  // Path query
  cairo_path_t* CairoShimCairo::copyPathFlat()
  {return cairo_copy_path_flat(cairo);}

  void CairoShimCairo::pathDestroy(cairo_path_t* path)
  {cairo_path_destroy(path);}

  // Access to underlying cairo_t*
  cairo_t* CairoShimCairo::cairoContext()
  {return cairo;}
}
