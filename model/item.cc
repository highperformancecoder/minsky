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

#include "item.h"
#include "group.h"
#include <ecolab_epilogue.h>

namespace minsky
{

  float Item::x() const 
  {
    if (auto g=group.lock())
      return m_x+g->x();
    else
      return m_x;
  }

  float Item::y() const 
  {
    if (auto g=group.lock())
      return m_y+g->y();
    else
      return m_y;
  }

  void Item::moveTo(float x, float y)
  {
    if (auto g=group.lock())
      {
        m_x=x-g->x();
        m_y=y-g->y();
      }
    else
      {
        m_x=x;
        m_y=y;
      }
    assert(x==this->x() && y==this->y());
  }

}
