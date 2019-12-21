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

//#ifdef RESTPROCESS_H
//#include "RESTProcess_epilogue.h"
//#endif
// Civita tensor library descriptors needed in Minsky
#ifdef CIVITA_DIMENSION_H
#include "dimension.cd"
#include "dimension.xcd"
#endif
#ifdef CIVITA_TENSORINTERFACE_H
#include "tensorInterface.cd"
#endif
#ifdef CIVITA_TENSORVAL_H
#include "tensorVal.cd"
#endif
#ifdef CIVITA_HYPERCUBE_H
#include "hypercube.cd"
#endif
#ifdef CIVITA_XVECTOR_H
namespace classdesc_access
{
  template<> struct access_pack<civita::XVector> {
    void operator()(classdesc::pack_t& b, const std::string&, const civita::XVector& a)
    {
      b<<a.name<<a.dimension<<a.size();
      for (auto& i: a)
        b<<civita::str(i);
    }
  };
  template<> struct access_unpack<civita::XVector> {
    void operator()(classdesc::pack_t& b, const std::string&, civita::XVector& a)
    {
      size_t size;
      std::string x;
      a.clear();
      b>>a.name>>a.dimension>>size;
      for (size_t i=0; i<size; ++i)
        {
          b>>x;
          a.push_back(x);
        }
    }
  };

//  template <>
//  struct access_RESTProcess<civita::XVector>: public classdesc::NullDescriptor<cd::RESTProcess_t> {};
  template <>
  struct access_json_pack<civita::XVector>: public classdesc::NullDescriptor<cd::json_pack_t> {};
  template <>
  struct access_json_unpack<civita::XVector>: public classdesc::NullDescriptor<cd::json_unpack_t> {};
}

#endif

#include <ecolab_epilogue.h>
