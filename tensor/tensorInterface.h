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

#ifndef CIVITA_TENSORINTERFACE_H
#define CIVITA_TENSORINTERFACE_H
#include "hypercube.h"
#include "index.h"

#ifndef CLASSDESC_ACCESS
#define CLASSDESC_ACCESS(x)
#endif
#include <chrono>
#include <set>

namespace civita
{
  class ITensor;
  using TensorPtr=std::shared_ptr<ITensor>;

  class ITensor
  {
  public:
    CLASSDESC_ACCESS(ITensor);
    ITensor() {}
    ITensor(const Hypercube& hc): m_hypercube(hc) {}
    ITensor(Hypercube&& hc): m_hypercube(std::move(hc)) {}
    ITensor(const std::vector<unsigned>& dims) {m_hypercube.dims(dims);}
    virtual ~ITensor() {}
    /// information describing the axes, types and labels of this tensor
    virtual const Hypercube& hypercube() const {return m_hypercube;}
    virtual const Hypercube& hypercube(const Hypercube& hc) {return m_hypercube=hc;}
    virtual const Hypercube& hypercube(Hypercube&& hc) {return m_hypercube=std::move(hc);}
    size_t rank() const {return hypercube().rank();}
    std::vector<unsigned> shape() const {return hypercube().dims();}

    /// impose dimensions according to dimension map \a dimensions
    void imposeDimensions(const Dimensions& dimensions) {
      auto hc=hypercube();
      for (auto& xv: hc.xvectors)
        {
          auto dim=dimensions.find(xv.name);
          if (dim!=dimensions.end())
            {
              xv.dimension=dim->second;
              xv.imposeDimension();
            }
        }
      hypercube(std::move(hc));
    }
    
    /// the index vector - assumed to be ordered and unique
    virtual const Index& index() const {return m_index;}
    /// return or compute data at a location
    virtual double operator[](size_t) const=0;
    /// return number of elements in tensor - maybe less than hypercube.numElements if sparse
    virtual size_t size() const {
      size_t s=index().size();
      return s? s: hypercube().numElements();
    }        
    
    /// returns the data value at hypercube index \a hcIdx, or NaN if 
    double atHCIndex(size_t hcIdx) const {
      auto& idx=index();
      if (idx.empty()) {// this is dense
        if (hcIdx<size())
          return (*this)[hcIdx]; 
      } else {
        auto i=idx.linealOffset(hcIdx);
        if (i<idx.size())
          return (*this)[i];
      }
      return nan(""); //element not found
    }

    template <class T>
    size_t hcIndex(const std::initializer_list<T>& indices) const
    {return hypercube().linealIndex(indices);}

    template <class T>
    double operator()(const std::initializer_list<T>& indices) const
    {return atHCIndex(hcIndex(indices));}
                       
    using Timestamp=std::chrono::time_point<std::chrono::high_resolution_clock>;
    /// timestamp indicating how old the dependendent data might
    /// be. Used in CachedTensorOp to determine when to invalidate the
    /// cache
    virtual Timestamp timestamp() const=0;

    /// arguments relevant for tensor expressions, not always meaningful. Exception thrown if not.
    virtual void setArgument(const TensorPtr&, const std::string& dimension={},
                             double argVal=0)  {notImpl();}
    virtual void setArguments(const TensorPtr&, const TensorPtr&) {notImpl();}
    virtual void setArguments(const std::vector<TensorPtr>& a,
                              const std::string& dimension={}, double argVal=0) 
    {if (a.size()) setArgument(a[0], dimension, argVal);}
    virtual void setArguments(const std::vector<TensorPtr>& a1,
                              const std::vector<TensorPtr>& a2)
    {setArguments(a1.empty()? TensorPtr(): a1[0], a2.empty()? TensorPtr(): a2[0]);}
   
  protected:
    Hypercube m_hypercube;
    Index m_index;
    void notImpl() const
    {throw std::runtime_error("setArgument(s) variant not implemented");}
  };

}

#endif
