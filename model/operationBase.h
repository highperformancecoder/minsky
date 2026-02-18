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
#ifndef OPERATIONBASE_H
#define OPERATIONBASE_H

#include "classdesc_access.h"

#include "item.h"
#include "slider.h"

#include <vector>
#include <cairo/cairo.h>

#include <arrays.h>

#include "polyBase.h"
#include "polyPackBase.h"
#include <pack_base.h>
#include "operationType.h"

namespace ecolab {class Pango;}

namespace minsky
{
  class OperationPtr;
  class Group;

  class OperationBase: virtual public classdesc::PolyPackBase,
                       public BottomRightResizerItem, public OperationType
  {
    CLASSDESC_ACCESS(OperationBase);
  protected:
    mutable classdesc::Exclude<std::shared_ptr<ecolab::Pango>> cachedPango;
    /// check if cachedPango is up to date, and if not recreate
    void setCachedText(cairo_t*, const std::string&, double) const;
  public:
    static constexpr float l=-8, h=12, r=12;
    typedef OperationType::Type Type;

    virtual std::size_t numPorts() const=0;
    ///factory method. 
    static OperationBase* create(Type type); 
    virtual Type type() const=0;

    const OperationBase* operationCast() const override {return this;}
    OperationBase* operationCast() override {return this;}

    /// visual representation of operation on the canvas
    virtual void iconDraw(cairo_t *) const=0;

    /// returns a list of values the ports currently have
    std::string portValues() const;

    // returns true if multiple input wires are allowed.
    bool multiWire() const;

    // manage the port structures associated with this operation
    virtual void addPorts();

    void drawUserFunction(cairo_t* cairo) const;
    
    void draw(cairo_t*) const override;
    void draw(ICairoShim&) const override;
    void resize(const LassoBox& b) override;
    float scaleFactor() const override;       

    /// current value of output port
    double value() const override;

    /// operation argument. For example, the offset used in a
    /// difference operator, or binsize in a binning op
    double arg=1;

    /// axis selector in tensor operations
    std::string axis;

    /// return dimension names of tensor object attached to input
    /// if binary op, then the union of dimension names is returned
    std::vector<std::string> dimensions() const;
    Units units(bool check=false) const override;
    
  protected:

    friend struct EvalOpBase;
    friend struct SchemaHelper;
  private:
    Units unitsBinOpCase(bool check) const;
  };

  
  /// shared_ptr class for polymorphic operation objects. Note, you
  /// may assume that this pointer is always valid, although currently
  /// the implementation doesn't guarantee it (eg reset() is exposed).
  class OperationPtr: public std::shared_ptr<OperationBase>
  {
  public:
    typedef std::shared_ptr<OperationBase> PtrBase;
    OperationPtr(OperationType::Type type=OperationType::numOps): 
      PtrBase(OperationBase::create(type)) {}
    // reset pointer to a newly created operation
    OperationPtr(OperationBase* op): PtrBase(op) {assert(op);}
    OperationPtr clone() const {return OperationPtr(ItemPtr(get()->clone()));}
    std::size_t use_count() const {return  classdesc::shared_ptr<OperationBase>::use_count();}
    OperationPtr(const PtrBase& x): PtrBase(x) {}
    OperationPtr& operator=(const PtrBase& x) {PtrBase::operator=(x); return *this;}
    OperationPtr(const ItemPtr& x): 
      PtrBase(std::dynamic_pointer_cast<OperationBase>(x)) {}
  };
}

  // for TCL interfacing
inline std::ostream& operator<<(std::ostream& x, const std::vector<int>& y)
{
  for (std::size_t i=0; i<y.size(); ++i)
    x<<(i==0?"":" ")<<y[i];
  return x;
}


#include "operationBase.cd"
#include "operationBase.xcd"
#endif
