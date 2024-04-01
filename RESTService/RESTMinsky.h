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

#ifndef RESTMINSKY_H
#define RESTMINSKY_H

#include "minsky.h"
#include "RESTProcess_base.h"
namespace minsky
{
  struct RESTMinsky: public Minsky
  {
    RESTProcess_t registry;
    CmdData getCommandData(const std::string&) const override;
    template <class RP>
    void commandHook(const std::string& command, const RP& args) {
        int nargs=1;
        switch (args.type())
          {
          case RESTProcessType::array:
            nargs=args.array().size();
            break;
          case RESTProcessType::null:
            nargs=0;
            break;
          default:
            break;
          }
        Minsky::commandHook(command,nargs);
    }
  };
}

#endif
