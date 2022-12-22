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

#include "interpolateHypercube.h"
#include <ecolab_epilogue.h>

namespace civita
{

  vector<size_t> InterpolateHC::splitAndRotate(size_t hcIndex) const
  {
    vector<size_t> r(rank());
    const auto& h=hypercube();
    for (size_t dim=0; dim<rank(); ++dim)
      r[rotation[dim]] = (hcIndex/strides[dim]) % h.xvectors[dim].size();
    return r;
  }

  
  void InterpolateHC::setArgument(const TensorPtr& a, const string&,double)
  {
    arg=a;
    if (rank()!=arg->rank())
      throw runtime_error("Rank of interpolated tensor doesn't match its argument");
    // reorder hypercube for type and name
    interimHC.xvectors.clear();
    size_t stride=1;
    const auto& targetHC=hypercube().xvectors;
    rotation.clear();
    rotation.resize(rank(), rank());
    // ensure rotation vector will contain unique indices
    std::map<size_t,size_t> tmpRotation;
    // construct interim hypercube and its rotation permutation
    for (size_t i=0; i<rank(); ++i)
      {
        const auto& src=arg->hypercube().xvectors[i];
        sortAndAdd(src);
        if (src.name.empty())
          {
            const auto& dst=targetHC[i];
            if (!dst.name.empty() || src.dimension.type!=dst.dimension.type ||
                src.dimension.units!=dst.dimension.units) //TODO handle conversion between different units
              throw runtime_error("mismatch between unnamed dimensions");
            interimHC.xvectors.push_back(dst);
            tmpRotation.emplace(make_pair(i,i));
          }
        else // find matching names dimension
          {
            auto dst=find_if(targetHC.begin(), targetHC.end(),
                             [&](const XVector& i){return i.name==src.name;});
            if (dst==targetHC.end())
            {
              // possible alternative when targetHC has no xvectors or undefined ones. for feature 147
              interimHC.xvectors.push_back(src);
              tmpRotation.emplace(make_pair(i,i));
            }
            else
              {
                interimHC.xvectors.push_back(*dst);
                tmpRotation.emplace(make_pair(dst-targetHC.begin(),i));
              }
          }
        strides.push_back(stride);
        stride*=targetHC[i].size();
      }
    if (tmpRotation.size()!=rank()) throw runtime_error("rotation of indices is not a permutation"); 
    for (auto& i: tmpRotation) rotation[i.first]=i.second;
#ifndef NDEBUG
    for (auto& i: rotation) assert(i<rank()); // check that no indices have been doubly assigned.
    // Now we're sure rotation is a permutation
#endif
    if (index().empty())
      for (size_t i=0; i<size(); ++i)
        weightedIndices.push_back(bodyCentredNeighbourhood(i));
    else
      for (auto i: index())
        weightedIndices.push_back(bodyCentredNeighbourhood(i));
  }

  void InterpolateHC::sortAndAdd(const XVector& xv)
  {
    sortedArgHC.emplace_back();
    auto& s=sortedArgHC.back();
    for (size_t i=0; i<xv.size(); ++i)
      s.second.push_back(i);
    AnyLess less;
    sort(s.second.begin(), s.second.end(),
         [&](size_t i, size_t j){return less(xv[i],xv[j]);});
    for (auto i: s.second)
      s.first.push_back(xv[i]);
    assert(sorted(s.first.begin(), s.first.end()));
  }

  
  double InterpolateHC::operator[](size_t idx) const
  {
    if (idx<weightedIndices.size())
      {
        if (weightedIndices[idx].empty()) return nan("");
        double r=0;
        for (const auto& i: weightedIndices[idx])
          {
            assert(i.index<arg->hypercube().numElements());
            r+=i.weight * arg->atHCIndex(i.index);
          }
        return r;
      }
    return nan("");
  }

  InterpolateHC::WeightedIndexVector InterpolateHC::bodyCentredNeighbourhood(size_t destIdx) const
  {
    // note this agorithm is limited in rank (typically 32 dims on 32bit machine, or 64 dims on 64bit)
    if (rank()>sizeof(size_t)*8)
      throw runtime_error("Ranks > "+to_string(sizeof(size_t)*8)+" not supported");
    size_t numNeighbours=size_t(1)<<rank();
    vector<size_t> iIdx=splitAndRotate(destIdx);
    const auto& argHC=arg->hypercube();
    // loop over the nearest neighbours in argument hypercube space of
    // this point in interimHypercube space
    double sumWeight=0;
    WeightedIndexVector r;
    auto argIndexVector=arg->index();
    // multivariate interpolation - eg see Abramowitz & Stegun 25.2.66
    for (size_t nbr=0; nbr<numNeighbours; ++nbr)
      {
        size_t argIdx=0;
        double weight=1;
        size_t idx=0;
        for (size_t dim=0, stride=1; dim<rank(); stride*=argHC.xvectors[dim].size(), ++dim)
          {
            const auto& x=sortedArgHC[dim].first;
            assert(!x.empty());
            assert(sorted(x.begin(),x.end()));
            auto v=interimHC.xvectors[dim][iIdx[dim]];
            auto lesserIt=std::upper_bound(x.begin(), x.end(), v, AnyLess());
            if (lesserIt!=x.begin()) --lesserIt; // find greatest value <= v, 
            any lesser=*lesserIt, greater;
            if (diff(lesser, v)>=0)
              greater=lesser;  // one sided interpolation for beginning or exact match 
            else if (lesserIt+1==x.end())
              greater=x.back(); // one sided interpolation for end
            else
              greater=*(lesserIt+1);

            bool greaterSide = nbr&(size_t(1)<<dim); // on higher side of hypercube
            double d=diff(greater,lesser);
            if (d==0 && greaterSide) goto nextNeighbour; // already taken lesserVal at weight 1.

            idx += sortedArgHC[dim].second[(lesserIt-x.begin()+greaterSide)]*stride;
            assert(idx<argHC.numElements());
            if (d>0)
              weight *= greaterSide? diff(v,lesser): d-diff(v,lesser);
          }
        if (index().empty())
          {
            r.emplace_back(WeightedIndex{idx,weight});
            sumWeight+=weight;
          }
        else
          {
            auto indexV=index();
            if (binary_search(indexV.begin(), indexV.end(), idx))
              {
                r.emplace_back(WeightedIndex{idx,weight});
                sumWeight+=weight;
              }
          }
      nextNeighbour:;
      }

    // normalise weights
    for (auto& i: r) i.weight/=sumWeight;
    return r;
  }

}
