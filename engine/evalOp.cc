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
#include "evalOp.h"
#include "variable.h"
#include "minsky.h"
#include "str.h"

#include <ecolab_epilogue.h>

#include <math.h>

//#ifdef __MINGW32_VERSION
//#define finite isfinite
//#endif

namespace minsky
{

  void EvalOpBase::eval(double fv[], const double sv[])
  {
    assert(out>=0);
    switch (numArgs())
      {
      case 0:
        fv[out]=evaluate(0,0);
        break;
      case 1:
        for (unsigned i=0; i<in1.size(); ++i)
          fv[out+i]=evaluate(flow1? fv[in1[i]]: sv[in1[i]], 0);
        break;
      case 2:
        for (unsigned i=0; i<in1.size(); ++i)
          fv[out+i]=evaluate(flow1? fv[in1[i]]: sv[in1[i]],
                             flow2? fv[in2[i]]: sv[in2[i]]);
        break;
      }

    // check for NaNs only on scalars. For tensors, NaNs just means
    // element not present
    if (in1.size()==1)
      for (unsigned i=0; i<in1.size(); ++i)
        if (!isfinite(fv[out+i]))
          {
            if (state)
              minsky().displayErrorItem(*state);
            string msg="Invalid: "+OperationBase::typeName(type())+"(";
            if (numArgs()>0)
              msg+=to_string(flow1? fv[in1[i]]: sv[in1[i]]);
            if (numArgs()>1)
              msg+=","+to_string(flow2? fv[in2[i]]: sv[in2[i]]);
            msg+=")";
            throw error(msg.c_str());
          }
  };

  /// TODO: handle tensors for implicit methods
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
          assert((flow2 && in2[0]<ValueVector::flowVars.size()) || 
                 (!flow2 && in2[0]<ValueVector::stockVars.size()));
          double x1=flow1? fv[in1[0]]: sv[in1[0]];
          double x2=flow2? fv[in2[0]]: sv[in2[0]];
          double dx1=flow1? df[in1[0]]: ds[in1[0]];
          double dx2=flow2? df[in2[0]]: ds[in2[0]];
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
  {throw error("ravel evaluation not supported");}
  template <> 
  double EvalOp<OperationType::ravel>::d1(double x1, double x2) const
  {throw error("ravel evaluation not supported");}
  template <> 
  double EvalOp<OperationType::ravel>::d2(double x1, double x2) const
  {throw error("ravel evaluation not supported");}

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

  void RavelEvalOp::eval(double fv[], const double sv[]) 
  {
    if (auto r=dynamic_cast<Ravel*>(state.get()))
      {
        r->loadDataCubeFromVariable(in);
        r->loadDataFromSlice(out);
      }
  }
  
  namespace {OperationFactory<EvalOpBase, EvalOp, OperationType::numOps-1> evalOpFactory;}

  EvalOpBase* EvalOpBase::create(Type op)
  {
    switch (op)
      {
      case constant:
        return new ConstantEvalOp;
      case ravel:
        return new RavelEvalOp;
      case numOps:
        return NULL;
      default:
        return evalOpFactory.create(op);
      }
  }

  namespace
  {
    struct OffsetMap: public map<string,map<string,size_t>>
    {
      vector<string> axes;
      OffsetMap(const VariableValue& v) {
        size_t stride=1;
        for (auto& i: v.xVector)
          {
            size_t offs=0;
            for (auto& j: i)
              {
                (*this)[i.name][j.second]=offs;
                offs+=stride;
              }
            stride*=i.size();
            axes.push_back(i.name);
          }
      }
    };

    template <class F>
    void apply(const OffsetMap& targetOffs, vector<string> axes, vector<pair<string,string>> index, F f)
    {
      if (axes.empty())
        f(index);
      else
        {
          auto j=axes.back();
          auto k=targetOffs.find(j);
          axes.pop_back();
          if (k!=targetOffs.end())
            for (auto& i: k->second)
              {
                index.emplace_back(j,i.first);
                apply(targetOffs, axes, index, f);
                index.pop_back();
              }
        }
    }

    // recursively apply f() to the indices of targetOffs
    template <class F>
    void apply(const OffsetMap& targetOffs, F f)
    {
      apply(targetOffs,targetOffs.axes,{},f);
    }

    typedef vector<VariableValue::XVector> VVV;
    void checkAllEntriesPresent(const VVV& x, const VVV& y)
    {
      for (auto& i: y)
        if (none_of(x.begin(), x.end(),
                    [&](const VariableValue::XVector& z)
                    {return z.name==i.name;}))
          throw error("invalidly initialised to variable"); 
    }
    
  }
 
  EvalOpPtr::EvalOpPtr(OperationType::Type op, VariableValue& to,
              const VariableValue& from1, const VariableValue& from2)
  {
    
      reset(EvalOpBase::create(op));
      auto t=get();
      assert(t->numArgs()==0 || from1.idx()>=0 && (t->numArgs()==1 || from2.idx()>=0));

      // check dimensionality is correct
      switch (op)
        {
        case OperationType::time:
          to.units.clear();
          if (!EvalOpBase::timeUnit.empty())
            to.units.emplace(EvalOpBase::timeUnit,1);
          break;
        case OperationType::multiply:
        case OperationType::divide:
          to.units=from1.units;
          for (auto& i: from2.units)
            {
              to.units[i.first]+=
                (op==OperationType::multiply? 1: -1)*i.second;
              if (to.units[i.first]==0)
                to.units.erase(i.first);
            }
          break;
        case OperationType::pow:
          if (!from1.units.empty())
            if (from2.type()==VariableType::constant)
              {
                char* ep;
                int e=strtol(from2.init.c_str(),&ep,10);
                if (*ep!='\0')
                  throw runtime_error("non integral power of dimensioned quantity requested");
                to.units=from1.units;
                for (auto& i: to.units)
                  i.second*=e;
              }
            else
              throw runtime_error("non constant power of dimensioned quantity requested");
          break;
        case OperationType::le: case OperationType::lt: case OperationType::eq:
          if (from1.units!=from2.units)
            throw runtime_error("incompatible units: "+from1.units.str()+"≠"+from2.units.str());
          to.units.clear(); // result of comparison ops is dimensionless
          break;
        case OperationType::and_: case OperationType::or_: case OperationType::not_:
          if (!from1.units.empty() || !from2.units.empty())
            throw runtime_error("logical ops must be applied to dimensionless quantities");
          to.units.clear(); // result of comparison ops is dimensionless
          break;
        case OperationType::integrate: case OperationType::differentiate:
          assert(false); // shouldn't be here
          break;
        case OperationType::copy:
          to.units=from1.units;
          break;
        default:
          if (t->numArgs()>=2)
            {
              if (from1.units!=from2.units)
                throw runtime_error("incompatible units: "+from1.units.str()+"≠"+from2.units.str());
            }
          else if (t->numArgs()==1 && !from1.units.empty())
            throw runtime_error("function argument not dimensionless, but "+from1.units.str());
          if (t->numArgs()>0)
            to.units=from1.units;
          break;
        }

      switch (t->numArgs())
        {
        case 2:
          {
            // check that all from2's xvector entries are present in to, and vice versa
            if (&to==&from1) checkAllEntriesPresent(to.xVector, from2.xVector);
            if (&to==&from2) checkAllEntriesPresent(to.xVector, from1.xVector);

            if (from1.numElements()==1 && from2.numElements()==1)
              {
                t->in1.push_back(from1.idx());
                t->in2.push_back(from2.idx());
                if (to.numElements()>1 && &to!=&from1 && &to!=&from2)
                  to.xVector.clear();
                break;
              }

            OffsetMap from1Offsets(from1), from2Offsets(from2);

            if (to.xVector.empty())
              // compute the common intersection of shared dimensions,
              // otherwise broadcast along the others
              {
                for (auto& i: from1.xVector)
                  {
                    to.xVector.emplace_back(i.name);
                    auto j=from2Offsets.find(i.name);
                    if (j!=from2Offsets.end())
                      for (auto& k: i)
                        {
                          auto l=j->second.find(k.second);
                          if (l!=j->second.end())
                            to.xVector.back().push_back(k);
                        }
                    else
                      to.xVector.back()=i;
                  }
                for (auto& i: from2.xVector)
                  if (!from1Offsets.count(i.name))
                    to.xVector.push_back(i);
              }

            apply(OffsetMap(to), [&](const vector<pair<string,string>>& x)
                  {
                    size_t offs1=from1.idx(), offs2=from2.idx();
                    for (auto& i: x)
                      {
                        auto j=from1Offsets.find(i.first);
                        if (j!=from1Offsets.end())
                          {
                            auto k=j->second.find(i.second);
                            if (k!=j->second.end())
                              offs1+=k->second;
                            else
                              throw error("invalid key");
                          }
                        // else allowed a missing dimension - add zero offset
                        j=from2Offsets.find(i.first);
                        if (j!=from2Offsets.end())
                          {
                            auto k=j->second.find(i.second);
                            if (k!=j->second.end())
                              offs2+=k->second;
                            else
                              throw error("invalid key");
                          }
                        // else allowed a missing dimension - add zero offset
                      }
                    t->in1.push_back(offs1);
                    t->in2.push_back(offs2);
                  });

            break;
          }
        case 1:
          if (to.idx()==-1 || to.xVector.empty())
            to.xVector=from1.xVector;
          if (to.xVector==from1.xVector)
            for (size_t i=0; i<from1.numElements(); ++i)
              t->in1.push_back(i+from1.idx());
          else
            {
              OffsetMap from1Offsets(from1);
              apply(OffsetMap(to), [&](const vector<pair<string,string>>& x)
                    {
                      size_t offs1=from1.idx();
                      for (auto& i: x)
                        {
                          auto j=from1Offsets.find(i.first);
                          if (j!=from1Offsets.end())
                            {
                              auto k=j->second.find(i.second);
                              if (k!=j->second.end())
                                offs1+=k->second;
                            else
                              throw error("invalid key");
                            }
                        // else allowed a missing dimension - add zero offset
                        }
                      t->in1.push_back(offs1);
                    });
            }
          break;
        }

      if (to.idx()==-1) to.allocValue();
      assert(t->numArgs()<1 || to.numElements()==t->in1.size());
      assert(t->numArgs()<2 || to.numElements()==t->in2.size());
      
      t->out=to.idx();
      t->flow1=from1.isFlowVar();
      t->flow2=from2.isFlowVar();

    }

}
