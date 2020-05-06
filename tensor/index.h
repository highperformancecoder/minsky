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

#ifndef CIVITA_INDEX_H
#define CIVITA_INDEX_H
#include <vector>
#include <set>

#ifndef CLASSDESC_ACCESS
#define CLASSDESC_ACCESS(x)
#endif

namespace civita
{
  /// represents index concept for sparse tensors
  class Index
    {
      std::vector<size_t> index; // sorted index vector
      CLASSDESC_ACCESS(Index);
    public:
      Index() {}
      Index(const std::set<size_t>& indices): index(indices.begin(), indices.end()) {}

      // assign an associative container (sorted keys)
      size_t key(const size_t& x) const {return x;}
      template <class V> size_t key(const std::pair<const size_t,V>& x) const {return x.first;}
      
      template <class T>
      Index& operator=(const T& indices) {
        std::vector<size_t> t; t.reserve(indices.size());
        for (auto& i: indices) t.push_back(key(i));
        index.swap(t);
        return *this;
      }

      /// return hypercube index corresponding to lineal index i 
      size_t operator[](size_t i) const {return index.empty()? i: index[i];}
      // invariant, should always be true
      bool sorted() const
      {std::set<size_t> tmp(index.begin(), index.end()); return tmp.size()==index.size();}
      bool empty() const {return index.empty();}
      size_t size() const {return index.size();}
      void clear() {index.clear();}
      /// return the lineal index of hypercube index h, or size if not present 
      ptrdiff_t linealOffset(size_t h) const;
      /// insert value into v at hc index h, maintaining sorted order
      void insert(size_t h, std::vector<double>& v, double x);
      std::vector<size_t>::const_iterator begin() const {return index.begin();}
      std::vector<size_t>::const_iterator end() const {return index.begin();}
    };
    

}
#endif
