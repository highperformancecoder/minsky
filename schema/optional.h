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

#ifndef MINSKY_OPTIONAL_H
#define MINSKY_OPTIONAL_H
#include <xsd_generate_base.h>
#include <memory>

namespace minsky
{
// see discussion in Stroustrup 4th ed 28.4.4
  template <class T> struct has_empty
  {
    template <class X>
    static auto check(X x)->decltype(x.empty());
    static std::false_type check(...);
    
    static constexpr const bool value = std::is_integral<decltype(check(std::declval<T>()))>::value;
  };
   
  /// convenience class to omit writing XML records when data absent or empty
  template <class T>
  struct Optional: shared_ptr<T>
  {
    Optional() {}
    Optional(const T& x) {assign(x);}
    template <class U>
    Optional(const Optional<U>& x) {if (x) *this=*x;}
    template <class U>
    typename classdesc::enable_if<has_empty<U>,void>::T
    assign(const U& x, classdesc::dummy<0> d=0) {
      if (!x.empty()) this->reset(new T(x));
    }
    template <class U>
    typename classdesc::enable_if<classdesc::Not<has_empty<U>>,void>::T
    assign(const U& x, classdesc::dummy<1> d=0) {this->reset(new T(x));}

    // if we access an optional, then create its target
    T& operator*() {if (!this->get()) this->reset(new T); return *this->get();}
    const T& operator*() const {return *this->get();}
    T* operator->() {return &**this;}
    const T* operator->() const {return &**this;}

    template <class U> Optional& operator=(const U& x) {assign(x); return *this;}
  };

}

namespace classdesc
{

#ifdef _CLASSDESC
#pragma omit xsd_generate minsky::Optional
#pragma omit xml_pack minsky::Optional
#endif

  /*
    This code ensure optional fields are not exported when empty 
  */
  template <class T>
  void xsd_generate(xsd_generate_t& g, const string& d, const minsky::Optional<T>& a) 
  {
    xsd_generate_t::Optional o(g,true);
    T tmp; // a may be null
    xsd_generate(g,d,tmp);
  }

  template <class T> inline void xml_pack(xml_pack_t& t,const string& d,minsky::Optional<T>& a)
  {if (a) ::xml_pack(t,d,*a);}
}

using classdesc::xsd_generate;
using classdesc::xml_pack;

#include "optional.cd"
#include "optional.xcd"
#endif
