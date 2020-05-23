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
    size_t m_fvSize=0;
    const double* m_stockVars=nullptr;
    ITensor::Timestamp m_timestamp;
  public:
    double* flowVars() const {return m_flowVars;}
    size_t fvSize() const {return m_fvSize;}
    const double* stockVars() const {return m_stockVars;}
    ITensor::Timestamp timestamp() const {return m_timestamp;}
    /// initialise flow and stock var array pointers
    /// @param fv - pointer to flow variable vector
    /// @param n - size of flow variable vector
    /// @param sv - pointer to stock variable vector
    void update(double* fv, size_t n, const double* sv)
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
    /// returns vector of tensor ops for all wires attached ports.
    std::vector<TensorPtr> tensorsFromPorts
    (const std::vector<std::shared_ptr<Port>>&) const;
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
    std::shared_ptr<ITensor> create(const Item&, const TensorsFromPort& tp={});
  };    
  
  extern TensorOpFactory tensorOpFactory;

  /// support for partial derivatives needed for implicit method
  struct DerivativeMixin
  {
    /// partial derivative of tensor component \a ti wrt flow variable \a fi
    virtual double dFlow(size_t ti, size_t fi) const=0;
    /// partial derivative of tensor component \a ti wrt stock variable \a si
    virtual double dStock(size_t ti, size_t si) const=0;
  };
  
  // a VariableValue that contains a references to overridable value vectors
  struct TensorVarVal: public VariableValue, public DerivativeMixin
  {
    /// reference to EvalOpVector owning this value, to extract
    /// flowVar and stockVarinfo
    shared_ptr<EvalCommon> ev;
    /// 
    Timestamp timestamp() const override {return ev->timestamp();}
    double operator[](size_t i) const override {
      return isFlowVar()? ev->flowVars()[idx()+i]: ev->stockVars()[idx()+i];
    }
    double& operator[](size_t i) override {
      assert(isFlowVar());
      assert(idx()+i<ev->fvSize());
      return ev->flowVars()[idx()+i];
    }
    TensorVarVal(const VariableValue& vv, const shared_ptr<EvalCommon>& ev):
      VariableValue(vv), ev(ev) {}
    const TensorVarVal& operator=(const ITensor& t) override {
      VariableValue::operator=(t);
      ev->update(ev->flowVars(), ev->fvSize(), ev->stockVars());
      return *this;
    }
    double dFlow(size_t ti, size_t fi) const override 
    {return isFlowVar() && fi==ti+idx();}
    double dStock(size_t ti, size_t si) const override 
    {return !isFlowVar() && si==ti+idx();}
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
    TensorEval(VariableValue& v, const shared_ptr<EvalCommon>& ev); 
    TensorEval(VariableValue& v, const shared_ptr<EvalCommon>& ev,
               const TensorPtr& rhs): result(v, ev), rhs(rhs) {
      result.hypercube(rhs->hypercube());
      result.index(rhs->index());
      v=result;
      assert(result.idx()>=0);
      assert(result.size()==rhs->size());
    } 
    TensorEval(const VariableValue& dest, const VariableValue& src);
               
    void eval(double fv[], size_t,const double sv[]) override;
    void deriv(double df[],size_t,const double ds[],const double sv[],const double fv[]) override;
  };
}
  
  
#endif
