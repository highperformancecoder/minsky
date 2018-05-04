/*
  @copyright Steve Keen 2013
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

#ifndef VARIABLETYPE_H
#define VARIABLETYPE_H
#include <string>
#include <ostream>
#include <map>

namespace minsky
{
  struct VariableType
  {
    enum Type {undefined, constant, parameter, flow, integral, stock, tempFlow, numVarTypes};
    static std::string typeName(int t);
  };
  inline std::ostream& operator<<(std::ostream& o, VariableType::Type t)
  {return o<<VariableType::typeName(t);}

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
  };

  inline std::ostream& operator<<(std::ostream& o, const Units& u)
  {
    bool first=true;
    for (auto& i: u)
      {
        if (!first) o<<" ";
        first=false;
        o<<i.first;
        if (i.second!=1)
          o<<"^"<<i.second;
      }
    return o;
  }
  

}


#include "variableType.cd"
#include "variableType.xcd"
#endif
