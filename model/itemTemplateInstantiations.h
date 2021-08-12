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
//#include "operation.h"
//#include "ravelWrap.h"
//#include "lock.h"
//#include "userFunction.h"
//#include "sheet.h"
//#include "switchIcon.h"
//#include "variable.h"

#define DEF(type, base)                                                 \
  template void ItemT<type,base>::RESTProcess(classdesc::RESTProcess_t&, const std::string&); \
  template void ItemT<type,base>::RESTProcess(classdesc::RESTProcess_t&, const std::string&) const; \
  template void ItemT<type,base>::json_pack(classdesc::json_pack_t&) const; 
#define DEFOP(type) DEF(Operation<OperationType::type>, OperationBase)
#define DEFVAR(type) DEF(Variable<VariableType::type>, VariableBase)

#endif

