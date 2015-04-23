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

struct VariableType
{
  enum Type {undefined, constant, parameter, flow, integral, stock, tempFlow, numVarTypes};
  static std::string typeName(int t);
};

inline std::ostream& operator<<(std::ostream& o, VariableType::Type t)
{return o<<VariableType::typeName(t);}

#include "variableType.cd"
#include "variableType.xcd"
#endif
