/*
  @copyright Steve Keen 2020
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

#include "expressionWalker.h"
#include "userFunction.h"
#include "minsky.h"
#include "minsky_epilogue.h"
using namespace std;
namespace minsky
{
  namespace {
    exprtk::parser<UnitsExpressionWalker> unitsParser;
  }

  Units UserFunction::units(bool check) const
  {
    unitsParser.enable_unknown_symbol_resolver();
    UnitsExpressionWalker x,y;
    x.units=ports[1]->units(check); x.check=check;
    y.units=ports[2]->units(check); y.check=check;

    exprtk::symbol_table<UnitsExpressionWalker> symbolTable, unknownVariables;
    exprtk::expression<UnitsExpressionWalker> compiled;
    compiled.register_symbol_table(unknownVariables);
    compiled.register_symbol_table(symbolTable);
    symbolTable.add_variable("x",x);
    symbolTable.add_variable("y",y);

    // do an initial parse to pick up references to external variables
    unitsParser.compile(expression, compiled);
    std::vector<std::string> externalVariables;
    unknownVariables.get_variable_list(externalVariables);

    unknownVariables.clear();
    vector<UnitsExpressionWalker> externalUnits;
    externalUnits.reserve(externalVariables.size());
    for (auto& i: externalVariables)
      {
        auto v=minsky().variableValues.find(VariableValue::valueIdFromScope(group.lock(),i));
        if (v!=minsky().variableValues.end())
          {
            externalUnits.emplace_back();
            externalUnits.back().units=v->second->units;
            externalUnits.back().check=check;
            unknownVariables.add_variable(i, externalUnits.back());
          }
      }

    unitsParser.compile(expression, compiled);
    try
      {
        return compiled.value().units;
      }
    catch (const std::exception& ex)
      {
        throw_error(ex.what());
      }
  }
}
