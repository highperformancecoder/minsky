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

#ifndef MDLREADER_H
#define MDLREADER_H
#include "group.h"
#include "rungeKutta.h"
#include <iostream>

namespace minsky
{
  /// import a Vensim mdl file into \a group, also populating \a simParms from the control block
  void readMdl(Group& group, RungeKutta& simParms, std::istream& mdlFile);
}
#endif
