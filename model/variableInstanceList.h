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

#ifndef VARIABLEINSTANCELIST_H
#define VARIABLEINSTANCELIST_H
#include "bookmark.h"
#include "item.h"

#include <string>
#include <vector>

/// @file create a list of bookmarks for all instance of a variable on
/// current canvas

namespace minsky
{
  class Group;
  
  class VariableInstanceList
  {
    Group& model;
    std::vector<Bookmark> bookmarks;
    std::vector<ItemPtr> items;
    CLASSDESC_ACCESS(VariableInstanceList);
  public:
    VariableInstanceList();
    /// @param model top level group to obtain instances from
    /// @param valueId id of variables to obtain the instances of
    VariableInstanceList(Group& model, const std::string& valueId);
    /// zoom model to instance number in the list
    void gotoInstance(std::size_t);
    /// return the list of names
    std::vector<std::string> names() const {
      std::vector<std::string> r;
      for (auto& b: bookmarks) r.push_back(b.name);
      return r;
    }
  };
}

#include "variableInstanceList.cd"
#include "variableInstanceList.rcd"
#include "variableInstanceList.xcd"
#endif
