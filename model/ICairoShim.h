/*
  Abstract interface for Cairo operations to enable testing and mocking
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
#ifndef ICAIROSHIM_H
#define ICAIROSHIM_H

#include <string>
#include <cairo/cairo.h>

namespace minsky
{
  /// Abstract interface for Cairo drawing operations
  class ICairoShim
  {
  public:
    virtual ~ICairoShim() = default;

    // Drawing operations
    virtual void moveTo(double x, double y) = 0;
    virtual void lineTo(double x, double y) = 0;
    virtual void relMoveTo(double x, double y) = 0;
    virtual void relLineTo(double x, double y) = 0;
    virtual void arc(double x, double y, double radius, double start, double end) = 0;
    virtual void curveTo(double x1, double y1, double x2, double y2, double x3, double y3) = 0;
    virtual void rectangle(double x, double y, double width, double height) = 0;

    // Path operations
    virtual void newPath() = 0;
    virtual void newSubPath() = 0;
    virtual void closePath() = 0;
    virtual void getCurrentPoint(double& x, double& y) = 0;

    // Fill and stroke operations
    virtual void fill() = 0;
    virtual void fillPreserve() = 0;
    virtual void stroke() = 0;
    virtual void strokePreserve() = 0;
    virtual void clip() = 0;
    virtual void resetClip() = 0;
    virtual void paint() = 0;

    // Line properties
    virtual void setLineWidth(double width) = 0;
    virtual double getLineWidth() = 0;
    virtual void setDash(const double* dashes, int num_dashes, double offset) = 0;
    virtual void setFillRule(cairo_fill_rule_t fill_rule) = 0;

    // Color operations
    virtual void setSourceRGB(double r, double g, double b) = 0;
    virtual void setSourceRGBA(double r, double g, double b, double a) = 0;

    // Text operations
    virtual void showText(const std::string& text) = 0;
    virtual void setFontSize(double size) = 0;
    virtual void textExtents(const std::string& text, cairo_text_extents_t& extents) = 0;

    // Transformation operations
    virtual void identityMatrix() = 0;
    virtual void translate(double x, double y) = 0;
    virtual void scale(double sx, double sy) = 0;
    virtual void rotate(double angle) = 0;
    virtual void userToDevice(double& x, double& y) = 0;

    // Context state operations
    virtual void save() = 0;
    virtual void restore() = 0;

    // Tolerance
    virtual void setTolerance(double tolerance) = 0;

    // Path query
    virtual cairo_path_t* copyPathFlat() = 0;
    virtual void pathDestroy(cairo_path_t* path) = 0;

    // Access to underlying cairo_t* for legacy code that needs it (e.g., Pango)
    virtual cairo_t* cairoContext() = 0;
  };
}

#endif // ICAIROSHIM_H
