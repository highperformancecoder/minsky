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
#include <variableType.h>

#include <memory>

namespace minsky
{
  struct UnitsExpressionWalker
  {
    Units units;
    UnitsExpressionWalker() {}
    UnitsExpressionWalker(double) {}
    UnitsExpressionWalker(const std::string& units): units(units) {}
    
    void checkSameDims(const UnitsExpressionWalker& x) const
    {if (x.units!=units) throw std::runtime_error("Incommensurate units");}
    void checkDimensionless() const
    {if (!units.empty()) throw std::runtime_error("Incommensurate units");}
    UnitsExpressionWalker operator+=(const UnitsExpressionWalker& x)
    {checkSameDims(x); return *this;}
    UnitsExpressionWalker operator+(const UnitsExpressionWalker& x) const
    {checkSameDims(x); return *this;}
    UnitsExpressionWalker operator-=(const UnitsExpressionWalker& x)
    {checkSameDims(x); return *this;}
    UnitsExpressionWalker operator-(const UnitsExpressionWalker& x) const
    {checkSameDims(x); return *this;}

    UnitsExpressionWalker operator*=(const UnitsExpressionWalker& x)
    {
      for (auto& i: x.units)
        units[i.first]+=i.second;
      units.normalise();
      return *this;
    }
    UnitsExpressionWalker operator*(const UnitsExpressionWalker& x) const
    {auto tmp=*this; return tmp*=x;}
    UnitsExpressionWalker operator/=(const UnitsExpressionWalker& x)
    {
      for (auto& i: x.units)
        units[i.first]-=i.second;
      units.normalise();
      return *this;
    }
    UnitsExpressionWalker operator/(const UnitsExpressionWalker& x) const
    {auto tmp=*this; return tmp/=x;}
    UnitsExpressionWalker operator%(const UnitsExpressionWalker& x) const
    {return x;}
    UnitsExpressionWalker operator&&(const UnitsExpressionWalker& x) const
    {checkSameDims(x);return x;}
    UnitsExpressionWalker operator^(const UnitsExpressionWalker& x) const
    {checkSameDims(x);return x;}
    UnitsExpressionWalker operator||(const UnitsExpressionWalker& x) const
    {checkSameDims(x);return x;}
    bool operator<(const UnitsExpressionWalker& x) const
    {checkSameDims(x); return true;}
    bool operator>(const UnitsExpressionWalker& x) const
    {checkSameDims(x); return true;}
    bool operator<=(const UnitsExpressionWalker& x) const
    {checkSameDims(x); return true;}
    bool operator>=(const UnitsExpressionWalker& x) const
    {checkSameDims(x); return true;}
    bool operator==(const UnitsExpressionWalker& x) const
    {checkSameDims(x); return true;}
    bool operator!=(const UnitsExpressionWalker& x) const
    {checkSameDims(x); return true;}
  };

  bool operator>(size_t, const UnitsExpressionWalker&) {return false;}
  
  UnitsExpressionWalker pow(const UnitsExpressionWalker& x, const UnitsExpressionWalker& y)
  {x.checkDimensionless(); y.checkDimensionless(); return {};}
  UnitsExpressionWalker nand(const UnitsExpressionWalker& x, const UnitsExpressionWalker& y)
  {x.checkDimensionless(); y.checkDimensionless(); return {};}
  UnitsExpressionWalker nor(const UnitsExpressionWalker& x, const UnitsExpressionWalker& y)
  {x.checkDimensionless(); y.checkDimensionless(); return {};}
  UnitsExpressionWalker xnor(const UnitsExpressionWalker& x, const UnitsExpressionWalker& y)
  {x.checkDimensionless(); y.checkDimensionless(); return {};}
//  template <class T>
//  UnitsExpressionWalker operator+(const UnitsExpressionWalker& x, T y)
//  {auto tmp=x; tmp+=y; return tmp;}
//  
//  UnitsExpressionWalker operator+(double x, const UnitsExpressionWalker& y)
//  {auto tmp=y; tmp+=x; return tmp;}
 
  
//  struct IWalker
//  {
//    std::shared_ptr<IWalker> clone()=0;
//    void add(const IWalker&)=0;
//    void sub(const IWalker&)=0;
//    void mul(const IWalker&)=0;
//    void div(const IWalker&)=0;
//  };
//  
//  struct ExpressionWalker
//  {
//    std::shared_ptr<IWalker> impl;
//    ExpressionWalker() {}
//    ExpressionWalker(const std::shared_ptr<IWalker>& impl): impl(impl) {}
//    ExpressionWalker operator+(const ExpressionWalker& y)
//    {auto tmp=impl->clone(); tmp->add(y); return tmp;}
//    ExpressionWalker operator+=(const ExpressionWalker& x, const ExpressionWalker& y)
//    {impl->add(y);}
//    ExpressionWalker operator-=(const ExpressionWalker& x, const ExpressionWalker& y)
//    {impl->sub(y);}
//    ExpressionWalker operator*(const ExpressionWalker& y)
//    {auto tmp=impl->clone(); tmp->mul(y); return tmp;}
//    ExpressionWalker operator*=(const ExpressionWalker& x, const ExpressionWalker& y)
//    {impl->mul(y);}
//    ExpressionWalker operator/(const ExpressionWalker& y)
//    {auto tmp=impl->clone(); tmp->div(y); return tmp;}
//  };
//
//  ExpressionWalker operator-(const ExpressionWalker

}

namespace exprtk
{
  namespace details
  {
    inline bool is_true(const minsky::UnitsExpressionWalker& ) {return true;}
  }
}

#include <exprtk/exprtk.hpp>

namespace exprtk
{
  namespace details
  {
    template <>
    inline bool string_to_real(const std::string& s, minsky::UnitsExpressionWalker& t)
    {t.units.clear(); return true;}

    namespace numeric
    {
#define exprtk_define_unary_function(Function)                          \
      template <>                                                       \
      inline minsky::UnitsExpressionWalker Function(const minsky::UnitsExpressionWalker x) \
      {x.checkDimensionless(); return x;}                                 
 
      exprtk_define_unary_function(abs  )
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
      exprtk_define_unary_function(neg  )
      exprtk_define_unary_function(pos  )
      exprtk_define_unary_function(round)
      exprtk_define_unary_function(sin  )
      exprtk_define_unary_function(sinc )
      exprtk_define_unary_function(sinh )
      exprtk_define_unary_function(sqrt )
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
      exprtk_define_unary_function(sgn  )
      exprtk_define_unary_function(erf  )
      exprtk_define_unary_function(erfc )
      exprtk_define_unary_function(ncdf )
      exprtk_define_unary_function(frac )
      exprtk_define_unary_function(trunc)

      template <> inline int to_int32(const minsky::UnitsExpressionWalker) {return 0;}
      template <> inline _int64_t to_int64(const minsky::UnitsExpressionWalker) {return 0;}
      template <> inline bool is_integer(const minsky::UnitsExpressionWalker) {return false;}
      template <> inline bool is_nan(const minsky::UnitsExpressionWalker) {return false;}
      
#define exprtk_define_binary_function(Function)                         \
      template <>                                                       \
      inline minsky::UnitsExpressionWalker Function                     \
      (const minsky::UnitsExpressionWalker x, const minsky::UnitsExpressionWalker y) \
      {x.checkSameDims(y); return x;}

      exprtk_define_binary_function(min);
      exprtk_define_binary_function(max);
      exprtk_define_binary_function(equal);
      exprtk_define_binary_function(nequal);
      exprtk_define_binary_function(modulus);
      exprtk_define_binary_function(pow);
      exprtk_define_binary_function(logn);
      exprtk_define_binary_function(root);
      exprtk_define_binary_function(roundn);
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


//      namespace details
//      {
//        inline bool is_true(const minsky::UnitsExpressionWalker& ) {return true;}
//      }

    }
    
#define exprtk_define_binary_op_impl(Op, def)                                    \
    template <>                                                         \
    struct Op##_op<minsky::UnitsExpressionWalker>:                           \
      public opr_base<minsky::UnitsExpressionWalker>                    \
    {                                                                   \
      using T=minsky::UnitsExpressionWalker;                            \
      typedef typename opr_base<T>::Type Type;                          \
      typedef typename opr_base<T>::RefType RefType;                    \
                                               \
      static inline T process(Type x, Type y)                           \
      {return def;}                                                  \
      static inline void assign(RefType t1, Type t2)                    \
      {t1=t2;}                                                          \
      static inline typename expression_node<T>::node_type type()       \
      { return expression_node<T>::e_and; }                             \
      static inline details::operator_type operation()                  \
      { return details::e_and; }                                        \
    };

#define exprtk_define_binary_op(Op,op) exprtk_define_binary_op_impl(Op, x op y)
#define exprtk_define_binary_fun_op(Op) exprtk_define_binary_op_impl(Op, Op(x,y))
    
    exprtk_define_binary_op( add,+)
    exprtk_define_binary_op( sub,-)
    exprtk_define_binary_op( mul,*)
    exprtk_define_binary_op( div,/)
    exprtk_define_binary_op( mod,%)
    exprtk_define_binary_fun_op( pow)
    exprtk_define_binary_op(  lt,<)
    exprtk_define_binary_op( lte,<=)
    exprtk_define_binary_op(  gt,>)
    exprtk_define_binary_op( gte,>=)
    exprtk_define_binary_op(  eq,==)
    exprtk_define_binary_op(  ne,!=)
    exprtk_define_binary_op( and,&&)
    exprtk_define_binary_fun_op(nand)
    exprtk_define_binary_op(  or,||)
    exprtk_define_binary_fun_op( nor)
    exprtk_define_binary_op( xor,^)
    exprtk_define_binary_fun_op(xnor)

    
    }
#undef exprtk_define_unary_function
#undef exprtk_define_binary_function
#undef exprtk_define_binary_op
#undef exprtk_define_binary_fun_op
#undef exprtk_define_binary_fun_op_impl
}
#endif
