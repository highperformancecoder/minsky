/*
  @copyright Steve Keen 2018
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

/**
   A simple spreadsheet view
 */

#ifndef SHEET_H
#define SHEET_H
#include "item.h"
#include "showSlice.h"

namespace minsky
{
  class Sheet: public ItemT<Sheet>
  {
    
    CLASSDESC_ACCESS(Sheet);
  public:
    Sheet();
    bool inItem(float, float) const override;
    ClickType::Type clickType(float x, float y) override;   
    void draw(cairo_t* cairo) const override;

    ShowSlice showSlice=ShowSlice::head; ///< whether to elide rows from beginning, end or middle
  };
}

#include "sheet.cd"
#include "sheet.xcd"
#endif
