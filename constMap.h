/*
  @copyright Steve Keen 2013
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
#ifndef CONSTMAP_H
#define CONSTMAP_H

namespace minsky
{
  // a map with a const [] that returns a default object
  template <class K, class V>
  struct ConstMap: public std::map<K,V>
  {
    ConstMap() {}
    template <class T>
    explicit ConstMap(const T& x): std::map<K,V>(x) {}
    template <class T, class U>
    ConstMap(const T& x, const U& y): std::map<K,V>(x,y) {}

    using std::map<K,V>::operator[];
    V operator[](const K& k) const {
      typename std::map<K,V>::const_iterator i=this->find(k);
      if (i!=this->end())
        return i->second;
      else
        return V();
    }
  };
}

#include "constMap.cd"
#endif
