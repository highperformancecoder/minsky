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

#include <classdesc.h>
#include "minskyTensorOps.h"
#include "interpolateHypercube.h"
#include "minsky.h"
#include "ravelWrap.h"
#include "minsky_epilogue.h"

using namespace civita;
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
  
  struct TimeOp: public ITensor
  {
    size_t size() const override {return 1;}
    double operator[](size_t) const override {return EvalOpBase::t;}
    Timestamp timestamp() const override {return {};}
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
    void setArguments(const std::vector<TensorPtr>& a,const std::string&,double) override
    {if (!a.empty()) setArgument(a[0],{},0);}
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
    void setArguments(const TensorPtr& a1, const TensorPtr& a2) override
    {
      if (a1 && a1->rank()>0)
        hypercube(a1->hypercube());
      else if (a2)
        hypercube(a2->hypercube());
      else
        hypercube(Hypercube());
      
      arg1=a1;

      if (a2)
        {
          if (a2->rank()==0 || a2->hypercube()==hypercube())
            arg2=a2;
          else
            {
              arg2=make_shared<InterpolateHC>();
              arg2->hypercube(hypercube());
              arg2->setArgument(a2);
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
    virtual void setArguments(const std::vector<TensorPtr>& a1,
                              const std::vector<TensorPtr>& a2)
    {
      TensorPtr pa1, pa2;
      if (a1.size()==1)
        pa1=a1[0];
      else
        {
          pa1 = make_shared<AccumArgs<op>>();
          pa1->setArguments(a1,{},0);
        }

      if (a2.size()==1)
        pa2=a2[0];
      else
        {
          pa2 = make_shared<AccumArgs<op>>();
          pa2->setArguments(a2,{},0);
        }
      
      TensorBinOp<op>::setArguments(pa1, pa2);
    }
  };
   
//  template <minsky::OperationType::Type T>
//  struct ReductionTraits
//  {
//    /// x op= y
//    static void accum(double& x, double y);
//    static const double init;
//  };

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
    void setArgument(const TensorPtr& a,const std::string& s,double d) override {
      civita::DimensionedArgCachedOp::setArgument(a,s,d);
      if (dimension>=rank() && rank()>1)
        throw error("axis name needs to be specified in difference operator");
      
      delta=d;
      // remove initial slice of hypercube
      auto hc=arg->hypercube();
      if (rank()==0) return;
      
      auto& xv=hc.xvectors[rank()==1? 0: dimension];
      if (delta>=0)
        xv.erase(xv.begin(), xv.begin()+delta);
      else
        xv.erase(xv.end()+delta, xv.end());
      cachedResult.hypercube(move(hc));
      
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
          if (t>=0 && t<ssize_t(size()) && idxSet.count(t))
            {
              argIndices.push_back(t);
              newIdx.insert(hypercube().linealIndex(arg->hypercube().splitIndex(t)));
            }
        }
      cachedResult.index(Index(newIdx));
    }

    bool sameSlice(size_t i, size_t j) const
    {
      return rank()==1 || (i%innerStride==j%innerStride && i/outerStride==j/outerStride);
    }
    
    void computeTensor() const override
    {
      if (argIndices.size())
        {
          assert(argIndices.size()==size());
          size_t idx=0;
          for (auto i: argIndices)
            {
              auto t=i+delta;
              if (sameSlice(t, i))
                cachedResult[idx++]=arg->atHCIndex(t)-arg->atHCIndex(i);
              else
                cachedResult[idx++]=nan("");
            }
        }
      else if (delta>=0)
        for (size_t i=0; i<size(); ++i)
          {
            auto t=i+delta;
            if (sameSlice(t, i))
              cachedResult[i]=arg->atHCIndex(t)-arg->atHCIndex(i);
            else
              cachedResult[i]=nan("");
          }
      else // with -ve delta, origin of result is shifted
        for (size_t i=0; i<size(); ++i)
          {
            auto t=i-delta;
            if (sameSlice(t,i))
              cachedResult[i]=arg->atHCIndex(i)-arg->atHCIndex(t);
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
		   
      size_t m=1, n=1;   
      if (arg1->rank()>1)
        for (size_t i=0; i<arg1->rank()-1; i++)
          m*=arg1->hypercube().dims()[i];
          
      if (arg2->rank()>1)
        for (size_t i=1; i<arg2->rank(); i++)
          n*=arg2->hypercube().dims()[i];     
  	
      size_t stride=arg2->hypercube().dims()[0];	 	 
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
    		            
      if (cachedResult.size()==0) 
        for (size_t i=0; i<m*n; i++) 
          cachedResult[i]=nan("");
    }
    Timestamp timestamp() const override {return max(arg1->timestamp(), arg2->timestamp());}
    void setArguments(const TensorPtr& a1, const TensorPtr& a2) override {
      arg1=a1; arg2=a2;
      if (arg1 && arg1->rank()!=0 && arg2 && arg2->rank()!=0) {
        if (arg1->hypercube().dims()[arg1->rank()-1]!=arg2->hypercube().dims()[0])
          throw std::runtime_error("inner dimensions of tensors do not match");
        
        auto xv1=arg1->hypercube().xvectors, xv2=arg2->hypercube().xvectors;
        Hypercube hc;
        hc.xvectors.insert(hc.xvectors.begin(), xv2.begin()+1, xv2.end());        
        hc.xvectors.insert(hc.xvectors.begin(), xv1.begin(), xv1.end()-1);
        cachedResult.hypercube(move(hc));
                
      }
    }    
  };

  template <>
  struct GeneralTensorOp<OperationType::outerProduct>: public civita::CachedTensorOp
  {
    std::shared_ptr<ITensor> arg1, arg2;
    void computeTensor() const override {//TODO Sparse implementation
      size_t m=arg1->size(), n=arg2->size();   
  	
	
      for (size_t i=0; i< m; i++)
       {
         auto v1=(*arg1)[i];  			
         for (size_t j=0; j< n; j++) 
         {
            auto v2=(*arg2)[j];			
            cachedResult[i+j*m]=v1*v2;			
 	 }
       }	     
    		            
      if (cachedResult.size()==0) 
        for (size_t i=0; i<m*n; i++) 
          cachedResult[i]=nan("");
    }
    Timestamp timestamp() const override {return max(arg1->timestamp(), arg2->timestamp());}
    void setArguments(const TensorPtr& a1, const TensorPtr& a2) override {
      arg1=a1; arg2=a2;
      auto xv1=arg1->hypercube().xvectors, xv2=arg2->hypercube().xvectors;
      Hypercube hc;
      hc.xvectors.insert(hc.xvectors.begin(), xv2.begin(), xv2.end());         
      hc.xvectors.insert(hc.xvectors.begin(), xv1.begin(), xv1.end());           
      cachedResult.hypercube(move(hc));
        
      set<size_t> newIdx;
      size_t stride=arg1->hypercube().numElements();
      
      for (auto& i: arg1->index())
        for (auto& j: arg2->index()) 
            newIdx.insert(i+stride*j);
         
      cachedResult.index(Index(newIdx));  
      
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
    void setArgument(const TensorPtr& a, const string&,double) override {
      arg=a; cachedResult.index(a->index()); cachedResult.hypercube(a->hypercube());
    }
    
    Timestamp timestamp() const override {return arg->timestamp();}
  };

  template <>
  struct GeneralTensorOp<OperationType::gather>: public civita::CachedTensorOp
  {
    std::shared_ptr<ITensor> arg1, arg2;
    void computeTensor() const override
    {
      for (size_t i=0; i<arg2->size(); ++i)
        {
          auto idx=(*arg2)[i];
          if (isfinite(idx))
            {
              if (idx>=0)
                {
                  if (idx==arg1->size()-1)
                    cachedResult[i]=(*arg1)[idx];
                  else if (idx<arg1->size()-1)
                    {
                      double s=idx-floor(idx);
                      cachedResult[i]=(1-s)*(*arg1)[idx]+s*(*arg1)[idx+1];
                    }
                }
              else if (idx>-1)
                cachedResult[i]=(*arg1)[0];
              else
                cachedResult[i]=nan("");
            }
          else
            cachedResult[i]=nan("");
        }              
    }
    Timestamp timestamp() const override {return max(arg1->timestamp(), arg2->timestamp());}
    void setArguments(const TensorPtr& a1, const TensorPtr& a2) override {
	
	// ensures that gather operation is x-vector aware, just like other tensor binops for ticket 1271	
      if (a1 && a1->rank()>0)
        hypercube(a1->hypercube());
      else if (a2)
        hypercube(a2->hypercube());
      else
        hypercube(Hypercube());
      
      arg1=a1;

      if (a2)
        {
          if (a2->rank()==0 || a2->hypercube()==hypercube())
            arg2=a2;
          else
            {
              arg2=make_shared<InterpolateHC>();
              arg2->hypercube(hypercube());
              arg2->setArgument(a2);
            }
        }    
        
      cachedResult.index(arg2->index());
      cachedResult.hypercube(arg2->hypercube());
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
  
  class SwitchTensor: public ITensor
  {
    size_t m_size=1;
    vector<TensorPtr> args;
  public:
    void setArguments(const std::vector<TensorPtr>& a,const std::string& axis={},double argv=0) override {
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
          else
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

    void setArgument(const TensorPtr& a,const std::string&,double) override {
      // not sure how to avoid this const cast here
      const_cast<Ravel&>(ravel).populateHypercube(a->hypercube());
      chain=civita::createRavelChain(ravel.getState(), a);
    }

    double operator[](size_t i) const override {return chain.empty()? 0: (*chain.back())[i];}
    size_t size() const override {return chain.empty()? 1: chain.back()->size();}
    const Index& index() const override
    {if (chain.empty()) return m_index; else return chain.back()->index();}
    Timestamp timestamp() const override
    {return chain.empty()? Timestamp(): chain.back()->timestamp();}
    const Hypercube& hypercube() const override {return chain.back()->hypercube();}
  };
       
  std::shared_ptr<ITensor> TensorOpFactory::create
  (const ItemPtr& it, const TensorsFromPort& tfp)
  {
    if (auto ravel=dynamic_cast<const Ravel*>(it.get()))
	    {
	      auto r=make_shared<RavelTensor>(*ravel);
	      r->setArguments(tfp.tensorsFromPorts(it->ports));
	      return r;
	    }
    else if (auto op=it->operationCast())
      try
        {
          TensorPtr r{create(op->type())};
          if (auto ss=dynamic_cast<SetState*>(r.get()))
            ss->setState(dynamic_pointer_cast<OperationBase>(it));
          switch (op->ports.size())
            {
            case 2:
              r->setArguments(tfp.tensorsFromPort(*op->ports[1]),op->axis,op->arg);
            break;
            case 3:
              r->setArguments(tfp.tensorsFromPort(*op->ports[1]), tfp.tensorsFromPort(*op->ports[2]));
              break;
            }
          return r;
        }
      catch (const InvalidType&)
        {return {};}
      catch (const std::exception& ex)
        {
          // rethrow with op attached to mark op on canvas
          op->throw_error(ex.what());
        }
    else if (auto v=it->variableCast())
      return make_shared<ConstTensorVarVal>(v->vValue(), tfp.ev);
    else if (auto sw=dynamic_cast<const SwitchIcon*>(it.get()))
      {
        auto r=make_shared<SwitchTensor>();
        r->setArguments(tfp.tensorsFromPorts(it->ports));
        return r;
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
                auto rhs=tensorsFromPort(*o->ports[1]);
                if (rhs.empty() || rhs[0]->size()==1)
                  throw FallBackToScalar();
                else
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


  vector<TensorPtr> TensorsFromPort::tensorsFromPorts(const vector<shared_ptr<Port>>& ports) const
  {
    vector<TensorPtr> r;
    for (auto& p: ports)
      if (p->input())
        {
          auto tensorArgs=tensorsFromPort(*p);
          r.insert(r.end(), tensorArgs.begin(), tensorArgs.end());
        }
    return r;
  }

  TensorEval::TensorEval(const shared_ptr<VariableValue>& v, const shared_ptr<EvalCommon>& ev):
    result(v, ev)
  {
    if (auto var=cminsky().definingVar(v->valueId()))
      if (var->lhs())
        {
          rhs=TensorsFromPort(ev).tensorsFromPort(*var->ports[1])[0];
          result.hypercube(rhs->hypercube());
          result.index(rhs->index());
          *v=result;
        }
  }
  
  TensorEval::TensorEval(const shared_ptr<VariableValue>& dest, const shared_ptr<VariableValue>& src):
    result(dest,make_shared<EvalCommon>())
  {
    result.index(src->index());
    result.hypercube(src->hypercube());
    OperationPtr tmp(OperationType::copy);
    auto copy=dynamic_pointer_cast<ITensor>(tensorOpFactory.create(tmp));
    copy->setArgument(make_shared<ConstTensorVarVal>(src,result.ev));
    rhs=move(copy);
    assert(result.size()==rhs->size());
  }   

  void TensorEval::eval(double fv[], size_t n, const double sv[])
  {
    if (rhs)
      {
        assert(result.idx()>=0);
        result.ev->update(fv, n, sv);
        //        assert(result.size()==rhs->size());
        result.hypercube(rhs->hypercube());
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
