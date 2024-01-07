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
#ifndef EVALOP_H
#define EVALOP_H

#include <ecolab.h>
#include <xml_pack_base.h>
#include <xml_unpack_base.h>

#include "variableValue.h"
#include "operation.h"
#include "group.h"
#include "polyPackBase.h"

#include <vector>
#include <cairo/cairo.h>

#include <arrays.h>

// override EcoLab's default CLASSDESC_ACCESS macro
#include "classdesc_access.h"


namespace minsky
{
  using namespace ecolab;
  using namespace classdesc;
  using namespace std;

  struct EvalOpBase: public classdesc::PolyBase<minsky::OperationType::Type>,
                     //                     virtual public classdesc::PolyPackBase,
                     public OperationType
  {
    typedef OperationType::Type Type;

    // value used for the time operator
    static double t;
    static std::string timeUnit;

    /// indexes into the flow/stock variables vector
    int out=-1;
    /** @{
        in1 - indices describing input 1's vector
        in2 - indices describing input 2's vector (if double op)
        in2b/interp - interpolation support:
           out[i]=op(inp[in1[i]], interp*inp[in2[i]]+(1-interp)*inp[in2b[i]])
    **/
    std::vector<unsigned> in1;
    struct Support
    {
      double weight;
      unsigned idx;
      Support() {}
      Support(double weight, unsigned idx): weight(weight), idx(idx) {}
    };
    std::vector<std::vector<Support>> in2;
    ///indicate whether in1/in2 are flow variables (out is always a flow variable)
    bool flow1=true, flow2=true, xflow=true; 

    /// state data (for those ops that need it)
    std::shared_ptr<OperationBase> state;
    virtual ~EvalOpBase() {}

    /**
       total derivate with respect to a variable, which is a function of the stock variables.
       @param sv - stock variables
       @param fv - flow variables (function of stock variables, computed by eval)
       @param ds - derivative of stock variables
       @param df - derivative of flow variables (updated by this function)
       @param n - size of df array

       To compute the partial derivatives with respect to stock variable
       i, seed ds with 1 in the ith position, 0 every else, and
       initialise df to zero.
    */
    virtual void deriv(double df[], std::size_t n, const double ds[], 
                       const double sv[], const double fv[])=0;

    /// evaluate expression on sv and current value of fv, storing result
    /// in output variable (of \a fv)
    /// @param n - size of fv array
    virtual void eval(double fv[]=ValueVector::flowVars.data(), std::size_t n=ValueVector::flowVars.size(), 
                      const double sv[]=ValueVector::stockVars.data())=0;
 

    /// set additional tensor operation related parameters
    virtual void setTensorParams(const VariableValue&,const OperationBase&) {}
  };

  /// Legacy EvalOp base interface
  struct ScalarEvalOp: public EvalOpBase
  {
    std::shared_ptr<VariableValue> result; ///< lifetime management of the resultant variableValue
    /// number of arguments to this operation
    virtual int numArgs() const =0;

    /// factory method
    static ScalarEvalOp* create(Type op/*=numOps*/, const ItemPtr& state);

    void deriv(double df[], std::size_t n, const double ds[], 
                       const double sv[], const double fv[]) override;

    void eval(double fv[], std::size_t, const double sv[]) override;
 
    /// evaluate expression on given arguments, returning result
    virtual double evaluate(double in1=0, double in2=0) const=0;
    /**
       @{
       derivatives with respect to 1st and second argument
    */
    virtual double d1(double x1=0, double x2=0) const=0;
    virtual double d2(double x1=0, double x2=0) const=0;
    /// @}
  };
  
  /// represents the operation when evaluating the equations
  template <minsky::OperationType::Type T>
  struct EvalOp: public classdesc::Poly<EvalOp<T>, ScalarEvalOp>
  {
    OperationType::Type type() const  override {return T;}
    int numArgs() const override {
      return OperationTypeInfo::numArguments<T>();
    }
    double evaluate(double in1=0, double in2=0) const override;
    double d1(double x1=0, double x2=0) const override;
    double d2(double x1=0, double x2=0) const override;
  };

 struct ConstantEvalOp: public EvalOp<minsky::OperationType::constant>
  {
    double value;
    double evaluate(double in1=0, double in2=0) const override;
 };

  struct EvalOpPtr: public classdesc::shared_ptr<EvalOpBase>, 
                    public OperationType
  {
    EvalOpPtr() {}
    EvalOpPtr(EvalOpBase* e): classdesc::shared_ptr<EvalOpBase>(e) {}
    EvalOpPtr(OperationType::Type op):
      classdesc::shared_ptr<EvalOpBase>(ScalarEvalOp::create(op,nullptr)) {}
    /// \a to cannot be null
    EvalOpPtr(OperationType::Type op,
              const ItemPtr& state,
              const std::shared_ptr<VariableValue>& to,
              const VariableValue& from1={}, 
              const VariableValue& from2={});
  };

  struct EvalOpVector: public vector<EvalOpPtr>
    {
      // override push_back for diagnostic purposes
//       void push_back(const EvalOpPtr& x) {
//         vector<EvalOpPtr>::push_back(x);
//         for (std::size_t i=0; i<x->in1.size(); ++i)
//           if (auto s=dynamic_cast<ScalarEvalOp*>(x.get()))
//           {
//             cout << OperationType::typeName(s->type())<<"(";
//             if (s->numArgs()>0)
//               cout << s->in1[i]<<(s->flow1?"":"s");
//             if (s->numArgs()>1)
//               for (auto& i: s->in2[i])
//                 cout<<","<<i.idx<<"("<<i.weight<<")"<<(s->flow2?"":"s");
//             cout <<")->"<<s->out+i<<endl;
//           }
//       }
  };

  /// exception throw if flowVars vector unexpectedly resized during evalEquations
  struct FlowVarsResized: public std::exception
  {
    const char* what() const noexcept {return "FlowVars unexpectedly resized";}
  };
}

#include "evalOp.cd"
#endif
