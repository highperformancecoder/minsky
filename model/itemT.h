/*
  @copyright Steve Keen 2015
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

#ifndef ITEMT_H
#define ITEMT_H

#include "item.h"

namespace minsky 
{
  /** curiously recursive template pattern for generating overrides */
  template <class T, class Base=Item>
  struct ItemT: public Base
  {
    std::string classType() const override {
      auto s=classdesc::typeName<T>();
      // remove minsky namespace
      static const char* ns="::minsky::";
      static const int eop=strlen(ns);
      if (s.substr(0,eop)==ns)
        s=s.substr(eop);
      return s;
    }
    ItemT* clone() const override {
      auto r=new T(*dynamic_cast<const T*>(this));
      r->group.reset();
      return r;
    }
    void RESTProcess(classdesc::RESTProcess_t& rp,const std::string& d) override
    {::RESTProcess(rp,d,dynamic_cast<T&>(*this));}
    void RESTProcess(classdesc::RESTProcess_t& rp,const std::string& d) const override
    {::RESTProcess(rp,d,dynamic_cast<const T&>(*this));}
    void json_pack(classdesc::json_pack_t& j) const override
    {::json_pack(j,"",dynamic_cast<const T&>(*this));}
    ItemT()=default;
    ItemT(const ItemT&)=default;
    ItemT& operator=(const ItemT&)=default;
    // delete move operations to avoid the dreaded virtual-move-assign warning
    ItemT(ItemT&&)=delete;
    ItemT& operator=(ItemT&&)=delete;
  };
}

#include "itemT.cd"
#include "itemT.xcd"
#endif

