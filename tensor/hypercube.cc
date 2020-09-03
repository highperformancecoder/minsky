/*
  @copyright Steve Keen 2019
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

#include "hypercube.h"
#include <error.h>
#include <set>

using namespace std;

namespace civita
{
  std::vector<unsigned> Hypercube::dims() const
  {
    std::vector<unsigned> d;
    for (auto& i: xvectors) d.push_back(i.size());
    return d;
  }

  const std::vector<unsigned>& Hypercube::dims(const std::vector<unsigned>& d) {
    xvectors.clear();
    for (size_t i=0; i<d.size(); ++i)
      {
        xvectors.emplace_back(std::to_string(i));
        xvectors.back().dimension.type=Dimension::value;
        for (size_t j=0; j<d[i]; ++j)
          xvectors.back().emplace_back(double(j));
      }
    return d;
  }

  size_t Hypercube::numElements() const
    {
      size_t s=1;
      for (auto& i: xvectors) s*=i.size();
      return s;
    }
  
  void Hypercube::makeConformant(const Hypercube& a)
  {
    for (auto& i: a.xvectors)
      {
        size_t j=0;
        for (j=0; j<xvectors.size(); ++j)
          if (xvectors[j].name==i.name)
            {
              if (xvectors[j].dimension.type!=i.dimension.type)
                throw ecolab::error("dimension %s has inconsistent type",i.name.c_str());
              // only match labels for string dimensions. Other types are interpolated.
              XVector newLabels;
              switch (i.dimension.type)
                {
                case Dimension::string:
                  {
                    set<string> alabels;
                    for (auto& k: i)
                      alabels.insert(str(k));
                    for (auto k: xvectors[j])
                      if (alabels.count(str(k)))
                        newLabels.push_back(k);
                    break;
                  }
                default:
                  {
                    // set overlapping value ranges
                    set<boost::any, AnyLess> vals(i.begin(), i.end());
                    for (auto k: xvectors[j])
                      if (diff(k, *vals.begin())>=0 && diff(k, *vals.rbegin())<=0)
                        newLabels.push_back(k);
                    break;
                  }
                }
              xvectors[j].swap(newLabels);
              break;
            }
        if (j==xvectors.size()) // axis not present on LHS, so increase rank
          xvectors.push_back(i);
      }
    if (rank()==0 && a.rank()>0)
      throw ecolab::error("tensors nonconformant");
  }

    /// split lineal index into components along each dimension
    vector<size_t> Hypercube::splitIndex(size_t i) const
    {
      std::vector<size_t> splitIndex;
      for (auto& d: dims())
        {
          auto res=div(ssize_t(i),ssize_t(d));
          splitIndex.push_back(res.rem);
          i=res.quot;
        }
      return splitIndex;
    }
}

