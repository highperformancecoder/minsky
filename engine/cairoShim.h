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
    virtual ~ICairoShim() = default;
    virtual void moveTo(double x, double y)=0;
    virtual void lineTo(double x, double y)=0;
    virtual void relMoveTo(double x, double y)=0;
    virtual void relLineTo(double x, double y)=0;
    virtual void arc(double x, double y, double radius, double start, double end)=0;

    virtual void setLineWidth(double)=0;

    // paths
    virtual void newPath()=0;
    virtual void closePath()=0;
    virtual void fill()=0;
    virtual void clip()=0;
    virtual void stroke()=0;
    virtual void strokePreserve()=0;

    // sources
    virtual void setSourceRGB(double r, double g, double b)=0;
    virtual void setSourceRGBA(double r, double g, double b, double a)=0;
    
    // text. Argument is in UTF8 encoding
    virtual void showText(const std::string&)=0;
    virtual void setTextExtents(const std::string&)=0;
    virtual double textWidth() const=0;
    virtual double textHeight() const=0;

    // matrix transformation
    virtual void identityMatrix()=0;
    virtual void translate(double x, double y)=0;
    virtual void scale(double sx, double sy)=0;
    virtual void rotate(double angle)=0; ///< angle in radians

    // context manipulation
    virtual void save()=0;
    virtual void restore()=0;
    
  };

}
#endif
