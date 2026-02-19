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
#include <cairo.h>

namespace minsky
{
  // Forward declarations for Pango
  namespace ecolab { class Pango; }

  /// Abstract interface for Cairo drawing operations
  class ICairoShim
  {
  public:
    virtual ~ICairoShim() = default;

    // Drawing operations
    virtual void moveTo(double x, double y) const = 0;
    virtual void lineTo(double x, double y) const = 0;
    virtual void relMoveTo(double x, double y) const = 0;
    virtual void relLineTo(double x, double y) const = 0;
    virtual void arc(double x, double y, double radius, double start, double end) const = 0;
    virtual void curveTo(double x1, double y1, double x2, double y2, double x3, double y3) const = 0;
    virtual void rectangle(double x, double y, double width, double height) const = 0;

    // Path operations
    virtual void newPath() const = 0;
    virtual void newSubPath() const = 0;
    virtual void closePath() const = 0;
    virtual void getCurrentPoint(double& x, double& y) const = 0;

    // Fill and stroke operations
    virtual void fill() const = 0;
    virtual void fillPreserve() const = 0;
    virtual void stroke() const = 0;
    virtual void strokePreserve() const = 0;
    virtual void clip() const = 0;
    virtual void resetClip() const = 0;
    virtual void paint() const = 0;

    // Line properties
    virtual void setLineWidth(double width) const = 0;
    virtual double getLineWidth() const = 0;
    virtual void setDash(const double* dashes, int num_dashes, double offset) const = 0;
    virtual void setFillRule(cairo_fill_rule_t fill_rule) const = 0;

    // Color operations
    virtual void setSourceRGB(double r, double g, double b) const = 0;
    virtual void setSourceRGBA(double r, double g, double b, double a) const = 0;

    // Text operations
    virtual void showText(const std::string& text) const = 0;
    virtual void setFontSize(double size) const = 0;
    virtual void textExtents(const std::string& text, cairo_text_extents_t& extents) const = 0;

    // Transformation operations
    virtual void identityMatrix() const = 0;
    virtual void translate(double x, double y) const = 0;
    virtual void scale(double sx, double sy) const = 0;
    virtual void rotate(double angle) const = 0;
    virtual void userToDevice(double& x, double& y) const = 0;

    // Context state operations
    virtual void save() const = 0;
    virtual void restore() const = 0;

    // Tolerance
    virtual void setTolerance(double tolerance) const = 0;

    // Pango support for text rendering
    virtual ecolab::Pango& pango() const = 0;
  };
}

#endif // ICAIROSHIM_H
