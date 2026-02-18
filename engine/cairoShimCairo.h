#ifndef CAIROSHIMCAIRO_H
#define CAIROSHIMCAIRO_H
#include "../model/ICairoShim.h"
#include <cairo.h>

namespace minsky
{
  /// Concrete implementation of ICairoShim using actual Cairo library
  class CairoShimCairo: public ICairoShim
  {
    cairo_t* cairo; 
    CairoShimCairo(const CairoShimCairo&)=delete;
    void operator=(const CairoShimCairo&)=delete;
  public:
    CairoShimCairo(cairo_t* c);
    ~CairoShimCairo() override;

    // Drawing operations
    void moveTo(double x, double y) override;
    void lineTo(double x, double y) override;
    void relMoveTo(double x, double y) override;
    void relLineTo(double x, double y) override;
    void arc(double x, double y, double radius, double start, double end) override;
    void curveTo(double x1, double y1, double x2, double y2, double x3, double y3) override;
    void rectangle(double x, double y, double width, double height) override;

    // Path operations
    void newPath() override;
    void newSubPath() override;
    void closePath() override;
    void getCurrentPoint(double& x, double& y) override;

    // Fill and stroke operations
    void fill() override;
    void fillPreserve() override;
    void stroke() override;
    void strokePreserve() override;
    void clip() override;
    void resetClip() override;
    void paint() override;

    // Line properties
    void setLineWidth(double width) override;
    double getLineWidth() override;
    void setDash(const double* dashes, int num_dashes, double offset) override;
    void setFillRule(cairo_fill_rule_t fill_rule) override;

    // Color operations
    void setSourceRGB(double r, double g, double b) override;
    void setSourceRGBA(double r, double g, double b, double a) override;

    // Text operations
    void showText(const std::string& text) override;
    void setFontSize(double size) override;
    void textExtents(const std::string& text, cairo_text_extents_t& extents) override;

    // Transformation operations
    void identityMatrix() override;
    void translate(double x, double y) override;
    void scale(double sx, double sy) override;
    void rotate(double angle) override;
    void userToDevice(double& x, double& y) override;

    // Context state operations
    void save() override;
    void restore() override;

    // Tolerance
    void setTolerance(double tolerance) override;

    // Path query
    cairo_path_t* copyPathFlat() override;
    void pathDestroy(cairo_path_t* path) override;

    // Access to underlying cairo_t* for legacy code
    cairo_t* cairoContext() override;
  };

}
#endif
