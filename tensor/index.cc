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


#include "index.h"
#include <assert.h>

namespace civita
{
  size_t Index::linealOffset(size_t h) const
  {
    auto lb=std::lower_bound(index.begin(), index.end(), h);
    if (lb!=index.end() && *lb==h)
      return size_t(lb-index.begin());
    return index.size();
  }

  void Index::insert(size_t h, std::vector<double>& v, double x)
  {
    auto lb=std::lower_bound(index.begin(), index.end(), h);
    if (lb==index.end() || *lb!=h)
      {
        v.insert(v.begin()+(lb-index.begin()),x);
        index.insert(lb, h);
      }
    else
      v[lb-index.begin()]=x;
    assert(sorted());
  }

}
