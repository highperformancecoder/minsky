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

  UnitsExpressionWalker timeUnit;
  bool UnitsExpressionWalker::check=true;
  
  exprtk::symbol_table<UnitsExpressionWalker>& UserFunction::globalUnitSymbols()
  {
    static exprtk::symbol_table<UnitsExpressionWalker> table;
    return table;
  }

  Units UserFunction::units(bool check) const
  {
    UnitsExpressionWalker::check=check;
    UnitsExpressionWalker x,y;
    x.units=ports[1]->units(check); x.check=check;
    y.units=ports[2]->units(check); y.check=check;

    timeUnit.units=Units(cminsky().timeUnit);
    
    vector<UnitsExpressionWalker> externalUnits;
    exprtk::symbol_table<UnitsExpressionWalker> symbolTable, unknownVariables;
    exprtk::expression<UnitsExpressionWalker> compiled;
    compiled.register_symbol_table(unknownVariables);
    compiled.register_symbol_table(globalUnitSymbols());
    compiled.register_symbol_table(symbolTable);
    symbolTable.add_variable("x",x);
    symbolTable.add_variable("y",y);

    std::vector<std::string> externalIds=const_cast<UserFunction*>(this)->externalSymbolNames();

    externalUnits.reserve(externalIds.size());
    for (auto& i: externalIds)
      {
        auto v=minsky().variableValues.find(VariableValue::valueIdFromScope(group.lock(),i));
        if (v!=minsky().variableValues.end())
          {
            externalUnits.emplace_back();
            externalUnits.back().units=v->second->units;
            externalUnits.back().check=check;
            unknownVariables.add_variable(i, externalUnits.back());
          }
        else
          if (check)
            throw_error("unknown variable: "+i);
          else
            return {};
      }

    try
      {
        unitsParser.compile(expression, compiled);
        return compiled.value().units;
      }
    catch (const std::exception& ex)
      {
        if (check)
          throw_error(ex.what());
        else
          return {};
      }
  }
}
