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

/// @file An nodejs-embedded REST Service
#include <napi.h>
#include "minskyRS.h"
#include "minsky_epilogue.h"

#include <exception>

using namespace Napi;
using namespace std;
using namespace classdesc;

namespace minsky
{
  namespace
  {
    Minsky* l_minsky=NULL;
  }

  Minsky& minsky()
  {
    static Minsky s_minsky;
    if (l_minsky)
      return *l_minsky;
    else
      return s_minsky;
  }

  LocalMinsky::LocalMinsky(Minsky& minsky) {l_minsky=&minsky;}
  LocalMinsky::~LocalMinsky() {l_minsky=NULL;}

  // GUI callback needed only to solve linkage problems
  void doOneEvent(bool idleTasksOnly) {}
}


RESTProcess_t registry;


Value RESTCall(const Napi::CallbackInfo& info)
{
  Env env = info.Env();
  if (info.Length() < 1)
  {
    Napi::TypeError::New(env, "Needs to be call(endpoint[, arguments])").ThrowAsJavaScriptException();
    return env.Null();
  }

  try
    {
      json_pack_t arguments(json_spirit::mValue{});
      if (info.Length()>1)
        {
          string jsonArguments=info[1].ToString();
          if (!jsonArguments.empty())
            read(info[1].ToString(), arguments);
        }
      string cmd=info[0].ToString();
      auto response=String::New(env, write(registry.process(cmd, arguments)));
      int nargs=arguments.type()==json_spirit::array_type? arguments.get_array().size(): 1;
      cmd.erase(0,1); // remove leading '/'
      replace(cmd.begin(), cmd.end(), '/', '.');
      minsky::minsky().commandHook(cmd,nargs);
      for (auto i: minsky::minsky().nativeWindowsToRedraw)
        i->draw();
      minsky::minsky().nativeWindowsToRedraw.clear();
      return response;
    }
  catch (const std::exception& ex)
    {
      // throw C++ exception as Javascript exception
      Napi::Error::New(env, ex.what()).ThrowAsJavaScriptException();
      return env.Null();
    }
  catch (...)
    {
      Napi::Error::New(env, "unknown exception caught").ThrowAsJavaScriptException();
      return env.Null();
    }
}

Object Init(Env env, Object exports) {
  RESTProcess(registry,"/minsky",minsky::minsky());
  
  exports.Set(String::New(env, "call"), Function::New(env, RESTCall));
  return exports;
}

NODE_API_MODULE(minskyRESTService, Init)
