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

namespace
{

    // all NAPI calls involving an Env must be called on Javascript's
    // thread. In order to pass something back to Javascript's thread,
    // we need to use a ThreadSafeFunction
    struct PromiseResolver
    {
      Promise::Deferred promise;
      bool success;
      string result;

      void doResolve(); 
      void resolve(const std::string& result) {
        success=true;
        this->result=result;
        doResolve();
      }
      void reject(const std::string& error) {
        success=false;
        result=error;
        doResolve();
      }
      
      PromiseResolver(const Napi::Env& env):
        promise(env)
      {}
    };

    // function called back from Javascript eventually
  static void jsCallback(Napi::Env env, Napi::Function, void*, PromiseResolver* promiseResolver);

  TypedThreadSafeFunction<void,PromiseResolver,jsCallback> tsPromiseResolver;
  
void jsCallback(Napi::Env env, Napi::Function, void*, PromiseResolver* promiseResolver)
{
      if (!promiseResolver) return;
      tsPromiseResolver.Acquire();
      cout << "jsCallback: ("<<pthread_self()<<") "<<promiseResolver->success<<" "<<promiseResolver->result.substr(0,50)<<endl;
      //EscapableHandleScope scope(promiseResolver->promise.Env());
      auto result=String::New(env, utf_to_utf<char16_t>(promiseResolver->result));
      cout << "result created"<<endl;
      if (promiseResolver->success)
        promiseResolver->promise.Resolve(result);
      else
        promiseResolver->promise.Reject(result);
      cout << "resolve done"<<std::endl;
      //delete promiseResolver; // cleans up object allocated in Command::Command() below
      tsPromiseResolver.Release();
    }

  void PromiseResolver::doResolve() {
    //tsPromiseResolver.Acquire();
    cout<<"Blocking call"<<endl;
    tsPromiseResolver.BlockingCall(this);
    cout<<"Blocking call done"<<endl;
    //tsPromiseResolver.Release();
  }


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
  
}

namespace minsky
{
  namespace
  {
    mutex minskyCmdMutex; // ensure access to only one global Minsky object at a time

    struct AddOnMinsky: public RESTMinsky
    {
      Env* env=nullptr;
      FunctionReference messageCallback;
      FunctionReference busyCursorCallback;
      deque<unique_ptr<Command>> minskyCommands;
      mutex cmdMutex;
      atomic<bool> running{true};
      std::thread thread;
      
      AddOnMinsky(): thread([this](){run();}) {
        flags=0;
        RESTProcess(registry,"/minsky",static_cast<Minsky&>(*this));
      }
      
      ~AddOnMinsky() {
        // because this object is used as a static object, suppress
        // the callback destructors to avoid freeing the references at
        // shutdown time.
        messageCallback.SuppressDestruct();
        busyCursorCallback.SuppressDestruct();
        running=false;
        if (thread.joinable()) thread.join();
      }

      Value queueCommand(Env env, const string& command, const json_pack_t& arguments)
      {
        lock_guard<mutex> lock(cmdMutex);
        minskyCommands.emplace_back(new Command{env,command,arguments});
        cout << "Invoking command "<<minskyCommands.back()->command<<" on thread "<<pthread_self()<<endl;
        return minskyCommands.back()->promiseResolver->promise.Promise();
      }
      
      void run()
      {
#if defined(_PTHREAD_H) && defined(__USE_GNU) && !defined(NDEBUG)
        pthread_setname_np(pthread_self(),"minsky thread");
#endif
        bool threadAcquired=false;

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
                      cout << "reset flag set"<<endl;
                      reset();
                    }
                  catch (...)
                    {}
                flags&=~reset_needed;
                for (auto i: nativeWindowsToRedraw)
                  try
                    {
                      //i->draw();
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
                //if (!threadAcquired) tsPromiseResolver.Acquire();
                threadAcquired=true;
                Env e=command->promiseResolver->promise.Env();
                env=&e; // TODO: can we do the callback env a little less clumsily?
                // disable quoting wide characters in UTF-8 strings
                cout << "Doing command: "<<command->command<<"("<<write(command->arguments)<<")"<<endl;
                lock_guard<mutex> lock(minskyCmdMutex);
                LocalMinsky lm(*this);
                auto result=write(registry.process(command->command, command->arguments),json5_parser::raw_utf8);
                // Javascript needs the result returns as UTF-16.
                cout << "Resolving command: "<<result.substr(0,50)<<endl;
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
                cout << "Rejecting command: "<<ex.what()<<endl;
                command->promiseResolver->reject(ex.what());
              }
            catch (...)
              {
                command->promiseResolver->reject("Unknown exception");
              }
          } 
        cout << "releasing..."<<endl;
       /*if (threadAcquired)*/ tsPromiseResolver.Release();
        cout << "released"<<endl;
      }
      
//      void message(const std::string& msg) override
//      {if (env) messageCallback({String::New(*env,msg),Array::New(*env)});}
//      bool checkMemAllocation(std::size_t bytes) const override {
//        bool r=true;
//        if (env && bytes>0.2*physicalMem())
//          {
//            auto buttons=Array::New(*env);
//            buttons[0U]=String::New(*env,"No");
//            buttons[1U]=String::New(*env,"Yes");
//            r=messageCallback({
//                String::New(*env,"Allocation will use more than 50% of available memory. Do you want to proceed?"),
//                  buttons}).As<Number>().Int32Value();
//          }
//        return r;
//      }
//      void setBusyCursor() override
//      {if (env) busyCursorCallback({Boolean::New(*env,true)});}
//      void clearBusyCursor() override
//      {if (env) busyCursorCallback({Boolean::New(*env,false)});}
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

struct MinskyAddon: public Addon<MinskyAddon>
{
  minsky::AddOnMinsky addOnMinsky;
  minsky::LocalMinsky lm{addOnMinsky};
  
  MinskyAddon(Env env, Object exports)
  {
    cout << "Initialising addon on thread "<<pthread_self()<<endl;
    tsPromiseResolver=TypedThreadSafeFunction<void,PromiseResolver,jsCallback>::New
      (env,"TSResolver", 0, 2, nullptr);
    //tsPromiseResolver.Release();
  
    
    DefineAddon(exports, {
        InstanceMethod("call", &MinskyAddon::call),
        InstanceMethod("setMessageCallback", &MinskyAddon::setMessageCallback),
        InstanceMethod("setBusyCursorCallback", &MinskyAddon::setBusyCursorCallback)
      });
  }
  

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


  Value call(const Napi::CallbackInfo& info)
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
        return addOnMinsky.queueCommand(env,info[0].ToString(),arguments);
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
};

NODE_API_ADDON(MinskyAddon);
