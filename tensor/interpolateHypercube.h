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
    /// hypercube that's been rotated to match the arguments hypercube
    Hypercube interimHC;
    std::vector<size_t> strides; ///<strides along each dimension of this->hypercube()
    std::vector<size_t> rotation; ///< permutation of axes of interimHC and this->hypercube()
    
    std::vector<std::pair<XVector, std::vector<size_t>>> sortedArgHC;
    void sortAndAdd(const XVector&);

    //
    vector<size_t> splitAndRotate(size_t) const;
    
    /// structure for referring to an argument index and its weight 
    struct WeightedIndex
    {
      size_t index;
      double weight;
      WeightedIndex(size_t index,double weight): index(index), weight(weight) {}
    };

    struct WeightedIndexVector: public vector<WeightedIndex>
    {
      bool argIsOnDestHypercube=false;
    };
  
    /// map from this tensor's index into the argument tensor
    vector<WeightedIndexVector> weightedIndices;

    /// computes the neighbourhood around a target argument index when
    /// the target index is not on the hypercube
    WeightedIndexVector bodyCentredNeighbourhood(size_t idx) const;

  public:
    void setArgument(const TensorPtr& a, const string&,double) override;
    double operator[](size_t) const;
    Timestamp timestamp() const override {return arg->timestamp();}
  };
  
}

#endif
