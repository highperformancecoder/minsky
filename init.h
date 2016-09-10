/*
  @copyright Steve Keen 2012
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
// export a reference to a table of initialisers 

// It seems that particularly on a Mac, Tcl/Tk library calls cannot be
// made before main() is called

#include <vector>

namespace minsky
{
  typedef int (*Fun)();
  inline std::vector<Fun>& initVec() 
  {
    static std::vector<Fun> v;
    return v;
  }
}
