/*
  @copyright Steve Keen 2018
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

#ifndef RAVELSTATE_SCHEMA2_H
#define RAVELSTATE_SCHEMA2_H
#include <map>
#include <string>
#include <vector>
#include "ravelState.h"

namespace schema2
{
  struct RavelState: public ravel::RavelState
  {
    // schema3 ravel state differs by how handle states are stored
    std::map<std::string, ravel::HandleState> handleStates;
    RavelState() {}
    ravel::RavelState toRavelRavelState() const {
      ravel::RavelState r(*this);
      r.handleStates.clear();
      for (auto& i: handleStates)
        {
          r.handleStates.push_back(i.second);
          r.handleStates.back().description=i.first;
        }
      return r;
    }
    RavelState(const ravel::RavelState& x): ravel::RavelState(x) {
      for (auto& i: x.handleStates)
        handleStates[i.description]=i;
    }
  };

}
#include "ravelState.xcd"
#include "ravelState-schema2.cd"
#include "ravelState-schema2.xcd"
#endif
