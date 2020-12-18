/*
  @copyright Steve Keen 2013
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
#ifndef OPERATIONTYPE_H
#define OPERATIONTYPE_H
#include <factory.h>
#include <string>
#include <ostream>

namespace minsky
{
  struct OperationType
  {
    enum Type {constant, // deprecated - left to support legacy schemas
               time,  // zero input port ops
               integrate, 
               differentiate, // with respect to time
               data, // an interpolated data item
               ravel, // Ravel™
               euler, pi, zero, one, inf, percent,// fundamental constants
               // multiwire binary ops
               add, subtract, multiply, divide, min, max, 
               and_, or_,
               // single wire binops
               log, pow, polygamma, 
               lt, le, eq, 
               userFunction,
               // underscores to avoid C++ keywords. Can be filtered at UI
               // functions
               copy, sqrt, exp, ln, sin, cos, tan, asin, acos, atan,
               sinh, cosh, tanh, abs,
               floor, frac, not_, gamma, fact,
               // reductions
               sum, product, infimum, supremum, any, all, infIndex, supIndex,
               // scans
               runningSum, runningProduct, difference,
               // other tensor ops
               innerProduct, outerProduct, index, gather,
               numOps // last operation, for iteration purposes
    };
    /// return the symbolic name of \a type
    static std::string typeName(int type);
    enum Group {general, constop, binop, function, reduction, scan, tensor};
    static Group classify(Type t);
  };

  namespace OperationTypeInfo
  {
    /// number of inputs this operator type has
    template <OperationType::Type type> int numArguments(); 
  }

  inline std::ostream& operator<<(std::ostream& o, OperationType::Type t)
  {return o<<OperationType::typeName(t);}

  /// factory class template, for creating objects of type T<O>. N is the maximum value of O
  template<class Base, template <minsky::OperationType::Type> class T, 
           int N=OperationType::numOps>
  class OperationFactory
  {
    struct CreatorBase
    {
      virtual ~CreatorBase() {}
      virtual Base* create() const=0;
    };

    std::vector<std::unique_ptr<CreatorBase>> creators;

    template <class U>
    struct Creator: public CreatorBase
    {
      Base* create() const {return new U();}
    };

  public:

    template <int I, int J>
    struct is_equal {const static bool value=I==J;};

    // recursively enumerate the enum at compile time.
    template <int I> 
    typename classdesc::enable_if<classdesc::Not<is_equal<I,N>>,void>::T
    registerNext()
    {
      creators.emplace_back(new Creator<T<OperationType::Type(I)>>);
      registerNext<I+1>();
    }
    template <int I> 
    typename classdesc::enable_if<is_equal<I,N>,void>::T
    registerNext()
    {creators.emplace_back(new Creator<T<OperationType::Type(I)>>);}

    /// create an object of type T<o> on the heap
    Base* create(OperationType::Type o) const {
      assert(o<creators.size());
      return creators[o]->create();
    }

    OperationFactory() {registerNext<OperationType::Type(0)>();}
  };


}

using minsky::operator<<;


#include "operationType.cd"
#include "operationType.xcd"
#endif
