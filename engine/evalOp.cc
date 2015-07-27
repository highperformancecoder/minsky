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
#include "evalOp.h"
#include "variable.h"
#include "portManager.h"
#include "minsky.h"
#include "cairoItems.h"
#include "str.h"

#include <ecolab_epilogue.h>

#include <math.h>

#ifdef __MINGW32_VERSION
#define finite isfinite
#endif

namespace
{
  inline double sqr(double x) {return x*x;}
}

namespace minsky
{

  void EvalOpBase::reset()
  {
    if (Constant* c=dynamic_cast<Constant*>(state.get()))
      ValueVector::flowVars[out]=c->value;
  }

  template <> int EvalOp<OperationType::constant>::numArgs() const {return 0;}
  template <> int EvalOp<OperationType::time>::numArgs() const {return 0;}
  template <> int EvalOp<OperationType::copy>::numArgs() const {return 1;}
  template <> int EvalOp<OperationType::integrate>::numArgs() const {return 1;}
  template <> int EvalOp<OperationType::data>::numArgs() const {return 1;}
  template <> int EvalOp<OperationType::sqrt>::numArgs() const {return 1;}
  template <> int EvalOp<OperationType::exp>::numArgs() const {return 1;}
  template <> int EvalOp<OperationType::ln>::numArgs() const {return 1;}
  template <> int EvalOp<OperationType::log>::numArgs() const {return 2;}
  template <> int EvalOp<OperationType::pow>::numArgs() const {return 2;}
  template <> int EvalOp<OperationType::sin>::numArgs() const {return 1;}
  template <> int EvalOp<OperationType::cos>::numArgs() const {return 1;}
  template <> int EvalOp<OperationType::tan>::numArgs() const {return 1;}
  template <> int EvalOp<OperationType::asin>::numArgs() const {return 1;}
  template <> int EvalOp<OperationType::acos>::numArgs() const {return 1;}
  template <> int EvalOp<OperationType::atan>::numArgs() const {return 1;}
  template <> int EvalOp<OperationType::sinh>::numArgs() const {return 1;}
  template <> int EvalOp<OperationType::cosh>::numArgs() const {return 1;}
  template <> int EvalOp<OperationType::tanh>::numArgs() const {return 1;}
  template <> int EvalOp<OperationType::abs>::numArgs() const {return 1;}
  template <> int EvalOp<OperationType::heaviside>::numArgs() const {return 1;}
  template <> int EvalOp<OperationType::add>::numArgs() const {return 2;}
  template <> int EvalOp<OperationType::subtract>::numArgs() const {return 2;}
  template <> int EvalOp<OperationType::multiply>::numArgs() const {return 2;}
  template <> int EvalOp<OperationType::divide>::numArgs() const {return 2;}
  template <> int EvalOp<OperationType::numOps>::numArgs() const {return 0;}


  void EvalOpBase::eval(double fv[], const double sv[])
  {
    switch (numArgs())
      {
      case 0:
        fv[out]=evaluate(0,0);
        break;
      case 1:
        fv[out]=evaluate(flow1? fv[in1]: sv[in1], 0);
        break;
      case 2:
        fv[out]=evaluate(flow1? fv[in1]: sv[in1], flow2? fv[in2]: sv[in2]);
        break;
      }
    if (!isfinite(fv[out]))
      {
        if (state)
          minsky().displayErrorItem(*state);
        string msg="Invalid: "+OperationBase::typeName(type())+"(";
        if (numArgs()>0)
          msg+=str(flow1? fv[in1]: sv[in1]);
        if (numArgs()>1)
          msg+=","+str(flow2? fv[in2]: sv[in2]);
        msg+=")";
        throw error(msg.c_str());
      }
  };

  void EvalOpBase::deriv(double df[], const double ds[],
                     const double sv[], const double fv[])
  {
    assert(out>=0 && size_t(out)<ValueVector::flowVars.size());
    switch (numArgs())
      {
      case 0:
        df[out]=0;
        return;
      case 1:
        {
          assert((flow1 && size_t(in1)<ValueVector::flowVars.size()) || 
                 (!flow1 && size_t(in1)<ValueVector::stockVars.size()));
          double x1=flow1? fv[in1]: sv[in1];
          double dx1=flow1? df[in1]: ds[in1];
          df[out] = dx1!=0? dx1 * d1(x1,0): 0;
          break;
        }
      case 2:
        {
          assert((flow1 && size_t(in1)<ValueVector::flowVars.size()) || 
                 (!flow1 && size_t(in1)<ValueVector::stockVars.size()));
          assert((flow2 && size_t(in2)<ValueVector::flowVars.size()) || 
                 (!flow2 && size_t(in2)<ValueVector::stockVars.size()));
          double x1=flow1? fv[in1]: sv[in1];
          double x2=flow2? fv[in2]: sv[in2];
          double dx1=flow1? df[in1]: ds[in1];
          double dx2=flow2? df[in2]: ds[in2];
          df[out] = (dx1!=0? dx1 * d1(x1,x2): 0) +
            (dx2!=0? dx2 * d2(x1,x2): 0);
          break;
        }
      }
    if (!isfinite(df[out]))
      throw error("Invalid operation detected on a %s operation",
                  OperationBase::typeName(type()).c_str());
  }

  double ConstantEvalOp::evaluate(double in1, double in2) const
  {return value;}
  template <>
  double EvalOp<OperationType::constant>::evaluate(double in1, double in2) const
  {return dynamic_cast<Constant&>(*state).value;}
  template <>
  double EvalOp<OperationType::constant>::d1(double x1, double x2) const
  {return 0;}
  template <>
  double EvalOp<OperationType::constant>::d2(double x1, double x2) const
  {return 0;}

  double EvalOpBase::t;

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
  EvalOp<OperationType::data>::evaluate(double in1, double in2) const
  {return state? dynamic_cast<DataOp&>(*state).interpolate(in1): 0;}
  template <> 
  double EvalOp<OperationType::data>::d1(double x1, double x2) const
  {return state? dynamic_cast<DataOp&>(*state).deriv(x1): 0;}
  template <> 
  double EvalOp<OperationType::data>::d2(double x1, double x2) const
  {return 0;}

  template <> 
  double EvalOp<OperationType::sqrt>::evaluate(double in1, double in2) const
  {return ::sqrt(in1);}
  template <>
  double EvalOp<OperationType::sqrt>::d1(double x1, double x2) const
  {return 0.5/::sqrt(x1);}
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
  double EvalOp<OperationType::heaviside>::evaluate(double in1, double in2) const
  {return (in1==0)? 0.5: (in1<0)? 0: 1;}
  template <>
  double EvalOp<OperationType::heaviside>::d1(double x1, double x2) const
  {return 0;} // TODO: thow exception if x1==0?
  template <>
  double EvalOp<OperationType::heaviside>::d2(double x1, double x2) const
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

  EvalOpBase* EvalOpBase::create
  (Type op, int out, int in1, int in2, bool flow1, bool flow2)
  {
    switch (op)
      {
      case constant:
        return new ConstantEvalOp(out,in1,in2,flow1,flow2);
      case time:
        return new EvalOp<time>(out,in1,in2,flow1,flow2);
      case copy:
        return new EvalOp<copy>(out,in1,in2,flow1,flow2);
      case integrate:
        return new EvalOp<integrate>(out,in1,in2,flow1,flow2);
      case data:
         {
          EvalOpBase* r=new EvalOp<data>(out,in1,in2,flow1,flow2);
          //          r->state.reset(new DataOp); // needed to ensure evaluate and d do not fail
          return r;
         }
      case sqrt:
       return new EvalOp<sqrt>(out,in1,in2,flow1,flow2);
      case exp:
        return new EvalOp<exp>(out,in1,in2,flow1,flow2);
      case ln:
        return new EvalOp<ln>(out,in1,in2,flow1,flow2);
      case log:
        return new EvalOp<log>(out,in1,in2,flow1,flow2);
      case pow:
        return new EvalOp<pow>(out,in1,in2,flow1,flow2);
      case sin:
        return new EvalOp<sin>(out,in1,in2,flow1,flow2);
      case cos:
        return new EvalOp<cos>(out,in1,in2,flow1,flow2);
      case tan:
        return new EvalOp<tan>(out,in1,in2,flow1,flow2);
      case asin:
        return new EvalOp<asin>(out,in1,in2,flow1,flow2);
      case acos:
        return new EvalOp<acos>(out,in1,in2,flow1,flow2);
      case atan:
        return new EvalOp<atan>(out,in1,in2,flow1,flow2);
      case sinh:
        return new EvalOp<sinh>(out,in1,in2,flow1,flow2);
      case cosh:
        return new EvalOp<cosh>(out,in1,in2,flow1,flow2);
      case tanh:
        return new EvalOp<tanh>(out,in1,in2,flow1,flow2);
      case abs:
        return new EvalOp<abs>(out,in1,in2,flow1,flow2);
      case heaviside:
        return new EvalOp<heaviside>(out,in1,in2,flow1,flow2);
      case add:
        return new EvalOp<add>(out,in1,in2,flow1,flow2);
      case subtract:
        return new EvalOp<subtract>(out,in1,in2,flow1,flow2);
      case multiply:
        return new EvalOp<multiply>(out,in1,in2,flow1,flow2);
      case divide:
        return new EvalOp<divide>(out,in1,in2,flow1,flow2);
      case numOps:
        return NULL;
      default:
        throw error("unknown operation type %s", typeName(op).c_str());
      }
  }

  EvalOpPtr::EvalOpPtr(OperationType::Type op,
              int out, int in1, int in2, bool flow1, bool flow2)
  {reset(EvalOpBase::create(op, out, in1, in2, flow1, flow2));}

}
