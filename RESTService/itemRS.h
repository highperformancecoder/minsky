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

#ifndef ITEMRS_H
#define ITEMRS_H
#include <RESTProcess_base.h>

//namespace classdesc
//{
//  template <>
//  void RESTProcess(RESTProcess_t& r, const string& d, minsky::Item& a)
//  {a.restProcess(r,d);}
//}

namespace classdesc_access
{
  namespace cd=classdesc;
  template <>
  struct access_RESTProcess<minsky::Item>
  {
    template <class U>
    void operator()(cd::RESTProcess_t& r, const cd::string& d, U& a)
    {a.restProcess(r,d);}
  };
  
  template <class T, class B>
  struct access_RESTProcess<minsky::ItemT<T,B>>:
    public cd::NullDescriptor<cd::RESTProcess_t> {};
}

#endif
