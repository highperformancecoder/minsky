/*
  @copyright Steve Keen 2019
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

/**
  @file
  Boilerplate code for instantiating template for all subclasses of Item.
 */

#ifndef ITEMTEMPLATEINSTANTIATION_H
#define ITEMTEMPLATEINSTANTIATION_H
#include "operation.h"
#include "ravelWrap.h"
#include "sheet.h"
#include "switchIcon.h"
#include "variable.h"

#define DEF(type, base)                                                 \
  template void ItemT<type,base>::restProcess(RESTProcess_t&, const string&); \
  template void ItemT<type,base>::restProcess(RESTProcess_t&, const string&) const; \
  template void ItemT<type,base>::json_pack(json_pack_t&) const; 
#define DEFOP(type) DEF(Operation<OperationType::type>, OperationBase)
#define DEFVAR(type) DEF(Variable<VariableType::type>, VariableBase)

namespace minsky
{
  DEFOP(constant)
  DEFOP(time)
  DEFOP(integrate)
  DEFOP(differentiate)
  DEFOP(data)
  DEFOP(ravel)
  DEFOP(euler)
  DEFOP(pi)
  DEFOP(zero)
  DEFOP(one)
  DEFOP(inf)
  DEFOP(add)
  DEFOP(subtract)
  DEFOP(multiply)
  DEFOP(divide)
  DEFOP(min)
  DEFOP(max)
  DEFOP(and_)
  DEFOP(or_)
  DEFOP(log)
  DEFOP(pow)
  DEFOP(polygamma)
  DEFOP(lt)
  DEFOP(le)
  DEFOP(eq)
  DEFOP(copy)
  DEFOP(sqrt)
  DEFOP(exp)
  DEFOP(ln)
  DEFOP(sin)
  DEFOP(cos)
  DEFOP(tan)
  DEFOP(asin)
  DEFOP(acos)
  DEFOP(atan)
  DEFOP(sinh)
  DEFOP(cosh)
  DEFOP(tanh)
  DEFOP(abs)
  DEFOP(floor)
  DEFOP(frac)
  DEFOP(not_)
  DEFOP(percent)
  DEFOP(gamma)
  DEFOP(fact)
  DEFOP(sum)
  DEFOP(product)
  DEFOP(infimum)
  DEFOP(supremum)
  DEFOP(any)
  DEFOP(all)
  DEFOP(infIndex)
  DEFOP(supIndex)
  DEFOP(runningSum)
  DEFOP(runningProduct)
  DEFOP(difference)
  DEFOP(innerProduct)
  DEFOP(outerProduct)
  DEFOP(index)
  DEFOP(gather)
  DEFOP(numOps)

  DEF(DataOp, Operation<OperationType::data>)
  DEF(IntOp, Operation<OperationType::integrate>)
  DEF(Ravel, Operation<OperationType::ravel>)

  DEFVAR(undefined)
  DEFVAR(constant)
  DEFVAR(parameter)
  DEFVAR(flow)
  DEFVAR(integral)
  DEFVAR(stock)
  DEFVAR(tempFlow)
  
  DEF(GodleyIcon,Item)
  DEF(Group,Item)
  DEF(PlotWidget,Item)
  DEF(Sheet,Item)
  DEF(SwitchIcon,BottomRightResizerItem)
}
#endif

