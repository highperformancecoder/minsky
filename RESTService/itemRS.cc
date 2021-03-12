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

#include "item.h"
#include "item.rcd"
#include "assetClass.h"
#include "assetClass.rcd"
#include "bookmark.h"
#include "bookmark.rcd"
#include "capiRenderer.h"
#include "capiRenderer.cd"
#include "capiRenderer.xcd"
#include "callableFunction.h"
#include "callableFunction.rcd"
#include "callableFunction.xcd"
#include "constMap.rcd"
#include "dynamicRavelCAPI.h"
#include "dynamicRavelCAPI.rcd"
#include "dynamicRavelCAPI.xcd"
#include "dataSpecSchema.h"
#include "dataSpecSchema.rcd"
#include "godleyIcon.h"
#include "godleyIcon.rcd"
#include "godleyTable.rcd"
#include "godleyTableWindow.h"
#include "godleyTableWindow.rcd"
#include "godleyTableWindow.xcd"
#include "group.h"
#include "group.rcd"
#include "hypercube.rcd"
#include "hypercube.xcd"
#include "lock.h"
#include "lock.rcd"
#include "lock.xcd"
#include "index.rcd"
#include "index.xcd"
#include "noteBase.rcd"
#include "operation.h"
#include "operation.rcd"
#include "operationType.rcd"
#include "port.rcd"
#include "CSVDialog.rcd"
#include "CSVParser.rcd"
#include "dimension.rcd"
#include "parameterTab.h"
#include "parameterTab.rcd"
#include "plotWidget.h"
#include "plotWidget.rcd"
#include "plot.xcd"
#include "plot.rcd"
#include "ravelWrap.h"
#include "ravelWrap.rcd"
#include "ravelWrap.xcd"
#include "ravelState.rcd"
#include "ravelState.xcd"
#include "signature.h"
#include "signature.xcd"
#include "signature.rcd"
#include "slider.rcd"
#include "selection.h"
#include "selection.rcd"
#include "sheet.h"
#include "sheet.rcd"
#include "switchIcon.h"
#include "switchIcon.rcd"
#include "SVGItem.rcd"
#include "tensorVal.rcd"
#include "tensorVal.xcd"
#include "tensorInterface.rcd"
#include "tensorInterface.xcd"
#include "userFunction.h"
#include "userFunction.rcd"
#include "variable.h"
#include "variable.rcd"
#include "variableType.rcd"
#include "variableValue.rcd"
#include "wire.rcd"
#include "xvector.rcd"

#include "nobble.h"
#include "minsky_epilogue.h"

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
      ptr->restProcess(rp,"");
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
          vector<minsky::Signature> signature{{ptr->classType(),{}}, {ptr->classType(),{ptr->classType()}}};
          r<<signature;
        }
      return r; 
    }
    json_pack_t list() const override {
      if (auto rp=RPItem())
        return rp->list();
      return json_pack_t(json_spirit::mArray());
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

  void Item::restProcess(RESTProcess_t& rp, const string& d)
  {
    ::RESTProcess(rp,d,*this);
  }

  void Item::restProcess(RESTProcess_t& rp, const string& d) const
  {
    ::RESTProcess(rp,d,*this);
  }

  void Item::json_pack(json_pack_t& j) const
  {
    ::json_pack(j,"",*this);
  }

  template <class T, class B>
  void ItemT<T,B>::restProcess(RESTProcess_t& rp, const string& d)
  {
    ::RESTProcess(rp,d,dynamic_cast<T&>(*this));
  }

  template <class T, class B>
  void ItemT<T,B>::restProcess(RESTProcess_t& rp, const string& d) const
  {
    ::RESTProcess(rp,d,dynamic_cast<const T&>(*this));
  }
  
  template <class T, class B>
  void ItemT<T,B>::json_pack(json_pack_t& j) const
  {
    ::json_pack(j,"",dynamic_cast<const T&>(*this));
  }

}

#include "itemTemplateInstantiations.h"  
