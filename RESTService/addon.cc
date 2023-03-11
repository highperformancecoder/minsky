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
#include "RESTMinsky.h"
#include "minsky_epilogue.h"

#include <exception>
#include <atomic>

using namespace Napi;
using namespace std;
using namespace classdesc;
using namespace boost::posix_time;

namespace minsky
{
  namespace
  {

    // all NAPI calls involving an Env must be called on Javascript's
    // thread. In order to pass something back to Javascript's thread,
    // we need to use a ThreadSafeFunction
    struct PromiseResolver
    {
      Napi::Promise::Deferred promise;
      using Arg=pair<bool,string>;

      void doResolve(Arg* arg) {
        resolvePromise.Acquire();
        resolvePromise.BlockingCall(arg);
        resolvePromise.Release();
      }
      void resolve(const std::string& result) {
        Arg arg{true, result};
        doResolve(&arg);
      }
      void reject(const std::string& error) {
        Arg arg{false, error};
        doResolve(&arg);
      }


      // function called back from Javascript eventually
      static void jsCallback(Napi::Env, Napi::Function, PromiseResolver* promiseResolver, Arg* arg)
      {
        if (!promiseResolver) return;
        auto result=String::New(promiseResolver->promise.Env(), utf_to_utf<char16_t>(arg->second));
        if (arg->first)
          promiseResolver->promise.Resolve(result);
        else
          promiseResolver->promise.Reject(result);
//        delete promiseResolver; // cleans up object allocated in Command::Command() below
      }
      Napi::TypedThreadSafeFunction<PromiseResolver,Arg,jsCallback> resolvePromise;
      
      PromiseResolver(const Napi::Env& env):
        promise(env),
        resolvePromise(Napi::TypedThreadSafeFunction<PromiseResolver,Arg,jsCallback>::New
                       (promise.Env(),/*Napi::Function::New(promise.Env(),jsCallback),*/ "TSFN", 0, 2, this))
      {}
    };
    
    struct Command
    {
      PromiseResolver* promiseResolver;
      string command;
      json_pack_t arguments;
      Command(const Napi::Env& env, const string& command, const json_pack_t& arguments):
        promiseResolver(new PromiseResolver(env)), // ownership passed to JS interpreter
        //promiseResolver(nullptr),
        command(command),
        arguments(arguments) {}
    };
  
    struct AddOnMinsky: public RESTMinsky
    {
      Env* env=nullptr;
      FunctionReference messageCallback;
      FunctionReference busyCursorCallback;
      deque<unique_ptr<Command>> minskyCommands;
      mutex cmdMutex;
      atomic<bool> running{true};
      std::thread thread;
      
      AddOnMinsky(): thread([this](){run();}) {flags=0;}
      
      ~AddOnMinsky() {
        // because this object is used as a static object, suppress
        // the callback destructors to avoid freeing the references at
        // shutdown time.
        messageCallback.SuppressDestruct();
        busyCursorCallback.SuppressDestruct();
        running=false;
        if (thread.joinable()) thread.join();
      }

      void run()
      {
#if defined(_PTHREAD_H) && defined(__USE_GNU) && !defined(NDEBUG)
        pthread_setname_np(pthread_self(),"minsky thread");
#endif

        while (running)
          {
            unique_ptr<Command> command;
            {
              lock_guard<mutex> lock(cmdMutex);
              if (!minskyCommands.empty())
                {
                  command=move(minskyCommands.front());
                  minskyCommands.pop_front();
                }
            }
            
            if (!command) // perform housekeeping
              {
                if (reset_flag())
                  try
                    {
                      reset();
                    }
                  catch (...)
                    {}
                for (auto i: nativeWindowsToRedraw)
                  try
                    {
                      i->draw();
                    }
                  catch (const std::exception& ex)
                    {
                      /* absorb and log any exceptions, cannot do anything anyway */
                      cerr << ex.what() << endl;
                      break;
                    }
                  catch (...) {break;}
                nativeWindowsToRedraw.clear();
                this_thread::sleep_for(chrono::milliseconds(10));
                continue;
              }

            try
              {
                Env e=command->promiseResolver->promise.Env();
                env=&e; // TODO: can we do the callback env a little less clumsily?
                // disable quoting wide characters in UTF-8 strings
                auto result=write(registry.process(command->command, command->arguments),json5_parser::raw_utf8);
                // Javascript needs the result returns as UTF-16.
                command->promiseResolver->resolve(result);
                int nargs=1;
                switch (command->arguments.type())
                  {
                  case json5_parser::array_type:
                    nargs=command->arguments.get_array().size();
                    break;
                  case json5_parser::null_type:
                    nargs=0;
                    break;
                  default:
                    break;
                  }
                command->command.erase(0,1); // remove leading '/'
                replace(command->command.begin(), command->command.end(), '/', '.');
                commandHook(command->command,nargs);
              }
            catch (const std::exception& ex)
              {
                command->promiseResolver->reject(ex.what());
              }
            catch (...)
              {
                command->promiseResolver->reject("Unknown exception");
              }
          }
      }
      
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
      void setBusyCursor() override
      {if (env) busyCursorCallback({Boolean::New(*env,true)});}
      void clearBusyCursor() override
      {if (env) busyCursorCallback({Boolean::New(*env,false)});}
    };
    
    Minsky* l_minsky=NULL;
  }

  Minsky& minsky()
  {
    static AddOnMinsky s_minsky;
    if (l_minsky)
      return *l_minsky;
    return s_minsky;
  }

  LocalMinsky::LocalMinsky(Minsky& minsky) {l_minsky=&minsky;}
  LocalMinsky::~LocalMinsky() {l_minsky=NULL;}

  // GUI callback needed only to solve linkage problems
  void doOneEvent(bool idleTasksOnly) {}
}


minsky::AddOnMinsky& addOnMinsky=static_cast<minsky::AddOnMinsky&>(minsky::minsky());

//mutex redrawMutex, resetMutex;
//
//// delay process redrawing to throttle redrawing
//struct RedrawThread: public thread
//{
//  RedrawThread(): thread([this]{run();}) {}
//  ~RedrawThread() {if (joinable()) join();}
//  atomic<bool> running; //< flag indicating thread is still running
//  void run() {
//#if defined(_PTHREAD_H) && defined(__USE_GNU) && !defined(NDEBUG)
//    pthread_setname_np(pthread_self(),"redraw thread");
//#endif
//    running=true;
//    // sleep slightly to throttle requests on this service
//    this_thread::sleep_for(chrono::milliseconds(10));
//
//    lock_guard<mutex> lock(redrawMutex);
//
//    for (auto i: minsky::minsky().nativeWindowsToRedraw)
//      try
//        {
//          i->draw();
//        }
//      catch (const std::exception& ex)
//        {
//          /* absorb and log any exceptions, cannot do anything anyway */
//          cerr << ex.what() << endl;
//          break;
//        }
//      catch (...) {break;}
//    minsky::minsky().nativeWindowsToRedraw.clear();
//    running=false;
//  }
//};
//
//unique_ptr<RedrawThread> redrawThread(new RedrawThread);
//
//struct ResetThread: public thread
//{
//  ResetThread(): thread([this]{run();}) {}
//  ~ResetThread() {if (joinable()) join();}
//  atomic<bool> running; //< flag indicating thread is still running
//  void run() {
//#if defined(_PTHREAD_H) && defined(__USE_GNU) && !defined(NDEBUG)
//    pthread_setname_np(pthread_self(),"reset thread");
//#endif
//    running=true;
//
//    while (std::chrono::system_clock::now()<minsky::minsky().resetAt)
//      this_thread::sleep_for(chrono::milliseconds(100));
//    lock_guard<mutex> lock(resetMutex);
//    try
//      {
//        minsky::minsky().reset();
//      }
//    catch (...)
//      {} // absorb all exceptions to prevent terminate being called.
//    if (!redrawThread->running)
//      redrawThread.reset(new RedrawThread);
//    running=false;
//  }
//};
//
//unique_ptr<ResetThread> resetThread;
 
Value setMessageCallback(const Napi::CallbackInfo& info)
{
  Env env = info.Env();
  if (info.Length()<1 || !info[0].IsFunction())
    {
      Napi::Error::New(env, "Callback not provided").ThrowAsJavaScriptException();
    }
  addOnMinsky.messageCallback=Persistent(info[0].As<Function>());
  return env.Null();
}

Value setBusyCursorCallback(const Napi::CallbackInfo& info)
{
  Env env = info.Env();
  if (info.Length()<1 || !info[0].IsFunction())
    {
      Napi::Error::New(env, "Callback not provided").ThrowAsJavaScriptException();
    }
  addOnMinsky.busyCursorCallback=Persistent(info[0].As<Function>());
  return env.Null();
}

//struct SetMinskyEnv
//{
//  SetMinskyEnv(Env& env) {addOnMinsky.env=&env;}
//  ~SetMinskyEnv() {addOnMinsky.env=nullptr;}
//};
//
//
//thread minskyThread([](){
// });
//


Value RESTCall(const Napi::CallbackInfo& info)
{
  Env env = info.Env();
  if (info.Length() < 1)
  {
    Napi::TypeError::New(env, "Needs to be call(endpoint[, arguments])").ThrowAsJavaScriptException();
    return env.Null();
  }
  
#if defined(_PTHREAD_H) && defined(__USE_GNU) && !defined(NDEBUG)
    pthread_setname_np(pthread_self(),"addon thread");
#endif

  try
    {
      json_pack_t arguments(json5_parser::mValue{});
      if (info.Length()>1)
        {
          string jsonArguments=info[1].ToString();
          if (!jsonArguments.empty())
            read(jsonArguments, arguments);
        }
      lock_guard<mutex> lock(addOnMinsky.cmdMutex);
      addOnMinsky.minskyCommands.emplace_back(new minsky::Command{env,info[0].ToString(),arguments});
      return addOnMinsky.minskyCommands.back()->promiseResolver->promise.Promise();
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
  RESTProcess(addOnMinsky.registry,"/minsky",minsky::minsky());
  
  exports.Set(String::New(env, "call"), Function::New(env, RESTCall));
  exports.Set(String::New(env, "setMessageCallback"), Function::New(env, setMessageCallback));
  exports.Set(String::New(env, "setBusyCursorCallback"), Function::New(env, setBusyCursorCallback));
  return exports;
}

NODE_API_MODULE(minskyRESTService, Init)
