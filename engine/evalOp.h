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
                     virtual public classdesc::PolyPackBase,
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

    /// operation this EvalOp refers to
    virtual OperationType::Type type() const=0; 

    /// number of arguments to this operation
    virtual int numArgs() const =0;
    /**
       total derivate with respect to a variable, which is a function of the stock variables.
       @param sv - stock variables
       @param fv - flow variables (function of stock variables, computed by eval)
       @param ds - derivative of stock variables
       @param df - derivative of flow variables (updated by this function)

       To compute the partial derivatives with respect to stock variable
       i, seed ds with 1 in the ith position, 0 every else, and
       initialise df to zero.
    */
    virtual void deriv(double df[], const double ds[], 
                       const double sv[], const double fv[])=0;

    /// evaluate expression on sv and current value of fv, storing result
    /// in output variable (of \a fv)
    virtual void eval(double fv[]=&ValueVector::flowVars[0], 
                      const double sv[]=&ValueVector::stockVars[0])=0;
 

    /// set additional tensor operation related parameters
    virtual void setTensorParams(const VariableValue&,const OperationBase&) {}
  };

  /// Legacy EvalOp base interface
  struct ScalarEvalOp: public EvalOpBase
  {
    /// factory method
    static ScalarEvalOp* create(Type op/*=numOps*/);

    virtual void deriv(double df[], const double ds[], 
                       const double sv[], const double fv[]);

    virtual void eval(double fv[]=&ValueVector::flowVars[0], 
                      const double sv[]=&ValueVector::stockVars[0]);
 
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
  struct EvalOp: public classdesc::Poly<EvalOp<T>, ScalarEvalOp>,
                 public classdesc::PolyPack<EvalOp<T> >
  {
    OperationType::Type type() const  override {return T;}
    int numArgs() const override {
      return OperationTypeInfo::numArguments<T>();
    }
    double evaluate(double in1=0, double in2=0) const override;
    double d1(double x1=0, double x2=0) const override;
    double d2(double x1=0, double x2=0) const override;
  };

  // dummy tensor EvalOps to get this mess to link
  template <minsky::OperationType::Type T>
  struct TensorEvalOp: public classdesc::Poly<TensorEvalOp<T>, EvalOpBase>,
                 public classdesc::PolyPack<TensorEvalOp<T> >
  {
    OperationType::Type type() const override {return T;} 
    int numArgs() const override {return OperationTypeInfo::numArguments<T>();}
    void deriv(double df[], const double ds[], 
               const double sv[], const double fv[]) override {}

    void eval(double fv[]=&ValueVector::flowVars[0], 
              const double sv[]=&ValueVector::stockVars[0]) override {}
    
  };

#define TMPEVALOP(t) template <> struct EvalOp<OperationType::t>: public TensorEvalOp<OperationType::t> {};

  TMPEVALOP(sum);
  TMPEVALOP(product);
  TMPEVALOP(infimum);
  TMPEVALOP(supremum);
  TMPEVALOP(any);
  TMPEVALOP(all);
  TMPEVALOP(infIndex);
  TMPEVALOP(supIndex);
  TMPEVALOP(runningSum);
  TMPEVALOP(runningProduct);
  TMPEVALOP(difference);
  TMPEVALOP(innerProduct);
  TMPEVALOP(outerProduct);
  TMPEVALOP(index);
  TMPEVALOP(gather);
  
//  template <minsky::OperationType::Type T>
//  struct ReductionEvalOp: public TensorEvalOp<T>
//  {
//    /// x op= y
//    inline void accum(double& x, double y) const;
//    inline double init() const;
//    void eval(double fv[]=&ValueVector::flowVars[0], 
//              const double sv[]=&ValueVector::stockVars[0]) override;
//  };
//
//  template<> inline
//  double ReductionEvalOp<OperationType::sum>::init() const {return 0;}
//  template<> inline
//  void ReductionEvalOp<OperationType::sum>::accum(double& x, double y) const
//  {x+=y;}
//  template<> inline
//  double ReductionEvalOp<OperationType::product>::init() const {return 1;}
//  template<> inline
//  void ReductionEvalOp<OperationType::product>::accum(double& x, double y) const
//  {x*=y;}
//  template<> inline
//  double ReductionEvalOp<OperationType::infimum>::init() const {return std::numeric_limits<double>::max();}
//  template<> inline
//  void ReductionEvalOp<OperationType::infimum>::accum(double& x, double y) const
//  {if (y<x) x=y;}
//  template<> inline
//  double ReductionEvalOp<OperationType::supremum>::init() const {return -std::numeric_limits<double>::max();}
//  template<> inline
//  void ReductionEvalOp<OperationType::supremum>::accum(double& x, double y) const
//  {if (y>x) x=y;}
//  template<> inline
//  double ReductionEvalOp<OperationType::any>::init() const {return 0;}
//  template<> inline
//  void ReductionEvalOp<OperationType::any>::accum(double& x, double y) const
//  {if (y>0.5) x=1;}
//  template<> inline
//  double ReductionEvalOp<OperationType::all>::init() const {return 1;}
//  template<> inline
//  void ReductionEvalOp<OperationType::all>::accum(double& x, double y) const
//  {x*=(y>0.5);}
//
//  
//  template <minsky::OperationType::Type T>
//  struct ScanEvalOp: public TensorEvalOp<T>
//  {
//    /// parameters describing the axis along which the scan is performed, and the window size of the scan
//    size_t stride=1, dimSz=1,  window=1;
//    inline double init() const;
//    /// x op= y
//    inline void accum(double& x, double y) const;
//    void eval(double fv[]=&ValueVector::flowVars[0], 
//              const double sv[]=&ValueVector::stockVars[0]) override;
//    void setTensorParams(const VariableValue& v,const OperationBase& op) override
//    {
//      v.hypercube().computeStrideAndSize(op.axis,stride,dimSz);
//      window=op.arg<0? dimSz: op.arg;
//    }
//  };
//
//  template<> inline
//  double ScanEvalOp<OperationType::runningSum>::init() const {return 0;}
//  template<> inline
//  void ScanEvalOp<OperationType::runningSum>::accum(double& x, double y) const
//  {x+=y;}
//
//  template<> inline
//  double ScanEvalOp<OperationType::runningProduct>::init() const {return 1;}
//  template<> inline
//  void ScanEvalOp<OperationType::runningProduct>::accum(double& x, double y) const
//  {x*=y;}
//
//
//  
//  template <> struct EvalOp<minsky::OperationType::sum>: public ReductionEvalOp<OperationType::sum> {};
//  template <> struct EvalOp<minsky::OperationType::product>: public ReductionEvalOp<OperationType::product> {};
//  template <> struct EvalOp<minsky::OperationType::infimum>: public ReductionEvalOp<OperationType::infimum> {};
//  template <> struct EvalOp<minsky::OperationType::supremum>: public ReductionEvalOp<OperationType::supremum> {};
//  template <> struct EvalOp<minsky::OperationType::any>: public ReductionEvalOp<OperationType::any> {};
//  template <> struct EvalOp<minsky::OperationType::all>: public ReductionEvalOp<OperationType::all> {};
//  
//  template <> struct EvalOp<minsky::OperationType::runningSum>: public ScanEvalOp<OperationType::runningSum> {};
//  template <> struct EvalOp<minsky::OperationType::runningProduct>: public ScanEvalOp<OperationType::runningProduct> {};
//
//  // not used, but needed for the linker
//  template <> struct EvalOp<minsky::OperationType::difference>: public TensorEvalOp<OperationType::difference> {};
//
//  template <> struct EvalOp<minsky::OperationType::innerProduct>: public TensorEvalOp<OperationType::innerProduct> {};
//  template <> struct EvalOp<minsky::OperationType::outerProduct>: public TensorEvalOp<OperationType::outerProduct> {};
//  template <> struct EvalOp<minsky::OperationType::index>: public TensorEvalOp<OperationType::index>
//  {
//    vector<unsigned> shape;  ///< input argument's shape
//    void eval(double fv[]=&ValueVector::flowVars[0], 
//              const double sv[]=&ValueVector::stockVars[0]) override;
//
//  };
//  template <> struct EvalOp<minsky::OperationType::infIndex>: public TensorEvalOp<OperationType::infIndex>
//  {
//    void eval(double fv[]=&ValueVector::flowVars[0], 
//              const double sv[]=&ValueVector::stockVars[0]) override;
//
//  };
//  template <> struct EvalOp<minsky::OperationType::supIndex>: public TensorEvalOp<OperationType::supIndex>
//  {
//    void eval(double fv[]=&ValueVector::flowVars[0], 
//              const double sv[]=&ValueVector::stockVars[0]) override;
//
//  };
//  template <> struct EvalOp<minsky::OperationType::gather>: public TensorEvalOp<OperationType::gather>
//  {
//    vector<unsigned> shape; ///< input argument's shape
//    void eval(double fv[]=&ValueVector::flowVars[0], 
//              const double sv[]=&ValueVector::stockVars[0]) override;
//  };
  
 struct ConstantEvalOp: public EvalOp<minsky::OperationType::constant>
  {
    double value;
    double evaluate(double in1=0, double in2=0) const override;
 };

//  struct RavelEvalOp: public EvalOp<minsky::OperationType::ravel>
//  {
//    VariableValue in, out;
//    RavelEvalOp() {}
//    RavelEvalOp(const VariableValue& in, const VariableValue& out):
//      in(in), out(out) {}
//    void eval(double*, const double* sv) override;
//    void deriv(double df[], const double ds[], 
//               const double sv[], const double fv[]) override {}
//  };
  
  struct EvalOpPtr: public classdesc::shared_ptr<ScalarEvalOp>, 
                    public OperationType
  {
    EvalOpPtr() {}
    EvalOpPtr(ScalarEvalOp* e): classdesc::shared_ptr<ScalarEvalOp>(e) {}
    EvalOpPtr(OperationType::Type op):
      classdesc::shared_ptr<ScalarEvalOp>(ScalarEvalOp::create(op)) {}
    EvalOpPtr(OperationType::Type op,
              const std::shared_ptr<OperationBase>& state,
              VariableValue& to,
              const VariableValue& from1=VariableValue(), 
              const VariableValue& from2=VariableValue());
  };

  struct EvalOpVector: public vector<EvalOpPtr>
    {
      // override push_back for diagnostic purposes
//       void push_back(const EvalOpPtr& x) {
//         vector<EvalOpPtr>::push_back(x);
//         for (size_t i=0; i<x->in1.size(); ++i)
//           {
//             cout << OperationType::typeName(x->type())<<"(";
//             if (x->numArgs()>0)
//               cout << x->in1[i]<<(x->flow1?"":"s");
//             if (x->numArgs()>1)
//               cout<<","<<x->in2[i]<<(x->flow2?"":"s");
//             cout <<")->"<<x->out+i<<endl;
//           }
//       }
  };


}

#include "evalOp.cd"
#endif
