/*
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
#ifndef OPERATION_H
#define OPERATION_H

// override EcoLab's default CLASSDESC_ACCESS macro
#include "classdesc_access.h"

#include "item.h"
#include "slider.h"

#include <vector>
#include <cairo/cairo.h>

#include <arrays.h>

#include "polyBase.h"
#include "polyPackBase.h"
#include <pack_base.h>
#include "operationBase.h"
#include "itemT.h"

namespace minsky
{
  template <minsky::OperationType::Type T>
  class Operation: public ItemT<Operation<T>, OperationBase>,
                   public classdesc::PolyPack<Operation<T> >
  {
    typedef ItemT<Operation<T>, OperationBase> Super;
  public:
    typedef OperationType::Type Type;
    Type type() const override {return T;}
    void iconDraw(cairo_t *) const override;
    std::size_t numPorts() const override 
    {return OperationTypeInfo::numArguments<T>()+1;}
    Operation() {
      this->addPorts();
      // custom arg defaults
      switch (T)  {
        case OperationType::runningSum: case OperationType::runningProduct:
          this->arg=-1;
          break;
        default:
          break;
        }
    }
    Operation(const Operation& x): Super(x) {this->addPorts();}
    Operation(Operation&& x): Super(x) {this->addPorts();}
    Operation& operator=(const Operation& x) {
      Super::operator=(x);
      this->addPorts();
      return *this;
    }
    Operation& operator=(Operation&& x) {
      Super::operator=(x);
      this->addPorts();
      return *this;
    }
    std::string classType() const override {return "Operation:"+OperationType::typeName(T);}
  };

  class Time: public Operation<OperationType::time>
  {
  public:
    Units units(bool) const override;
  };
  
  class Derivative: public Operation<OperationType::differentiate>
  {
  public:
    Units units(bool) const override;
  };

  class Copy: public Operation<OperationType::copy>
  {
  public:
    Units units(bool check) const override {return m_ports[1]->units(check);}
  };

  /// base class for operations that have names
  class NamedOp
  {
  protected:
    std::string m_description;
    virtual void updateBB()=0;
    CLASSDESC_ACCESS(NamedOp);
  public:
    /// @{ name of the associated data operation
    virtual std::string description() const;  
    virtual std::string description(const std::string&);    
    /// @}

  };

  /// helper class to draw port label symbols
  struct DrawBinOp
  {
    cairo_t *cairo;
    double zoomFactor;
    DrawBinOp(cairo_t *cairo, double z=1): cairo(cairo), zoomFactor(z) {}

    void drawPlus() const
    {
      cairo_move_to(cairo,0,-5);
      cairo_line_to(cairo,0,5);
      cairo_move_to(cairo,-5,0);
      cairo_line_to(cairo,5,0);
      cairo_stroke(cairo);
    }

    void drawMinus() const
    {
      cairo_move_to(cairo,-5,0);
      cairo_line_to(cairo,5,0);
      cairo_stroke(cairo);
    }

    void drawMultiply() const
    {
      cairo_move_to(cairo,-5,-5);
      cairo_line_to(cairo,5,5);
      cairo_move_to(cairo,-5,5);
      cairo_line_to(cairo,5,-5);
      cairo_stroke(cairo);
    }

    void drawDivide() const
    {
      cairo_move_to(cairo,-5,0);
      cairo_line_to(cairo,5,0);
      cairo_new_sub_path(cairo);
      cairo_arc(cairo,0,3,1,0,2*M_PI);
      cairo_new_sub_path(cairo);
      cairo_arc(cairo,0,-3,1,0,2*M_PI);
      cairo_stroke(cairo);
    }

    void drawSymbol(const char* s) const
    {
      cairo_scale(cairo,zoomFactor,zoomFactor);
      cairo_move_to(cairo,-5,0);
      cairo_show_text(cairo,s);
    }
  
    // puts a small symbol to identify port
    // x, y = position of symbol
    template <class F>
    void drawPort(F f, float x, float y, float rotation)  const
    {
      const ecolab::cairo::CairoSave cs(cairo);
      
      const double angle=rotation * M_PI / 180.0;
      if (minsky::flipped(rotation))
        y=-y;
      cairo_rotate(cairo, angle);
      
      cairo_translate(cairo,0.7*x,0.6*y);
      cairo_scale(cairo,0.5,0.5);
      
      // and counter-rotate
      cairo_rotate(cairo, -angle);
      f();
    }
  };
}

#include "operation.cd"
#include "operation.xcd"
#endif
