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
#include "minsky.h"
#include "minsky_epilogue.h"

using namespace civita;
namespace classdesc
{
  // postpone factory definition to TensorOpFactory()
  template <> Factory<TensorOp, minsky::OperationType::Type>::Factory() {}
}


namespace minsky
{
  using namespace classdesc;

  TensorOpFactory tensorOpFactory;

  struct DerivativeNotDefined: public std::exception
  {
    const char* what() const throw() {return "Derivative not defined";}
  };
  
  struct TimeOp: public TensorOp
  {
    size_t size() const override {return 1;}
    vector<size_t> index() const override {return {};}
    double operator[](size_t) const override {return EvalOpBase::t;}
    Timestamp timestamp() const override {return {};}
  };
  
  // Default template calls the regular legacy double function
  template <OperationType::Type op> struct MinskyTensorOp: public civita::ElementWiseOp, public DerivativeMixin
  {
    EvalOp<op> eo;
    MinskyTensorOp(): ElementWiseOp([this](double x){return eo.evaluate(x);}) {}
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

  template <OperationType::Type op> struct TensorBinOp: civita::BinOp, public DerivativeMixin
  {
    EvalOp<op> eo;
    TensorBinOp(): BinOp([this](double x,double y){return eo.evaluate(x,y);}) {}
    virtual void setArguments(const std::vector<TensorPtr>& a1, const std::vector<TensorPtr>& a2) override
    {
      civita::BinOp::setArguments
        (a1.empty()? TensorPtr(): a1[0],
         a2.empty()?  TensorPtr(): a2[0]
         );
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
      auto pa1=make_shared<AccumArgs<op>>(), pa2=make_shared<AccumArgs<op>>();
      pa1->setArguments(a1,{},0); pa2->setArguments(a2,{},0);
      civita::BinOp::setArguments(pa1, pa2);
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
    tensorOpFactory.registerType<TimeOp>(OperationType::time);
    registerOps<MultiWireBinOp, OperationType::add, OperationType::log>(*this);
    registerOps<TensorBinOp, OperationType::log, OperationType::copy>(*this);
    registerOps<MinskyTensorOp, OperationType::copy, OperationType::sum>(*this);
    registerOps<GeneralTensorOp, OperationType::sum, OperationType::numOps>(*this);
  }
                                                                                    
  template <>
  class GeneralTensorOp<OperationType::sum>: public civita::ReductionOp
  {
  public:
    GeneralTensorOp(): civita::ReductionOp([](double& x, double y,size_t){x+=y;},0){}
  };
  template <>
  class GeneralTensorOp<OperationType::product>: public civita::ReductionOp
  {
  public:
    GeneralTensorOp(): civita::ReductionOp([](double& x, double y,size_t){x*=y;},1){}
  };
  template <>
  class GeneralTensorOp<OperationType::infimum>: public civita::ReductionOp
  {
  public:
    GeneralTensorOp(): civita::ReductionOp([](double& x, double y,size_t){if (y<x) x=y;},std::numeric_limits<double>::max()){}
   };
  template <>
  class GeneralTensorOp<OperationType::supremum>: public civita::ReductionOp
  {
  public:
    GeneralTensorOp(): civita::ReductionOp([](double& x, double y,size_t){if (y>x) x=y;},-std::numeric_limits<double>::max()){}
   };
  template <>
  class GeneralTensorOp<OperationType::any>: public civita::ReductionOp
  {
  public:
    GeneralTensorOp(): civita::ReductionOp([](double& x, double y,size_t){if (y>0.5) x=1;},0){}
   };
  template <>
  class GeneralTensorOp<OperationType::all>: public civita::ReductionOp
  {
  public:
    GeneralTensorOp(): civita::ReductionOp([](double& x, double y,size_t){x*=(y>0.5);},1){}
   };

  template <>
  class GeneralTensorOp<OperationType::runningSum>: public civita::Scan
  {
  public:
    GeneralTensorOp(): civita::Scan([](double& x,double y,size_t){x+=y;}) {}
  };

  template <>
  class GeneralTensorOp<OperationType::runningProduct>: public civita::Scan
  {
  public:
    GeneralTensorOp(): civita::Scan([](double& x,double y,size_t){x*=y;}) {}
  };
  
  template <>
  class GeneralTensorOp<OperationType::difference>: public civita::Scan
  {
    ssize_t delta;
  public:
    GeneralTensorOp(): civita::Scan
                       ([this](double& x,double y,size_t i)
                        {
                          ssize_t t=ssize_t(i)-delta;
                          if (t>=0 && t<ssize_t(arg->size()))
                            x = y-arg->atHCIndex(t);
                        }) {}
    void setArgument(const TensorPtr& a,const std::string& s,double d) override {
      civita::Scan::setArgument(a,s,d);
      delta=d;
      // determine offset in hypercube space
      auto dims=arg->hypercube().dims();
      if (dimension<dims.size())
        for (size_t i=0; i<dimension; ++i)
          delta*=dims[i];
    }
  };
  
  template <>
  class GeneralTensorOp<OperationType::innerProduct>: public civita::CachedTensorOp
  {
    std::shared_ptr<ITensor> arg1, arg2;
    void computeTensor() const override {//TODO
      throw runtime_error("inner product not yet implemented");
    }
    Timestamp timestamp() const override {return max(arg1->timestamp(), arg2->timestamp());}
  };

  template <>
  class GeneralTensorOp<OperationType::outerProduct>: public civita::CachedTensorOp
  {
    std::shared_ptr<ITensor> arg1, arg2;
    void computeTensor() const override {//TODO
      throw runtime_error("outer product not yet implemented");
    }
    Timestamp timestamp() const override {return max(arg1->timestamp(), arg2->timestamp());}
  };

  template <>
  class GeneralTensorOp<OperationType::index>: public civita::CachedTensorOp
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
  class GeneralTensorOp<OperationType::gather>: public civita::CachedTensorOp
  {
    std::shared_ptr<ITensor> arg1, arg2;
    void computeTensor() const override {
      for (size_t i=0; i<arg2->size(); ++i) {
        auto idx=(*arg2)[i];
        if (isfinite(idx) && idx>=0 && idx<arg1->size())
          cachedResult[i]=(*arg1)[idx];
        else
          cachedResult[i]=nan("");
      }              
    }
    Timestamp timestamp() const override {return max(arg1->timestamp(), arg2->timestamp());}
    void setArguments(const TensorPtr& a1, const TensorPtr& a2) override {
      arg1=a1; arg2=a2;
      cachedResult.index(arg2->index());
      cachedResult.hypercube(arg2->hypercube());
    }
      
  };

  template <>
  class GeneralTensorOp<OperationType::supIndex>: public civita::ReductionOp
  {
    double maxValue; // scratch register for holding current max
  public:
    GeneralTensorOp(): civita::ReductionOp
                       ([this](double& r,double x,size_t i){
                          if (i==0 || x>maxValue) {
                            maxValue=x;
                            r=i;
                          }
                        },0) {}
  };
  
  template <>
  class GeneralTensorOp<OperationType::infIndex>: public civita::ReductionOp
  {
    double minValue; // scratch register for holding current min
  public:
    GeneralTensorOp(): civita::ReductionOp
                       ([this](double& r,double x,size_t i){
                          if (i==0 || x<minValue) {
                            minValue=x;
                            r=i;
                          }
                        },0) {}
  };
  
  class SwitchTensor: public TensorOp
  {
    size_t m_size=1;
    vector<size_t> m_index;
    vector<TensorPtr> args;
    size_t hcIndex(size_t i) const {
      if (m_index.empty()) return i;
      if (i>=m_index.size()) return numeric_limits<size_t>::max();
      return m_index[i];
    }
  public:
    void setArguments(const std::vector<TensorPtr>& a,const std::string&,double) override {
      args=a;
      set<size_t> indices; // collect the union of argument indices
      for (auto& i: args)
        {
          auto ai=i->index();
          indices.insert(ai.begin(), ai.end());
          if (i->size()>1)
            {
              if (m_size==1)
                m_size=i->size();
              else if (m_size!=i->size())
                throw runtime_error("noconformant tensor arguments in switch");
            }
        }
      m_index.clear();
      m_index.insert(m_index.end(),indices.begin(), indices.end());
      if (!m_index.empty()) m_size=m_index.size();
    }
    vector<size_t> index() const override {return m_index;}
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
            selector = args[0]->atHCIndex(hcIndex(i));
        }
      ssize_t idx = selector+1.5; // selector selects between args 1..n
      
      if (idx>0 && idx<int(args.size()))
        {
          if (args[idx]->rank()==0)
            return (*args[idx])[0];
          else
            return args[idx]->atHCIndex(hcIndex(i));
        }
      return nan("");
    }
  };

  class RavelTensor: public CachedTensorOp
  {
    Ravel& ravel;
    TensorPtr arg;
    void computeTensor() const override
    {
      ravel.loadDataCubeFromVariable(*arg);
      ravel.loadDataFromSlice(cachedResult);
      m_timestamp = Timestamp::clock::now();
    }
    
  public:
    RavelTensor(Ravel& ravel): ravel(ravel) {}
    void setArgument(const TensorPtr& a,const std::string& d,double) override {arg=a;}
    Timestamp timestamp() const override {return arg? arg->timestamp(): Timestamp();}
  };
  
  std::shared_ptr<TensorOp> TensorOpFactory::create
  (const OperationBase& op, const TensorsFromPort& tfp)
  {
    try
      {
        std::shared_ptr<TensorOp> r{create(op.type())};
        switch (op.ports.size())
          {
          case 2:
            r->setArguments(tfp.tensorsFromPort(*op.ports[1]),op.axis,op.arg);
            break;
          case 3:
            r->setArguments(tfp.tensorsFromPort(*op.ports[1]), tfp.tensorsFromPort(*op.ports[2]));
            break;
          }
        return r;
      }
    catch (const InvalidType&)
      {return {};}
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
            r.push_back(tensorOpFactory.create(*o, *this));
            assert(r.back());
          }
        else if (auto v=item.variableCast())
          r.push_back(make_shared<TensorVarVal>(*v->vValue(), ev));
        else if (auto s=dynamic_cast<SwitchIcon*>(&item))
          {
            auto st=make_shared<SwitchTensor>();
            vector<TensorPtr> args;
            for (auto& p: s->ports)
              {
                if (p->wires().empty())
                  s->throw_error("Unwired switch");
                args.push_back(tensorsFromPort(*p)[0]);
              }
            r.push_back(st);
          }
        else if (auto ravel=dynamic_cast<Ravel*>(&item))
          {
            auto rt=make_shared<RavelTensor>(*ravel);
            auto args=tensorsFromPort(*ravel->ports[1]);
            if (!args.empty()) rt->setArgument(args[0],{},0);
            r.push_back(rt);
          }
        else
          item.throw_error("Invalid item for TensorOp");
      }
    return r;
  }

  TensorEval::TensorEval(VariableValue& v, const shared_ptr<EvalCommon>& ev): result(v, ev)
  {
    if (auto var=cminsky().definingVar(v.valueId()))
      if (var->lhs())
        {
          rhs=TensorsFromPort(ev).tensorsFromPort(*var->ports[1])[0];
          result.hypercube(rhs->hypercube());
          result.index(rhs->index());
          v=result;
        }
  }
  
  TensorEval::TensorEval(const VariableValue& dest, const VariableValue& src):
    result(dest,make_shared<EvalCommon>())
  {
    result.index(src.index());
    result.hypercube(src.hypercube());
    Operation<OperationType::copy> tmp;
    auto copy=tensorOpFactory.create(tmp);
    copy->setArgument(make_shared<TensorVarVal>(src,result.ev));
    rhs=move(copy);
    assert(result.size()==rhs->size());
  }

  void TensorEval::eval(double fv[], const double sv[])
  {
    if (rhs)
      {
        assert(result.idx()>=0);
        assert(result.size()==rhs->size());
        result.ev->update(fv, sv);
        for (size_t i=0; i<rhs->size(); ++i)
          {
            result[i]=(*rhs)[i];
            assert(!finite(result[i]) || fv[result.idx()+i]==(*rhs)[i]);
            //            cout << "i="<<i<<"idx="<<result.idx()<<" set to "<< (*rhs)[i] << " should be "<<fv[result.idx()]<<endl;
          }
      }
  }

  
  void TensorEval::deriv(double df[], const double ds[],
                         const double sv[], const double fv[])
  {
    if (result.idx()<0) return;
    if (rhs)
      {
        result.ev->update(const_cast<double*>(fv), sv);
        if (auto deriv=dynamic_cast<DerivativeMixin*>(rhs.get()))
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
