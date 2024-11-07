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

#include "RESTMinsky.h"
#include "cairoItems.h"
#include "RESTProcess_base.h"
#include "signature.h"
#include "signature.xcd"
#include "minsky_epilogue.h"
using namespace std;

namespace minsky
{
  Minsky::CmdData RESTMinsky::getCommandData(const string& dottedCommand) const 
  {
    switch (registry.count(dottedCommand))
      {
      case 0:
        return no_command;
      case 1:
        {
          auto& cmd=*registry.find(dottedCommand)->second;
          return cmd.isConst()? is_const: cmd.isObject()? is_setterGetter: generic;
        }
      case 2:
        {
          auto firstCmd=registry.find(dottedCommand), secondCmd=firstCmd;
          ++secondCmd;
          if (firstCmd->second->arity()+secondCmd->second->arity() == 1)
            return is_setterGetter;
          return generic;
        }
      default:
        return generic;
      }
    
  }
}
