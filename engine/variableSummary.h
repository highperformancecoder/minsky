/*
  @copyright Steve Keen 2023
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

#ifndef VARIABLE_SUMMARY_H
#define VARIABLE_SUMMARY_H
#include "variableType.h"
#include <string>
#include <vector>
namespace minsky
{
  /// summary for the variable tab (aka summary tab).
  struct Summary
  {
    std::string valueId;
    std::string name; ///< LaTeXable name
    VariableType::Type type;
    std::string definition; ///< LaTeXable definition 
    std::string udfDefinition; ///< use function compatible definition
    std::string init; ///< initial value
    double sliderStep, sliderMin, sliderMax; ///< slider parameters
    double value=nan(""); ///< value, if scalar
    std::string scope; ///< name, id of scope if local, ":" if global
    std::string godley; ///< name or id of Godley table, if contained in a Godley table
    std::vector<unsigned> dimensions; ///< dimensions (empty if scalar
    std::string units;
  };

}

#include "variableSummary.cd"
#include "variableSummary.xcd"
#include "variableSummary.rcd"
#endif
