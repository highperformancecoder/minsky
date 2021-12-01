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
    struct AddOnMinsky: public Minsky
    {
      FunctionReference messageCallback;
      unique_ptr<Env> env;
      void message(const std::string& msg) override
      {if (env) messageCallback({String::New(*env,msg),Array::New(*env)});}
      bool checkMemAllocation(std::size_t bytes) const override {
        bool r=true;
        if (env && bytes>0.2*physicalMem())
          {
            auto buttons=Array::New(*env);
            buttons[0U]=String::New(*env,"No");
            buttons[1U]=String::New(*env,"Yes");
            r=messageCallback({
                String::New(*env,"Allocation will use more than 50% of available memory. Do you want to proceed?"),
                  buttons}).As<Number>().Int32Value();
          }
        return r;
      }
      
    };
    
    Minsky* l_minsky=NULL;
  }

  Minsky& minsky()
  {
    static AddOnMinsky s_minsky;
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

mutex redrawMutex;

// delay process redrawing to throttle redrawing
struct RedrawThread: public thread
{
  RedrawThread(): thread([this]{run();}) {}
  ~RedrawThread() {join();}
  atomic<bool> running; //< flag indicating thread is still running
  void run() {
    running=true;
    // sleep slightly to throttle requests on this service
    this_thread::sleep_for(chrono::milliseconds(10));
    lock_guard<mutex> lock(redrawMutex);
    for (auto i: minsky::minsky().nativeWindowsToRedraw)
      i->draw();
    minsky::minsky().nativeWindowsToRedraw.clear();
    running=false;
  }
};

unique_ptr<RedrawThread> redrawThread(new RedrawThread);

Value setMessageCallback(const Napi::CallbackInfo& info)
{
  Env env = info.Env();
  if (info.Length()<1 || !info[0].IsFunction())
    {
      Napi::Error::New(env, "Callback not provided").ThrowAsJavaScriptException();
    }
  if (auto m=dynamic_cast<minsky::AddOnMinsky*>(&minsky::minsky()))
    {
      m->env.reset(new Env(env));
      m->messageCallback=Persistent(info[0].As<Function>());
    }
  return env.Null();
}

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
            read(jsonArguments, arguments);
        }
      string cmd=info[0].ToString();
      Value response;
      {
        lock_guard<mutex> lock(redrawMutex);
        response=String::New(env, write(registry.process(cmd, arguments)));
        int nargs=arguments.type()==json_spirit::array_type? arguments.get_array().size(): 1;
        cmd.erase(0,1); // remove leading '/'
        replace(cmd.begin(), cmd.end(), '/', '.');
        minsky::minsky().commandHook(cmd,nargs);
      }
      if (!minsky::minsky().nativeWindowsToRedraw.empty())
        if (redrawThread->running)
          this_thread::yield(); // yield to the render thread
        else
          redrawThread.reset(new RedrawThread); // start a new render thread
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
  exports.Set(String::New(env, "setMessageCallback"), Function::New(env, setMessageCallback));
  return exports;
}

NODE_API_MODULE(minskyRESTService, Init)
