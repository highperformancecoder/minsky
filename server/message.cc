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

#include "message.h"
#include "schema1.h"
#include <ecolab_epilogue.h>

namespace classdesc
{
  template <> Factory<minsky::MsgBase, string>::Factory() {}
}

namespace minsky
{

  MsgFactory::MsgFactory()
  {
    schema1::enumerateRegisterItems(*this);
    schema1::enumerateRegisterLayout(*this);
    registerType<Msg<schema1::Minsky> >
      (suppressSchema(typeName<schema1::Minsky>()));
    // ... and other message payloads as needed ...
  }

  string MsgFactory::suppressSchema(const string& typeName) const
  {
    size_t p=typeName.rfind("::");
    if (p!=string::npos) 
      p+=2;
    else
      p=0;
    return typeName.substr(p);
  }

  MsgFactory msgFactory;

}
