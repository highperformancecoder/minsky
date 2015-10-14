/*
  @copyright Steve Keen 2014
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

// contains implementation of symbolic differentiation

#include "equations.h"
#include "expr.h"
#include <ecolab_epilogue.h>
#include <boost/regex.hpp>
using boost::regex;
using boost::smatch;
using namespace minsky;

namespace MathDAG
{
  NodePtr VariableDAG::derivative(SystemOfEquations& se) const
  {return se.derivative(*this);}
  NodePtr ConstantDAG::derivative(SystemOfEquations& se) const
  {return se.derivative(*this);}

  string differentiateName(const string& x)
  {
    int order=0; // current derivative order
    string varName=x; // base variable name
    regex singleDeriv(R"(d(.*)/dt)"),
      higherOrderDeriv(R"(d\^\{(\d*)\}(.*)/dt\^\{(\d*)\})");
    smatch m;
    if (regex_match(x,m,singleDeriv))
      {
        order=1;
        varName=m[1];
      }
    else if(regex_match(x,m,higherOrderDeriv) && m[1]==m[3])
      { 
        // for some reason, stoi is missing on MXE, so fake it
        order=atoi(m[3].str().c_str());
        varName=m[2];
      }
    order++;
    ostringstream r;
    if (order==1)
      r<<"d"<<varName<<"/dt";
    else
      r<<"d^{"<<order<<"}"<<varName<<"/dt^{"<<order<<"}";
    return r.str();
  }

  // applies the chain rule to expression x
  template <>
  NodePtr SystemOfEquations::chainRule(const Expr& x, const Expr& deriv)
  {
    NodePtr dx=x->derivative(*this);
    // perform some constant optimisation
    if (dx==zero) 
      return zero;
    else if (dx==one)
      return deriv;
    else
      return dx * deriv;
  }

  template <>
  NodePtr SystemOfEquations::derivative(const VariableDAG& expr)
  {
    string name=differentiateName(expr.name);
    VariableDAGPtr r(makeDAG(VariableManager::valueId(expr.scope,name),expr.scope,name,VariableType::flow));
    if (expr.rhs)
      r->rhs=expr.rhs->derivative(*this);
    else if (expr.type==VariableType::integral || expr.type==VariableType::stock)
      {
        auto ii=expressionCache.getIntegralInput(expr.valueId);
        if (!ii)
          throw error("integral input %s not defined",expr.valueId.c_str());
        if (ii->rhs)
          r->rhs=ii->rhs; // elide input variable, in case this is a temporary
        else
          r->rhs=ii.get();
      }
    else
      r->rhs=zero;
    assert(expressionCache.reverseLookup(*r));
    return r;
  }

  template <>
  NodePtr SystemOfEquations::derivative(const ConstantDAG&)
  {
    return zero;
  }

  template <>
  NodePtr SystemOfEquations::derivative
  (const OperationDAG<OperationType::constant>& expr)
  {
    return zero;
  }
    
  template <>
  NodePtr SystemOfEquations::derivative<>
  (const OperationDAG<OperationType::add>& expr)
  {
    CachedOp<OperationType::add> r(expressionCache);
    r->arguments.resize(expr.arguments.size());
    for (size_t i=0; i<expr.arguments.size(); ++i)
      for (WeakNodePtr n: expr.arguments[i])
        {
          assert(expressionCache.reverseLookup(*n));
          r->arguments[i].push_back(n->derivative(*this));
          assert(expressionCache.reverseLookup(*r->arguments[i].back()));
        }
    assert(expressionCache.reverseLookup(*r));
    return r;
  }
    
  template <>
  NodePtr SystemOfEquations::derivative
  (const OperationDAG<OperationType::subtract>& expr)
  {
    CachedOp<OperationType::subtract> r(expressionCache);
    r->arguments.resize(expr.arguments.size());
    for (size_t i=0; i<expr.arguments.size(); ++i)
      for (WeakNodePtr n: expr.arguments[i])
        r->arguments[i].push_back(n->derivative(*this));
    assert(expressionCache.reverseLookup(*r));
    return r;
  }
    
  template <>
  NodePtr SystemOfEquations::derivative
  (const OperationDAG<OperationType::multiply>& expr)
  {
    // unfold arguments into a single list
    vector<WeakNodePtr> args;
    for (size_t i=0; i<expr.arguments.size(); ++i)
      for (WeakNodePtr n: expr.arguments[i])
        args.push_back(n);

    // remember multiplies are n-ary, not binary. eg
    // (uvw)' = u'(vw)+v'(uw)+w'(uv)

    CachedOp<OperationType::add> r(expressionCache);
    assert(!r->arguments.empty());

    for (size_t i=0; i<args.size(); ++i)
      {
        CachedOp<OperationType::multiply> p(expressionCache);
        r->arguments[0].push_back(p);
        assert(!p->arguments.empty());
        for (size_t j=0; j<args.size(); ++j)
          if (j!=i)
            p->arguments[0].push_back(args[j]);
        p->arguments[0].push_back(args[i]->derivative(*this));
        assert(expressionCache.reverseLookup(*p->arguments[0].back()));
      }

    assert(expressionCache.reverseLookup(*r));
    return r;
  }
    
  template <>
  NodePtr SystemOfEquations::derivative
  (const OperationDAG<OperationType::divide>& expr)
  {
    // remember divides are n-ary, not binary. So convert each
    // argument list into a product expression, and then perform the
    // standard binary quotient rule. u=numerator, v=denominator
    // d(u/v) = (vdu-udv)/v^2

    CachedOp<OperationType::multiply> u1(expressionCache), v1(expressionCache);

    assert(expr.arguments.size()==2);
    assert(!u1->arguments.empty() && !v1->arguments.empty());

    // check all arguments are cached
#ifndef NDEBUG
    for (auto av: expr.arguments)
      for (auto a: av)
        assert(a && expressionCache.reverseLookup(*a));
#endif

    u1->arguments[0]=expr.arguments[0];
    v1->arguments[0]=expr.arguments[1];
    
    Expr u(expressionCache,u1), v(expressionCache,v1);
    Expr du(expressionCache, u->derivative(*this)), dv(expressionCache, v->derivative(*this));
    
    NodePtr r = (v*du-u*dv)/(v*v); 
    assert(expressionCache.reverseLookup(*r));
    return r;
  }
    
  template <>
  NodePtr SystemOfEquations::derivative(const OperationDAG<OperationType::log>& expr)
  {
    // log_b(x) = ln(x)/ln(b)
    assert(expr.arguments.size()==2);
    if (expr.arguments[0].empty())
      return zero;
    else 
      {
        Expr x(expressionCache, expressionCache.reverseLookup(*expr.arguments[0][0]));
        if (expr.arguments[1].empty())
          return x->derivative(*this)/x;
        else
          {
            Expr b(expressionCache, expressionCache.reverseLookup(*expr.arguments[1][0]));
            return (log(x)/log(b))->derivative(*this);
          }
      }
  }


  template <>
  NodePtr SystemOfEquations::derivative
  (const OperationDAG<OperationType::pow>& expr)
  {
    // x^y = exp(y*ln(x))
    assert(expr.arguments.size()==2);
    if (expr.arguments[0].empty())
      return zero;
    else if (expr.arguments[1].empty())
      {
        Expr dx(expressionCache, expr.arguments[0][0]->derivative(*this));
        return dx * expr.derivative(*this);
      }
    else
      {
        Expr x(expressionCache, expr.arguments[0][0]);
        Expr y(expressionCache,  expr.arguments[1][0]);
        return exp(y * log(x))->derivative(*this);
      }
  }

  template <>
  NodePtr SystemOfEquations::derivative
  (const OperationDAG<OperationType::lt>& expr)
  {
    return zero;
  }

  template <>
  NodePtr SystemOfEquations::derivative
  (const OperationDAG<OperationType::le>& expr)
  {
    return zero;
  }

  template <>
  NodePtr SystemOfEquations::derivative
  (const OperationDAG<OperationType::eq>& expr)
  {
    return zero;
  }

  template <>
  NodePtr SystemOfEquations::derivative
  (const OperationDAG<OperationType::and_>& expr)
  {
    return zero;
  }

  template <>
  NodePtr SystemOfEquations::derivative
  (const OperationDAG<OperationType::or_>& expr)
  {
    return zero;
  }

  template <>
  NodePtr SystemOfEquations::derivative
  (const OperationDAG<OperationType::not_>& expr)
  {
    return zero;
  }

  // nb strictly speaking, the derivative is undefined at x==y,
  // unless x(t)==y(t), but shouldn't cause problems on integration
  template <>
  NodePtr SystemOfEquations::derivative
  (const OperationDAG<OperationType::min>& expr)
  {
    assert(expr.arguments.size()==2);
    if (expr.arguments[0].empty())
      return zero;
    else if (expr.arguments[1].empty())
      {
        Expr x(expressionCache,expr.arguments[0][0]);
        return (x <= 0)*x->derivative(*this);
      }
    else if (expr.arguments[0].empty())
      {
        Expr y(expressionCache,expr.arguments[0][0]);
        return (y <= 0)*y->derivative(*this);
      }
    else
      {
        Expr x(expressionCache,expr.arguments[0][0]);
        Expr y(expressionCache,expr.arguments[1][0]);
        return (x<=y)*x->derivative(*this) +
          (1-(x<=y))*y->derivative(*this);
      }
  }

  // nb strictly speaking, the derivative is undefined at x==y,
  // unless x(t)==y(t), but shouldn't cause problems on integration
  template <>
  NodePtr SystemOfEquations::derivative
  (const OperationDAG<OperationType::max>& expr)
  {
    assert(expr.arguments.size()==2);
    Expr z(expressionCache,zero);
    if (expr.arguments[0].empty())
      return zero;
    else if (expr.arguments[1].empty())
      {
        Expr x(expressionCache,expr.arguments[0][0]);
        return (z<=x)*x->derivative(*this);
      }
    else if (expr.arguments[0].empty())
      {
        Expr y(expressionCache,expr.arguments[0][0]);
        return (z<=y)*y->derivative(*this);
      }
    else
      {
        Expr x(expressionCache,expr.arguments[0][0]);
        Expr y(expressionCache,expr.arguments[1][0]);
        return (x<=y)*y->derivative(*this) +
          (1-(x<=y))*x->derivative(*this);
      }
  }

  template <>
  NodePtr SystemOfEquations::derivative
  (const OperationDAG<OperationType::time>& expr)
  {
    return one;
  }

  template <>
  NodePtr SystemOfEquations::derivative
  (const OperationDAG<OperationType::copy>& expr)
  {
    if (!expr.arguments[0].empty() && expr.arguments[0][0])
      return expr.arguments[0][0]->derivative(*this);
    else
      return zero;
  }

  template <>
  NodePtr SystemOfEquations::derivative
  (const OperationDAG<OperationType::integrate>& expr)
  {
    if (!expr.arguments[0].empty() && expr.arguments[0][0])
      return expressionCache.reverseLookup(*expr.arguments[0][0]);
    else
      return zero;
  }

  template <>
  NodePtr SystemOfEquations::derivative
  (const OperationDAG<OperationType::differentiate>& expr)
  {
    if (!expr.arguments[0].empty() && expr.arguments[0][0])
      return expr.arguments[0][0]->derivative(*this)->derivative(*this);
    else
      return zero;
  }
 
  template <>
  NodePtr SystemOfEquations::derivative
  (const OperationDAG<OperationType::data>& expr)
  {
    throw error("cannot differentiate an empirical curve");
  }
 
  template <>
  NodePtr SystemOfEquations::derivative
  (const OperationDAG<OperationType::sqrt>& expr)
  {
    Expr x(expressionCache, expr.arguments[0][0]);
    return chainRule(x, 0.5/sqrt(x));
  }
 
  template <>
  NodePtr SystemOfEquations::derivative
  (const OperationDAG<OperationType::exp>& expr)
  {
    Expr x(expressionCache, expr.arguments[0][0]);
    Expr expx(expressionCache, expressionCache.reverseLookup(expr));
    return chainRule(x, exp(x));
  }
 
  template <>
  NodePtr SystemOfEquations::derivative
  (const OperationDAG<OperationType::ln>& expr)
  {
    if (expr.arguments[0].empty())
      return zero;
    else
      {
        Expr x(expressionCache, expr.arguments[0][0]);
        return chainRule(x, 1/x);
      }
  }

  template <>
  NodePtr SystemOfEquations::derivative
  (const OperationDAG<OperationType::sin>& expr)
  {
    if (expr.arguments[0].empty())
      return zero;
    else
      {
        Expr x(expressionCache, expr.arguments[0][0]);
        return chainRule(x,cos(x));
      }
  }

  template <>
  NodePtr SystemOfEquations::derivative<>
  (const OperationDAG<OperationType::cos>& expr)
  {
    if (expr.arguments[0].empty())
      return zero;
    else
      {
        Expr x(expressionCache, expr.arguments[0][0]);
        return chainRule(x,-1*sin(x));
      }
  }

 
  template <>
  NodePtr SystemOfEquations::derivative<>
  (const OperationDAG<OperationType::tan>& expr)
  {
    if (expr.arguments[0].empty())
      return zero;
    else
      {
        Expr x(expressionCache, expr.arguments[0][0]);
        Expr secx=1/cos(x);
        return chainRule(x,secx * secx);
      }
  }

  template <>
  NodePtr SystemOfEquations::derivative<>
  (const OperationDAG<OperationType::asin>& expr)
  {
    if (expr.arguments[0].empty())
      return zero;
    else
      {
        Expr x(expressionCache, expr.arguments[0][0]);
        return chainRule(x, 1/sqrt(1-x*x));
      }
  }

  template <>
  NodePtr SystemOfEquations::derivative<>
  (const OperationDAG<OperationType::acos>& expr)
  {
    if (expr.arguments[0].empty())
      return zero;
    else
      {
        Expr x(expressionCache, expr.arguments[0][0]);
        return chainRule(x, -1/sqrt(1-x*x));
      }
  }

  template <>
  NodePtr SystemOfEquations::derivative<>
  (const OperationDAG<OperationType::atan>& expr)
  {
    if (expr.arguments[0].empty())
      return zero;
    else
      {
        Expr x(expressionCache, expr.arguments[0][0]);
        return chainRule(x, 1/(1+x*x));
      }
  }

  template <>
  NodePtr SystemOfEquations::derivative<>
  (const OperationDAG<OperationType::sinh>& expr)
  {
    if (expr.arguments[0].empty())
      return zero;
    else
      {
        Expr x(expressionCache, expr.arguments[0][0]);
        return chainRule(x, cosh(x));
      }
  }

  template <>
  NodePtr SystemOfEquations::derivative<>
  (const OperationDAG<OperationType::cosh>& expr)
  {
    if (expr.arguments[0].empty())
      return zero;
    else
      {
        Expr x(expressionCache, expr.arguments[0][0]);
        return chainRule(x, sinh(x));
      }
  }

  template <>
  NodePtr SystemOfEquations::derivative<>
  (const OperationDAG<OperationType::tanh>& expr)
  {
    if (expr.arguments[0].empty())
      return zero;
    else
      {
        Expr x(expressionCache, expr.arguments[0][0]);
        Expr sech=1/cosh(x);
        return chainRule(x, sech*sech);
      }
  }

  template <>
  NodePtr SystemOfEquations::derivative<>
  (const OperationDAG<OperationType::abs>& expr)
  {
    if (expr.arguments[0].empty())
      return zero;
    else
      {
        Expr x(expressionCache, expr.arguments[0][0]);
        return chainRule(x, (one-(x<=zero)) - (x<=zero));
      }
  }

}
