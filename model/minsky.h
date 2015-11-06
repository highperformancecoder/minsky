/*
  @copyright Steve Keen 2015
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

#ifndef MINSKY_H
#define MINSKY_H

#include "group.h"

namespace minsky
{

  class Minsky: public Group
  {
    int nextId{0};        ///< next id to assign to an item
  public:
     /// returns next integral ID to allocate to an item
    inline int getNewId()
    {
      return nextId++;
    }
    /// resets the nextId counter to the next available
    void resetNextId();
   
  };
}

#endif
