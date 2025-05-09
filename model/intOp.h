/*
  @copyright Steve Keen 2021
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

#ifndef INTOP_H
#define INTOP_H
#include "operation.h"
#include "variable.h"

namespace minsky
{
  class IntOp;
  
  class IntOp: public ItemT<IntOp, Operation<minsky::OperationType::integrate>>
  {
    typedef ItemT<IntOp, Operation<minsky::OperationType::integrate>> Super;
    // integrals have named integration variables
    ///integration variable associated with this op.
    CLASSDESC_ACCESS(IntOp);
    friend struct SchemaHelper;
    bool m_coupled=true;
    void insertControlled(Selection& selection) override;
    void removeControlledItems(minsky::GroupItems&) override;
  public:
    // offset for coupled integration variable, tr
    static constexpr float intVarOffset=10;

    IntOp() {
      description("");
    }
    // ensure that copies create a new integral variable
    IntOp(const IntOp& x): Super(x) {intVar.reset(); description(x.description());}
    ~IntOp() {Item::removeControlledItems();}
    
    IntOp& operator=(const IntOp& x); 

    /// @{ name of the associated integral variable
    std::string description(const std::string& desc);
    std::string description() const {return intVar? intVar->name(): "";}
    /// @}

    std::weak_ptr<Port> ports(std::size_t i) const override {
      // if coupled, the output port is the intVar's output
      if (i==0 && coupled() && intVar) return intVar->ports(0);
      return Item::ports(i);
    }
      
    std::string valueId() const 
    {return intVar->valueId();}
    
    void draw(cairo_t*) const override;
    void resize(const LassoBox& b) override;  

    /// return reference to integration variable
    VariablePtr intVar; 

    bool onKeyPress(int keySym, const std::string& utf8, int state) override {
      if (intVar) return intVar->onKeyPress(keySym, utf8, state);
      return false;
    }

    /// toggles coupled state of integration variable. Only valid for integrate
    /// @return coupled state
    bool toggleCoupled();
    bool coupled() const {return m_coupled;}
    Units units(bool) const override;

    void pack(classdesc::pack_t& x, const std::string& d) const override;
    void unpack(classdesc::unpack_t& x, const std::string& d) override;

    using Item::removeControlledItems;
  };

}

#include "intOp.cd"
#include "intOp.xcd"
#endif
