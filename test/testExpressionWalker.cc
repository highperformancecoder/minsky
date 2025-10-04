
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

#include <gtest/gtest.h>
#include "expressionWalker.h"
#include "exprtk/exprtk.hpp"
#include "minsky_epilogue.h"
using namespace minsky;
using namespace std;

void checkCompile(const std::string& expr, exprtk::expression<UnitsExpressionWalker>& expression)
{
  exprtk::parser<UnitsExpressionWalker> parser;
  EXPECT_TRUE(parser.compile(expr,expression));
  for (size_t i=0; i<parser.error_count(); ++i)
    cout << parser.get_error(i).diagnostic<<endl;
}

TEST(ExpressionWalker, UnitsExpressionWalker)
  {
      UnitsExpressionWalker metre("m"), second("s"), x;
      exprtk::symbol_table<UnitsExpressionWalker> symbolTable;
      
      symbolTable.add_variable("metre",metre);
      symbolTable.add_variable("second",second);
      symbolTable.add_variable("x",x);

      exprtk::expression<UnitsExpressionWalker> expression;
      expression.register_symbol_table(symbolTable);
      
      checkCompile("-metre",expression);
      EXPECT_EQ("m",expression.value().units.str());
      checkCompile("+metre",expression);
      EXPECT_EQ("m",expression.value().units.str());
      checkCompile("metre+metre",expression);
      EXPECT_EQ("m",expression.value().units.str());
      checkCompile("metre+second",expression);
      EXPECT_THROW(expression.value(),std::exception);
      checkCompile("metre-metre",expression);
      EXPECT_EQ("m",expression.value().units.str());
      checkCompile("metre-second",expression);
      EXPECT_THROW(expression.value(),std::exception);
      checkCompile("metre*metre",expression);
      EXPECT_EQ("m^2",expression.value().units.str());
      checkCompile("metre*second",expression);
      EXPECT_EQ("m s",expression.value().units.str());
      checkCompile("metre/metre",expression);
      EXPECT_EQ("",expression.value().units.str());
      checkCompile("metre/second",expression);
      EXPECT_EQ("m s^-1",expression.value().units.str());
      checkCompile("metre%second",expression);
      EXPECT_EQ("m",expression.value().units.str());
      checkCompile("x^3.5",expression);
      EXPECT_EQ("",expression.value().units.str());
      checkCompile("metre^3.5",expression);
      EXPECT_THROW(expression.value(),std::exception);
      checkCompile("metre^2",expression);
      EXPECT_EQ("m^2",expression.value().units.str());
      checkCompile("pow(metre,2.0)",expression);
      EXPECT_EQ("m^2",expression.value().units.str());
      checkCompile("metre^64",expression);
      EXPECT_EQ("m^64",expression.value().units.str());
      checkCompile("pow(metre,64)",expression);
      EXPECT_EQ("m^64",expression.value().units.str());

#define testFunction(f)                                 \
      checkCompile(#f "(metre)",expression);          \
      EXPECT_THROW(expression.value(),std::exception);   \
      checkCompile(#f "(x)",expression);              \
      EXPECT_EQ("",expression.value().units.str());

      checkCompile("abs(metre)",expression); 
      EXPECT_EQ("m",expression.value().units.str());
      
      checkCompile("avg(metre,metre)",expression); 
      EXPECT_EQ("m",expression.value().units.str());

      testFunction(ceil);
      testFunction(erf);
      testFunction(erfc);
      testFunction(exp);
      testFunction(expm1);
      testFunction(floor);
      checkCompile("frac(metre)",expression); 
      EXPECT_EQ("m",expression.value().units.str());
      testFunction(log);
      testFunction(log10);
      testFunction(log1p);
      testFunction(log2);
      testFunction(ncdf);

      checkCompile("sgn(metre)",expression);            \
      EXPECT_EQ("",expression.value().units.str());
      checkCompile("sgn(x)",expression);              \
      EXPECT_EQ("",expression.value().units.str());
      
      checkCompile("sqrt(metre^2)",expression);
      EXPECT_EQ("m",expression.value().units.str()); 

      checkCompile("trunc(metre)",expression);
      EXPECT_EQ("m",expression.value().units.str()); 

      testFunction(acos);
      testFunction(acosh);
      testFunction(asin);
      testFunction(asinh);
      testFunction(atan);
      testFunction(atanh);
      testFunction(cos);
      testFunction(cosh);
      testFunction(cot);
      testFunction(sec);
      testFunction(csc);
      testFunction(rad2deg);
      testFunction(deg2rad);
      testFunction(deg2grad);
      testFunction(grad2deg);
      testFunction(not);
      testFunction(sin);
      testFunction(sinc);
      testFunction(sinh);
      testFunction(sqrt);
      testFunction(tan);
      testFunction(tanh);

#define testLogicalOp(op)                                 \
      checkCompile("metre " #op " second",expression);  \
      EXPECT_THROW(expression.value(),std::exception);     \
      checkCompile("metre " #op " metre",expression);   \
      EXPECT_EQ("",expression.value().units.str()); 

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
      testLogicalOp(xnor);

#define testBinaryFunction(f)                                 \
      checkCompile(#f "(metre, second)",expression);  \
      EXPECT_THROW(expression.value(),std::exception);     \
      checkCompile(#f "(x, x)",expression);  \
      EXPECT_EQ("",expression.value().units.str()); 

      testBinaryFunction(atan2);
      testBinaryFunction(equal);
      checkCompile("equal(metre, metre)",expression);
      EXPECT_EQ("",expression.value().units.str());

      // parser doesn't seem to understand nequal
      //      checkCompile("nequal(metre, metre)",expression);
      //      EXPECT_EQ("",expression.value().units.str()); 
    
      
      testBinaryFunction(hypot);
      testBinaryFunction(logn);
      testBinaryFunction(shr);
      testBinaryFunction(shl);
      testBinaryFunction(root);

      checkCompile("root(metre^3, 3)",expression);
      EXPECT_EQ("m",expression.value().units.str()); 

      checkCompile("roundn(metre, 3)",expression);
      EXPECT_EQ("m",expression.value().units.str()); 
      checkCompile("round(metre)",expression);
      EXPECT_EQ("m",expression.value().units.str()); 
  
      checkCompile("mod(metre,second)",expression);
      EXPECT_EQ("m",expression.value().units.str());

      checkCompile("clamp(metre,metre,3*metre)",expression);
      EXPECT_EQ("m",expression.value().units.str());
      checkCompile("iclamp(metre,metre,3*metre)",expression);
      EXPECT_EQ("m",expression.value().units.str());
      checkCompile("inrange(metre,metre,3*metre)",expression);
      EXPECT_EQ("",expression.value().units.str());

      testBinaryFunction(max);
      checkCompile("max(metre,metre)",expression);
      EXPECT_EQ("m",expression.value().units.str());
      testBinaryFunction(min);
      checkCompile("min(metre,metre)",expression);
      EXPECT_EQ("m",expression.value().units.str());
      checkCompile("mul(metre,metre)",expression);
      EXPECT_EQ("m^2",expression.value().units.str());
      checkCompile("mul(metre,second)",expression);
      EXPECT_EQ("m s",expression.value().units.str());

    }
