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
#include "item.h"
#include "lasso.h"
#include "RESTProcess_base.h"
#include "signature.h"
#include "item.rcd"

#include "group.h"
#include "group.rcd"
#include "lasso.h"
#include "lasso.rcd"
#include "noteBase.rcd"
#include "operation.h"
#include "operation.rcd"
#include "operationType.rcd"
#include "plot.h"
#include "plot.rcd"
#include "plot.xcd"
#include "port.rcd"
#include "wire.h"
#include "wire.rcd"
#include "units.rcd"
#include "nobble.h"

#include "itemTemplateInstantiations.h"

using namespace std;

namespace classdesc
{
  template <>
  struct RESTProcessPtr<minsky::ItemPtr>: public RESTProcessBase
  {
    minsky::ItemPtr& ptr;
    /// returns the RESTProcess item corresponding to this, if it exists, null if it doesn't
    /// ownership not returned
    RESTProcessBase* RPItem() const
    {
      RESTProcess_t rp;
      ptr->RESTProcess(rp,"");
      auto i=rp.find("");
      if (i!=rp.end())
        return i->second.get();
      return nullptr;
    }
    
    RESTProcessPtr(minsky::ItemPtr& ptr): ptr(ptr) {}
    json_pack_t process(const string& remainder, const json_pack_t& arguments) override
    {
      if (auto rp=RPItem())
        return rp->process(remainder, arguments);
      return {};
    }
    json_pack_t signature() const override
    {
      json_pack_t r;
      if (ptr)
        {
          vector<classdesc::Signature> signature{{ptr->classType(),{}}, {ptr->classType(),{ptr->classType()}}};
          r<<signature;
        }
      return r; 
    }
    json_pack_t list() const override {
      if (auto rp=RPItem())
        return rp->list();
      return json_pack_t(json5_parser::mArray());
    }
    json_pack_t type() const override {
      if (auto rp=RPItem())
        return rp->type();
      return json_pack_t("void");}

  };
}

namespace minsky
{
  using classdesc::RESTProcess_t;
  using classdesc::json_pack_t;
  using std::string;

  template <class T, class B>
  void ItemT<T,B>::RESTProcess(RESTProcess_t& rp, const string& d)
  {
    ::RESTProcess(rp,d,dynamic_cast<T&>(*this));
  }

  template <class T, class B>
  void ItemT<T,B>::RESTProcess(RESTProcess_t& rp, const string& d) const
  {
    ::RESTProcess(rp,d,dynamic_cast<const T&>(*this));
  }
  
  template <class T, class B>
  void ItemT<T,B>::json_pack(json_pack_t& j) const
  {
    ::json_pack(j,"",dynamic_cast<const T&>(*this));
  }

}

#endif
