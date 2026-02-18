#ifndef CAIROSHIMCAIRO_H
#define CAIROSHIMCAIRO_H
#include "cairoShim.h"
#include <cairo.h>

namespace minsky
{
  class CairoShimCairo: public ICairoShim
  {
    cairo_t* cairo; 
    CairoShimCairo(const CairoShimCairo&)=delete;
    void operator=(const CairoShimCairo&)=delete;
  public:
    // template parameter G = cairo_t* or HDC
    CairoShim(cairo_t*);
    ~CairoShim();

    void moveTo(double x, double y);
    void lineTo(double x, double y);
    void relMoveTo(double x, double y);
    void relLineTo(double x, double y);
    void arc(double x, double y, double radius, double start, double end);

    void setLineWidth(double);

    // paths
    void newPath();
    void closePath();
    void fill();
    void clip();
    void stroke();
    void strokePreserve();

    // sources
    void setSourceRGB(double r, double g, double b);
    void setSourceRGBA(double r, double g, double b, double a);
    
    // text. Argument is in UTF8 encoding
    void showText(const std::string&);
    void setTextExtents(const std::string&);
    double textWidth() const;
    double textHeight() const;

    // matrix transformation
    void identityMatrix();
    void translate(double x, double y);
    void scale(double sx, double sy);
    void rotate(double angle); ///< angle in radians

    // context manipulation
    void save();
    void restore();
    
  };

}
#endif
