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
#include "selection.h"
#include "minsky_epilogue.h"

namespace minsky
{
  /// dummy template definition for use with gui executable
  template <class T, class B>
  void ItemT<T,B>::RESTProcess(classdesc::RESTProcess_t& rp, const std::string& d)
  {}
  template <class T, class B>
  void ItemT<T,B>::RESTProcess(classdesc::RESTProcess_t& rp, const std::string& d) const
  {}
  template <class T, class B>
  void ItemT<T,B>::json_pack(classdesc::json_pack_t& j) const
  {}

  /// dummy definition for use with gui executable
  void Item::RESTProcess(classdesc::RESTProcess_t& rp, const std::string& d)
  {}
  void Item::RESTProcess(classdesc::RESTProcess_t& rp, const std::string& d) const
  {}
  void Item::json_pack(classdesc::json_pack_t& rp) const
  {}
}

#include "itemTemplateInstantiations1.h"  
#include "itemTemplateInstantiations2.h"  
#include "itemTemplateInstantiations3.h"  
