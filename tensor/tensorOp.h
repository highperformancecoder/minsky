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

#include <functional>
#include <memory>
#include <vector>

namespace civita
{
  using TensorPtr=std::shared_ptr<ITensor>;
  
  class TensorOp: public ITensor
  {
    void notImpl() const
    {throw std::runtime_error("setArgument(s) variant not implemented");}
  public:
    virtual void setArgument(const TensorPtr&, const std::string& dimension={}) {notImpl();}
    virtual void setArguments(const TensorPtr&, const TensorPtr&) {notImpl();}
    virtual void setArguments(const std::vector<TensorPtr>&) {notImpl();}
    virtual void setArguments(const std::vector<TensorPtr>&,
                              const std::vector<TensorPtr>&) {notImpl();}
  };

  /// perform an operation elementwise over a tensor valued argument
  struct ElementWiseOp: public TensorOp
  {
    std::function<double(double)> f;
    std::shared_ptr<ITensor> arg;
    template <class F>
    ElementWiseOp(F f, const std::shared_ptr<ITensor>& arg={}): f(f), arg(arg) {}
    void setArgument(const TensorPtr& a,const std::string&) override {arg=a;}
    std::vector<size_t> index() const override {return arg->index();}
    double operator[](size_t i) const override {return f((*arg)[i]);}
    size_t size() const override {return arg->size();}
    Timestamp timestamp() const override {return arg->timestamp();}
  };

  /// perform a binary operation elementwise over two tensor arguments.
  /// Arguments need to be conformal: at least one must be a scalar, or both arguments have the same shape
  class BinOp: public TensorOp
  {
    std::function<double(double,double)> f;
    TensorPtr arg1, arg2;
  public:
    template <class F>
    BinOp(F f, const TensorPtr& arg1={},const TensorPtr& arg2={}):
      f(f) {setArguments(arg1,arg2);}
    
    void setArguments(const TensorPtr& a1, const TensorPtr& a2) override {
      if (arg1->rank()!=0 && arg2->rank()!=0 && arg1->hypercube().dims()==arg2->hypercube().dims())
        throw std::runtime_error("arguments not conformal");
    }
    
    std::vector<size_t> index() const override {return {};}
    double operator[](size_t i) const override
    {return f(arg1->atHCIndex(i),arg2->atHCIndex(i));}
    size_t size() const override {return arg1->size();}
    Timestamp timestamp() const override
    {return max(arg1->timestamp(), arg2->timestamp());}
  };

  /// elementwise reduction over a vector of arguments
  class ReduceArguments: public TensorOp
  {
    std::vector<TensorPtr> args;
    std::vector<size_t> m_index;
    std::function<void(double&,double)> f;
    
  public:
    template <class F> ReduceArguments(F f): f(f) {}
    void setArguments(const std::vector<TensorPtr>& a) override;
    std::vector<size_t> index() const override {return m_index;}
    double operator[](size_t i) const override;
    size_t size() const override {return m_index.empty()? hypercube().numElements(): m_index.size();}
    Timestamp timestamp() const override;
  };
    
    
  /// reduce all elements to a single number
  struct ReduceAllOp: public TensorOp
  {
    std::function<void(double&,double)> f;
    std::shared_ptr<ITensor> arg;
    void setArgument(const TensorPtr& a,const std::string&) override {arg=a;}

    template <class F>
    ReduceAllOp(F f, const std::shared_ptr<ITensor>& arg={}): f(f), arg(arg) {}

    std::vector<size_t> index() const override {return {};}
    size_t size() const override {return 1;}
    double operator[](size_t) const override;
    Timestamp timestamp() const override {return arg->timestamp();}
  };

  /// compute the reduction along the indicated dimension, ignoring
  /// any missing entry (NaNs)
  class ReductionOp: public ReduceAllOp
  {
    size_t dimension;
  public:
   
    template <class F>
    ReductionOp(F f, const TensorPtr& arg={}, const std::string& dimName=""):
      ReduceAllOp(f) {setArgument(arg,dimName);}

    void setArgument(const TensorPtr& a, const std::string& dimName={}) override;
    size_t size() const override {return hypercube().numElements();}
    double operator[](size_t i) const override;
  };

  // general tensor expression - all elements calculated and cached
  class CachedTensorOp: public TensorOp
  {
  protected:
    mutable TensorVal cachedResult;
    mutable Timestamp m_timestamp;
    /// computeTensor updates the above two mutable fields, but is
    /// logically const
    virtual void computeTensor() const=0;
  public:
    std::vector<size_t> index() const override {return cachedResult.index();}
    size_t size() const override {return cachedResult.size();}
    double operator[](size_t i) const override;
  };

  class Scan: public CachedTensorOp
  {
    size_t dimension; // dimension to scan over. >rank = all dims
    TensorPtr arg;
  public:
    std::function<void(double&,double)> f;
    template <class F>
    Scan(F f, const TensorPtr& arg={}, const std::string& dimName="") {setArgument(arg,dimName);}
    void setArgument(const TensorPtr& a, const std::string& dimName={}) override;
    void computeTensor() const override;
    Timestamp timestamp() const override {return arg->timestamp();}
  };

}

#endif
