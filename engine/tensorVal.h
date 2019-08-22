/*
  @copyright Steve Keen 2018
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
#ifndef TENSOR_VAL
#define TENSOR_VAL

#include <vector>

namespace minsky
{
  /// represent a tensor in initialisation expressions
  struct TensorVal
  {
    std::vector<unsigned> dims;
    std::vector<double> data;
    TensorVal() {}
    TensorVal(double x): data(1,x) {}
  };

  inline TensorVal operator*(double a, const TensorVal& x)
  {
    TensorVal r;
    r.dims=x.dims;
    r.data.reserve(x.data.size());
    for (auto i: x.data) r.data.push_back(a*i);
    return r;
  }
}

#include "tensorVal.cd"
#include "tensorVal.xcd"
#endif
