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

// some nice syntactic sugar for representing expressions

#ifndef EXPR_H
#define EXPR_H
#include "equations.h"
#include <ecolab_epilogue.h>
namespace MathDAG
{
  // some nice syntactic sugar for representing expressions
  struct Expr: public NodePtr
  {
    SubexpressionCache& cache; //for anonymous nodes
    Expr(SubexpressionCache& cache, const NodePtr& x): 
      NodePtr(x), cache(cache) {cache.insertAnonymous(x);}
    Expr(SubexpressionCache& cache, const shared_ptr<OperationDAGBase>& x):
      NodePtr(x), cache(cache) {cache.insertAnonymous(x);}
    Expr(SubexpressionCache& cache, const Node& x): 
      NodePtr(cache.reverseLookup(x)), cache(cache) {assert(*this);}
    Expr(SubexpressionCache& cache, const WeakNodePtr& x): 
      NodePtr(cache.reverseLookup(*x)), cache(cache) {assert(*this);}

    shared_ptr<OperationDAGBase> newNode(OperationType::Type type) const {
      shared_ptr<OperationDAGBase> r(OperationDAGBase::create(type));
      cache.insertAnonymous(r);
      return r;
    }

    Expr operator+(const NodePtr& x) const {
      cache.insertAnonymous(x);
      shared_ptr<OperationDAGBase> r=newNode(OperationType::add);
      r->arguments[0].push_back(*this);
      r->arguments[1].push_back(x);
      return Expr(cache,r);
    }
    Expr operator-(const NodePtr& x) const {
      cache.insertAnonymous(x);
      shared_ptr<OperationDAGBase> r=newNode(OperationType::subtract);
      r->arguments[0].push_back(*this);
      r->arguments[1].push_back(x);
      return Expr(cache,r);
    }
    Expr operator*(const NodePtr& x) const {
      cache.insertAnonymous(x);
      shared_ptr<OperationDAGBase> r=newNode(OperationType::multiply);
      r->arguments[0].push_back(*this);
      r->arguments[1].push_back(x);
      return Expr(cache,r);
    }
    Expr operator/(const NodePtr& x) const {
      cache.insertAnonymous(x);
      shared_ptr<OperationDAGBase> r=newNode(OperationType::divide);
      r->arguments[0].push_back(*this);
      r->arguments[1].push_back(x);
      return Expr(cache,r);
    }
  };

  inline Expr operator+(const NodePtr& x, const Expr& y) {
    return y+x;
  }
  inline Expr operator+(const Expr& x, const Expr& y) {
    return x+NodePtr(y);
  }
  inline Expr operator+(double x, const Expr& y) {
    return y+NodePtr(new ConstantDAG(x));
  }

  inline Expr operator-(const NodePtr& x, const Expr& y) {
    return Expr(y.cache, x)-NodePtr(y);
  }
  inline Expr operator-(const Expr& x, const Expr& y) {
    return x-NodePtr(y);
  }
  inline Expr operator-(double x, const Expr& y) {
    return NodePtr(new ConstantDAG(x))-y;
  }
  inline Expr operator-(const Expr& x, double y) {
    return x-NodePtr(new ConstantDAG(y));
  }

  inline Expr operator*(const NodePtr& x, const Expr& y) {
    return y*x;
  }
  inline Expr operator*(const Expr& x, const Expr& y) {
    return x*NodePtr(y);
  }
  inline Expr operator*(double x, const Expr& y) {
    return y*NodePtr(new ConstantDAG(x));
  }

  inline Expr operator/(const Expr& x, const Expr& y) {
    return x/NodePtr(y);
  }
  inline Expr operator/(const NodePtr& x, const Expr& y) {
    return Expr(y.cache, x)/y;
  }
  inline Expr operator/(double x, const Expr& y) {
    return NodePtr(new ConstantDAG(x))/y;
  }

  inline Expr log(const Expr& x) {
    shared_ptr<OperationDAGBase> r(x.newNode(OperationType::ln));
    r->arguments[0].push_back(x);
    return Expr(x.cache,r);
  }

  inline Expr exp(const Expr& x) {
    shared_ptr<OperationDAGBase> r(x.newNode(OperationType::exp));
    r->arguments[0].push_back(x);
    return Expr(x.cache,r);
  }
  inline Expr sin(const Expr& x) {
    shared_ptr<OperationDAGBase> r(x.newNode(OperationType::sin));
    r->arguments[0].push_back(x);
    return Expr(x.cache,r);
  }

  inline Expr cos(const Expr& x) {
    shared_ptr<OperationDAGBase> r(x.newNode(OperationType::cos));
    r->arguments[0].push_back(x);
    return Expr(x.cache,r);
  }
  inline Expr sinh(const Expr& x) {
    shared_ptr<OperationDAGBase> r(x.newNode(OperationType::sinh));
    r->arguments[0].push_back(x);
    return Expr(x.cache,r);
  }

  inline Expr cosh(const Expr& x) {
    shared_ptr<OperationDAGBase> r(x.newNode(OperationType::cosh));
    r->arguments[0].push_back(x);
    return Expr(x.cache,r);
  }

  inline Expr sqrt(const Expr& x) {
    shared_ptr<OperationDAGBase> r(x.newNode(OperationType::sqrt));
    r->arguments[0].push_back(x);
    return Expr(x.cache,r);
  }

  inline Expr operator<=(const Expr& x, const NodePtr& y) {
    shared_ptr<OperationDAGBase> r(x.newNode(OperationType::le));
    r->arguments[0].push_back(x);    
    r->arguments[1].push_back(y);
    return Expr(x.cache,r);
  }

  inline Expr operator<=(const Expr& x, double y) {
    return x <= Expr(x.cache, NodePtr(new ConstantDAG(y)));
  }

  inline Expr operator<(const Expr& x, const NodePtr& y) {
    shared_ptr<OperationDAGBase> r(x.newNode(OperationType::lt));
    r->arguments[0].push_back(x);    
    r->arguments[1].push_back(y);
    return Expr(x.cache,r);
  }

  inline Expr operator<(const Expr& x, double y) {
    return x < Expr(x.cache, NodePtr(new ConstantDAG(y)));
  }

  template <OperationType::Type T>
  struct CachedOp: std::shared_ptr<OperationDAGBase>
  {
    CachedOp(SubexpressionCache& ec):
      std::shared_ptr<OperationDAGBase>(OperationDAGBase::create(T))
    {ec.insertAnonymous(*this);}
  };
}

#endif
