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

#include "dataOp.h"
#include "intOp.h"
#include "item.h"
#include "lasso.h"
#include "lock.h"
#include "operation.h"
#include "ravelWrap.h"
#include "selection.h"
#include "userFunction.h"
#include "sheet.h"
#include "switchIcon.h"
#include "variable.h"

#include "minsky_epilogue.h"

namespace classdesc
{
  class RESTProcess_t;
  class json_pack_t;
}

namespace minsky
{
  /// dummy template definition for use with gui executable
  template <class T, class B>
  void ItemT<T,B>::RESTProcess(classdesc::RESTProcess_t& rp, const std::string& d)
  {}
  template <class T, class B>
  void ItemT<T,B>::RESTProcess(classdesc::RESTProcess_t& rp, const std::string& d) const
  {}
  template <class T, class B>
  void ItemT<T,B>::json_pack(classdesc::json_pack_t& j) const
  {}

  /// dummy definition for use with gui executable
  void Item::RESTProcess(classdesc::RESTProcess_t& rp, const std::string& d)
  {}
  void Item::RESTProcess(classdesc::RESTProcess_t& rp, const std::string& d) const
  {}
  void Item::json_pack(classdesc::json_pack_t& rp) const
  {}
}

#include "itemTemplateInstantiations.h"

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
  DEFOP(percent)
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
  DEFOP(userFunction)
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
  DEFOP(Gamma)
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
  DEFOP(meld)
  DEFOP(merge)
  DEFOP(numOps)

  DEFVAR(undefined)
  DEFVAR(constant)
  DEFVAR(parameter)
  DEFVAR(flow)
  DEFVAR(integral)
  DEFVAR(stock)
  DEFVAR(tempFlow)
  
  DEF(GodleyIcon,Item)
  DEF(Group,Item)
  DEF(Lock,Item)
  DEF(PlotWidget,Item)
  DEF(Sheet,Item)
  DEF(SwitchIcon,BottomRightResizerItem)

  DEF(DataOp, Operation<OperationType::data>)
  DEF(IntOp, Operation<OperationType::integrate>)
  DEF(Ravel, Operation<OperationType::ravel>)
  DEF(UserFunction, Operation<OperationType::userFunction>)
}
