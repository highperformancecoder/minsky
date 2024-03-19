/*
  @copyright Steve Keen 2021
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
#include "lasso.h"
#include "port.h"
#include "wire.h"
#include "tensorInterface.h"
#include "tensorInterface.xcd"
#include "tensorInterface.rcd"
#include "tensorVal.h"
#include "tensorVal.xcd"
#include "tensorVal.rcd"
#include "minsky_epilogue.h"

#include <utility>

minsky::LassoBox::LassoBox(float x0, float y0, float x1, float y1): 
  x0(x0), y0(y0), x1(x1), y1(y1)
{
  if (x0>x1) std::swap(this->x0,this->x1);
  if (y0>y1) std::swap(this->y0,this->y1);
}


bool minsky::LassoBox::contains(const Wire& wire) const
{
  // Make sure both ends of wires are selected in all cases. For ticket 1147
  return (intersects(wire.from()->item()) && intersects(wire.to()->item())); 
}

//CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::LassoBox);

