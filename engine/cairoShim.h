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

namespace minsky
{
  class ICairoShim
  {
  public:
    volatile void moveTo(double x, double y)=0;
    volatile void lineTo(double x, double y)=0;
    volatile void relMoveTo(double x, double y)=0;
    volatile void relLineTo(double x, double y)=0;
    volatile void arc(double x, double y, double radius, double start, double end)=0;

    volatile void setLineWidth(double)=0;

    // paths
    volatile void newPath()=0;
    volatile void closePath()=0;
    volatile void fill()=0;
    volatile void clip()=0;
    volatile void stroke()=0;
    volatile void strokePreserve()=0;

    // sources
    volatile void setSourceRGB(double r, double g, double b)=0;
    volatile void setSourceRGBA(double r, double g, double b, double a)=0;
    
    // text. Argument is in UTF8 encoding
    volatile void showText(const std::string&)=0;
    volatile void setTextExtents(const std::string&)=0;
    volatile double textWidth() const=0;
    volatile double textHeight() const=0;

    // matrix transformation
    volatile void identityMatrix()=0;
    volatile void translate(double x, double y)=0;
    volatile void scale(double sx, double sy)=0;
    volatile void rotate(double angle)=0; ///< angle in radians

    // context manipulation
    volatile void save()=0;
    volatile void restore()=0;
    
  };

}
#endif
