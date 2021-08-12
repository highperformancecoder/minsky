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

// broken into several .o files to get around XCOFF format limitations with the MXE build

#include "itemRS.h"
#include "item.rcd"

#include "lasso.h"
#include "lasso.rcd"
#include "noteBase.rcd"
#include "operation.h"
#include "operation.rcd"
#include "operationType.rcd"
#include "port.rcd"
#include "polyRESTProcessBase.rcd"
#include "polyRESTProcessBase.xcd"
#include "wire.h"
#include "wire.rcd"
#include "units.rcd"
#include "nobble.h"

#include "itemTemplateInstantiations.h"
#include "minsky_epilogue.h"

namespace minsky
{
  using classdesc::RESTProcess_t;
  using classdesc::json_pack_t;
  using std::string;

  void Item::RESTProcess(RESTProcess_t& rp, const string& d)
  {
    ::RESTProcess(rp,d,*this);
  }

  void Item::RESTProcess(RESTProcess_t& rp, const string& d) const
  {
    ::RESTProcess(rp,d,*this);
  }

  void Item::json_pack(json_pack_t& j) const
  {
    ::json_pack(j,"",*this);
  }

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
}
