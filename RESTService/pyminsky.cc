/*
  @copyright Steve Keen 2023
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

// TODO figure out how to switch buffer types... For now, use JSON
//#define REST_PROCESS_BUFFER classdesc::PythonBuffer
#include "minsky.h"
#include "minsky_epilogue.h"
#include "pythonBuffer.h"

namespace pyminsky
{
  minsky::Minsky minsky;
  CLASSDESC_ADD_GLOBAL(minsky);

  minsky::Item& findObject(const std::string& typeName)
  {
    using namespace minsky;
    auto& canvas = minsky.canvas;

    canvas.item.reset();

    if (typeName == "Group" && !canvas.model->groups.empty())
      canvas.item = canvas.model->groups.front();
    else
      minsky.model->recursiveDo(&GroupItems::items, [&](const Items&, Items::const_iterator i)
      {
        if ((*i)->classType() == typeName)
          {
            canvas.item = *i;
            return true; // Stop recursion
          }
        return false;
      });

    return *canvas.item;
  }

  CLASSDESC_ADD_FUNCTION(findObject);
}

CLASSDESC_PYTHON_MODULE(pyminsky);

namespace minsky
{
  LocalMinsky::LocalMinsky(Minsky&) {}
  LocalMinsky::~LocalMinsky() {}
  // GUI callback needed only to solve linkage problems
  void doOneEvent(bool) {}
  Minsky& minsky() {return pyminsky::minsky;}
}
