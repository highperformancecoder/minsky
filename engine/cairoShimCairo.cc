#include "cairoShimCairo.h"
#include "minsky_epilogue.h"
#define CAIRO_WIN32_STATIC_BUILD
#include <cairo.h>
#undef CAIRO_WIN32_STATIC_BUILD
#include <pango.h>
#include <librsvg/rsvg.h>

// if not #ifdef protected, you get a deprecated warning, which is
// made fatal by -Werror
#ifndef RSVG_CAIRO_H
#include <librsvg/rsvg-cairo.h>
#endif

using namespace std;

namespace minsky
{
  CairoShimCairo::CairoShimCairo(cairo_t* c) : cairo(c) {}
  
  CairoShimCairo::~CairoShimCairo() = default;

  // Drawing operations
  void CairoShimCairo::moveTo(double x, double y) const
  {cairo_move_to(cairo,x,y);}

  void CairoShimCairo::lineTo(double x, double y) const
  {cairo_line_to(cairo,x,y);}

  void CairoShimCairo::relMoveTo(double x, double y) const
  {cairo_rel_move_to(cairo,x,y);}

  void CairoShimCairo::relLineTo(double x, double y) const
  {cairo_rel_line_to(cairo,x,y);}

  void CairoShimCairo::arc(double x, double y, double radius, double start, double end) const
  {cairo_arc(cairo,x,y,radius,start,end);}

  void CairoShimCairo::curveTo(double x1, double y1, double x2, double y2, double x3, double y3) const
  {cairo_curve_to(cairo,x1,y1,x2,y2,x3,y3);}

  void CairoShimCairo::rectangle(double x, double y, double width, double height) const
  {cairo_rectangle(cairo,x,y,width,height);}

  // Path operations
  void CairoShimCairo::newPath() const
  {cairo_new_path(cairo);}

  void CairoShimCairo::newSubPath() const
  {cairo_new_sub_path(cairo);}

  void CairoShimCairo::closePath() const
  {cairo_close_path(cairo);}

  void CairoShimCairo::getCurrentPoint(double& x, double& y) const
  {cairo_get_current_point(cairo, &x, &y);}

  // Fill and stroke operations
  void CairoShimCairo::fill() const
  {cairo_fill(cairo);}

  void CairoShimCairo::fillPreserve() const
  {cairo_fill_preserve(cairo);}
  
  void CairoShimCairo::clip() const
  {cairo_clip(cairo);}

  void CairoShimCairo::resetClip() const
  {cairo_reset_clip(cairo);}

  void CairoShimCairo::stroke() const
  {cairo_stroke(cairo);}
  
  void CairoShimCairo::strokePreserve() const
  {cairo_stroke_preserve(cairo);}

  void CairoShimCairo::paint() const
  {cairo_paint(cairo);}

  // Line properties
  void CairoShimCairo::setLineWidth(double w) const
  {cairo_set_line_width(cairo, w);}

  double CairoShimCairo::getLineWidth() const
  {return cairo_get_line_width(cairo);}

  void CairoShimCairo::setDash(const double* dashes, int num_dashes, double offset) const
  {cairo_set_dash(cairo, dashes, num_dashes, offset);}

  void CairoShimCairo::setFillRule(cairo_fill_rule_t fill_rule) const
  {cairo_set_fill_rule(cairo, fill_rule);}

  // Color operations
  void CairoShimCairo::setSourceRGB(double r, double g, double b) const
  {cairo_set_source_rgb(cairo,r,g,b);}

  void CairoShimCairo::setSourceRGBA(double r, double g, double b, double a) const
  {cairo_set_source_rgba(cairo,r,g,b,a);}

  // Text operations
  void CairoShimCairo::showText(const std::string& text) const
  {cairo_show_text(cairo,text.c_str());}

  void CairoShimCairo::setFontSize(double size) const
  {cairo_set_font_size(cairo, size);}

  void CairoShimCairo::selectFontFace(const std::string& family, cairo_font_slant_t slant, cairo_font_weight_t weight) const
  {cairo_select_font_face(cairo, family.c_str(), slant, weight);}

  void CairoShimCairo::textExtents(const std::string& text, cairo_text_extents_t& extents) const
  {cairo_text_extents(cairo,text.c_str(),&extents);}

  // Transformation operations
  void CairoShimCairo::identityMatrix() const
  {cairo_identity_matrix(cairo);}
  
  void CairoShimCairo::translate(double x, double y) const
  {cairo_translate(cairo,x,y);}

  void CairoShimCairo::scale(double sx, double sy) const
  {cairo_scale(cairo,sx,sy);}

  void CairoShimCairo::rotate(double angle) const
  {cairo_rotate(cairo,angle);}

  void CairoShimCairo::userToDevice(double& x, double& y) const
  {cairo_user_to_device(cairo, &x, &y);}

  // Context state operations
  void CairoShimCairo::save() const
  {cairo_save(cairo);}

  void CairoShimCairo::restore() const
  {cairo_restore(cairo);}

  // Tolerance
  void CairoShimCairo::setTolerance(double tolerance) const
  {cairo_set_tolerance(cairo, tolerance);}

  // Pango support
  ecolab::Pango& CairoShimCairo::pango() const
  {
    if (!m_pango) newPango();
    return *m_pango;
  }
  ecolab::Pango& CairoShimCairo::newPango() const
  {
    m_pango.reset(new ecolab::Pango(cairo));
    return *m_pango;
  }

  // SVG rendering support
  void CairoShimCairo::renderSVG(const SVGRenderer& svgRenderer, double width, double height) const
  {
#ifdef MXE // MXE doesn't currently have a Rust compiler, so librsvg can be no later than 2.40.21
    RsvgDimensionData dims;
    rsvg_handle_get_dimensions(svgRenderer.svg, &dims);
    cairo_scale(cairo, width/dims.width, height/dims.height);
    rsvg_handle_render_cairo(svgRenderer.svg, cairo);
#else
    GError* err=nullptr;
    const RsvgRectangle rect{0,0,width,height};
    rsvg_handle_render_document(svgRenderer.svg, cairo, &rect, &err);
    if (err)
      g_error_free(err);
#endif
  }
}
