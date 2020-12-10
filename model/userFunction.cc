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
#include "evalOp.h"
#include "selection.h"
#include "minsky_epilogue.h"

namespace minsky
{
  int UserFunction::nextId=0;

  namespace {
    exprtk::parser<double> parser;
    exprtk::parser<UnitsExpressionWalker> unitsParser;
  }
  
  template <> void Operation<OperationType::userFunction>::iconDraw(cairo_t*) const
  {assert(false);}
  
  void UserFunction::compile()
  {
    symbolTable.add_variable("x",x);
    symbolTable.add_variable("y",y);
    compiledExpression.register_symbol_table(symbolTable);
    // TODO bind any other external references to the variableValues table
    exprtk::parser<double> parser;
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

  Units UserFunction::units(bool check) const
  {
    UnitsExpressionWalker x,y;
    x.units=ports[1]->units(check); x.check=check;
    y.units=ports[2]->units(check); y.check=check;

    exprtk::symbol_table<UnitsExpressionWalker> symbolTable;
    symbolTable.add_variable("x",x);
    symbolTable.add_variable("y",y);
    exprtk::expression<UnitsExpressionWalker> compiled;
    compiled.register_symbol_table(symbolTable);
    unitsParser.compile(expression, compiled);
    return compiled.value().units;
  }

  
}

