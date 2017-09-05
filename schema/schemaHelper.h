/*
  @copyright Steve Keen 2012
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

#ifndef SCHEMA_HELPER
#define SCHEMA_HELPER

#include "../model/operation.h"
#include "../model/variable.h"
#include "../engine/variableValue.h"

template <class T>
ecolab::array<T> toArray(const std::vector<T>& v) 
{
  ecolab::array<T> a(v.size());
  for (size_t i=0; i<v.size(); ++i) a[i]=v[i];
  return a;
}

template <class T>
std::vector<T> toVector(const ecolab::array<T>& a) 
{
  std::vector<T> v(a.size());
  for (size_t i=0; i<v.size(); ++i) v[i]=a[i];
  return v;
}

template <class T>
std::vector<double> toDoubleVector(const ecolab::array<T>& a) 
{
  std::vector<double> v(a.size());
  for (size_t i=0; i<v.size(); ++i) v[i]=a[i];
  return v;
}

namespace minsky
{
  /**
     A bridge pattern to allow schemas to update private members of
     various classes, whilst retaining desired
     encapsulation. SchemaHelper is priveleged to allow access to
     private parts of the class to be initialised, but should only be
     used by schema classes.
  */
  struct SchemaHelper
  {

    static void setPrivates
    (minsky::GodleyTable& g, const vector<vector<string> >& data, 
     const vector<GodleyTable::AssetClass>& assetClass)
    {
      g.data=data;
      g.m_assetClass=assetClass;
    }

    static void scaleGodley(minsky::GodleyIcon& g, double globalZoomFactor)
    {g.iconScale*=g.zoomFactor/globalZoomFactor;
    }
  };

}

#endif
