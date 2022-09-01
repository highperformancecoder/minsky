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
#ifndef ENG_NOTATION
#define ENG_NOTATION

#include <string>
namespace minsky
{
  struct EngNotation {int sciExp, engExp;};
  /// return formatted mantissa and exponent in engineering format
  EngNotation engExp(double value);
  std::string mantissa(double value, const EngNotation&,int digits=3);
  std::string expMultiplier(int exp);
}

#include "engNotation.xcd"
#endif
