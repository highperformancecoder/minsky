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
#define OPNAMEDEF
#include "cairoItems.h"
#include "dataOp.h"
#include "evalOp.h"
#include "variable.h"
#include "userFunction.h"
#include "minsky.h"
#include "str.h"

#include "minsky_epilogue.h"

#include <math.h>
#undef Complex 
#include <boost/math/special_functions/gamma.hpp>
#include <boost/math/special_functions/polygamma.hpp>

using namespace boost::posix_time;
using namespace boost::math;

namespace minsky
{

  void ScalarEvalOp::eval(double fv[], size_t n, const double sv[])
  {
    assert(out>=0);
    switch (numArgs())
      {
      case 0:
        assert(size_t(out)<n);
        fv[out]=evaluate(0,0);
        break;
      case 1:
        assert(out+in1.size()<=n);
        for (unsigned i=0; i<in1.size(); ++i)
          fv[out+i]=evaluate(flow1? fv[in1[i]]: sv[in1[i]], 0);
        break;
      case 2:
        assert(out+in1.size()<=n);
        for (unsigned i=0; i<in1.size(); ++i)
          {
            double x2=0;
            const double* v=flow2? fv: sv;
            for (auto& j: in2[i])
              x2+=j.weight*v[j.idx];
            fv[out+i]=evaluate(flow1? fv[in1[i]]: sv[in1[i]], x2);
          }
        break;
      }

    // This check may well be obsolete. ecolab::Plot will now elide
    // non-finite data, and it appears that the RK solver can
    // integrate through infinities. Leaving the code here for now,
    // just in case we decide to enable it via a user preference.
//    // check for NaNs only on scalars. For tensors, NaNs just means
//    // element not present
//    if (in1.size()==1)
//      for (unsigned i=0; i<in1.size(); ++i)
//        if (!std::isfinite(fv[out+i]))
//          {
//            if (state)
//              cminsky().displayErrorItem(*state);
//            string msg="Invalid: ";
//            if (auto uf=dynamic_cast<UserFunction*>(state.get()))
//              msg+=uf->description()+"(";
//            else
//              msg+=OperationBase::typeName(type())+"(";
//            if (numArgs()>0)
//              msg+=std::to_string(flow1? fv[in1[i]]: sv[in1[i]]);
//            if (numArgs()>1)
//              msg+=","+std::to_string(flow2? fv[in2[i][0].idx]: sv[in2[i][0].idx]);
//            msg+=")";
//            throw runtime_error(msg.c_str());
//          }
  };

  void ScalarEvalOp::deriv(double df[], size_t n, const double ds[],
                     const double sv[], const double fv[])
  {
    assert(out>=0 && size_t(out)<n);
    switch (numArgs())
      {
      case 0:
        df[out]=0;
        return;
      case 1:
        {
          assert((flow1 && in1[0]<ValueVector::flowVars.size()) || 
                 (!flow1 && in1[0]<ValueVector::stockVars.size()));
          double x1=flow1? fv[in1[0]]: sv[in1[0]];
          double dx1=flow1? df[in1[0]]: ds[in1[0]];
          df[out] = dx1!=0? dx1 * d1(x1,0): 0;
          break;
        }
      case 2:
        {
          assert((flow1 && in1[0]<ValueVector::flowVars.size()) || 
                 (!flow1 && in1[0]<ValueVector::stockVars.size()));
          assert((flow2 && in2[0][0].idx<ValueVector::flowVars.size()) || 
                 (!flow2 && in2[0][0].idx<ValueVector::stockVars.size()));
          double x1=flow1? fv[in1[0]]: sv[in1[0]];
          double x2=flow2? fv[in2[0][0].idx]: sv[in2[0][0].idx];
          double dx1=flow1? df[in1[0]]: ds[in1[0]];
          double dx2=flow2? df[in2[0][0].idx]: ds[in2[0][0].idx];
          df[out] = (dx1!=0? dx1 * d1(x1,x2): 0) +
            (dx2!=0? dx2 * d2(x1,x2): 0);
          break;
        }
      }
    if (!std::isfinite(df[out]))
      throw error("Invalid operation detected on a %s operation",
                  OperationBase::typeName(type()).c_str());
  }

  double ConstantEvalOp::evaluate(double in1, double in2) const
  {return value;}
  template <>
  double EvalOp<OperationType::constant>::evaluate(double in1, double in2) const
  {return 0;}
  template <>
  double EvalOp<OperationType::constant>::d1(double x1, double x2) const
  {return 0;}
  template <>
  double EvalOp<OperationType::constant>::d2(double x1, double x2) const
  {return 0;}

  double EvalOpBase::t;
  string EvalOpBase::timeUnit;

  template <>
  double EvalOp<OperationType::time>::evaluate(double in1, double in2) const
  {return t;}
  template <> 
  double EvalOp<OperationType::time>::d1(double x1, double x2) const
  {return 0;}
  template <>
  double EvalOp<OperationType::time>::d2(double x1, double x2) const
  {return 0;}
  
  template <>
  double EvalOp<OperationType::euler>::evaluate(double in1, double in2) const
  {return 2.71828182845904523536028747135266249775724709369995;}
  template <> 
  double EvalOp<OperationType::euler>::d1(double x1, double x2) const
  {return 0;}
  template <>
  double EvalOp<OperationType::euler>::d2(double x1, double x2) const
  {return 0;} 
  
  template <>
  double EvalOp<OperationType::pi>::evaluate(double in1, double in2) const
  {return 3.14159265358979323846264338327950288419716939937510;}
  template <> 
  double EvalOp<OperationType::pi>::d1(double x1, double x2) const
  {return 0;}
  template <>
  double EvalOp<OperationType::pi>::d2(double x1, double x2) const
  {return 0;}      
  
  template <>
  double EvalOp<OperationType::zero>::evaluate(double in1, double in2) const
  {return 0;}
  template <> 
  double EvalOp<OperationType::zero>::d1(double x1, double x2) const
  {return 0;}
  template <>
  double EvalOp<OperationType::zero>::d2(double x1, double x2) const
  {return 0;} 
  
  template <>
  double EvalOp<OperationType::one>::evaluate(double in1, double in2) const
  {return 1;}
  template <> 
  double EvalOp<OperationType::one>::d1(double x1, double x2) const
  {return 0;}
  template <>
  double EvalOp<OperationType::one>::d2(double x1, double x2) const
  {return 0;}     
  
  template <>
  double EvalOp<OperationType::inf>::evaluate(double in1, double in2) const
  {return numeric_limits<double>::max();}
  template <> 
  double EvalOp<OperationType::inf>::d1(double x1, double x2) const
  {return 0;}
  template <>
  double EvalOp<OperationType::inf>::d2(double x1, double x2) const
  {return 0;}
  
  template <>
  double EvalOp<OperationType::percent>::evaluate(double in1, double in2) const
  {return 100.0*in1;}
  template <>
  double EvalOp<OperationType::percent>::d1(double x1, double x2) const
  {return 100.0;}
  template <>
  double EvalOp<OperationType::percent>::d2(double x1, double x2) const
  {return 0;}    
  
  template <>
  double EvalOp<OperationType::copy>::evaluate(double in1, double in2) const
  {return in1;}
  template <>
  double EvalOp<OperationType::copy>::d1(double x1, double x2) const
  {return 1;}
  template <>
  double EvalOp<OperationType::copy>::d2(double x1, double x2) const
  {return 0;}

  template <> double
  EvalOp<OperationType::integrate>::evaluate(double in1, double in2) const
  {throw error("evaluate for integral operator should not be called");}
  template <>
  double EvalOp<OperationType::integrate>::d1(double x1, double x2) const
  {return x1;}
  template <>
  double EvalOp<OperationType::integrate>::d2(double x1, double x2) const
  {return 0;}

  template <> double
  EvalOp<OperationType::differentiate>::evaluate(double in1, double in2) const
  {throw error("evaluate for derivative operator should not be called");}
  template <>
  double EvalOp<OperationType::differentiate>::d1(double x1, double x2) const
  {return x1;}
  template <>
  double EvalOp<OperationType::differentiate>::d2(double x1, double x2) const
  {return 0;}

  template <> double 
  EvalOp<OperationType::data>::evaluate(double in1, double in2) const
  {return state? dynamic_cast<DataOp&>(*state).interpolate(in1): 0;}
  template <> 
  double EvalOp<OperationType::data>::d1(double x1, double x2) const
  {return state? dynamic_cast<DataOp&>(*state).deriv(x1): 0;}
  template <> 
  double EvalOp<OperationType::data>::d2(double x1, double x2) const
  {return 0;}

  template <> double 
  EvalOp<OperationType::ravel>::evaluate(double in1, double in2) const
  {return in1;}
  template <> 
  double EvalOp<OperationType::ravel>::d1(double x1, double x2) const
  {return 1;}
  template <> 
  double EvalOp<OperationType::ravel>::d2(double x1, double x2) const
  {return 0;}

  template <> 
  double EvalOp<OperationType::sqrt>::evaluate(double in1, double in2) const
  {return ::sqrt(fabs(in1));}
  template <>
  double EvalOp<OperationType::sqrt>::d1(double x1, double x2) const
  {return 0.5/::sqrt(fabs(x1));}
  template <>
  double EvalOp<OperationType::sqrt>::d2(double x1, double x2) const
  {return 0;}

  template <>
  double EvalOp<OperationType::exp>::evaluate(double in1, double in2) const
  {return ::exp(in1);}
  template <>
  double EvalOp<OperationType::exp>::d1(double x1, double x2) const
  {return ::exp(x1);}
  template <>
  double EvalOp<OperationType::exp>::d2(double x1, double x2) const
  {return 0;}

  template <>
  double EvalOp<OperationType::ln>::evaluate(double in1, double in2) const
  {return ::log(in1);}
  template <>
  double EvalOp<OperationType::ln>::d1(double x1, double x2) const
  {return 1/x1;}
  template <>
  double EvalOp<OperationType::ln>::d2(double x1, double x2) const
  {return 0;}

  template <>
  double EvalOp<OperationType::log>::evaluate(double in1, double in2) const
  {return ::log(in1)/::log(in2);}
  template <>
  double EvalOp<OperationType::log>::d1(double x1, double x2) const
  {return 1/(x1*::log(x2));}
  template <>
  double EvalOp<OperationType::log>::d2(double x1, double x2) const
  {return -::log(x1)/(x2*sqr(::log(x2)));}

  template <>
  double EvalOp<OperationType::pow>::evaluate(double in1, double in2) const
  {return ::pow(in1,in2);}
  template <>
  double EvalOp<OperationType::pow>::d1(double x1, double x2) const
  {return ::pow(x1,x2)*x2/x1;}
  template <>
  double EvalOp<OperationType::pow>::d2(double x1, double x2) const
  {return ::pow(x1,x2)*::log(x1);}

  template <>
  double EvalOp<OperationType::lt>::evaluate(double in1, double in2) const
  {return in1<in2;}
  template <>
  double EvalOp<OperationType::lt>::d1(double x1, double x2) const
  {throw error("lt cannot be used with an implicit method");}
  template <>
  double EvalOp<OperationType::lt>::d2(double x1, double x2) const
  {return 0;}

  template <>
  double EvalOp<OperationType::le>::evaluate(double in1, double in2) const
  {return in1<=in2;}
  template <>
  double EvalOp<OperationType::le>::d1(double x1, double x2) const
  {throw error("le cannot be used with an implicit method");}
  template <>
  double EvalOp<OperationType::le>::d2(double x1, double x2) const
  {return 0;}

  template <>
  double EvalOp<OperationType::eq>::evaluate(double in1, double in2) const
  {return in1==in2;}
  template <>
  double EvalOp<OperationType::eq>::d1(double x1, double x2) const
  {throw error("eq cannot be used with an implicit method");}
  template <>
  double EvalOp<OperationType::eq>::d2(double x1, double x2) const
  {return 0;}

  template <>
  double EvalOp<OperationType::userFunction>::evaluate(double in1, double in2) const
  {return state? dynamic_cast<UserFunction&>(*state).evaluate(in1,in2): 0;}
  template <>
  double EvalOp<OperationType::userFunction>::d1(double x1, double x2) const
  {throw error("user functions cannot be used with an implicit method");}
  template <>
  double EvalOp<OperationType::userFunction>::d2(double x1, double x2) const
  {throw error("user functions cannot be used with an implicit method");}

  template <>
  double EvalOp<OperationType::min>::evaluate(double in1, double in2) const
  {return std::min(in1,in2);}
  template <>
  double EvalOp<OperationType::min>::d1(double x1, double x2) const
  {return x1<=x2;} // TODO: thow exception if x1==x2?
  template <>
  double EvalOp<OperationType::min>::d2(double x1, double x2) const
  {return x1>x2;}

  template <>
  double EvalOp<OperationType::max>::evaluate(double in1, double in2) const
  {return std::max(in1,in2);}
  template <>
  double EvalOp<OperationType::max>::d1(double x1, double x2) const
  {return x1>x2;} // TODO: thow exception if x1==x2?
  template <>
  double EvalOp<OperationType::max>::d2(double x1, double x2) const
  {return x1<=x2;}

  template <>
  double EvalOp<OperationType::and_>::evaluate(double in1, double in2) const
  {return in1>0.5 && in2>0.5;}
  template <>
  double EvalOp<OperationType::and_>::d1(double x1, double x2) const
  {throw error("and cannot be used with an implicit method");}
  template <>
  double EvalOp<OperationType::and_>::d2(double x1, double x2) const
  {return 0;}

  template <>
  double EvalOp<OperationType::or_>::evaluate(double in1, double in2) const
  {return in1>0.5 || in2>0.5;}
  template <>
  double EvalOp<OperationType::or_>::d1(double x1, double x2) const
  {throw error("or cannot be used with an implicit method");}
  template <>
  double EvalOp<OperationType::or_>::d2(double x1, double x2) const
  {return 0;}

  template <>
  double EvalOp<OperationType::not_>::evaluate(double in1, double in2) const
  {return in1<=0.5;}
  template <>
  double EvalOp<OperationType::not_>::d1(double x1, double x2) const
  {throw error("not cannot be used with an implicit method");}
  template <>
  double EvalOp<OperationType::not_>::d2(double x1, double x2) const
  {return 0;}

  template <>
  double EvalOp<OperationType::sin>::evaluate(double in1, double in2) const
  {return ::sin(in1);}
  template <>
  double EvalOp<OperationType::sin>::d1(double x1, double x2) const
  {return ::cos(x1);}
  template <>
  double EvalOp<OperationType::sin>::d2(double x1, double x2) const
  {return 0;}

  template <>
  double EvalOp<OperationType::cos>::evaluate(double in1, double in2) const
  {return ::cos(in1);}
  template <>
  double EvalOp<OperationType::cos>::d1(double x1, double x2) const
  {return -::sin(x1);}
  template <>
  double EvalOp<OperationType::cos>::d2(double x1, double x2) const
  {return 0;}

  template <>
  double EvalOp<OperationType::tan>::evaluate(double in1, double in2) const
  {return ::tan(in1);}
  template <>
  double EvalOp<OperationType::tan>::d1(double x1, double x2) const
  {return 1/sqr(::cos(x1));}
  template <>
  double EvalOp<OperationType::tan>::d2(double x1, double x2) const
  {return 0;}

  template <>
  double EvalOp<OperationType::asin>::evaluate(double in1, double in2) const
  {return ::asin(in1);}
  template <>
  double EvalOp<OperationType::asin>::d1(double x1, double x2) const
  {return 1/::sqrt(1-sqr(x1));}
  template <>
  double EvalOp<OperationType::asin>::d2(double x1, double x2) const
  {return 0;}

  template <>
  double EvalOp<OperationType::acos>::evaluate(double in1, double in2) const
  {return ::acos(in1);}
  template <>
  double EvalOp<OperationType::acos>::d1(double x1, double x2) const
  {return -1/::sqrt(1-sqr(x1));}
  template <>
  double EvalOp<OperationType::acos>::d2(double x1, double x2) const
  {return 0;}

  template <>
  double EvalOp<OperationType::atan>::evaluate(double in1, double in2) const
  {return ::atan(in1);}
  template <>
  double EvalOp<OperationType::atan>::d1(double x1, double x2) const
  {return 1/(1+sqr(x1));}
  template <>
  double EvalOp<OperationType::atan>::d2(double x1, double x2) const
  {return 0;}

  template <>
  double EvalOp<OperationType::sinh>::evaluate(double in1, double in2) const
  {return ::sinh(in1);}
  template <>
  double EvalOp<OperationType::sinh>::d1(double x1, double x2) const
  {return ::cosh(x1);}
  template <>
  double EvalOp<OperationType::sinh>::d2(double x1, double x2) const
  {return 0;}

  template <>
  double EvalOp<OperationType::cosh>::evaluate(double in1, double in2) const
  {return ::cosh(in1);}
  template <>
  double EvalOp<OperationType::cosh>::d1(double x1, double x2) const
  {return ::sinh(x1);}
  template <>
  double EvalOp<OperationType::cosh>::d2(double x1, double x2) const
  {return 0;}

  template <>
  double EvalOp<OperationType::tanh>::evaluate(double in1, double in2) const
  {return ::tanh(in1);}
  template <>
  double EvalOp<OperationType::tanh>::d1(double x1, double x2) const
  {return 1/sqr(::cosh(x1));}
  template <>
  double EvalOp<OperationType::tanh>::d2(double x1, double x2) const
  {return 0;}

  template <>
  double EvalOp<OperationType::abs>::evaluate(double in1, double in2) const
  {return ::fabs(in1);}
  template <>
  double EvalOp<OperationType::abs>::d1(double x1, double x2) const
  {return (x1<0)? -1: 1;}
  template <>
  double EvalOp<OperationType::abs>::d2(double x1, double x2) const
  {return 0;}

  template <>
  double EvalOp<OperationType::floor>::evaluate(double in1, double in2) const
  {return ::floor(in1);}
  template <>
  double EvalOp<OperationType::floor>::d1(double x1, double x2) const
  {throw error("floor cannot be used with an implicit method");}
  template <>
  double EvalOp<OperationType::floor>::d2(double x1, double x2) const
  {return 0;}

  template <>
  double EvalOp<OperationType::frac>::evaluate(double in1, double in2) const
  {return in1-::floor(in1);}
  template <>
  double EvalOp<OperationType::frac>::d1(double x1, double x2) const
  {throw error("frac cannot be used with an implicit method");}
  template <>
  double EvalOp<OperationType::frac>::d2(double x1, double x2) const
  {return 0;}
  
  template <>
  double EvalOp<OperationType::Gamma>::evaluate(double in1, double in2) const
  {return in1 > 0? boost::math::tgamma(in1) : numeric_limits<double>::max();}
  template <>
  double EvalOp<OperationType::Gamma>::d1(double x1, double x2) const
  {return boost::math::digamma(fabs(x1))*boost::math::tgamma(fabs(x1));}
  template <>
  double EvalOp<OperationType::Gamma>::d2(double x1, double x2) const
  {return 0;} 
  
  template <>
  double EvalOp<OperationType::polygamma>::evaluate(double in1, double in2) const
  {return in1 > 0? boost::math::polygamma(::floor(in2),in1) : numeric_limits<double>::max();}
  template <>
  double EvalOp<OperationType::polygamma>::d1(double x1, double x2) const
  {return boost::math::polygamma(::floor(x2)+1,fabs(x1));}
  template <>
  double EvalOp<OperationType::polygamma>::d2(double x1, double x2) const
  {return 0;}     
  
  template <>
  double EvalOp<OperationType::fact>::evaluate(double in1, double in2) const
  {return in1 > -1? boost::math::tgamma(in1+1) : 1;}
  template <>
  double EvalOp<OperationType::fact>::d1(double x1, double x2) const
  {return x1 > -1? boost::math::polygamma(0,x1+1)*boost::math::tgamma(x1+1) : 0;}
  template <>
  double EvalOp<OperationType::fact>::d2(double x1, double x2) const
  {return 0;}          

  template <>
  double EvalOp<OperationType::add>::evaluate(double in1, double in2) const
  {return in1+in2;}
  template <>
  double EvalOp<OperationType::add>::d1(double x1, double x2) const
  {return 1;}
  template <>
  double EvalOp<OperationType::add>::d2(double x1, double x2) const
  {return 1;}

  template <>
  double EvalOp<OperationType::subtract>::evaluate(double in1, double in2) const
  {return in1-in2;}
  template <>
  double EvalOp<OperationType::subtract>::d1(double x1, double x2) const
  {return 1;}
  template <>
  double EvalOp<OperationType::subtract>::d2(double x1, double x2) const
  {return -1;}

  template <>
  double EvalOp<OperationType::multiply>::evaluate(double in1, double in2) const
  {return in1*in2;}
  template <>
  double EvalOp<OperationType::multiply>::d1(double x1, double x2) const
  {return x2;}
  template <>
  double EvalOp<OperationType::multiply>::d2(double x1, double x2) const
  {return x1;}

  template <>
  double EvalOp<OperationType::divide>::evaluate(double in1, double in2) const
  {return in1/in2;}
  template <>
  double EvalOp<OperationType::divide>::d1(double x1, double x2) const
  {return 1/x2;}
  template <>
  double EvalOp<OperationType::divide>::d2(double x1, double x2) const
  {return -x1/(x2*x2);}


  template <>
  double EvalOp<OperationType::numOps>::evaluate(double in1, double in2) const
  {throw error("calling evaluate() on EvalOp<numOps> invalid");}
  template <>
  double EvalOp<OperationType::numOps>::d1(double x1, double x2) const
  {throw error("calling d1 on EvalOp<numOps> invalid");}
  template <>
  double EvalOp<OperationType::numOps>::d2(double x1, double x2) const
  {throw error("calling d2() on EvalOp<numOps> invalid");}

  namespace {OperationFactory<ScalarEvalOp, EvalOp, OperationType::sum-1> evalOpFactory;}

  ScalarEvalOp* ScalarEvalOp::create(Type op, const ItemPtr& state)
  {
    switch (classify(op))
      {
      case general:
      case binop:
      case constop:
      case function:
        switch (op)
          {
          case constant:
            return new ConstantEvalOp;
          case numOps:
            return nullptr;
          case userFunction:
            if (auto f=dynamic_cast<UserFunction*>(state.get()))
              f->compile();
            [[fallthrough]];
          default:
            auto r=evalOpFactory.create(op);
            r->state=dynamic_pointer_cast<OperationBase>(state);
            return r;
          }
      case reduction:
      case scan:
      case tensor:
        return nullptr; //TODO should we be here?
      }
    assert(false); //shouldn't be here
    return nullptr;
  }

  EvalOpPtr::EvalOpPtr(OperationType::Type op, const ItemPtr& state,
                       VariableValue& to, const VariableValue& from1, const VariableValue& from2)
  {
    auto t=ScalarEvalOp::create(op,state);
    reset(t);
    assert(t->numArgs()==0 || (from1.idx()>=0 && (t->numArgs()==1 || from2.idx()>=0)));

    if (auto f=dynamic_cast<UserFunction*>(state.get()))
      f->compile();

    if (t->numArgs()>0)
      t->in1.push_back(from1.idx());
    if (t->numArgs()>1)
      t->in2.emplace_back(1,EvalOpBase::Support{1,unsigned(from2.idx())});

    if (to.idx()==-1) to.allocValue();
    t->out=to.idx();
    t->flow1=from1.isFlowVar();
    t->flow2=from2.isFlowVar();

  }

}
