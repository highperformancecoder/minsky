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

#ifndef CLASSDESC_ACCESS
#define CLASSDESC_ACCESS(x)
#endif
#include <chrono>

namespace civita
{
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
    
    /// the index vector - assumed to be ordered and unique
    virtual std::vector<size_t> index() const=0;
    /// return or compute data at a location
    virtual double operator[](size_t) const=0;
    /// return number of elements in tensor - maybe less than hypercube.numElements if sparse
    virtual size_t size() const=0;
    
    /// returns the data value at hypercube index \a hcIdx, or NaN if 
    double atHCIndex(size_t hcIdx) const {
      auto idx=index();
      if (idx.empty()) {
        assert(hcIdx<size());
        return (*this)[hcIdx]; // this is dense
      }
      assert(idx.size()==size());
      assert(std::all_of(idx.begin()+1, idx.end(), [](const size_t& i){return i>*(&i-1);}));
      auto i=std::lower_bound(idx.begin(), idx.end(), hcIdx);
      if (*i==hcIdx) return (*this)[i-idx.begin()]; // hcIdx found, return data element with same offset
      return nan("");
    }

    using Timestamp=std::chrono::time_point<std::chrono::high_resolution_clock>;
    /// timestamp indicating how old the dependendent data might
    /// be. Used in CachedTensorOp to determine when to invalidate the
    /// cache
    virtual Timestamp timestamp() const=0;

    
  protected:
    Hypercube m_hypercube;
  };
}

#endif
