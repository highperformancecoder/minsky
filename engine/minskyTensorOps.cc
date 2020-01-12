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

  // Default template calls the regular legacy double function
  template <OperationType::Type op> struct MinskyTensorOp: public civita::ElementWiseOp
  {
    EvalOp<op> eo;
    MinskyTensorOp(): ElementWiseOp([this](double x){return eo.evaluate(x);}) {}
  };

  template <OperationType::Type op> struct TensorBinOp: civita::BinOp
  {
    EvalOp<op> eo;
    TensorBinOp(): BinOp([this](double x,double y){return eo.evaluate(x,y);}) {}
  };

  
  template <minsky::OperationType::Type T>
  struct ReductionTraits
  {
    /// x op= y
    static void accum(double& x, double y);
    static const double init;
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
    registerOps<TensorBinOp, OperationType::add, OperationType::copy>(*this);
    registerOps<MinskyTensorOp, OperationType::copy, OperationType::sum>(*this);
    registerOps<GeneralTensorOp, OperationType::sum, OperationType::numOps>(*this);
  }
                                                                                    
  template <>
  class GeneralTensorOp<OperationType::sum>: public civita::ReductionOp
  {
  public:
    GeneralTensorOp(): civita::ReductionOp([](double& x, double y){x+=y;}){}
  };
  template <>
  class GeneralTensorOp<OperationType::product>: public civita::ReductionOp
  {
  public:
    GeneralTensorOp(): civita::ReductionOp([](double& x, double y){x*=y;}){}
  };
  template <>
  class GeneralTensorOp<OperationType::infimum>: public civita::ReductionOp
  {
  public:
    GeneralTensorOp(): civita::ReductionOp([](double& x, double y){if (y<x) x=y;}){}
   };
  template <>
  class GeneralTensorOp<OperationType::supremum>: public civita::ReductionOp
  {
  public:
    GeneralTensorOp(): civita::ReductionOp([](double& x, double y){if (y>x) x=y;}){}
   };
  template <>
  class GeneralTensorOp<OperationType::any>: public civita::ReductionOp
  {
  public:
    GeneralTensorOp(): civita::ReductionOp([](double& x, double y){if (y>0.5) x=1;}){}
   };
  template <>
  class GeneralTensorOp<OperationType::all>: public civita::ReductionOp
  {
  public:
    GeneralTensorOp(): civita::ReductionOp([](double& x, double y){x*=(y>0.5);}){}
   };

  template <>
  class GeneralTensorOp<OperationType::runningSum>: public civita::Scan
  {
  public:
    GeneralTensorOp(): civita::Scan([](double& x,double y){x+=y;}) {}
  };

  template <>
  class GeneralTensorOp<OperationType::runningProduct>: public civita::Scan
  {
  public:
    GeneralTensorOp(): civita::Scan([](double& x,double y){x+=y;}) {}
  };
  
  template <>
  class GeneralTensorOp<OperationType::difference>: public civita::CachedTensorOp
  {
    TensorPtr arg;
  public:
    void computeTensor() const override {//TODO
    }
    Timestamp timestamp() const override {return arg->timestamp();}
  };
  
  template <>
  class GeneralTensorOp<OperationType::innerProduct>: public civita::CachedTensorOp
  {
    std::shared_ptr<ITensor> arg1, arg2;
    void computeTensor() const override {//TODO
    }
    Timestamp timestamp() const override {return max(arg1->timestamp(), arg2->timestamp());}
  };

  template <>
  class GeneralTensorOp<OperationType::outerProduct>: public civita::CachedTensorOp
  {
    std::shared_ptr<ITensor> arg1, arg2;
    void computeTensor() const override {//TODO
    }
    Timestamp timestamp() const override {return max(arg1->timestamp(), arg2->timestamp());}
  };

  template <>
  class GeneralTensorOp<OperationType::index>: public civita::CachedTensorOp
  {
    std::shared_ptr<ITensor> arg1, arg2;
    void computeTensor() const override {//TODO
    }
    Timestamp timestamp() const override {return max(arg1->timestamp(), arg2->timestamp());}
  };

  template <>
  class GeneralTensorOp<OperationType::gather>: public civita::CachedTensorOp
  {
    std::shared_ptr<ITensor> arg1, arg2;
    void computeTensor() const override {//TODO
    }
    Timestamp timestamp() const override {return max(arg1->timestamp(), arg2->timestamp());}
  };

  template <>
  class GeneralTensorOp<OperationType::supIndex>: public civita::CachedTensorOp
  {
    std::shared_ptr<ITensor> arg1, arg2;
    void computeTensor() const override {//TODO
    }
    Timestamp timestamp() const override {return max(arg1->timestamp(), arg2->timestamp());}
  };

  template <>
  class GeneralTensorOp<OperationType::infIndex>: public civita::CachedTensorOp
  {
    std::shared_ptr<ITensor> arg1, arg2;
    void computeTensor() const override {//TODO
    }
    Timestamp timestamp() const override {return max(arg1->timestamp(), arg2->timestamp());}
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
    void setArguments(const std::vector<TensorPtr>& a) override {
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
        if (args[0]->rank()==0) // scalar selector, so broadcast
          selector = (*args[0])[0];
        else
          selector = args[0]->atHCIndex(hcIndex(i));

      ssize_t idx = selector+1.5; // selector selects between args 1..n
      
      if (idx>0 && idx<args.size())
        if (args[idx]->rank()==0)
          return (*args[idx])[0];
        else
          return args[idx]->atHCIndex(hcIndex(i));
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
    void setArgument(const TensorPtr& a,const std::string& d={}) override {arg=a;}
    Timestamp timestamp() const override {return arg? arg->timestamp(): Timestamp();}
  };
  
  std::shared_ptr<TensorOp> TensorOpFactory::create
  (const OperationBase& op, const TensorsFromPort& tfp)
  {
    std::shared_ptr<TensorOp> r{create(op.type())};
    switch (op.ports.size())
      {
      case 2:
        r->setArguments(tfp.tensorsFromPort(*op.ports[1]));
        break;
      case 3:
        r->setArguments(tfp.tensorsFromPort(*op.ports[1]), tfp.tensorsFromPort(*op.ports[2]));
        break;
      }
    return r;
  }

  vector<TensorPtr> TensorsFromPort::tensorsFromPort(const Port& p) const
  {
    if (!p.input()) return {};
    vector<TensorPtr> r;
    for (auto w: p.wires())
      if (auto o=w->from()->item().operationCast())
        r.push_back(tensorOpFactory.create(*o, *this));
      else if (auto v=w->from()->item().variableCast())
        r.push_back(make_shared<TensorVarVal>(*v->vValue(), ev));
      else if (auto s=dynamic_cast<SwitchIcon*>(&w->from()->item()))
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
      else if (auto ravel=dynamic_cast<Ravel*>(&w->from()->item()))
        {
          auto rt=make_shared<RavelTensor>(*ravel);
          auto args=tensorsFromPort(*ravel->ports[1]);
          if (!args.empty()) rt->setArgument(args[0]);
          r.push_back(rt);
        }
    return r;
  }

  TensorEval::TensorEval(const VariableValue& v, const shared_ptr<EvalCommon>& ev): result(v, ev)
  {
    if (auto var=cminsky().definingVar(v.valueId()))
      if (var->lhs())
        {
          rhs=TensorsFromPort(ev).tensorsFromPort(*var->ports[1])[0];
          result.hypercube(rhs->hypercube());
          result.index(rhs->index());
        }
  }

}
