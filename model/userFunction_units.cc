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

#ifndef NO_EXPRTK
#include "minsky.h"
#include "expressionWalker.h"
#include "userFunction.h"
#include "minsky_epilogue.h"
using namespace std;
namespace minsky
{
  namespace {
    exprtk::parser<UnitsExpressionWalker> unitsParser;

    struct ExprTkCallableFunction: public exprtk::ivararg_function<UnitsExpressionWalker>
    {
      std::weak_ptr<CallableFunction> f;
      ExprTkCallableFunction(const std::weak_ptr<CallableFunction>& f): f(f) {}
      UnitsExpressionWalker operator()(const std::vector<UnitsExpressionWalker>&) {
        // TODO (#1290) Actually call units checking on callable function
        return {};
      }
    };
  }

  UnitsExpressionWalker timeUnit;
  bool UnitsExpressionWalker::check=true;
  Units UserFunction::units(bool check) const
  {
    UnitsExpressionWalker::check=check;
    vector<UnitsExpressionWalker> args(argNames.size());
    if (!args.empty()) args[0].units=m_ports[1]->units(check);
    if (args.size()>1) args[1].units=m_ports[2]->units(check);
    
    timeUnit.units=Units(cminsky().timeUnit);
    
    vector<UnitsExpressionWalker> externalUnits;
    exprtk::symbol_table<UnitsExpressionWalker> symbolTable, unknownVariables;
    exprtk::expression<UnitsExpressionWalker> compiled;
    compiled.register_symbol_table(unknownVariables);
    compiled.register_symbol_table(symbolTable);
    for (size_t i=0; i<args.size(); ++i)
      {
        args[i].check=check;
        symbolTable.add_variable(argNames[i],args[i]);
      }

    std::vector<std::string> externalIds=symbolNames();
    vector<ExprTkCallableFunction> externalFunctions;
    
    externalUnits.reserve(externalIds.size());
    for (auto& i: externalIds)
      {
        if (find(argNames.begin(), argNames.end(), i)!=argNames.end()) continue; // skip arguments
        auto id=valueIdFromScope(group.lock(),canonicalName(i));
        auto v=minsky().variableValues.find(id);
        if (v!=minsky().variableValues.end())
          {
            externalUnits.emplace_back();
            externalUnits.back().units=v->second->units;
            externalUnits.back().check=check;
            unknownVariables.add_variable(i, externalUnits.back());
            continue;
          }
        auto f=minsky().userFunctions.find(id);
        if (f!=minsky().userFunctions.end())
          {
            externalFunctions.emplace_back(f->second);
            unknownVariables.add_function(i,externalFunctions.back());
          }
      }
    unknownVariables.add_variable("time",minsky::timeUnit);

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
#endif
