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

/** Utilities for walking an exprtk expression object.

To use, define the actions you want to happen for each type of expression
*/

#ifndef EXPRESSIONWALKER_H
#define EXPRESSIONWALKER_H
#include "unitsExpressionWalker.h"

#include <memory>

namespace minsky
{
  inline bool operator>(std::size_t x, const UnitsExpressionWalker& y) {return x>y.value;}
  
  inline UnitsExpressionWalker root(const UnitsExpressionWalker& x, const UnitsExpressionWalker& y)
  {
    if (x.units.empty()) return {};
    int index=y.value;
    if (index!=y.value)
      throw std::runtime_error("index must be an integer for roots of dimensioned quanitites");
    UnitsExpressionWalker r;
    for (auto& i: x.units)
      {
        if (i.second % index)
          throw std::runtime_error("input dimension "+i.first+" not a power to the index "+std::to_string(index));
        r.units[i.first]=i.second/index;
      }
    return r;
  }                                 

  inline UnitsExpressionWalker pow(const UnitsExpressionWalker& x, const UnitsExpressionWalker& y)
  {
    if (y.value<1) return root(x,1/y.value);
    auto exponent=int(y.value);
    if (exponent==y.value)
      {
        auto tmp=x;
        for (auto& i: tmp.units)
          i.second*=exponent;
        return tmp;
      }
    x.checkDimensionless();
    y.checkDimensionless();
    return x;
  }
  
  inline UnitsExpressionWalker checkDimensionless(const UnitsExpressionWalker& x, const UnitsExpressionWalker& y)
  {x.checkDimensionless(); y.checkDimensionless(); return x;}
  
  inline UnitsExpressionWalker checkSameDims(const UnitsExpressionWalker& x, const UnitsExpressionWalker& y)
  {x.checkSameDims(y); auto tmp=x; tmp.units.clear(); return tmp;}
}

// preload these system headers here, to prevent them from being loaded into anonymous namespace
#include <algorithm>
#include <cctype>
#include <cmath>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <deque>
#include <exception>
#include <functional>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <set>
#include <stack>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#   include <windows.h>
#else
#   include <sys/time.h>
#   include <sys/types.h>
#endif

namespace exprtk
{
  namespace details
  {
    inline bool is_true(const minsky::UnitsExpressionWalker& x) {return x.value!=0;}
  }
}

#include <exprtk/exprtk.hpp>

namespace exprtk
{
  namespace details
  {
    template <>
    inline bool string_to_real(const std::string& s, minsky::UnitsExpressionWalker& t)
    {
      t.units.clear();
      try
        {
          t.value=stod(s);
        }
      catch (...)
        {return false;}
      return true;
    }

    namespace numeric
    {
#define exprtk_define_unary_function(Function)                          \
      template <>                                                       \
      inline minsky::UnitsExpressionWalker Function(const minsky::UnitsExpressionWalker x) \
      {x.checkDimensionless(); return x;}                                 
 
#define exprtk_define_unary_function_not_dimensionless(Function)                          \
      template <>                                                       \
      inline minsky::UnitsExpressionWalker Function(const minsky::UnitsExpressionWalker x) \
      {return x;}                                 
                     
      exprtk_define_unary_function_not_dimensionless(abs )
      exprtk_define_unary_function(acos )
      exprtk_define_unary_function(acosh)
      exprtk_define_unary_function(asin )
      exprtk_define_unary_function(asinh)
      exprtk_define_unary_function(atan )
      exprtk_define_unary_function(atanh)
      exprtk_define_unary_function(ceil )
      exprtk_define_unary_function(cos  )
      exprtk_define_unary_function(cosh )
      exprtk_define_unary_function(exp  )
      exprtk_define_unary_function(expm1)
      exprtk_define_unary_function(floor)
      exprtk_define_unary_function(log  )
      exprtk_define_unary_function(log10)
      exprtk_define_unary_function(log2 )
      exprtk_define_unary_function(log1p)
      exprtk_define_unary_function_not_dimensionless(neg  )
      exprtk_define_unary_function_not_dimensionless(pos  ) // apparently not used
      exprtk_define_unary_function_not_dimensionless(round)
      exprtk_define_unary_function(sin  )
      exprtk_define_unary_function(sinc )
      exprtk_define_unary_function(sinh )
      exprtk_define_unary_function(tan  )
      exprtk_define_unary_function(tanh )
      exprtk_define_unary_function(cot  )
      exprtk_define_unary_function(sec  )
      exprtk_define_unary_function(csc  )
      exprtk_define_unary_function(r2d  )
      exprtk_define_unary_function(d2r  )
      exprtk_define_unary_function(d2g  )
      exprtk_define_unary_function(g2d  )
      exprtk_define_unary_function(notl )
      template <> inline minsky::UnitsExpressionWalker sgn(const minsky::UnitsExpressionWalker x) {return {};}                                 
      exprtk_define_unary_function(erf  )
      exprtk_define_unary_function(erfc )
      exprtk_define_unary_function(ncdf )
      exprtk_define_unary_function_not_dimensionless(frac )
      exprtk_define_unary_function_not_dimensionless(trunc)

      template <> inline minsky::UnitsExpressionWalker sqrt(const minsky::UnitsExpressionWalker x)
      {
        minsky::UnitsExpressionWalker r;
        for (auto& i: x.units)
          {
            if (i.second % 2)
              throw std::runtime_error("input dimension "+i.first+" not a square");
            r.units[i.first]=i.second/2;
          }
        return r;
      }                                 
      
      
      template <> inline int to_int32(const minsky::UnitsExpressionWalker x) {return int(x.value);}
      template <> inline _int64_t to_int64(const minsky::UnitsExpressionWalker x) {return int64_t(x.value);}
      template <> inline bool is_integer(const minsky::UnitsExpressionWalker x) {return int64_t(x.value)==x.value;}
      template <> inline bool is_nan(const minsky::UnitsExpressionWalker x) {return false;}
      
#define exprtk_define_binary_function(Function)                         \
      template <>                                                       \
      inline minsky::UnitsExpressionWalker Function                     \
      (const minsky::UnitsExpressionWalker x, const minsky::UnitsExpressionWalker y) \
      {x.checkSameDims(y); return x;}
      
#define exprtk_define_binary_function_first_arg(Function)                         \
      template <>                                                       \
      inline minsky::UnitsExpressionWalker Function                     \
      (const minsky::UnitsExpressionWalker x, const minsky::UnitsExpressionWalker y) \
      {return x;}
      
#define exprtk_define_binary_function_dimensionless(Function)                         \
      template <>                                                       \
      inline minsky::UnitsExpressionWalker Function                     \
      (const minsky::UnitsExpressionWalker x, const minsky::UnitsExpressionWalker y) \
      {x.checkSameDims(y); return {};}

      exprtk_define_binary_function(min);
      exprtk_define_binary_function(max);
      exprtk_define_binary_function_dimensionless(equal);
      exprtk_define_binary_function_dimensionless(nequal);
      exprtk_define_binary_function_first_arg(modulus);
      exprtk_define_binary_function(pow);
      exprtk_define_binary_function(logn);

      template <> inline minsky::UnitsExpressionWalker root
      (const minsky::UnitsExpressionWalker x, const minsky::UnitsExpressionWalker y)
      {return minsky:: root(x,y);}

      exprtk_define_binary_function_first_arg(roundn);
      exprtk_define_binary_function(hypot);
      exprtk_define_binary_function(atan2);
      exprtk_define_binary_function(shr);
      exprtk_define_binary_function(shl);
      exprtk_define_binary_function(and_opr);
      exprtk_define_binary_function(nand_opr);
      exprtk_define_binary_function(or_opr);
      exprtk_define_binary_function(nor_opr);
      exprtk_define_binary_function(xor_opr);
      exprtk_define_binary_function(xnor_opr);
    }
    
#define exprtk_define_binary_op_impl(Op, def)                           \
    template <>                                                         \
    struct Op##_op<minsky::UnitsExpressionWalker>:                      \
      public opr_base<minsky::UnitsExpressionWalker>                    \
    {                                                                   \
      using T=minsky::UnitsExpressionWalker;                            \
      typedef typename opr_base<T>::Type Type;                          \
      typedef typename opr_base<T>::RefType RefType;                    \
                                                                        \
      static inline T process(Type x, Type y)                           \
      {return def;}                                                     \
      static inline void assign(RefType t1, Type t2)                    \
      {t1=t2;}                                                          \
      static inline typename expression_node<T>::node_type type()       \
      { return expression_node<T>::e_##Op; }                             \
      static inline details::operator_type operation()                  \
      { return details::e_##Op; }                                        \
    };

#define exprtk_define_binary_op(Op,op) exprtk_define_binary_op_impl(Op, x op y)
#define exprtk_define_binary_fun_op(Op,f) exprtk_define_binary_op_impl(Op, f(x,y))
    
    exprtk_define_binary_op( add,+)
    exprtk_define_binary_op( sub,-)
    exprtk_define_binary_op( mul,*)
    exprtk_define_binary_op( div,/)
    exprtk_define_binary_op_impl( mod, x)
    exprtk_define_binary_fun_op( pow, pow)
    exprtk_define_binary_fun_op(  lt,checkSameDims)
    exprtk_define_binary_fun_op( lte,checkSameDims)
    exprtk_define_binary_fun_op(  gt,checkSameDims)
    exprtk_define_binary_fun_op( gte,checkSameDims)
    exprtk_define_binary_fun_op(  eq,checkSameDims)
    exprtk_define_binary_fun_op(  ne,checkSameDims)
    exprtk_define_binary_fun_op( and,checkSameDims)
    exprtk_define_binary_fun_op(nand,checkSameDims)
    exprtk_define_binary_fun_op(  or,checkSameDims)
    exprtk_define_binary_fun_op( nor,checkSameDims)
    exprtk_define_binary_fun_op( xor,checkSameDims)
    exprtk_define_binary_fun_op(xnor,checkSameDims)

    
    }
#undef exprtk_define_unary_function
#undef exprtk_define_binary_function
#undef exprtk_define_binary_op
#undef exprtk_define_binary_fun_op
#undef exprtk_define_binary_fun_op_impl
}
#endif
