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

#include <cmath>

namespace minsky
{
  int UserFunction::nextId=0;
  
  exprtk::symbol_table<double>& UserFunction::globalSymbols()
  {
    static exprtk::symbol_table<double> table;
    return table;
  }

  namespace {
    // resolve overloads
    inline double isfinite(double x) {return std::isfinite(x);}
    inline double isinf(double x) {return std::isinf(x);}
    inline double isnan(double x) {return std::isnan(x);}
    // add extra function definitions here
    int dum=(
             UserFunction::globalSymbols().add_function("isfinite",isfinite),
             UserFunction::globalSymbols().add_function("isinf",isinf),
             UserFunction::globalSymbols().add_function("isnan",isnan),
             0);

//    // exprtk goes up to 15 arguments only
//#define ARGS0(T) ()
//#define ARGS1(T) (T x1)
//#define ARGS2(T) (T x1, T x2)
//    
//#define CALLFUNCTION(N)                                                 \
//    [f]ARGS##N(double){return f->evaluate ARGS##N();}
//
//    void addUserFunctionToGlobalTable(const ItemPtr& item)
//    {
//      if (auto f=dynamic_pointer_cast<UserFunction>(item))
//        {
//          switch (f->argNames.size())
//            {
//            case 0:
//              UserFunction::globalSymbols().add_function(f->description(), CALLFUNCTION(0));
//              break;
//            case 1:
//              UserFunction::globalSymbols().add_function(f->description(), CALLFUNCTION(1));
//              break;
//            case 2:
//              UserFunction::globalSymbols().add_function(f->description(), CALLFUNCTION(2));
//              break;
//            default:
//              f->throw_error("Too many arguments: "+f->argNames.size());
//            }
//        }
//    }
  }
  
  namespace {
    exprtk::parser<double> parser;
  }
  
  template <> void Operation<OperationType::userFunction>::iconDraw(cairo_t*) const
  {assert(false);}

  UserFunction::UserFunction(const string& name, const string& expression): argNames{"x","y"}, expression(expression)  {
    UserFunction::description(name);
      
    compiledExpression.register_symbol_table(globalSymbols());
    compiledExpression.register_symbol_table(externalSymbols);
    compiledExpression.register_symbol_table(localSymbols);
  }

  vector<string> UserFunction::externalSymbolNames() const
  {
    // do an initial parse to pick up references to external variables
    exprtk::symbol_table<double> externalSymbols, localSymbols=this->localSymbols;
    exprtk::expression<double> compiledExpression;
    compiledExpression.register_symbol_table(externalSymbols);
    compiledExpression.register_symbol_table(globalSymbols());
    compiledExpression.register_symbol_table(localSymbols);
    parser.enable_unknown_symbol_resolver();
    parser.compile(expression, compiledExpression);
    parser.disable_unknown_symbol_resolver();
    std::vector<std::string> externalVariables;
    externalSymbols.get_variable_list(externalVariables);
    return externalVariables;
  }

  void UserFunction::compile()
  {
    localSymbols.clear();
    argVals.resize(argNames.size());
    for (size_t i=0; i<argNames.size(); ++i)
      localSymbols.add_variable(argNames[i], argVals[i]);
    // add them back in with their correct definitions
    externalSymbols.clear();
    for (auto& i: externalSymbolNames())
      {
        auto v=minsky().variableValues.find(VariableValue::valueIdFromScope(group.lock(),i));
        if (v!=minsky().variableValues.end())
          externalSymbols.add_variable(i, (*v->second)[0]);
        else
          throw_error("unknown variable: "+i);
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
    if (argVals.size()>0) argVals[0]=in1;
    if (argVals.size()>1) argVals[1]=in2;
    for (size_t i=2; i<argVals.size(); ++i) argVals[i]=0;
    return compiledExpression.value();
  }

  double UserFunction::operator()(const std::vector<double>& p)
  {
    size_t i=0;
    for (; i<p.size() && i<argVals.size(); ++i) argVals[i]=p[i];
    for (; i<argVals.size(); ++i) argVals[i]=0;
    return compiledExpression.value();
  }

  string UserFunction::description(const string& nm)
  {
    NamedOp::description(nm);
    static regex extractArgList(R"([^(]*\(([^)]*)\))");
    smatch match;
    string argList;
    if (regex_match(nm,match,extractArgList))
      argList=match[1];

    argNames.clear();
    auto end=argList.find(',');
    decltype(end) begin=0;
    for (; end!=string::npos; begin=end+1, end=argList.find(',',begin))
      argNames.push_back(argList.substr(begin,end-begin));
    argNames.push_back(argList.substr(begin));
    return nm;
  }

  string UserFunction::name() const
  {
    static regex extractName(R"(([^(]*).*)");
    smatch match;
    auto d=description();
    regex_match(d, match, extractName);
    assert (match.size()>1);
    return match[1];
  }    


  
}

