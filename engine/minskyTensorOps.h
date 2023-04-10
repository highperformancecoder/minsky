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
#ifndef MINSKYTENSOROPS_H
#define MINSKYTENSOROPS_H
#include "variableValue.h"
#include "evalOp.h"
#include <tensorOp.h>

namespace minsky
{
  using namespace civita;

  struct FallBackToScalar: public std::exception
  {
    const char* what() const throw() {return "Falling back to scalar processing";}
  };

  /// A place to store common data shared between TensorVarVals within a give calculation
  class EvalCommon
  {
    double* m_flowVars=nullptr;
    std::size_t m_fvSize=0;
    const double* m_stockVars=nullptr;
    ITensor::Timestamp m_timestamp;
  public:
    double* flowVars() const {return m_flowVars;}
    std::size_t fvSize() const {return m_fvSize;}
    const double* stockVars() const {return m_stockVars;}
    ITensor::Timestamp timestamp() const {return m_timestamp;}
    /// initialise flow and stock var array pointers
    /// @param fv - pointer to flow variable vector
    /// @param n - size of flow variable vector
    /// @param sv - pointer to stock variable vector
    void update(double* fv, std::size_t n, const double* sv)
    {
      m_flowVars=fv; m_fvSize=n; m_stockVars=sv; m_timestamp=ITensor::Timestamp::clock::now();
    }
  };

  struct TensorsFromPort
  {
    shared_ptr<EvalCommon> ev;
    TensorsFromPort() {}
    TensorsFromPort(const shared_ptr<EvalCommon>& ev): ev(ev) {}
    
    /// returns vector of tensor ops for all wires attach to port. Port
    /// must be an input port
    std::vector<TensorPtr> tensorsFromPort(const Port&) const;
    /// returns vector of tensor ops for all wires attached to item
    std::vector<TensorPtr> tensorsFromPorts(const Item&) const;
  };

  /// As it says on the tin, this is a factory for creating a TensorOp
  /// which can compute the result of op applied to its arguments and
  /// so on until all the argument terminate in variables.
  class TensorOpFactory: public classdesc::Factory<civita::ITensor, OperationType::Type>
  {
    using classdesc::Factory<civita::ITensor, OperationType::Type>::create;
  public:
    TensorOpFactory();
    /// create a tensor representation of the expression rooted at
    /// op. If expression doesn't contain any references variables,
    /// then the \a tp parameter may be omitted.
    std::shared_ptr<ITensor> create(const ItemPtr&, const TensorsFromPort& tp={});
  };    
  
  extern TensorOpFactory tensorOpFactory;

  /// support for partial derivatives needed for implicit method
  struct DerivativeMixin
  {
    /// partial derivative of tensor component \a ti wrt flow variable \a fi
    virtual double dFlow(std::size_t ti, std::size_t fi) const=0;
    /// partial derivative of tensor component \a ti wrt stock variable \a si
    virtual double dStock(std::size_t ti, std::size_t si) const=0;
  };
  
  // a VariableValue that contains a references to overridable value vectors
  template <class VV=const VariableValue, class I=ITensor>
  struct TensorVarValBase: public I, public DerivativeMixin
  {
    std::shared_ptr<VV> value;
    /// reference to EvalOpVector owning this value, to extract
    /// flowVar and stockVarinfo
    shared_ptr<EvalCommon> ev;

    int idx() const {return value->idx();}
    
    /// 
    ITensor::Timestamp timestamp() const override {return ev->timestamp();}
    double operator[](std::size_t i) const override {
      return value->isFlowVar()? ev->flowVars()[value->idx()+i]: ev->stockVars()[value->idx()+i];
    }
    TensorVarValBase(const std::shared_ptr<VV>& vv, const shared_ptr<EvalCommon>& ev):
      value(vv), ev(ev) {}
    const Hypercube& hypercube() const override {return value->hypercube();}
    const Index& index() const override {return value->index();}
    std::size_t size() const override {return value->size();}
   
    double dFlow(std::size_t ti, std::size_t fi) const override 
    {return value->isFlowVar() && fi==ti+value->idx();}
    double dStock(std::size_t ti, std::size_t si) const override 
    {return !value->isFlowVar() && si==ti+value->idx();}
  };

  using ConstTensorVarVal=TensorVarValBase<>;

  struct TensorVarVal: public TensorVarValBase<VariableValue,ITensorVal>
  {
    TensorVarVal(const std::shared_ptr<VariableValue>& vv, const shared_ptr<EvalCommon>& ev):
      TensorVarValBase<VariableValue,ITensorVal>(vv,ev) {}
    using ITensorVal::index;
    const Index& index(Index&& x) override {return value->index(std::move(x));}
    const Index& index() const override {return value->index();}
    const Hypercube& hypercube(const Hypercube& hc) override {return value->hypercube(hc);}
    const Hypercube& hypercube(Hypercube&& hc) override {return value->hypercube(std::move(hc));}
    const Hypercube& hypercube() const override {return value->hypercube();}
    using ITensorVal::operator[];
    double& operator[](std::size_t i) override {
      assert(value->isFlowVar());
      assert(
             (ev->flowVars()==ValueVector::flowVars.data() &&
              value->idx()+i<ValueVector::flowVars.size())
             || value->idx()+i<ev->fvSize());
      return ev->flowVars()[value->idx()+i];
    }
    TensorVarVal& operator=(const ITensor& t) override {
      index(t.index());
      hypercube(t.hypercube());
      for (size_t i=0; i<size(); ++i)
        operator[](i)=t[i];
      // update timestamp
      ev->update(ev->flowVars(), ev->fvSize(), ev->stockVars());
      return *this;
    }
  };

  
  /// A helper to evaluate a variable value
  class TensorEval: public classdesc::Poly<TensorEval, EvalOpBase>
  //                    public classdesc::PolyPack<TensorEval>
  {
    TensorVarVal result;
    TensorPtr rhs;

  public:
    // not used, but required to make this a concrete type
    Type type() const override {assert(false); return OperationType::numOps;} 
    TensorEval(const std::shared_ptr<VariableValue>& v, const shared_ptr<EvalCommon>& ev,
               const TensorPtr& rhs): result(v, ev), rhs(rhs) {
      result.index(rhs->index());
      result.hypercube(rhs->hypercube());
      assert(result.idx()>=0);
      assert(result.size()==rhs->size());
    } 
    TensorEval(const std::shared_ptr<VariableValue>& dest, const std::shared_ptr<VariableValue>& src);
               
    void eval(double fv[], std::size_t,const double sv[]) override;
    void deriv(double df[],std::size_t,const double ds[],const double sv[],const double fv[]) override;
  };
}
  
  
#endif
