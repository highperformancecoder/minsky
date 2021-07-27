
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

#include <UnitTest++/UnitTest++.h>
#include "expressionWalker.h"
#include "exprtk/exprtk.hpp"
#include "minsky_epilogue.h"
using namespace minsky;
using namespace std;

SUITE(ExpressionWalker)
{
  void checkCompile(const std::string& expr, exprtk::expression<UnitsExpressionWalker>& expression)
  {
    exprtk::parser<UnitsExpressionWalker> parser;
    CHECK(parser.compile(expr,expression));
    for (size_t i=0; i<parser.error_count(); ++i)
      cout << parser.get_error(i).diagnostic<<endl;
  }
  
  TEST(UnitsExpressionWalker)
    {
      UnitsExpressionWalker metre("m"), second("s"), x;
      exprtk::symbol_table<UnitsExpressionWalker> symbolTable;
      
      symbolTable.add_variable("metre",metre);
      symbolTable.add_variable("second",second);
      symbolTable.add_variable("x",x);

      exprtk::expression<UnitsExpressionWalker> expression;
      expression.register_symbol_table(symbolTable);
      
      checkCompile("metre+metre",expression);
      CHECK_EQUAL("m",expression.value().units.str());
      checkCompile("metre+second",expression);
      CHECK_THROW(expression.value(),std::exception);
      checkCompile("metre-metre",expression);
      CHECK_EQUAL("m",expression.value().units.str());
      checkCompile("metre-second",expression);
      CHECK_THROW(expression.value(),std::exception);
      checkCompile("metre*metre",expression);
      CHECK_EQUAL("m^2",expression.value().units.str());
      checkCompile("metre*second",expression);
      CHECK_EQUAL("m s",expression.value().units.str());
      checkCompile("metre/metre",expression);
      CHECK_EQUAL("",expression.value().units.str());
      checkCompile("metre/second",expression);
      CHECK_EQUAL("m s^-1",expression.value().units.str());
      checkCompile("x^3.5",expression);
      CHECK_EQUAL("",expression.value().units.str());
      checkCompile("metre^3.5",expression);
      CHECK_THROW(expression.value(),std::exception);
      checkCompile("metre^2",expression);
      CHECK_EQUAL("m^2",expression.value().units.str());
      checkCompile("pow(metre,2.0)",expression);
      CHECK_EQUAL("m^2",expression.value().units.str());
      checkCompile("metre^64",expression);
      CHECK_EQUAL("m^64",expression.value().units.str());
      checkCompile("pow(metre,64)",expression);
      CHECK_EQUAL("m^64",expression.value().units.str());

#define testFunction(f)                                 \
      checkCompile(#f "(metre)",expression);          \
      CHECK_THROW(expression.value(),std::exception);   \
      checkCompile(#f "(x)",expression);              \
      CHECK_EQUAL("",expression.value().units.str());

      //testFunction(abs); // TODO
      testFunction(ceil);
      // testFunction(clamp); //TODO
      //testFunction(equal); //TODO
      testFunction(erf);
      testFunction(erfc);
      testFunction(exp);
      testFunction(expm1);
      testFunction(floor);
      testFunction(frac);
      testFunction(log);
      testFunction(log10);
      testFunction(log1p);
      testFunction(log2);
      //testFunction(logn); // TODO
      //testFunction(max); //TODO
      //testFunction(min); //TODO
      //testFunction(mul); //TODO
      testFunction(ncdf);
      //testFunction(root); //TODO
      testFunction(round);
      //testFunction(roundn); //TODO
      testFunction(sgn);
      testFunction(sqrt);
      testFunction(trunc);

      testFunction(acos);
      testFunction(acosh);
      testFunction(asin);
      testFunction(atan);

#define testLogicalOp(op)                                 \
      checkCompile("metre " #op " second",expression);  \
      CHECK_THROW(expression.value(),std::exception);     \
      checkCompile("metre " #op " metre",expression);   \
      CHECK_EQUAL("",expression.value().units.str()); 

      testLogicalOp(==);
      testLogicalOp(!=);
      testLogicalOp(<);
      testLogicalOp(<=);
      testLogicalOp(>);
      testLogicalOp(>=);
      testLogicalOp(and);
      testLogicalOp(or);
      testLogicalOp(nand);
      testLogicalOp(nor);
      testLogicalOp(xor);
    }
}
