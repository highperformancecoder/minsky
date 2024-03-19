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

#ifndef UNITS_H
#define UNITS_H
#include <string>
#include <ostream>
#include <map>

namespace minsky
{
  /// represents the units (in sense of dimensional analysis) of a
  /// variable.
  /**
  first argument is the base unit (eg metre, second), and the second
   is it's power (eg {{"m",1},{"s",-1}} => m/s)
  **/
  struct Units: public std::map<std::string,int>
  {
    Units() {}
    Units(const std::string&);
    std::string str() const;
    /// insert braces around exponents for LaTeX processing
    std::string latexStr() const;
    // remove entries that are unitary
    void normalise() {
      for (auto i=begin(); i!=end(); ) {
        auto j=i; ++i;
        if (j->second==0 || j->first.empty()) erase(j);
      }
    }
  
  };

  inline std::ostream& operator<<(std::ostream& o, const Units& u)
  {
    bool first=true;
    for (auto& i: u)
      {
        if (i.first.empty() || i.second==0) continue; // don't display empty units
        if (!first) o<<" ";
        first=false;
        o<<i.first;
        if (i.second!=1)
          o<<"^"<<i.second;
      }
    return o;
  }
  

}


#include "units.cd"
#include "units.xcd"
#include "units.rcd"
#endif
