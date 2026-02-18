/*
  Because Cairo built with Visual C++ appears to be unstable when used
  with the Win32 surface, we abstract the cairo interface here, and
  provide 2 implementations: a Cairo one, and one directly in GDI.

  Using static polymorphism.

  Pimpl pattern deployed to prevent leakage of platform specific details
*/
#ifndef CAIROSHIM_H
#define CAIROSHIM_H
#include <string>
#include <memory>

namespace ravel
{
  template <class G> class CairoShimImpl;

  template <class G>
  class CairoShim
  {
    CairoShimImpl<G>* impl; 
    CairoShim(const CairoShim&)=delete;
    void operator=(const CairoShim&)=delete;
  public:
    // template parameter G = cairo_t* or HDC
    CairoShim(G);
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
