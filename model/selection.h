/*
  @copyright Steve Keen 2013
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

#ifndef SELECTION_H
#define SELECTION_H

#include "group.h"

#include <cairo_base.h>

#include <vector>

namespace minsky
{
  /// represents items that have been selected
  struct Selection: public Group
  {
    void clear();

    /// add item to selection if not present, or removes it if present
    /// groups handled as well
    void toggleItemMembership(const ItemPtr& item);

    /// check if item already present, and if not, inserts item
    /// delegates to ensureGroupInserted if passed a group
    void ensureItemInserted(const ItemPtr& item);
    
    /// check if \a group already present, and if not, inserts it
    void ensureGroupInserted(const GroupPtr& item);

    void insertItem(const ItemPtr& item);

    void insertGroup(const GroupPtr& g) {
      groups.push_back(g);
      g->selected=true;
    }
    /// return if item is contained in selection
    bool contains(const ItemPtr& item) const;
    using Item::contains;
  };

}

#include "selection.cd"
#include "selection.rcd"
#include "selection.xcd"
#endif
