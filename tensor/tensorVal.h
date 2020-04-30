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

#ifndef CIVITA_TENSORVAL_H
#define CIVITA_TENSORVAL_H

#include "tensorInterface.h"
#include <vector>
#include <chrono>

namespace civita
{
  /// abstraction of a tensor variable, stored in contiguous memory
  struct ITensorVal: public ITensor
  {
    ITensorVal() {}
    ITensorVal(const Hypercube& hc): ITensor(hc) {}
    ITensorVal(Hypercube&& hc): ITensor(std::move(hc)) {}
    ITensorVal(const std::vector<unsigned>& dims): ITensor(dims) {}
    virtual const ITensorVal& operator=(const ITensor&)=0;

    virtual double& operator[](size_t)=0;
    using ITensor::operator[];
    using ITensor::operator();
    template <class T>
    double& operator()(const std::initializer_list<T>& indices)
    {
      auto idx=index();
      auto hcIdx=hcIndex(indices);
      if (idx.empty())
        return operator[](hcIdx);
      else
        {
          auto i=std::lower_bound(idx.begin(), idx.end(), hcIdx);
          if (i!=idx.end() && *i==hcIdx)
            return operator[](i-idx.begin()); 
          static double noValue;
          return noValue;
        }
    }
          
    virtual void index(const std::vector<size_t>&)=0;
    using ITensor::index;
    
    typedef double* iterator;
    typedef const double* const_iterator;

    const_iterator begin() const {return const_cast<ITensorVal*>(this)->begin();}
    const_iterator end() const {return begin()+size();}
    iterator begin() {return &((*this)[0]);}
    iterator end() {return begin()+size();}
  };
  
  /// represent a tensor in initialisation expressions
  class TensorVal: public ITensorVal
  {
    std::vector<double> data;
    Timestamp m_timestamp;
    CLASSDESC_ACCESS(TensorVal);
  public:
    TensorVal() {}
    TensorVal(double x): data(1,x) {}
    TensorVal(const Hypercube& hc): ITensorVal(hc) {}
    TensorVal(Hypercube&& hc): ITensorVal(std::move(hc)) {}
    TensorVal(const std::vector<unsigned>& dims): ITensorVal(dims) {}

    using ITensorVal::index;
    void index(const std::vector<size_t>& idx) override
    {m_index=idx; if (!idx.empty()) data.resize(idx.size());}
    const Hypercube& hypercube(const Hypercube& hc) override
    {m_hypercube=hc; allocVal(); return m_hypercube;}
    const Hypercube& hypercube(Hypercube&& hc) override 
    {m_hypercube=std::move(hc);allocVal();return m_hypercube;}
    using ITensor::hypercube;

    void allocVal() {if (m_index.empty()) data.resize(hypercube().numElements());}

    void push_back(size_t index, double val) {
      if (m_index.empty()) data.clear();
      m_index.push_back(index);
      data.push_back(val);
    }
    
    double operator[](size_t i) const override {return data[i];}
    double& operator[](size_t i) override {return data[i];}
    size_t size() const override {return data.size();}
    const TensorVal& operator=(const ITensor& x) override {
      hypercube(x.hypercube());
      m_index=index();
      data.resize(x.size());
      for (size_t i=0; i<x.size(); ++i) data[i]=x[i];
      updateTimestamp();
      return *this;
    }

    Timestamp timestamp() const override {return m_timestamp;}
    // timestamp should be updated every time the data r index vectors
    // is updated, if using the CachedTensorOp functionality
    void updateTimestamp() {m_timestamp=std::chrono::high_resolution_clock::now();}
  };

  /// for use in Minsky init expressions
  inline TensorVal operator*(double a, const TensorVal& x)
  {
    TensorVal r(x);
    for (auto& i: r) i*=a;
    return r;
  }
}

//#include "tensorVal.cd"
//#include "tensorVal.xcd"
#endif
