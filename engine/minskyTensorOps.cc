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

#include "minsky.h"
#include <classdesc.h>
#include "minskyTensorOps.h"
#include "interpolateHypercube.h"
#include "ravelWrap.h"
#include "minsky_epilogue.h"

using namespace civita;
using namespace boost::gregorian;
using namespace boost::posix_time;

namespace classdesc
{
  // postpone factory definition to TensorOpFactory()
  template <> Factory<ITensor, minsky::OperationType::Type>::Factory() {}
}


namespace minsky
{
  using namespace classdesc;

  TensorOpFactory tensorOpFactory;

  struct DerivativeNotDefined: public std::exception
  {
    const char* what() const throw() {return "Derivative not defined";}
  };
  
  struct TimeOp: public ITensor, public DerivativeMixin
  {
    size_t size() const override {return 1;}
    double operator[](size_t) const override {return EvalOpBase::t;}
    Timestamp timestamp() const override {return {};}
    double dFlow(std::size_t, std::size_t) const override {return 0;}
    double dStock(std::size_t, std::size_t) const override {return 0;}
  };

  // insert a setState virtual call for those that need
  struct SetState
  {
    virtual ~SetState() {}
    virtual void setState(const OperationPtr&)=0;
  };
  
  // Default template calls the regular legacy double function
  template <OperationType::Type op> struct MinskyTensorOp: public civita::ElementWiseOp, public DerivativeMixin, public SetState
  {
    EvalOp<op> eo;
    MinskyTensorOp(): ElementWiseOp([this](double x){return eo.evaluate(x);}) {}
    void setState(const OperationPtr& state) override {eo.state=state;}
    void setArguments(const std::vector<TensorPtr>& a,const Args&) override
    {if (!a.empty()) setArgument(a[0],{"",0});}
    double dFlow(size_t ti, size_t fi) const override {
      auto deriv=dynamic_cast<DerivativeMixin*>(arg.get());
      if (!deriv) throw DerivativeNotDefined();
      if (double df=deriv->dFlow(ti,fi))
        return eo.d1((*arg)[ti])*df;
      return 0;
    }
    double dStock(size_t ti, size_t si) const override {
      auto deriv=dynamic_cast<DerivativeMixin*>(arg.get());
      if (!deriv) throw DerivativeNotDefined();
      if (double ds=deriv->dStock(ti,si))
        return eo.d1((*arg)[ti])*ds;
      return 0;
    }
  };

  template <OperationType::Type op> struct TensorBinOp: civita::BinOp, public DerivativeMixin, public SetState
  {
    EvalOp<op> eo;
    TensorBinOp(): BinOp([this](double x,double y){return eo.evaluate(x,y);}) {}
    void setState(const OperationPtr& state) override {eo.state=state;}
    void setArguments(const TensorPtr& a1, const TensorPtr& a2,
                      const ITensor::Args& args={"",0}) override
    {
      if (!a1 || a1->rank()==0 || !a2 || a2->rank()==0 || a1->hypercube()==a2->hypercube())
        civita::BinOp::setArguments(a1,a2,args);
      else
          {
            // pivot a1, a2 such that common axes are at end (resp beginning)
            auto pivotArg1=make_shared<Pivot>(), pivotArg2=make_shared<Pivot>();
            pivotArg1->setArgument(a1,{});
            pivotArg2->setArgument(a2,{});

            set <string> a2Axes;
            for (auto& xv: a2->hypercube().xvectors) a2Axes.insert(xv.name);

            // compute pivot orders and spread dimensions
            std::vector<string> a1Order, common;
            Hypercube hcSpread1, hcSpread2;
            for (auto& i: a1->hypercube().xvectors)
              if (a2Axes.count(i.name))
                {
                  common.push_back(i.name);
                  a2Axes.erase(i.name);
                }
              else
                {
                  a1Order.push_back(i.name);
                  hcSpread2.xvectors.push_back(i);
                }
            for (auto& xv: a2->hypercube().xvectors)
              if (a2Axes.count(xv.name))
                hcSpread1.xvectors.push_back(xv);

            // append common dimensions to make up a1 final order
            a1Order.insert(a1Order.end(), common.begin(), common.end());
            pivotArg1->setOrientation(a1Order);
            // add in remaining a2 axes to make up a2 order, reusing common.
            common.insert(common.end(), a2Axes.begin(), a2Axes.end());
            pivotArg2->setOrientation(common);

            // now spread pivoted arguments across remaining dimensions
            auto spread1=make_shared<SpreadLast>();
            spread1->setArgument(pivotArg1,{});
            spread1->setSpreadDimensions(hcSpread1);
            auto spread2=make_shared<SpreadFirst>();
            spread2->setArgument(pivotArg2,{});
            spread2->setSpreadDimensions(hcSpread2);

            if (spread1->hypercube()==spread2->hypercube())
              setArguments(spread1, spread2);
            else 
              { // hypercubes not equal, interpolate the second argument
                Hypercube unionHC=spread1->hypercube();
                civita::unionHypercube(unionHC,spread2->hypercube());
                TensorPtr arg1=spread1, arg2=spread2;
                if (unionHC!=spread1->hypercube())
                  {
                    auto interpolate=make_shared<InterpolateHC>();
                    interpolate->hypercube(unionHC);
                    interpolate->setArgument(spread1,{});
                    arg1=interpolate;
                  }
                if (unionHC!=spread2->hypercube())
                  {
                    auto interpolate=make_shared<InterpolateHC>();
                    interpolate->hypercube(unionHC);
                    interpolate->setArgument(spread2,{});
                    arg2=interpolate;
                  }
                civita::BinOp::setArguments(arg1, arg2, args);
              }
          }
    }
    double dFlow(size_t ti, size_t fi) const override {
      auto deriv1=dynamic_cast<DerivativeMixin*>(arg1.get());
      auto deriv2=dynamic_cast<DerivativeMixin*>(arg2.get());
      if (!deriv1 || !deriv2) throw DerivativeNotDefined();
      double r=0;
      if (double df=deriv1->dFlow(ti,fi))
        r += eo.d1((*arg1)[ti])*df;
      if (double df=deriv2->dFlow(ti,fi))
        r += eo.d2((*arg2)[ti])*df;
      return r;
    }
    double dStock(size_t ti, size_t si) const override {
      auto deriv1=dynamic_cast<DerivativeMixin*>(arg1.get());
      auto deriv2=dynamic_cast<DerivativeMixin*>(arg2.get());
      if (!deriv1 || !deriv2) throw DerivativeNotDefined();
      double r=0;
      if (double ds=deriv1->dStock(ti,si))
        r += eo.d1((*arg1)[ti])*ds;
      if (double ds=deriv2->dStock(ti,si))
        r += eo.d2((*arg2)[ti])*ds;
      return r;
    }
  };

  template <OperationType::Type op> struct AccumArgs;

  template <> struct AccumArgs<OperationType::add>: public civita::ReduceArguments
  {
    AccumArgs(): civita::ReduceArguments([](double& x,double y){x+=y;},0) {}
  };
  template <> struct AccumArgs<OperationType::subtract>: public AccumArgs<OperationType::add> {};

  template <> struct AccumArgs<OperationType::multiply>: public civita::ReduceArguments
  {
    AccumArgs(): civita::ReduceArguments([](double& x,double y){x*=y;},1) {}
  };
  template <> struct AccumArgs<OperationType::divide>: public AccumArgs<OperationType::multiply> {};

  template <> struct AccumArgs<OperationType::min>: public civita::ReduceArguments
  {
    AccumArgs(): civita::ReduceArguments([](double& x,double y){if (y<x) x=y;},std::numeric_limits<double>::max()) {}
  };
  template <> struct AccumArgs<OperationType::max>: public civita::ReduceArguments
  {
    AccumArgs(): civita::ReduceArguments([](double& x,double y){if (y>x) x=y;},-std::numeric_limits<double>::max()) {}
  };

  template <> struct AccumArgs<OperationType::and_>: public civita::ReduceArguments
  {
    AccumArgs(): civita::ReduceArguments([](double& x,double y){x*=(y>0.5);},1) {}
  };
  template <> struct AccumArgs<OperationType::or_>: public civita::ReduceArguments
  {
    AccumArgs(): civita::ReduceArguments([](double& x,double y){if (y>0.5) x=1;},0) {}
  };

  
  
  template <OperationType::Type op> struct MultiWireBinOp: public TensorBinOp<op>
  {
    using TensorBinOp<op>::setArguments;
    void setArguments(const std::vector<TensorPtr>& a1,
                      const std::vector<TensorPtr>& a2,
                      const ITensor::Args&) override
    {
      TensorPtr pa1, pa2;
      if (a1.size()==1)
        pa1=a1[0];
      else
        {
          pa1 = make_shared<AccumArgs<op>>();
          pa1->setArguments(a1,{"",0});
        }

      if (a2.size()==1)
        pa2=a2[0];
      else
        {
          pa2 = make_shared<AccumArgs<op>>();
          pa2->setArguments(a2,{"",0});
        }
      
      TensorBinOp<op>::setArguments(pa1, pa2,{"",0});
    }
  };
   
  template <OperationType::Type op> struct GeneralTensorOp;
                                                                                      
  namespace
  {
    template <int I, int J>
    struct is_equal {const static bool value=I==J;};
  }

  //register factory functions for all binary ops
  template <template<OperationType::Type> class T, int op, int to>
  typename classdesc::enable_if<Not<is_equal<op, to>>, void>::T
  registerOps(TensorOpFactory& tensorOpFactory)
  {
    tensorOpFactory.registerType<T<OperationType::Type(op)>>(OperationType::Type(op));
    registerOps<T, op+1, to>(tensorOpFactory);
  }

  template <template<OperationType::Type> class T, int op, int to>
  typename classdesc::enable_if<is_equal<op, to>, void>::T
  registerOps(TensorOpFactory& tensorOpFactory)
  {}  //terminates recursion

  TensorOpFactory::TensorOpFactory()
  {
    registerType<TimeOp>(OperationType::time);
    registerOps<MinskyTensorOp, OperationType::euler, OperationType::add>(*this);
    registerOps<MultiWireBinOp, OperationType::add, OperationType::log>(*this); 
    registerOps<TensorBinOp, OperationType::log, OperationType::copy>(*this);   
    registerOps<MinskyTensorOp, OperationType::copy, OperationType::sum>(*this);
    registerOps<GeneralTensorOp, OperationType::sum, OperationType::numOps>(*this);
  }
                                                                                    
  template <> struct GeneralTensorOp<OperationType::sum>: public civita::Sum {};
  template <> struct GeneralTensorOp<OperationType::product>: public civita::Product {};
  template <> struct GeneralTensorOp<OperationType::infimum>: public civita::Min {};
  template <> struct GeneralTensorOp<OperationType::supremum>: public civita::Max {};
  template <>
  struct GeneralTensorOp<OperationType::any>: public civita::ReductionOp
  {
    GeneralTensorOp(): civita::ReductionOp([](double& x, double y,size_t){if (y>0.5) x=1;},0){}
   };
  template <>
  struct GeneralTensorOp<OperationType::all>: public civita::ReductionOp
  {
    GeneralTensorOp(): civita::ReductionOp([](double& x, double y,size_t){x*=(y>0.5);},1){}
   };

  template <>
  struct GeneralTensorOp<OperationType::runningSum>: public civita::Scan
  {
    GeneralTensorOp(): civita::Scan([](double& x,double y,size_t){x+=y;}) {}
  };

  template <>
  struct GeneralTensorOp<OperationType::runningProduct>: public civita::Scan
  {
    GeneralTensorOp(): civita::Scan([](double& x,double y,size_t){x*=y;}) {}
  };
  
  template <>
  struct GeneralTensorOp<OperationType::difference>: public civita::DimensionedArgCachedOp
  {
    ssize_t delta=0;
    size_t innerStride=1, outerStride;
    vector<size_t> argIndices;
    void setArgument(const TensorPtr& a,const ITensor::Args& args) override {
      civita::DimensionedArgCachedOp::setArgument(a,args);
      if (dimension>=rank() && rank()>1)
        throw error("axis name needs to be specified in difference operator");
      
      delta=args.val;
      // remove initial slice of hypercube
      auto hc=arg->hypercube();
      if (rank()==0) return;
      
      auto& xv=hc.xvectors[rank()==1? 0: dimension];
      if (size_t(abs(delta))>=xv.size())
        throw std::runtime_error("Δ ("+to_string(abs(delta))+") larger than dimension size ("+to_string(xv.size())+")");
      if (delta>=0)
        xv.erase(xv.begin(), xv.begin()+delta);
      else 
        xv.erase(xv.end()+delta, xv.end());
      cachedResult.hypercube(std::move(hc));
      
      // determine offset in hypercube space
      auto dims=arg->hypercube().dims();
      innerStride=1;
      if (dimension<dims.size())
        {
          for (size_t i=0; i<dimension; ++i)
            {
              delta*=dims[i];
              innerStride*=dims[i];
            }
          outerStride=innerStride*dims[dimension];
        }
      else
        outerStride=arg->hypercube().numElements();
      auto idx=arg->index();
      set<size_t> idxSet(idx.begin(),idx.end()), newIdx;
      for (auto& i: idx)
        {
          // strip of any indices outside the output range
          auto t=ssize_t(i)-delta;
          if (t>=0 && t<ssize_t(arg->hypercube().numElements()) && idxSet.count(t) && sameSlice(t,i))
            {
              argIndices.push_back(t);
              newIdx.insert(hypercube().linealIndex(arg->hypercube().splitIndex(t)));
              assert(argIndices.size()==newIdx.size());
            }
        }
      cachedResult.index(Index(newIdx));
    }

    bool sameSlice(size_t i, size_t j) const
    {
      return rank()<=1 || (i%innerStride==j%innerStride && i/outerStride==j/outerStride);
    }
    
    void computeTensor() const override
    {
      if (!argIndices.empty())
        {
          assert(argIndices.size()==size());
          size_t idx=0;
          for (auto i: argIndices)
            {
              auto t=i+delta;
              cachedResult[idx++]=arg->atHCIndex(t)-arg->atHCIndex(i);
            }
        }
      else if (delta>=0)
        for (size_t i=0; i<size(); ++i)
          {
            auto ai=arg->hypercube().linealIndex(hypercube().splitIndex(i));
            auto t=ai+delta;
            if (sameSlice(t, ai))
              cachedResult[i]=arg->atHCIndex(t)-arg->atHCIndex(ai);
            else
              cachedResult[i]=nan("");
          }
      else // with -ve delta, origin of result is shifted
        for (size_t i=0; i<size(); ++i)
          {
            auto ai=arg->hypercube().linealIndex(hypercube().splitIndex(i));
            auto t=ai-delta;
            if (sameSlice(t,ai))
              cachedResult[i]=arg->atHCIndex(ai)-arg->atHCIndex(t);
            else
              cachedResult[i]=nan("");
          }
    }

  };
  
  template <>
  struct GeneralTensorOp<OperationType::innerProduct>: public civita::CachedTensorOp
  {
    std::shared_ptr<ITensor> arg1, arg2;
    void computeTensor() const override {//TODO: tensors of arbitrary rank

      if (!arg1 || !arg2) return;
      size_t m=1, n=1;   
      if (arg1->rank()>1)
        for (size_t i=0; i<arg1->rank()-1; i++)
          m*=arg1->hypercube().dims()[i];
          
      if (arg2->rank()>1)
        for (size_t i=1; i<arg2->rank(); i++)
          n*=arg2->hypercube().dims()[i];     
  	
      size_t stride=arg2->rank()>0? arg2->hypercube().dims()[0]: 1;	 	 
      double tmpSum;
      for (size_t i=0; i< m; i++)
        for (size_t j=0; j< n; j++)
          {
            tmpSum=0;
            for (size_t k=0; k<stride; k++)  
              {
                auto v1=m>1? arg1->atHCIndex(k*m+i) : (*arg1)[k];  
                auto v2=n>1? arg2->atHCIndex(j*stride + k) : (*arg2)[k];  
                if (!isnan(v1) && !isnan(v2)) tmpSum+=v1*v2;
              }
            cachedResult[i+m*j]=tmpSum;
          }
    }
    Timestamp timestamp() const override {return max(arg1? arg1->timestamp(): Timestamp(), arg2? arg2->timestamp(): Timestamp());}
    void setArguments(const TensorPtr& a1, const TensorPtr& a2,
                      const Args&) override {
      arg1=a1; arg2=a2;
      if (arg1 && arg1->rank()!=0 && arg2 && arg2->rank()!=0) {
        if (arg1->hypercube().dims()[arg1->rank()-1]!=arg2->hypercube().dims()[0])
          throw std::runtime_error("inner dimensions of tensors do not match");
        
        auto xv1=arg1->hypercube().xvectors, xv2=arg2->hypercube().xvectors;
        Hypercube hc;
        hc.xvectors.insert(hc.xvectors.begin(), xv2.begin()+1, xv2.end());        
        hc.xvectors.insert(hc.xvectors.begin(), xv1.begin(), xv1.end()-1);
        cachedResult.hypercube(std::move(hc));
                
      }
    }    
  };

  template <>
  struct GeneralTensorOp<OperationType::outerProduct>: public civita::CachedTensorOp
  {
    std::shared_ptr<ITensor> arg1, arg2;
    void computeTensor() const override {
      if (!arg1 || !arg2) return;
      size_t m=arg1->size(), n=arg2->size();   
      assert(cachedResult.size()==m*n);
	
      for (size_t i=0; i< m; i++)
       {
         auto v1=(*arg1)[i];  			
         for (size_t j=0; j< n; j++) 
         {
            auto v2=(*arg2)[j];			
            cachedResult[i+j*m]=v1*v2;			
 	 }
       }	     
    }
    Timestamp timestamp() const override
    {return max(arg1? arg1->timestamp(): Timestamp(), arg2? arg2->timestamp(): Timestamp());}
    void setArguments(const TensorPtr& a1, const TensorPtr& a2,
                      const Args&) override {
      arg1=a1; arg2=a2;
      if (!arg1 || !arg2) return;
      
      set<size_t> newIdx;
      size_t stride=arg1->hypercube().numElements();

      vector<size_t> idx1(arg1->index().begin(), arg1->index().end()), idx2(arg2->index().begin(), arg2->index().end());
      if (!idx1.empty() || !idx2.empty())
        {
          if (idx1.empty()) // dense arg1, generate a corresponding sparse index vector
            for (size_t i=0; i<arg1->size(); ++i)
              idx1.push_back(i);
          if (idx2.empty()) // dense arg2, generate a corresponding sparse index vector
            for (size_t i=0; i<arg2->size(); ++i)
              idx2.push_back(i);
          
          for (auto& i: idx1)
            for (auto& j: idx2) 
              newIdx.insert(i+stride*j);
         
          cachedResult.index(Index(newIdx));
        }

      auto xv1=arg1->hypercube().xvectors, xv2=arg2->hypercube().xvectors;
      Hypercube hc;
      hc.xvectors.insert(hc.xvectors.begin(), xv2.begin(), xv2.end());         
      hc.xvectors.insert(hc.xvectors.begin(), xv1.begin(), xv1.end());           
      cachedResult.hypercube(std::move(hc));
        
      
    }      
  };

  template <>
  struct GeneralTensorOp<OperationType::index>: public civita::CachedTensorOp
  {
    std::shared_ptr<ITensor> arg;
    void computeTensor() const override {
      size_t i=0, j=0;
      for (; i<arg->size(); ++i)
        if ((*arg)[i]>0.5)
          cachedResult[j++]=i;
      for (; j<cachedResult.size(); ++j)
        cachedResult[j]=nan("");
    }
    void setArgument(const TensorPtr& a, const Args&) override {
      arg=a; cachedResult.index(a->index()); cachedResult.hypercube(a->hypercube());
    }
    
    Timestamp timestamp() const override {return arg? arg->timestamp(): Timestamp();}
  };

  template <>
  struct GeneralTensorOp<OperationType::gather>: public civita::CachedTensorOp
  {
    std::shared_ptr<ITensor> arg1, arg2;
    size_t dimension=numeric_limits<size_t>::max();
    std::vector<size_t> offsets; // offsets for each 1D slice of the tensor
    
    double interpolateString(double idx, size_t stride, size_t offset) const
    {
      if (arg1->size()==0)
        throw std::runtime_error("No data to interpolate");
      auto maxIdx=arg1->rank()==1? arg1->size()-1: arg1->hypercube().xvectors[dimension].size()-1;
      if (idx<=-1 || idx>maxIdx)
        return nan("");
      if (idx==maxIdx)
        return arg1->atHCIndex(idx*stride+offset);
      // expand range to skip over any nan in arg1
      unsigned lesser=idx, greater=idx+1;
      double lv=arg1->atHCIndex(lesser*stride+offset), gv=arg1->atHCIndex(greater*stride+offset);
      for (; lesser>0 && isnan(lv); --lesser, lv=arg1->atHCIndex(lesser*stride+offset));
      for (; greater<maxIdx && isnan(gv); ++greater, gv=arg1->atHCIndex(greater*stride+offset));
      double s=(idx-lesser)/(greater-lesser);
      // special cases to avoid unncessarily including nans/infs in the computation
      if (s==0) return lv;
      if (s==1) return gv;
      return (1-s)*lv + s*gv;
    }

    double interpolateAny(const XVector& xv, const civita::any& x, size_t stride, size_t offset) const
    {
      if (xv.size()<2 || diff(x,xv.front())<0 || diff(x,xv.back())>0)
        return nan("");
      auto i=xv.begin();
      for (; diff(x,*(i+1))>0; ++i); // loop will terminate b/c diff(x,xv.back())<=0
      // expand i & i+1 to finite values on the hypercube, if possible
      auto greater=i+1;
      double lv=arg1->atHCIndex((i-xv.begin())*stride+offset);
      double gv=arg1->atHCIndex((greater-xv.begin())*stride+offset);
      for (; i>xv.begin() && isnan(lv); --i, lv=arg1->atHCIndex((i-xv.begin())*stride+offset));
      for (; greater<xv.end()-1 && isnan(gv); ++greater, gv=arg1->atHCIndex((greater-xv.begin())*stride+offset));
      double s=diff(x,*i)/diff(*greater,*i);
      // special cases to avoid unncessarily including nans/infs in the computation
      if (s==0) return lv;
      if (s==1) return gv;
      return (1-s)*lv + s*gv;
    }

    void computeTensor() const override
    {
      size_t d=dimension;
      if (arg1 && d>=arg1->rank()) d=0;
      if (!arg1 || arg1->hypercube().xvectors.size()<=d) return;
      auto& xv=arg1->hypercube().xvectors[d];
      function<double(double,size_t)> interpolate;
      
      size_t stride=1;
      auto dims=arg1->hypercube().dims();
      for (size_t i=0; i<d; ++i)
        stride*=dims[i];
      
      switch (xv.dimension.type)
        {
        case Dimension::string:
          interpolate=[&](double x, size_t offset){
            return interpolateString(x,stride,offset);};
          break;
        case Dimension::time:
          interpolate=[&](double x, size_t offset){
            // interpret as "year" in a common era date (Gregorian calendar)
            int year=x;
            int daysInYear=(date(year+1,Jan,1)-date(year,Jan,1)).days();
            double dayInYearF=daysInYear*(x-year);
            int dayInYear=dayInYearF;
            ptime xtime(date(year,Jan,1)+date_duration(dayInYear), seconds(int(3600*24*(dayInYearF-dayInYear))));
            return interpolateAny(xv, xtime, stride, offset);
          };
          break;
        case Dimension::value:
          interpolate=[&](double x, size_t offset){
            return interpolateAny(xv,x,stride,offset);};
          break;
        }

      for (size_t j=0; j<offsets.size(); ++j)
        for (size_t i=0; i<arg2->size(); ++i)
          {
            auto idx=(*arg2)[i];
            if (isfinite(idx))
              {
                assert(i+arg2->size()*j<cachedResult.size());
                cachedResult[i+arg2->size()*j]=interpolate(idx, offsets[j]);
              }
            else
              cachedResult[i]=nan("");
        }
    }
    Timestamp timestamp() const override {return max(arg1? arg1->timestamp(): Timestamp(), arg2? arg2->timestamp(): Timestamp());}
    void setArguments(const TensorPtr& a1, const TensorPtr& a2,
                      const Args& args) override {
      
      arg1=a1; arg2=a2;
      if (!arg1 || !arg2) return;
      auto& xv=arg1->hypercube().xvectors;
      dimension=find_if(xv.begin(), xv.end(), [&](const XVector& i)
                        {return i.name==args.dimension;})-xv.begin();
                        
      switch (arg1->rank())
        {
        case 0:
          throw runtime_error("Cannot apply gather to a scalar");
        case 1:
          dimension=0;
          break;
        default:
          if (dimension>=arg1->rank())
            throw runtime_error("Need to specify which dimension to gather");
          break;
        }
      
      // find reduced dimensions of arg1
      auto arg1Dims=arg1->hypercube().dims();
      size_t lowerStride=1;
      for (size_t i=0; i<dimension; ++i)
        lowerStride*=arg1Dims[i];
      size_t upperStride=1;
      for (size_t i=dimension+1; i<arg1Dims.size(); ++i)
        upperStride*=arg1Dims[i];

      // calculate offsets
      set<size_t> offsetSet;
      if (arg1->index().empty()) //dense case
        {
          size_t numLower=dimension? arg1Dims[dimension-1]: 1;
          for (size_t i=0; i<upperStride; ++i)
            for (size_t j=0; j<numLower; ++j)
              offsetSet.insert(lowerStride*i*arg1Dims[dimension]+j);
        }
      else
        for (auto i: arg1->index())
          {
            auto splitted=arg1->hypercube().splitIndex(i);
            splitted[dimension]=0;
            offsetSet.insert(arg1->hypercube().linealIndex(splitted));
          }
      offsets.clear(); offsets.insert(offsets.end(), offsetSet.begin(), offsetSet.end());

      // resulting hypercube is a tensor product of arg2 and the reduced arg1.
      size_t arg2NumElements=arg2->hypercube().numElements();
      Hypercube hc=arg2->hypercube();
      hc.xvectors.insert(hc.xvectors.end(), arg1->hypercube().xvectors.begin(), arg1->hypercube().xvectors.begin()+dimension);
      hc.xvectors.insert(hc.xvectors.end(), arg1->hypercube().xvectors.begin()+dimension+1, arg1->hypercube().xvectors.end());

      if (!arg1->index().empty() || !arg2->index().empty())
        {
          vector<size_t> arg1Idx(arg1->index().begin(), arg1->index().end()),
            arg2Idx(arg2->index().begin(), arg2->index().end());

          if (arg1Idx.empty())
            // no need to duplicate elements along the reduced dimension
            for (size_t i=0; i<upperStride; ++i)
              for (size_t j=0; j<lowerStride; ++j)
                arg1Idx.push_back(i*lowerStride*arg1Dims[dimension]+j);
          if (arg2Idx.empty())
            for (size_t i=0; i<arg2->size(); ++i) arg2Idx.push_back(i);
      
          set<size_t> resultantIndex;
          size_t lastOuter=numeric_limits<size_t>::max();
          for (auto i: arg1Idx)
            {
              auto splitIdx=arg1->hypercube().splitIndex(i);
              size_t outerIdx=0, stride=1;
              for (size_t j=0; j<arg1->rank(); ++j)
                if (j!=dimension)
                  {
                    outerIdx+=stride*splitIdx[j];
                    stride*=arg1Dims[j];
                  }
              if (outerIdx==lastOuter) continue;
              lastOuter=outerIdx;
              for (auto j: arg2Idx)
                resultantIndex.insert(outerIdx*arg2NumElements+j);
            }
          cachedResult.index(resultantIndex);
        }

      // relabel any axis that has a duplicate name
      int axisName=0;
      set<string> axisNames;
      for (auto& xv: hc.xvectors)
        if (!axisNames.insert(xv.name).second)
          {
            while (!axisNames.insert(to_string(++axisName)).second); // find a name that hasn't been used
            xv.name=to_string(axisName);
          }
      cachedResult.hypercube(std::move(hc));
    }
      
  };

  template <>
  struct GeneralTensorOp<OperationType::supIndex>: public civita::ReductionOp
  {
    double maxValue; // scratch register for holding current max
    GeneralTensorOp(): civita::ReductionOp
                       ([this](double& r,double x,size_t i){
                          if (i==0 || x>maxValue) {
                            maxValue=x;
                            r=i;
                          }
                        },0) {}
  };
  
  template <>
  struct GeneralTensorOp<OperationType::infIndex>: public civita::ReductionOp
  {
    double minValue; // scratch register for holding current min
    GeneralTensorOp(): civita::ReductionOp
                       ([this](double& r,double x,size_t i){
                          if (i==0 || x<minValue) {
                            minValue=x;
                            r=i;
                          }
                        },0) {}
  };

  namespace {
      // arrange for arguments to be expanded over a common hypercube
    void meldArgsIntoCommonHypercube(vector<TensorPtr>& args) {
      Hypercube hc;
      for (auto& i: args)
        unionHypercube(hc, i->hypercube(), false);

      // list of final dimension names in order
      vector<string> unionDims;
      for (auto& i: hc.xvectors) unionDims.push_back(i.name);

      // create tensor chains for each argument to permute it into the common hypercube
      for (auto& i: args)
        {
          set <string> argDims;
          for (auto& j: i->hypercube().xvectors) argDims.insert(j.name);
          auto spread=make_shared<SpreadLast>();
          Hypercube spreadHC;
          for (auto& j: hc.xvectors)
            if (!argDims.count(j.name))
              spreadHC.xvectors.push_back(j);
          spread->setArgument(i,{});
          spread->setSpreadDimensions(spreadHC);

          auto pivot=make_shared<Pivot>();
          pivot->setArgument(spread,{});
          pivot->setOrientation(unionDims);

          if (pivot->hypercube()==hc)
            i=pivot;
          else
            {
              auto spreadOverHC=make_shared<SpreadOverHC>();
              spreadOverHC->hypercube(hc);
              spreadOverHC->setArgument(pivot,{});
              i=spreadOverHC;
            }
        }
    }
  }
  
  template <>
  struct GeneralTensorOp<OperationType::meld>: public civita::Meld
  {
    using civita::Meld::setArguments;
    void setArguments(const std::vector<TensorPtr>& a1,
                      const std::vector<TensorPtr>& a2,
                      const Args& opArgs) override
    {
      if (a1.empty() && a2.empty()) return;
      vector<TensorPtr> args=a1;
      args.insert(args.end(), a2.begin(), a2.end());
      meldArgsIntoCommonHypercube(args);
      civita::Meld::setArguments(args,opArgs);
    }
  };

  template <>
  struct GeneralTensorOp<OperationType::merge>: public civita::Merge, public SetState
  {
    OperationPtr state;
    using civita::Merge::setArguments;
    void setArguments(const std::vector<TensorPtr>& a1,
                      const std::vector<TensorPtr>& a2,
                      const Args& opArgs) override
    {
      if (a1.empty() && a2.empty()) return;
      vector<TensorPtr> args=a1;
      args.insert(args.end(), a2.begin(), a2.end());
      meldArgsIntoCommonHypercube(args);
      civita::Merge::setArguments(args,opArgs);

      static const char distinctErrorMsg[]="Please use a distinct name for the synthetic dimension produced by this operation";
      if (!hypercube().dimsAreDistinct())
        {
          if (state) state->throw_error(distinctErrorMsg);
          else throw runtime_error(distinctErrorMsg);
        }
      
      // relabel slices along new dimension with variable names if available
      int stream=0;
      for (size_t i=0; state && i<state->portsSize(); ++i)
        if (auto p=state->ports(i).lock())
          if (p->input())
            for (auto w:p->wires())
              if (auto from=w->from())
                {
                  auto v=from->item().variableCast();
                  if (v && !v->name().empty())
                    m_hypercube.xvectors.back()[stream]=v->name();
                  else
                    m_hypercube.xvectors.back()[stream]=to_string(stream);
                  stream++;
                }
    }

    void setState(const OperationPtr& op) override {state=op;}
   };

  class SwitchTensor: public ITensor
  {
    size_t m_size=1;
    vector<TensorPtr> args;
  public:
    void setArguments(const std::vector<TensorPtr>& a,const Args& av={"",0}) override {
      args=a;
      if (args.size()<2)
        hypercube(Hypercube());
      else
        hypercube(args[1]->hypercube());
      
      m_size=1;
      set<size_t> indices;
      for (auto& i: args)
        {
          for (auto& j: i->index())
            indices.insert(j);
          if (i->size()>1)
            {
              if (m_size==1)
                m_size=i->size();
              else if (m_size!=i->size())
                // TODO - should we check and throw on nonconformat hypercubes?
                throw runtime_error("noconformant tensor arguments in switch");
            }
        }
      m_index=indices;
      if (!m_index.empty()) m_size=m_index.size();
    }
    size_t size() const override {return m_size;}
    Timestamp timestamp() const override {
      Timestamp t;
      for (auto& i: args)
        {
          auto tt=i->timestamp();
          if (tt>t) t=tt;
        }
      return t;
    }
    double operator[](size_t i) const override {
      if (args.size()<2) return nan("");

      double selector=0;
      if (args[0])
        {
          if (args[0]->rank()==0) // scalar selector, so broadcast
            selector = (*args[0])[0];
          else
            selector = (*args[0])[i];
        }
      ssize_t idx = selector+1.5; // selector selects between args 1..n
      
      if (idx>0 && idx<int(args.size()))
        {
          if (args[idx]->rank()==0)
            return (*args[idx])[0];
          return args[idx]->atHCIndex(index()[i]);
        }
      return nan("");
    }
  };

  class RavelTensor: public civita::ITensor
  {
    const Ravel& ravel;
    vector<TensorPtr> chain;
    
    CLASSDESC_ACCESS(Ravel);
  public:
    RavelTensor(const Ravel& ravel): ravel(ravel) {}

    void setArgument(const TensorPtr& a,const Args&) override {
      // not sure how to avoid this const cast here
      const_cast<Ravel&>(ravel).populateHypercube(a->hypercube());
      chain=ravel::createRavelChain(ravel.getState(), a);
    }

    double operator[](size_t i) const override {return chain.empty()? 0: (*chain.back())[i];}
    size_t size() const override {return chain.empty()? 1: chain.back()->size();}
    const Index& index() const override
    {return chain.empty()? m_index: chain.back()->index();}
    Timestamp timestamp() const override
    {return chain.empty()? Timestamp(): chain.back()->timestamp();}
    const Hypercube& hypercube() const override {return chain.empty()? m_hypercube: chain.back()->hypercube();}
  };

  namespace
  {
    // used to mark the exception as already dealt with, in terms of displayErrorItem
    struct TensorOpError: public runtime_error
    {
      TensorOpError(const string& msg): runtime_error(msg) {}
    };
  }
  
  std::shared_ptr<ITensor> TensorOpFactory::create
  (const ItemPtr& it, const TensorsFromPort& tfp)
  {
    if (auto ravel=dynamic_cast<const Ravel*>(it.get()))
	    {
	      auto r=make_shared<RavelTensor>(*ravel);
	      r->setArguments(tfp.tensorsFromPorts(*it));
	      return r;
	    }
    if (auto op=it->operationCast())
      try
        {
          TensorPtr r{create(op->type())};
          if (auto ss=dynamic_cast<SetState*>(r.get()))
            ss->setState(dynamic_pointer_cast<OperationBase>(it));
          switch (op->portsSize())
            {
            case 2:
              r->setArguments(tfp.tensorsFromPort(*op->ports(1).lock()),{op->axis,op->arg});
            break;
            case 3:
              r->setArguments(tfp.tensorsFromPort(*op->ports(1).lock()), tfp.tensorsFromPort(*op->ports(2).lock()),{op->axis,op->arg});
              break;
            }
          return r;
        }
      catch (const InvalidType&)
        {return {};}
      catch (const TensorOpError&)
        {throw;}
      catch (const FallBackToScalar&)
        {throw;}
      catch (const std::exception& ex)
        {
          // mark op on canvas, and rethrow as TensorOpError to indicate op is marked
          cminsky().displayErrorItem(*op);
          throw TensorOpError(ex.what());
        }
    else if (auto v=it->variableCast())
      return make_shared<ConstTensorVarVal>(v->vValue(), tfp.ev);
    else if (auto sw=dynamic_cast<const SwitchIcon*>(it.get()))
      {
        auto r=make_shared<SwitchTensor>();
        r->setArguments(tfp.tensorsFromPorts(*it));
        return r;
      }
    else if (auto l=dynamic_cast<const Lock*>(it.get()))
      {
        if (l->locked())
          {
            if (auto r=l->ravelInput())
              if (auto p=r->ports(1).lock())
                {
                  assert(!tfp.tensorsFromPort(*p).empty());
                  auto chain=createRavelChain(l->lockedState, tfp.tensorsFromPort(*p)[0]);
                  if (!chain.empty())
                    return chain.back();
                }
          }
        else
          {
            assert(!tfp.tensorsFromPort(*l->ports(1).lock()).empty());
            return tfp.tensorsFromPort(*l->ports(1).lock())[0];
          }
      }
    return {};
  }

  vector<TensorPtr> TensorsFromPort::tensorsFromPort(const Port& p) const
  {
    if (!p.input()) return {};
    vector<TensorPtr> r;
    for (auto w: p.wires())
      {
        Item& item=w->from()->item();
        if (auto o=item.operationCast())
          {
            if (o->type()==OperationType::differentiate)
              {
                // check if we're differentiating a scalar or tensor
                // expression, and throw accordingly
                auto rhs=tensorsFromPort(*o->ports(1).lock());
                if (rhs.empty() || rhs[0]->size()==1)
                  throw FallBackToScalar();
                // TODO - implement symbolic differentiation of
                // tensor operations
                throw std::runtime_error("Tensor derivative not implemented");
              }
          }
        r.push_back(tensorOpFactory.create(item.itemPtrFromThis(), *this));
        assert(r.back());
      }
    return r;
  }


  vector<TensorPtr> TensorsFromPort::tensorsFromPorts(const Item& item) const
  {
    vector<TensorPtr> r;
    for (size_t i=0; i<item.portsSize(); ++i)
      if (auto p=item.ports(i).lock())
        if (p->input())
          {
            auto tensorArgs=tensorsFromPort(*p);
            r.insert(r.end(), tensorArgs.begin(), tensorArgs.end());
          }
    return r;
  }

  TensorEval::TensorEval(const shared_ptr<VariableValue>& dest, const shared_ptr<VariableValue>& src):
    result(dest,make_shared<EvalCommon>())
  {
    result.index(src->index());
    result.hypercube(src->hypercube());
    OperationPtr tmp(OperationType::copy);
    auto copy=dynamic_pointer_cast<ITensor>(tensorOpFactory.create(tmp));
    copy->setArgument(make_shared<ConstTensorVarVal>(src,result.ev));
    rhs=std::move(copy);
    assert(result.size()==rhs->size());
  }   

  void TensorEval::eval(double fv[], size_t n, const double sv[])
  {
    if (rhs)
      {
        assert(result.idx()>=0);
        bool fvIsGlobalFlowVars=fv==ValueVector::flowVars.data();
        result.index(rhs->index());
        result.hypercube(rhs->hypercube());
        if (fvIsGlobalFlowVars) // hypercube operation may have resized flowVars, invalidating fv
          {
            fv=ValueVector::flowVars.data();
            n=ValueVector::flowVars.size();
          }
        else if (n!=ValueVector::flowVars.size())
          throw FlowVarsResized();
        result.ev->update(fv, n, sv);
        assert(result.size()==rhs->size());
        for (size_t i=0; i<rhs->size(); ++i)
          {
            auto v=(*rhs)[i];
            result[i]=v;
            assert(!isfinite(result[i]) || fv[result.idx()+i]==v);
          }
      }
  }
   
  void TensorEval::deriv(double df[], size_t n, const double ds[],
                         const double sv[], const double fv[])
  {
    if (result.idx()<0) return;
    if (rhs)
      {
        result.ev->update(const_cast<double*>(fv), n, sv);
        if (auto deriv=dynamic_cast<DerivativeMixin*>(rhs.get()))
          {
            assert(result.idx()+rhs->size()<=n);
            for (size_t i=0; i<rhs->size(); ++i)
              {
                df[result.idx()+i]=0;
                for (int j=0; j<result.idx(); ++j)
                  df[result.idx()+i] += df[j]*deriv->dFlow(i,j);
                // skip self variables
                for (size_t j=result.idx()+result.size(); j<ValueVector::flowVars.size(); ++j)
                  df[result.idx()+i] += df[j]*deriv->dFlow(i,j);
                for (size_t j=0; j<ValueVector::stockVars.size(); ++j)
                  df[result.idx()+i] += ds[j]*deriv->dStock(i,j);
              }
          }
      }
  }
}
