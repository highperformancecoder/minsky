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

#include "minskyRS.h"
#include "minsky_epilogue.h"

using namespace classdesc;
using namespace std;

#include <readline/readline.h>
#include <readline/history.h>


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
  string buffer;
  
  //  while ((c=readline("cmd>"))!=nullptr)
  while (getline(cin,buffer))
    {
      //      string buffer=c;
      if (buffer[0]!='/')
        cerr << buffer << "command doesn't starts with /"<<endl;
      else if (buffer=="/list")
        for (auto& i: registry)
          cout << i.first << endl;
      else
        {
          try
            {
              auto n=buffer.find(' ');
              json_pack_t jin(json_spirit::mValue::null);
              string cmd;
              if (n==string::npos)
                cmd=buffer;
              else
                { // read argument(s)
                  cmd=buffer.substr(0,n);
                  read(buffer.substr(n),jin);
                }
              write(registry.process(cmd, jin),cout);
              cout << endl;
            }
          catch (const std::exception& ex)
            {
              cerr << "Exception: "<<ex.what() << endl;
            }
        }
//      if (strlen(c)) add_history(c); 
//      free(c);
    }
}
