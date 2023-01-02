/*
  @copyright Russell Standish 2019
  @author Russell Standish
  This file is part of Civita.

  Civita is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Civita is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Civita.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CIVITA_TENSOROP_H
#define CIVITA_TENSOROP_H
#include "tensorVal.h"
#include "ravelState.h"

#include <functional>
#include <memory>
#include <vector>

namespace civita
{

  /// perform an operation elementwise over a tensor valued argument
  struct ElementWiseOp: public ITensor
  {
    std::function<double(double)> f;
    std::shared_ptr<ITensor> arg;
    template <class F>
    ElementWiseOp(F f, const std::shared_ptr<ITensor>& arg={}): f(f), arg(arg) {}
    void setArgument(const TensorPtr& a,const std::string&,double) override {arg=a;}
    const Hypercube& hypercube() const override {return arg? arg->hypercube(): m_hypercube;}
    const Index& index() const override {return arg? arg->index(): m_index;}
    double operator[](std::size_t i) const override {return arg? f((*arg)[i]): 0;}
    std::size_t size() const override {return arg? arg->size(): 1;}
    Timestamp timestamp() const override {return arg? arg->timestamp(): Timestamp();}
  };

  /// perform a binary operation elementwise over two tensor arguments.
  /// Arguments need to be conformal: at least one must be a scalar, or both arguments have the same shape
  class BinOp: public ITensor
  {
  protected:
    std::function<double(double,double)> f;
    TensorPtr arg1, arg2;
  public:
    template <class F>
    BinOp(F f, const TensorPtr& arg1={},const TensorPtr& arg2={}):
      f(f) {BinOp::setArguments(arg1,arg2,{},0);}
    
    void setArguments(const TensorPtr& a1, const TensorPtr& a2, const std::string&, double) override;

    double operator[](std::size_t i) const override {
      // missing arguments treated as group identity
      if (!arg1)
        {
          if (arg2)
            return (*arg2)[i];
          else
            throw std::runtime_error("inputs undefined");
        }
      if (!arg2) return (*arg1)[i];
      assert(index().size()==0 || i<index().size());
      auto hcIndex=index().size()? index()[i]: i;
      // scalars are broadcast
      return f(arg1->rank()? arg1->atHCIndex(hcIndex): (*arg1)[0],
               arg2->rank()? arg2->atHCIndex(hcIndex): (*arg2)[0]);
    }
    Timestamp timestamp() const override
    {return max(arg1->timestamp(), arg2->timestamp());}
  };

  /// elementwise reduction over a vector of arguments
  class ReduceArguments: public ITensor
  {
    std::vector<TensorPtr> args;
    std::function<void(double&,double)> f;
    double init;
  public:
    template <class F> ReduceArguments(F f, double init): f(f), init(init) {}
    void setArguments(const std::vector<TensorPtr>& a,const std::string&,double) override;
    double operator[](std::size_t i) const override;
    Timestamp timestamp() const override;
  };
    
    
  /// reduce all elements to a single number
  struct ReduceAllOp: public ITensor
  {
    std::function<void(double&,double,std::size_t)> f;
    double init;
    std::shared_ptr<ITensor> arg;
    void setArgument(const TensorPtr& a,const std::string&,double) override {arg=a;}

    template <class F>
    ReduceAllOp(F f, double init, const std::shared_ptr<ITensor>& arg={}):
      f(f),init(init), arg(arg) {}

    double operator[](std::size_t) const override;
    Timestamp timestamp() const override {return arg->timestamp();}
  };

  /// compute the reduction along the indicated dimension, ignoring
  /// any missing entry (NaNs)
  class ReductionOp: public ReduceAllOp
  {
    std::size_t dimension;
    struct SOI {std::size_t index, dimIndex;};
    std::map<std::size_t, std::vector<SOI>> sumOverIndices;
  public:
   
    template <class F>
    ReductionOp(F f, double init, const TensorPtr& arg={}, const std::string& dimName=""):
      ReduceAllOp(f,init) {ReduceAllOp::setArgument(arg,dimName,0);}

    void setArgument(const TensorPtr& a, const std::string&,double) override;
    double operator[](std::size_t i) const override;
  };

  // general tensor expression - all elements calculated and cached
  class CachedTensorOp: public ITensor
  {
  protected:
    mutable TensorVal cachedResult;
    mutable Timestamp m_timestamp;
    /// computeTensor updates the above two mutable fields, but is
    /// logically const
    virtual void computeTensor() const=0;
  public:
    const Index& index() const override {return cachedResult.index();}
    std::size_t size() const override {return cachedResult.size();}
    double operator[](std::size_t i) const override;
    const Hypercube& hypercube() const override {return cachedResult.hypercube();}
    const Hypercube& hypercube(const Hypercube& hc) override {return cachedResult.hypercube(hc);}
    const Hypercube& hypercube(Hypercube&& hc) override {return cachedResult.hypercube(std::move(hc));}
  };

  /// calculate the sum along an axis or whole tensor
  struct Sum: public ReductionOp
  {
  public:
    Sum(): ReductionOp([](double& x, double y,std::size_t){x+=y;},0) {}
  };
  
  /// calculate the product along an axis or whole tensor
  struct Product: public ReductionOp
  {
  public:
    Product(): ReductionOp([](double& x, double y,std::size_t){x*=y;},1) {}
  };
  
  /// calculate the minimum along an axis or whole tensor
  class Min: public civita::ReductionOp
  {
  public:
    Min(): civita::ReductionOp([](double& x, double y,std::size_t){if (y<x) x=y;},std::numeric_limits<double>::max()){}
   };
  /// calculate the maximum along an axis or whole tensor
  class Max: public civita::ReductionOp
  {
  public:
    Max(): civita::ReductionOp([](double& x, double y,std::size_t){if (y>x) x=y;},-std::numeric_limits<double>::max()){}
   };

  /// calculates the average along an axis or whole tensor
  struct Average: public ReductionOp
  {
    mutable std::size_t count;
  public:
    Average(): ReductionOp([this](double& x, double y,std::size_t){x+=y; ++count;},0) {}
    double operator[](std::size_t i) const override
    {count=0; return ReductionOp::operator[](i)/count;}
  };

  /// calculates the standard deviation along an axis or whole tensor
  struct StdDeviation: public ReductionOp
  {
    mutable std::size_t count;
    mutable double sqr;
  public:
    StdDeviation(): ReductionOp([this](double& x, double y,std::size_t){x+=y; sqr+=y*y; ++count;},0) {}
    double operator[](std::size_t i) const override {
      count=0; sqr=0;
      double av=ReductionOp::operator[](i)/count;
      return sqrt(std::max(0.0, sqr/count-av*av));
    }
  };
  
  struct DimensionedArgCachedOp: public CachedTensorOp
  {
    /// dimension to apply operation over. >rank = all dims
    std::size_t dimension=std::numeric_limits<std::size_t>::max(); 
    /// op arg value, eg binsize or delta in difference op
    double argVal=0;
    TensorPtr arg;
    void setArgument(const TensorPtr& a, const std::string&,double) override;
    Timestamp timestamp() const override {return arg? arg->timestamp(): Timestamp();}
  };
  
  class Scan: public DimensionedArgCachedOp
  {
  public:
    std::function<void(double&,double,std::size_t)> f;
    template <class F>
    Scan(F f, const TensorPtr& arg={}, const std::string& dimName="", double av=0): f(f) 
    {Scan::setArgument(arg,dimName,av);}
    void setArgument(const TensorPtr& arg, const std::string& dimName,double argVal) override {
      DimensionedArgCachedOp::setArgument(arg,dimName,argVal);
      if (arg)
        cachedResult.hypercube(arg->hypercube());
      // TODO - can we handle sparse data?
    }      
    void computeTensor() const override;
  };

  /// corresponds to OLAP slice operation
  class Slice: public ITensor
  {
    std::size_t stride=1, split=1, sliceIndex=0;
    TensorPtr arg;
    std::vector<std::size_t> arg_index;
  public:
    void setArgument(const TensorPtr& a,const std::string&,double) override;
    double operator[](std::size_t i) const override;
    Timestamp timestamp() const override {return arg->timestamp();}
  };

  /// corresponds to the OLAP pivot operation
  class Pivot: public ITensor
  {
    std::vector<std::size_t> permutation;   /// permutation of axes
    std::vector<std::size_t> permutedIndex; /// argument indices corresponding to this indices, when sparse
    TensorPtr arg;
    // returns hypercube index of arg given hypercube index of this
    std::size_t pivotIndex(std::size_t i) const;
  public:
    void setArgument(const TensorPtr& a,const std::string& axis="",double arg=0) override;
    /// set's the pivots orientation
    /// @param axes - list of axes that are the output
    void setOrientation(const std::vector<std::string>& axes);
    double operator[](std::size_t i) const override;
    Timestamp timestamp() const override {return arg->timestamp();}
  };

  class PermuteAxis: public ITensor
  {
    TensorPtr arg;
    std::size_t m_axis;
    std::vector<std::size_t> m_permutation;
    std::vector<std::size_t> permutedIndex; /// argument indices corresponding to this indices, when sparse
  public:
    void setArgument(const TensorPtr& a,const std::string& axis="",double arg=0) override;
    void setPermutation(const std::vector<std::size_t>& p)
    {auto pp(p); setPermutation(std::move(pp));}
    void setPermutation(std::vector<std::size_t>&&);
    std::size_t axis() const {return m_axis;}
    const std::vector<std::size_t>& permutation() const {return m_permutation;}
    double operator[](std::size_t i) const override;
    Timestamp timestamp() const override {return arg->timestamp();}
  };

  /// If a rank 1 argument, sort by the value of the argument
  class SortByValue: public CachedTensorOp
  {
    TensorPtr arg;
    ravel::HandleSort::Order order;
  public:
    SortByValue(ravel::HandleSort::Order order): order(order) {}
    void setArgument(const TensorPtr& a,const std::string& ={},double=0) override {
      if (a->rank()!=1)
        throw std::runtime_error("Sort by Value only applicable for rank 1 tensors");
      else
        arg=a;
      cachedResult.hypercube(a->hypercube()); // no data, unsorted
    }
    void computeTensor() const override;
    Timestamp timestamp() const override {return arg->timestamp();}
    const Hypercube& hypercube() const override {
      if (timestamp()>m_timestamp) computeTensor();
      return cachedResult.hypercube();
    }
    std::size_t size() const override {
      if (timestamp()>m_timestamp) computeTensor();
      return cachedResult.size();
    }
  };

  class SpreadBase: public ITensor
  {
  protected:
    TensorPtr arg;
    std::size_t numSpreadElements=1;
  public:
    void setArgument(const TensorPtr& a,const std::string& ax="",double ag=0) override {
      arg=a;
      if (arg){
        hypercube(arg->hypercube());
        m_index=arg->index();
      }
    }
    Timestamp timestamp() const override {return arg? arg->timestamp(): Timestamp();}
  };
    
  class SpreadFirst: public SpreadBase
  {
  public:
    void setSpreadDimensions(const Hypercube& hc) {
      if (!arg) return;
      numSpreadElements=hc.numElements();
      m_hypercube=hc;
      m_hypercube.xvectors.insert(m_hypercube.xvectors.end(), arg->hypercube().xvectors.begin(),
                                  arg->hypercube().xvectors.end());
      std::set<std::size_t> idx;
      for (auto& i: arg->index())
        for (std::size_t j=0; j<numSpreadElements; ++j)
          idx.insert(j+i*numSpreadElements);
      m_index=idx;
    }
    
    double operator[](std::size_t i) const override {
      if (arg) return (*arg)[i/numSpreadElements];
      return nan("");
    }
  };

  class SpreadLast: public SpreadBase
  {
  public:
    void setSpreadDimensions(const Hypercube& hc) {
      if (!arg) return;
      numSpreadElements=hc.numElements();
      m_hypercube=arg->hypercube();
      m_hypercube.xvectors.insert(m_hypercube.xvectors.end(), hc.xvectors.begin(), hc.xvectors.end());
      std::set<std::size_t> idx;
      for (auto& i: arg->index())
        for (std::size_t j=0; j<numSpreadElements; ++j)
          idx.insert(i+j*arg->size());
      m_index=idx;
    }
    
    double operator[](std::size_t i) const override {
      if (arg) return (*arg)[i%arg->size()];
      return nan("");
    }
  };

  /// spread the argument over the given hypercube. Entries in hypercube not in arg are NAN.
  class SpreadOverHC: public ITensor
  {
    TensorPtr arg;
    std::vector<std::vector<std::size_t>> permutations;
  public:
    /// set the destination hypercube prior to calling this
    /// order of axes must mathc hypercube
    void setArgument(const TensorPtr& a,const std::string& axis="",double ag=0) override;
    double operator[](size_t i) const override;
    Timestamp timestamp() const override {return arg? arg->timestamp(): Timestamp();}
  };
  
  /// Combines tensors in such a way that if the first argument doesn't return a value, the second argument is checked and so on
  class Meld: public ITensor
  {
    std::vector<TensorPtr> args;
  public:
    
    /// all arguments must have the same hypercube
    void setArguments(const std::vector<TensorPtr>& a, const std::string& d="", double av=0) override;
    Timestamp timestamp() const override;
    double operator[](std::size_t) const override;
  };

  /// stacks tensors along an extra dimension
  class Merge: public ITensor
  {
    std::vector<TensorPtr> args;
  public:
    
    /// all arguments must have the same hypercube
    /// @param dimension - name of the new dimension
    void setArguments(const std::vector<TensorPtr>& a, const std::string& dimension, double) override;
    Timestamp timestamp() const override;
    double operator[](size_t i) const override;
  };
    
  /// creates a chain of tensor operations that represents a Ravel in
  /// state \a state, operating on \a arg
  std::vector<TensorPtr> createRavelChain(const ravel::RavelState&, const TensorPtr& arg);

}

#endif
