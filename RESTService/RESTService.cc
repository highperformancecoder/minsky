/*
  @copyright Steve Keen 2019
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

#include "minsky.h"
#include "minsky.xcd"
#include "canvas.xcd"
#include "constMap.xcd"
#include "evalGodley.xcd"
#include "panopticon.xcd"
#include "selection.xcd"
#include "minsky_epilogue.h"

using namespace classdesc;
using namespace std;

namespace classdesc
{
  template <>
  struct RESTProcessPtr<minsky::ItemPtr>: public RESTProcessBase
  {
    minsky::ItemPtr& ptr;
    RESTProcessPtr(minsky::ItemPtr& ptr): ptr(ptr) {}
    json_pack_t process(const string& remainder, const json_pack_t& arguments) override
    {
      if (ptr)
        return ptr->restProcess()->process(remainder, arguments);
      else
        return {};
    }
    json_pack_t signature() const override
    {
      vector<minsky::Signature> signature{{ptr->classType(),{}}, {ptr->classType(),{ptr->classType()}}};
      json_pack_t r;
      return r<<signature;
    }
  };
  
}

namespace minsky
{
  Minsky& minsky() {
    static Minsky m;
    return m;
  }
  // GUI callback needed only to solve linkage problems
  void doOneEvent(bool idleTasksOnly) {}
  // not used, but needed for the linker
  LocalMinsky::LocalMinsky(Minsky& m) {}
  LocalMinsky::~LocalMinsky() {}
}

int main()
{
  RESTProcess_t registry;
  RESTProcess(registry,"/minsky",minsky::minsky());
  
  for (;;)
    {
      string cmd;
      getline(cin,cmd);
      
      if (cmd[0]!='/')
        {
          cerr << cmd << "command doesn't starts with /"<<endl;
          continue;
        }
      if (cmd=="/list")
        {
          for (auto& i: registry)
            cout << i.first << endl;
          continue;
        }

      try
        {
          registry.process(cmd, cin, cout);
        }
      catch (const std::exception& ex)
        {
          cerr << "Exception: "<<ex.what() << endl;
        }
    }
}
