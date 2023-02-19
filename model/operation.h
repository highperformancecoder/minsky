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

#include <ecolab.h>
#include <xml_pack_base.h>
#include <xml_unpack_base.h>

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
  class NamedOp: public ecolab::TCLAccessor<NamedOp,std::string>
  {
  protected:
    std::string m_description;
    virtual void updateBB()=0;
    CLASSDESC_ACCESS(NamedOp);
  public:
    NamedOp(): ecolab::TCLAccessor<NamedOp,std::string>
      ("description",(ecolab::TCLAccessor<NamedOp,std::string>::Getter)&NamedOp::description,
       (ecolab::TCLAccessor<NamedOp,std::string>::Setter)&NamedOp::description)
    {}
    /// @{ name of the associated data operation
    virtual std::string description() const;  
    virtual std::string description(const std::string&);    
    /// @}

  };
}

#include "operation.cd"
#include "operation.xcd"
#endif
