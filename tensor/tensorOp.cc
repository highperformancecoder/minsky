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

#include "tensorOp.h"
#include <exception>
#include <set>
#include <ecolab_epilogue.h>
using namespace std;

namespace civita
{
  void ReduceArguments::setArguments(const vector<TensorPtr>& a,const std::string&,double)
  {
    set<size_t> indices;
    hypercube({});
    if (!a.empty())
      {
        auto hc=a[0]->hypercube();
        hypercube(hc);
        for (auto& i: a)
          {
            if (i->rank()>0 && i->hypercube()!=hc)
              throw runtime_error("arguments not conformal");
            auto idx=i->index();
            indices.insert(idx.begin(), idx.end());
          }
      }
    args=a;
    m_index.clear();
    m_index.insert(m_index.end(), indices.begin(), indices.end());
  }

  double ReduceArguments::operator[](size_t i) const
  {
    if (args.empty()) return init;
    double r=init; 
    for (auto j: args)
      {
        auto x=j->rank()==0? (*j)[0]: (*j)[i];
        if (!isnan(x)) f(r, x);
      }
    return r;
  }

  ITensor::Timestamp ReduceArguments::timestamp() const
  {
    Timestamp t;
    for (auto& i: args)
      t=max(t, i->timestamp());
    return t;
  }
  
  double ReduceAllOp::operator[](size_t) const
  {
    assert(arg->size()>0);
    double r=init;
    for (size_t i=0; i<arg->size(); ++i)
      {
        double x=(*arg)[i];
        if (!isnan(x)) f(r,x,i);
      }
    return r;
  }

  void ReductionOp::setArgument(const TensorPtr& a, const std::string& dimName,double)
  {
    arg=a;
    dimension=std::numeric_limits<size_t>::max();
    if (arg)
      {
        m_hypercube=arg->hypercube();
        auto& xv=m_hypercube.xvectors;
        for (auto i=xv.begin(); i!=xv.end(); ++i)
          if (i->name==dimName)
            dimension=i-xv.begin();
        if (dimension<arg->rank())
          m_hypercube.xvectors.erase(m_hypercube.xvectors.begin()+dimension);
        else
          m_hypercube.xvectors.clear(); //reduce all, return scalar
      }
    else
      m_hypercube.xvectors.clear();
  }

  
  double ReductionOp::operator[](size_t i) const
  {
    if (dimension>arg->rank())
      return ReduceAllOp::operator[](i);
    else
      {
        auto argDims=arg->hypercube().dims();
        size_t stride=1;
        for (size_t j=0; j<dimension; ++j)
          stride*=argDims[j];
        auto quotRem=ldiv(i, stride); // quotient and remainder calc in one hit
        auto start=quotRem.quot*stride*argDims[dimension] + quotRem.rem;
        assert(stride*argDims[dimension]>0);
        double r=init;
        for (size_t j=0; j<argDims[dimension]; ++j)
          {
            double x=arg->atHCIndex(j*stride+start);
            if (!isnan(x)) f(r,x,j);
          }
        return r;
      }
  }

  double CachedTensorOp::operator[](size_t i) const
  {
    if (m_timestamp<timestamp()) {
      computeTensor();
      m_timestamp=Timestamp::clock::now();
    }
    return cachedResult[i];
  }

  void DimensionedArgCachedOp::setArgument(const TensorPtr& a, const std::string& dimName, double av)
  {
    arg=a;
    argVal=av;
    if (!arg) {m_hypercube.xvectors.clear(); return;}
    m_hypercube=arg->hypercube();
    dimension=std::numeric_limits<size_t>::max();
    auto& xv=m_hypercube.xvectors;
    for (auto i=xv.begin(); i!=xv.end(); ++i)
      if (i->name==dimName)
        dimension=i-xv.begin();
  }

  
  void Scan::computeTensor() const
  {
    if (dimension<rank())
      {
        auto argDims=arg->hypercube().dims();
        size_t stride=1;
        for (size_t j=0; j<dimension; ++j)
          stride*=argDims[j];
        if (argVal>=1 && argVal<argDims[dimension])
          // argVal is interpreted as the binning window. -ve argVal ignored
          for (size_t i=0; i<hypercube().numElements(); i+=stride*argDims[dimension])
            for (size_t j=0; j<stride; ++j)
              for (size_t j1=0; j1<argDims[dimension]*stride; j1+=stride)
                {
                  size_t k=i+j+max(ssize_t(0), ssize_t(j1-ssize_t(argVal-1)*stride));
                  cachedResult[i+j+j1]=arg->atHCIndex(i+j+j1);
                  for (; k<i+j+j1; k+=stride)
                    {
                      f(cachedResult[i+j+j1], arg->atHCIndex(k), k);
                    }
              }
        else // scan over whole dimension
          for (size_t i=0; i<hypercube().numElements(); i+=stride*argDims[dimension])
            for (size_t j=0; j<stride; ++j)
              {
                cachedResult[i+j]=arg->atHCIndex(i+j);
                for (size_t k=i+j+stride; k<i+j+stride*argDims[dimension]; k+=stride)
                  {
                    cachedResult[k] = cachedResult[k-stride];
                    f(cachedResult[k], arg->atHCIndex(k), k);
                  }
              }
          }
    else
      {
        cachedResult[0]=arg->atHCIndex(0);
        for (size_t i=1; i<hypercube().numElements(); ++i)
          {
            cachedResult[i]=cachedResult[i-1];
            f(cachedResult[i], arg->atHCIndex(i), i);
          }
      }
  }

  void Slice::setArgument(const TensorPtr& a,const string& axis, double index)
  {
    arg=a;
    if (arg)
      {
        auto& xv=arg->hypercube().xvectors;
        Hypercube hc;
        // find axis where slicing along
        split=1;
        auto i=xv.begin();
        for (; i!=xv.end(); ++i)
          if (i->name==axis)
            {
              stride=split*i->size();
              break;
            }
          else
            {
              hc.xvectors.push_back(*i);
              split*=i->size();
            }

        if (i==xv.end())
          split=stride=1;
        else
          for (; i!=xv.end(); ++i)
            // finish building hypercube
            hc.xvectors.push_back(*i);
        hypercube(hc);
      }
  }

  void Pivot::setArgument(const TensorPtr& a,const std::string&,double)
  {
    arg=a;
    vector<string> axes;
    for (auto& i: arg->hypercube().xvectors)
      axes.push_back(i.name);
    setOrientation(axes);
  }

  
  void Pivot::setOrientation(const vector<string>& axes)
  {
    map<string,size_t> pMap;
    map<string,XVector> xVectorMap;
    auto& ahc=arg->hypercube();
    for (size_t i=0; i<ahc.xvectors.size(); ++i)
      {
        pMap[ahc.xvectors[i].name]=i;
        xVectorMap[ahc.xvectors[i].name]=ahc.xvectors[i];
      }
    Hypercube hc;
    permutation.clear();
    set<string> axisSet;
    for (auto& i: axes)
      {
        axisSet.insert(i);
        auto v=pMap.find(i);
        if (v==pMap.end())
          throw runtime_error("axis "+i+" not found in argument");
        permutation.push_back(v->second);
        hc.xvectors.push_back(xVectorMap[i]);
      }
    // add remaining axes to permutation in found order
    for (size_t i=0; i<ahc.xvectors.size(); ++i)
      if (!axisSet.count(ahc.xvectors[i].name))
        {
          permutation.push_back(i);
          hc.xvectors.push_back(ahc.xvectors[i]);
        }

    assert(hc.rank()==arg->rank());
    // permute the index vector
    m_index=arg->index();
    for (auto& i: m_index)
      {
        auto idx=arg->hypercube().splitIndex(i);
        vector<size_t> pidx(idx.size());
        for (size_t i=0; i<idx.size(); ++i)
          pidx[i]=idx[permutation[i]];
        i=hc.linealIndex(pidx);
      }
    hypercube(move(hc));
  }

  size_t Pivot::pivotIndex(size_t i) const
  {
    auto idx=hypercube().splitIndex(i);
    vector<size_t> pidx(idx.size());
    for (size_t i=0; i<idx.size(); ++i)
      pidx[permutation[i]]=idx[i];
    return arg->hypercube().linealIndex(pidx);
  }

  namespace
  {
    /// factory method for creating reduction operations
    TensorPtr createReductionOp(minsky::RavelState::HandleState::ReductionOp op)
    {
      switch (op)
        {
        case minsky::RavelState::HandleState::sum: return make_shared<Sum>();
        case minsky::RavelState::HandleState::prod: return make_shared<Product>();
        case minsky::RavelState::HandleState::av: return make_shared<civita::Average>();
        case minsky::RavelState::HandleState::stddev: return make_shared<civita::StdDeviation>();
        case minsky::RavelState::HandleState::min: return make_shared<Min>();
        case minsky::RavelState::HandleState::max: return make_shared<Max>();
        default: throw runtime_error("Reduction "+to_string(op)+" not understood");
        }
    }
  } 

  
  vector<TensorPtr> createRavelChain(const minsky::RavelState& state, const TensorPtr& arg)
  {
    set<string> outputHandles(state.outputHandles.begin(), state.outputHandles.end());
    vector<TensorPtr> chain{arg};
    // TODO sorts and calipers
    for (auto& i: state.handleStates)
      if (!outputHandles.count(i.first))
        {
          auto arg=chain.back();
          if (i.second.collapsed)
            {
              chain.emplace_back(createReductionOp(i.second.reductionOp));
              chain.back()->setArgument(arg, i.first);
            }
          else
            {
              chain.emplace_back(new Slice);
              auto& xv=arg->hypercube().xvectors;
              auto axisIt=find_if(xv.begin(), xv.end(),
                                  [&](const XVector& j){return j.name==i.first;});
              if (axisIt==xv.end()) throw runtime_error("axis "+i.first+" not found");
              auto sliceIt=find_if(axisIt->begin(), axisIt->end(),
                                   [&](const boost::any& j){return str(j,axisIt->dimension.units)==i.second.sliceLabel;});
              // determine slice index
              size_t sliceIdx=0;
              if (sliceIt!=axisIt->end())
                sliceIdx=sliceIt-axisIt->begin();
              chain.back()->setArgument(arg, i.first, sliceIdx);
            }
        }
    auto finalPivot=make_shared<Pivot>();
    finalPivot->setArgument(chain.back());
    finalPivot->setOrientation(state.outputHandles);
    chain.push_back(finalPivot);
    
  }
  
}
