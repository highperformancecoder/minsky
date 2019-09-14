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

#include <readline/readline.h>
#include <readline/history.h>
//extern "C" char *readline(const char *);
//extern "C" void add_history(const char *);

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

  char* c;
  
  while ((c=readline("cmd>"))!=nullptr)
    {
      string cmd=c;
      if (cmd[0]!='/')
        cerr << cmd << "command doesn't starts with /"<<endl;
      else if (cmd=="/list")
        for (auto& i: registry)
          cout << i.first << endl;
      else
        {
          try
            {
              json_pack_t jin(json_spirit::mValue::null);
              if (cin.peek()!='\n')
                read(cin,jin);
              else
                {
                  string t;
                  getline(cin,t); // absorb '\n'
                }
              write(registry.process(cmd, jin),cout);
              cout << endl;
            }
          catch (const std::exception& ex)
            {
              cerr << "Exception: "<<ex.what() << endl;
            }
        }
      if (strlen(c)) add_history(c); 
      free(c);
    }
}
