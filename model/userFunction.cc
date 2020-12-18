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

#include "userFunction.h"
#include "evalOp.h"
#include "selection.h"
#include "minsky.h"
#include "minsky_epilogue.h"

namespace minsky
{
  int UserFunction::nextId=0;

  namespace {
    exprtk::parser<double> parser;
  }
  
  template <> void Operation<OperationType::userFunction>::iconDraw(cairo_t*) const
  {assert(false);}

  UserFunction::UserFunction() {
    description("uf"+std::to_string(nextId++)+"(x,y)");
    localSymbols.add_variable("x",x);
    localSymbols.add_variable("y",y);
    compiledExpression.register_symbol_table(externalSymbols);
    compiledExpression.register_symbol_table(localSymbols);
  }

  
  void UserFunction::compile()
  {
    // do an initial parse to pick up references to external variables
    externalSymbols.clear();
    parser.compile(expression, compiledExpression);
    parser.disable_unknown_symbol_resolver();
    std::vector<std::string> externalVariables;
    externalSymbols.get_variable_list(externalVariables);

    // add them back in with their correct definitions
    externalSymbols.clear();
    for (auto& i: externalVariables)
      {
        auto v=minsky().variableValues.find(VariableValue::valueIdFromScope(group.lock(),i));
        if (v!=minsky().variableValues.end())
          externalSymbols.add_variable(i, (*v->second)[0]);
      }
    
      // TODO bind any other external references to the variableValues table
    if (!parser.compile(expression, compiledExpression))
      {
        string errorInfo;
        for (size_t i=0; i<parser.error_count(); ++i)
          errorInfo+=parser.get_error(i).diagnostic+'\n';
        throw_error("Invalid function expression:\n"+errorInfo);
      }
  }
  
  double UserFunction::evaluate(double in1, double in2)
  {
    x=in1, y=in2;
    return compiledExpression.value();
  }


  
}

