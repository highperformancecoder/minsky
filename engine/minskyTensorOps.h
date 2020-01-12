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

  /// A place to store common data shared between TensorVarVals within a give calculation
  class EvalCommon
  {
    double* m_flowVars=nullptr;
    const double* m_stockVars=nullptr;
    ITensor::Timestamp m_timestamp;
  public:
    double* flowVars() const {return m_flowVars;}
    const double* stockVars() const {return m_stockVars;}
    ITensor::Timestamp timestamp() const {return m_timestamp;}
    void update(double* fv, const double* sv)
    {
      m_flowVars=fv; m_stockVars=sv; m_timestamp=ITensor::Timestamp::clock::now();
    }
  };

  struct TensorsFromPort
  {
    shared_ptr<EvalCommon> ev;
    TensorsFromPort(const shared_ptr<EvalCommon>& ev): ev(ev) {}
    
    /// returns vector of tensor ops for all wires attach to port. Port
    /// must be an input port
    std::vector<TensorPtr> tensorsFromPort(const Port&) const;
  };

  /// As it says on the tin, this is a factory for creating a TensorOp
  /// which can compute the result of op applied to its arguments and
  /// so on until all the argument terminate in variables.
  class TensorOpFactory: public classdesc::Factory<TensorOp, OperationType::Type>
  {
    using classdesc::Factory<civita::TensorOp, OperationType::Type>::create;
  public:
    TensorOpFactory();
    std::shared_ptr<TensorOp> create(const OperationBase&, const TensorsFromPort&);
  };    
  
  extern TensorOpFactory tensorOpFactory;


  // a VariableValue that contains a references to overridable value vectors
  struct TensorVarVal: public VariableValue
  {
    /// reference to EvalOpVector owning this value, to extract
    /// flowVar and stockVarinfo
    shared_ptr<EvalCommon> ev;
    /// 
    Timestamp timestamp() const {return ev->timestamp();}
    using VariableValue::operator[];
    double operator[](size_t i) const override {
      return isFlowVar()? ev->flowVars()[idx()+i]: ev->stockVars()[idx()+i];
    }
    TensorVarVal(const VariableValue& vv, const shared_ptr<EvalCommon>& ev):
      VariableValue(vv), ev(ev) {}
    const TensorVarVal& operator=(const ITensor& t) override {
      VariableValue::operator=(t);
      ev->update(ev->flowVars(),ev->stockVars());
      return *this;
    }
  };


  /// A helper to evaluate a variable value
  class TensorEval: EvalOpBase
  {
    TensorVarVal result;
    TensorPtr rhs;

  public:
    TensorEval(const VariableValue& v, const shared_ptr<EvalCommon>& ev);
    
    void eval(double fv[], const double sv[]) override {
      if (rhs)
        {
          result.ev->update(fv, sv);
          for (size_t i=0; i<rhs->size(); ++i)
            result[i]=(*rhs)[i];
        }
    }
    
    void deriv(double df[], const double ds[],
               const double sv[], const double fv[]) override
    {throw error("derivative not yet implemented");}
  };


  
//  template <> struct TensorOp<OperationType::add>: public TensorBinOp<OperationType::add> {};
//  template <> struct TensorOp<OperationType::subtract>: public TensorBinOp<OperationType::subtract> {};
//  template <> struct TensorOp<OperationType::multiply>: public TensorBinOp<OperationType::multiple> {};
//  template <> struct TensorOp<OperationType::divide>: public TensorBinOp<OperationType::divide> {};
//  template <> struct TensorOp<OperationType::log>: public TensorBinOp<OperationType::log> {};
//  template <> struct TensorOp<OperationType::pow>: public TensorBinOp<OperationType::pow> {};
//  template <> struct TensorOp<OperationType::lt>: public TensorBinOp<OperationType::lt> {};
//  template <> struct TensorOp<OperationType::le>: public TensorBinOp<OperationType::le> {};
//  template <> struct TensorOp<OperationType::eq>: public TensorBinOp<OperationType::eq> {};
//  template <> struct TensorOp<OperationType::min>: public TensorBinOp<OperationType::min> {};
//  template <> struct TensorOp<OperationType::max>: public TensorBinOp<OperationType::max> {};
//  template <> struct TensorOp<OperationType::and_>: public TensorBinOp<OperationType::and_> {};
//  template <> struct TensorOp<OperationType::or_>: public TensorBinOp<OperationType::or_> {};

  
}
  
  
#endif
