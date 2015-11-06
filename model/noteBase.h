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

#ifndef NOTEBASE_H
#define NOTEBASE_H

#include <string>

namespace minsky 
{
  struct NoteBase
  {
    bool mouseFocus; ///<true if target of a mouseover
    bool selected; ///<true if selected for cut, copy or group operation
    std::string detailedText, tooltip;
  };
}

#include "noteBase.cd"
#endif
