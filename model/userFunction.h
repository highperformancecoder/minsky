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

#ifndef USERFUNCTION_H
#define USERFUNCTION_H
#include "operation.h"
#include "unitsExpressionWalker.h"
#include "exprtk/exprtk.hpp"
namespace  minsky
{
  class UserFunction: public ItemT<UserFunction, Operation<OperationType::userFunction>>, public NamedOp
  {
    exprtk::symbol_table<double> localSymbols, externalSymbols;
    exprtk::expression<double> compiledExpression;
    void updateBB() override {bb.update(*this);}
    CLASSDESC_ACCESS(UserFunction);
  public:
    static exprtk::symbol_table<double>& globalSymbols();
    static exprtk::symbol_table<UnitsExpressionWalker>& globalUnitSymbols();
    static int nextId;
    double x, y;
    std::string expression;
    UserFunction(): UserFunction("uf"+std::to_string(nextId++)+"(x,y)") {}
    UserFunction(const std::string& name, const std::string& expression="");
    std::vector<std::string> externalSymbolNames() const;
    void compile();
    double evaluate(double x, double y);
    Units units(bool check=false) const override;
    void addVariable(const std::string& name, double& x) {
      localSymbols.add_variable(name,x);
    }
    template <class F>
    void addFunction(const std::string& name, F f) {
      localSymbols.add_function(name,f);
    }
    void displayTooltip(cairo_t* cr, const std::string& tt) const override
    {Item::displayTooltip(cr,tt.empty()? expression: tt+" "+expression);}
  };

  // single argument user function
  class UserFunction1: public UserFunction
  {
  public:
    UserFunction1() {}
    UserFunction1(const std::string& name, const std::string& expression=""): UserFunction(name,expression) {}
    double evaluate(double x) {return UserFunction::evaluate(x,0);}
  };

  // static UnitExpressionWalker that is initialised to the time unit
  extern UnitsExpressionWalker timeUnit;

}
#include "userFunction.cd"
#include "userFunction.xcd"
#endif
