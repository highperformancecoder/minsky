/*
  @copyright Steve Keen 2020
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

#ifndef NOBBLE_H
#define NOBBLE_H

// deal with multiple template arguments
#define NOBBLE_TARG(...) __VA_ARGS__
#define NOBBLE_STR(...) #__VA_ARGS__


#define NOBBLE(type, tdecl)                                             \
  namespace classdesc                                                   \
{                                                                       \
  template <tdecl> struct tn<type>                                      \
  {                                                                     \
    static string name() {return NOBBLE_STR(type);}                     \
  };                                                                    \
}                                                                       \
  namespace classdesc_access                                            \
{                                                                       \
  template <tdecl> struct access_RESTProcess<type>:                     \
        public classdesc::NullDescriptor<classdesc::RESTProcess_t> {};  \
                                                                        \
  template <tdecl> struct access_json_pack<type>:                       \
        public classdesc::NullDescriptor<classdesc::json_pack_t> {};    \
                                                                        \
  template <tdecl> struct access_json_unpack<type>:                     \
        public classdesc::NullDescriptor<classdesc::json_unpack_t> {};  \
  }                                                                     

#include <chrono>

// nobble various system types we're not going to expose to the REST API
NOBBLE(NOBBLE_TARG(std::chrono::time_point<C,D>), NOBBLE_TARG(class C, class D))
NOBBLE(NOBBLE_TARG(std::chrono::duration<R,P>), NOBBLE_TARG(class R, class P))
NOBBLE(std::istream,)
NOBBLE(std::initializer_list<T>,class T)
#if defined(__GNUC__) && !defined(__clang__)
NOBBLE(NOBBLE_TARG(__gnu_cxx::__normal_iterator<const unsigned int*, std::vector<T>>),class T)
NOBBLE(NOBBLE_TARG(__gnu_cxx::__normal_iterator<const long unsigned int*, std::vector<T>>),class T)
NOBBLE(NOBBLE_TARG(__gnu_cxx::__normal_iterator<const long long unsigned int*, std::vector<T>>),class T)
#elif defined(MAC_OSX_TK)
NOBBLE(std::__1::__wrap_iter<const unsigned long *>,)
#elif defined(__clang__)
NOBBLE(NOBBLE_TARG(__gnu_cxx::__normal_iterator<const unsigned long*, std::vector<T>>),class T)
#endif
NOBBLE(NOBBLE_TARG(boost::geometry::model::d2::point_xy<T,S>),NOBBLE_TARG(class T,class S))
NOBBLE(civita::any,)

#endif
