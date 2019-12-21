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

namespace civita
{
  /// represent a tensor in initialisation expressions
  struct TensorVal: public ITensor
  {
    std::vector<size_t> m_index;
    std::vector<double> data;
    TensorVal() {}
    TensorVal(double x): data(1,x) {}
    TensorVal(const Hypercube& hc): ITensor(hc) {}
    TensorVal(Hypercube&& hc): ITensor(std::move(hc)) {}
    TensorVal(const std::vector<unsigned>& dims): ITensor(dims) {}
    
    const std::vector<size_t>& index() const override {return m_index;}
    double operator[](size_t i) const override {return data[i];}
    size_t size() const override {return data.size();}
  };

  /// for use in Minsky init expressions
  inline TensorVal operator*(double a, const TensorVal& x)
  {
    TensorVal r(x);
    for (auto& i: r.data) i*=a;
    return r;
  }
}

//#include "tensorVal.cd"
//#include "tensorVal.xcd"
#endif
