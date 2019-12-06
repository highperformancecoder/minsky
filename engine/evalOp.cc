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

#include "minsky_epilogue.h"

#include <math.h>

using boost::any;
using boost::any_cast;
using namespace boost::posix_time;

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
            {
              double x2=0;
              const double* v=flow2? fv: sv;
              for (auto& j: in2[i])
                  x2+=j.weight*v[j.idx];
              fv[out+i]=evaluate(flow1? fv[in1[i]]: sv[in1[i]], x2);
            }
        break;
      }

    // check for NaNs only on scalars. For tensors, NaNs just means
    // element not present                                                             
    if (in1.size()==1)
      for (unsigned i=0; i<in1.size(); ++i)
        if (!isfinite(fv[out+i]))
          {
            if (state)
              cminsky().displayErrorItem(*state);
            string msg="Invalid: "+OperationBase::typeName(type())+"(";
            if (numArgs()>0)
              msg+=std::to_string(flow1? fv[in1[i]]: sv[in1[i]]);
            if (numArgs()>1)
              msg+=","+std::to_string(flow2? fv[in2[i][0].idx]: sv[in2[i][0].idx]);
            msg+=")";
            throw runtime_error(msg.c_str());
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
				if (isfinite(boost::any_cast<double>(j)))  
                  (*this)[i.name][str(j)]=offs;                                    
                offs+=stride;
              }
            stride*=i.size();
            axes.push_back(i.name);
          }
      }
      size_t offset(const vector<pair<string,string>>& x) {                    
        size_t offs=0;
        for (auto& i: x) {
          auto j=find(i.first);
          if (j!=end()) {
            auto k=j->second.find(i.second);
            if (k!=j->second.end())
              offs+=k->second;
            else
              throw error("invalid key");
          }
        }
        return offs;
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

    typedef vector<XVector> VVV;
    void checkAllEntriesPresent(const VVV& x, const VVV& y)                          
    {
      for (auto& i: y)
        if (none_of(x.begin(), x.end(),
                    [&](const XVector& z)
                    {return z.name==i.name;}))
          throw error("invalidly initialised to variable"); 
    }

    struct ComparableBase: any
    {
      ComparableBase(const any& x): any(x) {}
      virtual ~ComparableBase() {}
      virtual bool operator<(const ComparableBase& x) const=0;
      static ComparableBase* create(const any&);
      // converts a string var to an any, using this any type.
      virtual any toAny(const string&) const=0;
    };

    //bool stringCmp(const string& x, const string& y) {return x<y;}
    //bool stringCmp(const char*& x, const string& y) {return x<y;}
    template <class T>
    bool stringCmp(const T& x, const string& y) {throw error("invalid string comparison");}
    
    template <class T>
    class ComparableAny: public ComparableBase
    {
      const T* data;
    public:
      ComparableAny(const any& x): ComparableBase(x) {data=any_cast<T>(this);}
      bool operator<(const ComparableBase& x) const override
      {
        if (auto y=dynamic_cast<const ComparableAny<T>*>(&x))
          return *data<*y->data;
        return false;
      }      
      any toAny(const string&) const override;
    };

    ComparableBase* ComparableBase::create(const any& x) {
      if (any_cast<string>(&x)) return new ComparableAny<string>(x);
      if (any_cast<const char*>(&x)) return new ComparableAny<const char*>(x);
      if (any_cast<double>(&x)) return new ComparableAny<double>(x);
      if (any_cast<ptime>(&x)) return new ComparableAny<ptime>(x);
      assert(false); // shouldn't be here...
      return nullptr;
    }

    template <> any ComparableAny<string>::toAny(const string& x) const {return x;}
    template <> any ComparableAny<const char*>::toAny(const string& x) const {throw error("invalid toAny");} // impossible to define...
    template <> any ComparableAny<double>::toAny(const string& x) const {return stod(x);}
    template <> any ComparableAny<ptime>::toAny(const string& x) const {return sToPtime(x);}
    
    
    struct OrderedPtr: public unique_ptr<ComparableBase>
    {
      OrderedPtr() {}
      OrderedPtr(const any& x): unique_ptr<ComparableBase>(ComparableBase::create(x)) {}
      bool operator<(const OrderedPtr& x) const {return **this<*x;}
    };

    struct Bounds
    {
      string dimName;
      any lesser, greater;
      Bounds() {}
      Bounds(const string& d, const any& l, const any& g):
        dimName(d), lesser(l), greater(g) {}
    };

    struct GetBounds                                                           
    {
      // like an xvector, but sorted so that labels can be quickly found
      map<string, set<OrderedPtr> > xvector;
      GetBounds(const vector<XVector>& xv) {
        for (auto& i: xv)
          xvector.emplace(i.name, set<OrderedPtr>{i.begin(),i.end()});
      }
      
      // returns the two closest values in xvector to the value given by x
      // if x is found exactly, then return x in both the lesser and greater field
      vector<Bounds> operator()(const vector<pair<string,string>>& x) const
      {
        vector<Bounds> r;
        for (auto& i: x)
          {
            auto j=xvector.find(i.first);
            if (j!=xvector.end())
              {
                if (j->second.empty())
                  return {};
                else if (dynamic_cast<ComparableAny<string>*>(j->second.begin()->get()) ||
                         dynamic_cast<ComparableAny<const char*>*>(j->second.begin()->get()))
                  {
                    any label(i.second);
                    auto k=j->second.find(label);
                    if (k==j->second.end()) return {};
                    r.emplace_back(i.first,label,label);
                  }
                else
                  {
                    OrderedPtr val((*j->second.begin())->toAny(i.second));
                    auto k=j->second.lower_bound(val); // first k >= val
                    if (k==j->second.end() && j->second.size()>1)
                      {
                        auto l=--k;
                        --l;
                        r.emplace_back(i.first,**k,**l); // extrapolate from above
                      }
                    else if (val<*k)
                      {
                        if (j->second.size()>1)
                          {
                            if (k==j->second.begin())
                              {
                                auto l=k;
                                ++l;
                                r.emplace_back(i.first,**l,**k); // extrapolate from below
                              }
                            else
                              {
                                auto l=k;
                                --l;
                                r.emplace_back(i.first,**l,**k); // interpolate
                              }
                          }
                        else
                          return {};  // cannot interpolate with one point
                      }
                    else // exact match
                      r.emplace_back(i.first,*val,*val);
                  }
              }
          }
        return r;
      }
    };

    // generate the in1 offset vector for the generic single argument case
    void generic1ArgIndices(EvalOpBase& t, const VariableValue& to, const VariableValue& from)                     
    {
      OffsetMap from1Offsets(from);
      apply(OffsetMap(to), [&](const vector<pair<string,string>>& x)
                           {
                             size_t offs1=from.idx();
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
                             t.in1.push_back(offs1);
                           });
    }
    
  }
 
  EvalOpPtr::EvalOpPtr(OperationType::Type op, const std::shared_ptr<OperationBase>& state,
                       VariableValue& to, const VariableValue& from1, const VariableValue& from2, double fv[], const double sv[])
  {
    
      reset(EvalOpBase::create(op));
      auto t=get();
      assert(t->numArgs()==0 || (from1.idx()>=0 && (t->numArgs()==1 || from2.idx()>=0)));
      t->state=state;
      
      switch (t->numArgs())
        {
        case 2:
          switch (op)
            {
            case gather:
              {
                auto& e=dynamic_cast<EvalOp<gather>&>(*t);
                e.shape=from1.dims();
                if (to.xVector!=from1.xVector)
                  to.setXVector(from1.xVector);
                  
                  //for (size_t i=0; i<from1.numElements(); ++i)                          
                  //  t->in1.push_back(i+from1.idx());          
                          
                // For feature 47
                if (from1.index.empty())   
                  for (size_t i=0; i<from1.numDenseElements(); ++i)                          
                    t->in1.push_back(i+from1.idx());
                else for (size_t i=0; i<from1.numSparseElements; ++i)
					  t->in1.push_back(from1.index[i]+from1.idx());

                auto from2Dims=from2.dims();
                if (from2Dims.size()>0)
                   {
				  // For feature 47	   
                  if (from2.index.empty()) for (size_t i=0; i<from2.numDenseElements(); i+=from2Dims[0])
                    {
                      t->in2.emplace_back();
                      for (size_t j=0; j<from2Dims[0]; ++j)
                         t->in2.back().emplace_back(1,i+j+from2.idx()); 
					}
                    else for (size_t i=0; i<from2.numSparseElements; i+=from2Dims[0])
                        {
						   t->in2.emplace_back();
						   for (size_t j=0; j<from2Dims[0]; ++j)
					             t->in2.back().emplace_back(1,i+from2.index[j]+from2.idx());
						}
                    }
                else
                  t->in2.emplace_back(1,EvalOpBase::Support{1,unsigned(from2.idx())});
              }
//                switch (from1.rank())
//                  {
//                  case 0: break;
//                  case 1:
//                    if (from2.rank()>2)
//                      throw error("index argument needs to be rank 2 or less");
//                    if (from2.rank()==2 && from2.dims()[0]!=from1.rank())
//                      throw error("leading dimension of index argument needs to match input argument rank");
//                    break;
//                  default:
//                    if (from2.rank()!=2)
//                      throw error("index argument needs to be rank 2");
//                    if (from2.dims()[0]!=from1.rank())
//                      throw error("leading dimension of index argument needs to match input argument rank");
//                    break;
//                  
//                
//                // determine stride based on state of the operation argument
//                size_t stride=1;
//                if (state && !state->axis.empty())
//                  for (auto& j: from1.xVector)
//                    {
//                      if (j.name==state->axis)
//                        break;
//                      stride*=j.size();
//                    }
//                for (size_t i=0; i<from1.numElements(); ++i)
//                  t->in1.push_back(i*stride+from1.idx());
//                if (from2.xVector.size()!=1)
//                  throw error("index argument should have rank 1");
//                else
//                  for (unsigned i=0; i<from2.xVector[0].size(); ++i)
//                    t->in2.emplace_back(1,EvalOpBase::Support{1,i+from2.idx()});
//              }
              break;
            default:
              {
                map<string,const XVector&> from2XVectorMap;
                for (auto& i: from2.xVector)
                  from2XVectorMap.emplace(i.name, i);
                for (auto& i: from1.xVector)
                  {
                    auto j=from2XVectorMap.find(i.name);
                    if (j!=from2XVectorMap.end() && j->second.dimension.type!=i.dimension.type)
                      throw error("incompatible dimension type");
                  }
            
                // check that all from2's xvector entries are present in to, and vice versa
                if (&to==&from1) checkAllEntriesPresent(to.xVector, from2.xVector);
                if (&to==&from2) checkAllEntriesPresent(to.xVector, from1.xVector);
                
                // For feature 47            
                if ((from1.numDenseElements()==1 && from2.numDenseElements()==1) || (from1.numSparseElements==1 && from2.numSparseElements==1))
                  {
                    t->in1.push_back(from1.idx());
                    t->in2.emplace_back(1,EvalOpBase::Support{1,unsigned(from2.idx())});
                    if ((to.numDenseElements()>1 || to.numSparseElements>1) && &to!=&from1 && &to!=&from2)                                  
                      to.setXVector(vector<XVector>());
                    break;
                  }

                OffsetMap from1Offsets(from1), from2Offsets(from2);              

                if (to.xVector.empty())                                         
                  {
                    vector<XVector> xv;
                    for (auto& i: from1.xVector)
                      if (i.dimension.type==Dimension::string)
                        {
                          // compute the common intersection of shared dimensions,
                          // otherwise broadcast along the others
                          auto j=from2Offsets.find(i.name);                                  
                          if (j!=from2Offsets.end())
                            {
                              xv.emplace_back(i.name);
                              xv.back().dimension=i.dimension;
                              for (auto& k: i)
                                {
                                  auto l=j->second.find(str(k));
                                  if (l!=j->second.end())
                                    xv.back().push_back(k);
                                }
                            }
                          else
                            xv.push_back(i);
                        }
                      else
                        xv.push_back(i);
                     
                    for (auto& i: from2.xVector)
                      if (!from1Offsets.count(i.name))
                        xv.push_back(i);
                    to.setXVector(move(xv));
                  }
                else
                  to.setXVector(from1.xVector);

                GetBounds from1GetBounds(from1.xVector), from2GetBounds(from2.xVector);
                apply(OffsetMap(to), [&](const vector<pair<string,string>>& x)
                                     { 
                                       t->in1.push_back(from1Offsets.offset(x)+from1.idx());
                                       t->in2.emplace_back();
                                       auto from1Bounds=from1GetBounds(x);
                                       auto from2Bounds=from2GetBounds(x);
                                       double sumD=0;

                                       // loop over edges of a binary hypercube
                                       for (size_t i=0; i<(1ULL<<from2Bounds.size()); ++i)
                                         {
                                           vector<pair<string,string>> key;
                                           // add verbatim key entries along axes only present in from1
                                           for (auto& j: x)
                                             if (!from2XVectorMap.count(j.first))
                                               key.push_back(j);
                        
                                           double weight=1;
                                           for (auto& b: from2Bounds)
                                             {
                                               if (b.lesser.empty()) goto dontAddKey; // string mismatch
                                               auto ref=find_if(from1Bounds.begin(), from1Bounds.end(),
                                                                [&](const Bounds& x) {return x.dimName==b.dimName;});
                                               // multivariate interpolation - eg see Abramowitz & Stegun 25.2.66
                                               if (i&(1ULL<<(&b-&from2Bounds[0]))) // on greater edge
                                                 if (diff(b.lesser,b.greater)==0 || ref==from1Bounds.end())
                                                   // if lesser==greater, then needn't add greater key, as already sharply contained in bounds
                                                   goto dontAddKey;
                                                 else
                                                   {
                                                     key.emplace_back(b.dimName, str(b.greater));
                                                     weight*=diff(ref->lesser, b.lesser) / diff(b.greater,b.lesser);
                                                   }
                                               else
                                                 {
                                                   key.emplace_back(b.dimName, str(b.lesser));
                                                   double d=diff(b.greater,b.lesser);
                                                   if (ref!=from1Bounds.end() && d!=0)
                                                     weight*=1-diff(ref->lesser, b.lesser)/d;
                                                 }
                                             }
                                           if (weight!=0)
                                             t->in2.back().emplace_back(weight, from2Offsets.offset(key)+from2.idx());
                                         dontAddKey:;
                                         }
#ifndef NDEBUG
                                       double sumWeight=0;
                                       for (auto& i: t->in2.back()) sumWeight+=i.weight;
                                       assert(sumWeight-1 < 1e-5);
#endif
                                     });

                break;
              }
            }
//          if (op==gather) // we need to compute the offsets of each slice and place them in the Support::weight attribute
//            {
//              if (t->in1.size()!=t->in2.size())
//                throw error("gather arguments not conformant");
//              size_t stride=1;
//              if (state && !state->axis.empty())
//                for (auto& j: from1.xVector)
//                  {
//                    stride*=j.size();
//                    if (j.name==state->axis)
//                      break;
//                  }
//              for (size_t i=0; i<t->in1.size(); ++i)
//                {
//                  if (t->in2[i].size()!=1)
//                    throw error("gather's arguments must have compatible x-vectors");
//                  t->in2[i][0].weight = (t->in1[i] / stride) * stride;
//                }
//            }
          break;
        case 1:
          switch (OperationType::classify(op))
            {
            case general: case function: 
                if (to.idx()==-1 || to.xVector.empty())
                  to.setXVector(from1.xVector);
                if (to.xVector==from1.xVector) 
                  { 
					// For feature 47  
                    if (from1.index.empty())   
                      for (size_t i=0; i<from1.numDenseElements(); ++i)                          
                        t->in1.push_back(i+from1.idx());
                    else for (size_t i=0; i<from1.numSparseElements; ++i)
				    	  t->in1.push_back(from1.index[i]+from1.idx());
                  }
                else
                  generic1ArgIndices(*t, to, from1);
              break;
            case reduction: case scan:
              {
                // determine stride based on state of the operation argument
                size_t stride=1;
                if (state && !state->axis.empty())
                  for (auto& j: from1.xVector)
                    {
                      if (j.name==state->axis)
                        break;
                      stride*=j.size();
                    }
                      //for (size_t i=0; i<from1.numElements(); ++i)
                      //  t->in1.push_back(i*stride+from1.idx());
                      
                      // For feature 47
                     if (from1.index.empty())   
                       for (size_t i=0; i<from1.numDenseElements(); ++i)                          
                         t->in1.push_back(i*stride+from1.idx());
                     else for (size_t i=0; i<from1.numSparseElements; ++i)
				     	  t->in1.push_back(from1.index[i]*stride+from1.idx());					  					  
              }
              break;
            case binop: assert(false); break; // shouldn't be here
            case tensor:
              switch (op)
                {
                case index:
                  {  
					vector<unsigned> targetDims;  
                    if (from1.index.empty()) targetDims={unsigned(from1.rank()),unsigned(from1.numDenseElements())};
                    else targetDims={unsigned(from1.rank()),unsigned(from1.numSparseElements)};
                    if (to.dims()!=targetDims)
                      to.dims(targetDims);
                    
                    //for (size_t i=0; i < targetDims[1]; ++i)     // from1.numElements()
                    //  t->in1.push_back(i+from1.idx());
                    
                    
                    // For feature 47
                    if (from1.index.empty())   
                      for (size_t i=0; i<from1.numDenseElements(); ++i)                          
                        t->in1.push_back(i+from1.idx());
                    else for (size_t i=0; i<from1.numSparseElements; ++i)
				    	  t->in1.push_back(from1.index[i]+from1.idx());

                    auto& e=dynamic_cast<EvalOp<index>&>(*t);
                    e.shape=from1.dims();
                  }
                  break;
                default: // TODO
                  break;
                }
              break;  
            }
          break;
        }

      if (to.idx()==-1) to.allocValue();
#ifndef NDEBUG
      switch (OperationType::classify(op))
        {
        case general: case binop: case function: case scan:
          assert(t->numArgs()<1 || to.numDenseElements()==t->in1.size() || to.numSparseElements==t->in1.size());
          assert(t->numArgs()<2 || to.numDenseElements()==t->in2.size() || to.numSparseElements==t->in2.size());
          break;
        case reduction:
          assert(t->numArgs()==1 && (to.numDenseElements()==1 || to.numSparseElements==1));
          break;
        case tensor:
          break;
        }
#endif
      t->out=to.idx();
      t->flow1=from1.isFlowVar();
      t->flow2=from2.isFlowVar();

    }

  template<OperationType::Type T>
  void ReductionEvalOp<T>::eval(double fv[], const double sv[])
  {
    fv[this->out]=init();
    const double* src=this->flow1? fv: sv;
    for (auto i: this->in1)
      accum(fv[this->out], src[i]);
  }

  template<OperationType::Type T>
  void ScanEvalOp<T>::eval(double fv[], const double sv[])                                      
  {
    // input vector assumed to be consecutive locations starting at in1[0]                                 
    const double* src=this->flow1? &fv[this->in1[0]]: &sv[this->in1[0]];
    for (size_t i0=0; i0<this->in1.size(); i0+=dimSz*stride) // loop over outer dimensions
      for (size_t i=i0; i<i0+stride; ++i) // loop over inner dimensions
        for (size_t j=0; j<dimSz; j++) // loop over dimension being scanned
          {
            double s=src[i+j*stride];
            size_t k0= j>window? j-window: 0;
            for (size_t k=k0; k<j; k++) 
              accum(s, src[i+k*stride]);
            fv[this->out+i+j*stride]=s;
          }
  }

  namespace {
    inline vector<unsigned> unravelIndex(const vector<unsigned>& shape, size_t i)    
    {
      vector<unsigned> r;
      size_t stride=1;
      for (auto d: shape)
        {
          r.push_back(i%d);
          i/=d;
        }
      return r;
    }
  }
  
  void EvalOp<minsky::OperationType::index>::eval(double fv[], const double sv[])   
  {
    const double* src=this->flow1? fv: sv;
    size_t o=out;
    for (size_t i=0; i<in1.size(); ++i)
      if (src[in1[i]]>0.5)
        for (auto j: unravelIndex(shape,i))
          fv[o++]=j;

    //pad with NaNs to indicate invalid data
    for (; o<out+in1.size()*shape.size(); ++o)
      fv[o]=nan("");
  }

  void EvalOp<minsky::OperationType::infIndex>::eval(double fv[], const double sv[])
  {
    const double* src=this->flow1? fv: sv;
    double m=numeric_limits<double>::max();
    for (size_t i=0; i<in1.size(); ++i)
      if (src[in1[i]]<m)
        {
          m=src[in1[i]];
          fv[out]=i;
        }
  }
  void EvalOp<minsky::OperationType::supIndex>::eval(double fv[], const double sv[])
  {
    const double* src=this->flow1? fv: sv;
    double m=-numeric_limits<double>::max();
    for (size_t i=0; i<in1.size(); ++i)
      if (src[in1[i]]>m)
        {
          m=src[in1[i]];
          fv[out]=i;
        }
  }

  void EvalOp<minsky::OperationType::gather>::eval(double fv[], const double sv[])          
  {
    const double* src=this->flow1? fv: sv;
    const double* idx=this->flow2? fv: sv;
    // prefill with NaNs to indicate invalid data
    for (size_t i=0; i<in1.size(); ++i)
      fv[out+i]=nan("");
    
    for (auto& i: in2)
      {
        double idx1=0;
        size_t stride=1;
        assert(i.size()==shape.size());
        for (size_t j=0; j<i.size(); ++j)
          {
            idx1+=idx[i[j].idx]*stride;
            stride*=shape[j];
          }
        if (isfinite(idx1))
          {
            size_t idx2=idx1;
            fv[out+idx2]=src[in1[idx2]];
          }
      }
  }


}
