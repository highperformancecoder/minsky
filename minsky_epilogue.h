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

#ifndef MINSKY_EPILOGUE_H
#define MINSKY_EPILOGUE_H


#include "cairoSurfaceImage.h"
#if defined(CLASSDESC_RESTPROCESS_BASE_H)

namespace classdesc
{
  // needed for MXE
  template <>
  struct tn<typename std::vector<unsigned long long>::const_iterator>
  {
    static string name() {return "std::vector<unsigned long long>::const_iterator";}
  };
  
#ifdef BOOST_GEOMETRY_GEOMETRIES_POINT_XY_HPP
  template <>
  struct tn<boost::geometry::model::d2::point_xy<float>>
  {
    static string name() {return "boost::geometry::model::d2::point_xy<float>";}
  };
#endif
 
  
}

namespace classdesc_access
{
  namespace cd=classdesc;
#ifdef ACCESSOR_H
  template <class T, class G, class S>
  struct access_RESTProcess<ecolab::Accessor<T,G,S>>
  {
    template <class U>
    void operator()(cd::RESTProcess_t& r, const cd::string& d, U& a)
    {
      ::RESTProcess(r,d,a.g);
      ::RESTProcess(r,d,a.s);
    }
  };

  template <class T, class V, int N>
  struct access_RESTProcess<ecolab::TCLAccessor<T,V,N>>: public cd::NullDescriptor<cd::RESTProcess_t> {};
#endif
  
#ifdef ARRAYS_H
  template <class T>
  struct access_RESTProcess<ecolab::array<T>>
  {
    template <class U>
    void operator()(cd::RESTProcess_t& r, const cd::string& d, U& a)
    {
      r.add(d,new cd::RESTProcessSequence<ecolab::array<T>>(a));
    }
  };
#endif
  
//#ifdef CAIRO_H
//  template <> struct access_json_pack<ecolab::cairo::Surface>:
//    public cd::NullDescriptor<cd::json_pack_t> {};
//  template <> struct access_json_unpack<ecolab::cairo::Surface>:
//    public cd::NullDescriptor<cd::json_unpack_t> {};
//  template <> struct access_RESTProcess<ecolab::cairo::Surface>:
//    public cd::NullDescriptor<cd::RESTProcess_t> {};
//#endif
  
#ifdef BOOST_GEOMETRY_GEOMETRIES_POINT_XY_HPP
  template <>
  struct access_json_pack<boost::geometry::model::d2::point_xy<float>>: public classdesc::NullDescriptor<cd::json_pack_t> {};
  template <>
  struct access_json_unpack<boost::geometry::model::d2::point_xy<float>>: public classdesc::NullDescriptor<cd::json_unpack_t> {};
  template <>
  struct access_RESTProcess<boost::geometry::model::d2::point_xy<float>>: public classdesc::NullDescriptor<cd::RESTProcess_t> {};
#endif
}

#endif

#ifdef CIVITA_DIMENSION_H
#include "dimension.cd"
#include "dimension.xcd"
#endif
#ifdef CIVITA_INDEX_H
#include "index.cd"
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

namespace classdesc
{
  class json_pack_t;
  struct RESTProcess_t;
}

#include "dimension.h"

namespace classdesc_access
{
#ifdef JSON_PACK_NO_FALL_THROUGH_TO_STREAMING
  template <class T,class E> struct access_json_pack
  {
    template <class U>
    void operator()(classdesc::json_pack_t& targ, const classdesc::string& desc,U& arg);
  };
  
  template <class T, class E> struct access_json_unpack
  {
    template <class U>
    void operator()(classdesc::json_pack_t& targ, const classdesc::string& desc,U& arg);
  };
#endif

  template <class T, class E> struct access_RESTProcess
  {
    template <class U>
    void operator()(classdesc::RESTProcess_t& targ, const classdesc::string& desc,U& arg);
  };
  
  template <>
  struct access_RESTProcess<civita::any,void>: public classdesc::NullDescriptor<classdesc::RESTProcess_t> {};
}

#ifdef CIVITA_XVECTOR_H
#include "xvector.xcd"
#endif

#include <ecolab_epilogue.h>

/*(json_pack_t&,const std::string&,type&); \*/
#define CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(type)                   \
  namespace classdesc_access                                            \
  {                                                                     \
    template void access_json_pack<type>::operator()(classdesc::json_pack_t&,const std::string&,type&); \
    template void access_json_unpack<type>::operator()(classdesc::json_pack_t&,const std::string&,type&); \
    template void access_RESTProcess<type>::operator()(classdesc::RESTProcess_t&,const std::string&,type&); \
    template void access_RESTProcess<type>::operator()(classdesc::RESTProcess_t&,const std::string&,const type&); \
  }

#endif

// these macros are defined in XLib, but interfere with gtest, for example.
#undef True
#undef Bool
#undef None

