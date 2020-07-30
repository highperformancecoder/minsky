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

#ifndef RAVELSTATE_H
#define RAVELSTATE_H
#include <map>
#include <string>
#include <vector>

namespace minsky
{
  static const double ravelDefaultRadius=100; ///< initial size of a Ravel widget

  struct RavelState
  {
   // representing the state of the handles
    struct HandleState
    {
      double x,y; ///< handle tip coordinates (only angle important, not length)
      bool collapsed=false, displayFilterCaliper=false;
      enum ReductionOp {sum, prod, av, stddev, min, max};
      ReductionOp reductionOp=sum;
      enum HandleSort {none, forward, reverse, custom};
      HandleSort order=none;
      // note this member must appear after all members of
      // CAPIHandleState from the Ravel CAPI
      std::vector<std::string> customOrder; // used if order==custom
      std::string minLabel, maxLabel, sliceLabel;
    };

    double radius=ravelDefaultRadius;
    /// sort 1D ravel by value. Ignored for any other rank.
    HandleState::HandleSort sortByValue=HandleState::none;
    std::map<std::string, HandleState> handleStates;
    std::vector<std::string> outputHandles;
    bool empty() const {return handleStates.empty();}
    void clear() {handleStates.clear(); outputHandles.clear();}
  };

}
#include "ravelState.cd"
#include "ravelState.xcd"

#endif
