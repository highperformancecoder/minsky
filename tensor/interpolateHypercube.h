/*
  @copyright Russell Standish 2020
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

#ifndef CIVITA_INTERPOLATE_HYPERCUBE_H
#define CIVITA_INTERPOLATE_HYPERCUBE_H
#include "tensorOp.h"
using namespace std;

namespace civita
{
  /// Interpolates its argument to its hypercube
  /// rank must equal arg->rank(), and xvectors type must match
  class InterpolateHC: public ITensor
  {
    TensorPtr arg;
    ///< hypercube that's been rotated to match the arguments hypercube
    Hypercube interimHC;
    std::vector<size_t> strides; ///<strides along each dimension of this->hypercube()
    std::vector<size_t> rotation; ///< permutation of axes of interimHC and this->hypercube()
    //
    vector<size_t> splitAndRotate(size_t) const;
  public:
    void setArgument(const TensorPtr& a, const string&,double) override;
    std::vector<size_t> index() const {return {};}
    double operator[](size_t) const;
    size_t size() const {return hypercube().numElements();}
    Timestamp timestamp() const override {return arg->timestamp();}
  };
  
}

#endif
