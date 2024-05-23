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
#include "RESTMinsky.h"
#include "minsky_epilogue.h"

#include <exception>
#include <atomic>
#include <future>

#ifdef _WIN32
#include <time.h>
#else
#include <sys/times.h>
#endif

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
  void resolvePromise(Napi::Env env, Napi::Function, void*, PromiseResolver* promiseResolver)
  {
    if (!promiseResolver) return;
    // Javascript needs the result returned as UTF-16.
    auto result=String::New(env, utf_to_utf<char16_t>(promiseResolver->result));
    if (promiseResolver->success)
      promiseResolver->promise.Resolve(result);
    else
      promiseResolver->promise.Reject(result);
    delete promiseResolver; // cleans up object allocated in Command::Command() below
  }

  TypedThreadSafeFunction<void,PromiseResolver,resolvePromise> tsPromiseResolver;
  
  void PromiseResolver::doResolve() {
    tsPromiseResolver.BlockingCall(this);
  }

    struct Command
    {
      PromiseResolver* promiseResolver;
      string command;
      json_pack_t arguments;
      Command(const Napi::Env& env, const string& command, const json_pack_t& arguments):
        promiseResolver(new PromiseResolver(env)), // ownership passed to JS interpreter
        command(command),
        arguments(arguments) {}
    };

  struct Times
  {
    unsigned long counts;
    double elapsed, user, system;
    clock_t start_e, start_u, start_s;
    bool started;
    Times(): counts(0), elapsed(0), user(0), system(0), started(false) {}

    void start()
    {
      if (started) return;
      started = true;
      counts++;
          
#ifdef _WIN32
      start_e=clock();
#else
      struct tms tbuf;
      start_e = times(&tbuf);
      start_u = tbuf.tms_utime;
      start_s = tbuf.tms_stime;
#endif
    }
    
    void stop()
    {
      if (!started) return;
      started = false;

#ifdef _WIN32
      static const double seconds=1.0/CLOCKS_PER_SEC;
      elapsed+=(clock()-start_e)*seconds;
#else
#ifdef __linux__
      static const double seconds=1.0/sysconf(_SC_CLK_TCK);
#else
      static const double seconds=1.0/CLK_TCK;
#endif
      struct tms tbuf;
      elapsed += (times(&tbuf)-start_e)*seconds;
      user += (tbuf.tms_utime-start_u)*seconds;
      system += (tbuf.tms_stime-start_s)*seconds;
#endif
    }
  };

#ifdef TIMERS
  class Timer
  {
    Times& timer;
  public:
    Timer(Times& timer): timer(timer) {timer.start();}
    ~Timer() {timer.stop();}
  };

  class Timers: public map<string, Times>
  {
    Times overall;
  public:
    Timers() {overall.start();}
    ~Timers() {
      overall.stop();
      vector<pair<string,Times>> times(begin(),end());
      sort(times.begin(),times.end(),[](auto& i, auto& j){return i.second.elapsed>j.second.elapsed;});
      cout<<setw(40)<<"Times"<<setw(10)<<"Elapsed"<<setw(10)<<"User"<<setw(10)<<"System"<<endl;
      cout<<setw(40)<<"Overall"<<setw(10)<<overall.elapsed<<setw(10)<<overall.user<<setw(10)<<overall.system<<endl;
      for (auto& [command,timer]: times)
        cout<<setw(40)<<command<<setw(10)<<timer.elapsed<<setw(10)<<timer.user<<setw(10)<<timer.system<<endl;
    }
  };
#else // dummiable
  struct Timer
  {
    Timer(int) {}
  };
  struct Timers
  {
    int operator[](const string&) {return 0;}
  };
#endif
 
}

namespace minsky
{
  namespace
  {
    // ensure access to only one global Minsky object at a time,
    // particular needed for jest tests, which run in parallel
    mutex minskyCmdMutex; 

    struct AddOnMinsky: public RESTMinsky
    {
      Timers timers;
      deque<unique_ptr<Command>> minskyCommands;
      mutex cmdMutex;
      atomic<bool> running{true};
      std::thread thread;
      bool inputBufferExceeded=false;
      
      AddOnMinsky(): thread([this](){run();}) {
        flags=0;
        RESTProcess(registry,"minsky",static_cast<Minsky&>(*this));
      }
      
      ~AddOnMinsky() {
        running=false;
        if (thread.joinable()) thread.join();
      }

      Value queueCommand(Env env, string command, const json_pack_t& arguments)
      {
        static const std::string sync=".$sync";
        if (command.ends_with(sync))
          {
            command.erase(command.size()-sync.length());
            // Javascript needs the result returned as UTF-16.
            return String::New(env, utf_to_utf<char16_t>(doCommand(command, arguments)));
          }
#ifdef _WIN32
        // renderFrame needs to be called synchronously, otherwise inexplicable hangs occur on Windows.
        if (command.ends_with(".renderFrame"))
          return String::New(env, utf_to_utf<char16_t>(doCommand(command, arguments)));
#endif
        if (minskyCommands.size()>20)
          {
            if (!inputBufferExceeded) setBusyCursor();
            inputBufferExceeded=true;
            return env.Null();
          }
        if (inputBufferExceeded) clearBusyCursor(); // single shot clear of busy curser
        inputBufferExceeded=false;
        const lock_guard<mutex> lock(cmdMutex);
        minskyCommands.emplace_back(new Command{env,command,arguments});
        return minskyCommands.back()->promiseResolver->promise.Promise();
      }

      string doCommand(const string& command, const json_pack_t& arguments)
      {
        const lock_guard<mutex> lock(minskyCmdMutex);
        const Timer timer(timers[command]);
        const LocalMinsky lm(*this); // sets this to be the global minsky object

        // if reset requested, postpone it
        if (reset_flag()) requestReset();

        civita::ITensor::cancel(false);
        // disable quoting wide characters in UTF-8 strings
        auto result=write(registry.process(command, arguments),json5_parser::raw_utf8);
        commandHook(command,arguments);
        return result;
      }

      void drawNativeWindows()
      {
        const lock_guard<mutex> lock(minskyCmdMutex);
        const Timer timer(timers["draw"]);
        for (auto i: nativeWindowsToRedraw)
          try
            {
              const LocalMinsky lm(*this); // sets this to be the global minsky object
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
      }

      // arrange for native window drawing to happen on node's main thread, required for MacOSX.
      atomic<bool> drawLaunched{false};
      static void tsDrawNativeWindows(Napi::Env env, Napi::Function, AddOnMinsky* minsky, void*)
      {minsky->macOSXDrawNativeWindows();}
      
      TypedThreadSafeFunction<AddOnMinsky,void,tsDrawNativeWindows> tsDrawNativeWindows_;

      void macOSXDrawNativeWindows()
      {
        const lock_guard<mutex> lock(minskyCmdMutex);
        const Timer timer(timers["draw"]);
        for (auto i: nativeWindowsToRedraw)
          i->macOSXRedraw();
        nativeWindowsToRedraw.clear();
        drawLaunched=false;
       }
      
      void macOSXLaunchDrawNativeWindows()
      {
        drawLaunched=true;
        tsDrawNativeWindows_.BlockingCall(this);
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
              const lock_guard<mutex> lock(cmdMutex);
              if (!minskyCommands.empty())
                {
                  command=std::move(minskyCommands.front());
                  minskyCommands.pop_front();
                }
            }
            
            if (!command) // perform housekeeping
              {
                if (reset_flag() && resetDuration<resetPostponedThreshold && resetAt<std::chrono::system_clock::now())
                  try
                    {
                      const lock_guard<mutex> lock(minskyCmdMutex);
                      if (reset_flag()) // check again, in case another thread got there first
                        {
                          const LocalMinsky lm(*this); // sets this to be the global minsky object
                          const Timer timer(timers["minsky.reset"]);
                          reset();
                        }
                    }
                  catch (...)
                    {flags&=~reset_needed;}
#ifdef MAC_OSX_TK
                if (!drawLaunched && nativeWindowsToRedraw.size())
                  macOSXLaunchDrawNativeWindows();
#else
                drawNativeWindows();
#endif
                if (inputBufferExceeded && minskyCommands.empty())
                  {
                    // clears busy cursor when no commands are being received or processed
                    clearBusyCursor();
                    inputBufferExceeded=false;
                  }
                this_thread::sleep_for(chrono::milliseconds(10));
                continue;
              }

            if (!running) return;
            try
              {
                auto result=doCommand(command->command, command->arguments);
                if (!running) return; // prevent crashes on shutdown
                command->promiseResolver->resolve(result);
              }
            catch (const std::exception& ex)
              {
                if (!running) return; // prevent crashes on shutdown
                command->promiseResolver->reject(ex.what());
              }
            catch (...)
              {
                if (!running) return; // prevent crashes on shutdown
                command->promiseResolver->reject("Unknown exception");
              }
          } 
      }

      mutable string theMessage;
      mutable vector<string> messageButtons;
      mutable promise<unsigned> userResponse;
      static void messageCallback(Napi::Env env, Napi::Function fn, void*, AddOnMinsky* addonMinsky)
      {
        auto buttons=Array::New(env);
        for (unsigned i=0; i< addonMinsky->messageButtons.size(); ++i)
          buttons[i]=String::New(env,addonMinsky->messageButtons[i]);
        addonMinsky->userResponse.set_value(fn({String::New(env,addonMinsky->theMessage), buttons}).As<Number>().Int32Value());
      }

      bool messageCallbackSet=false;
      TypedThreadSafeFunction<void,AddOnMinsky,messageCallback> tsMessageCallback;
      Value setMessageCallback(const Napi::CallbackInfo& info)
      {
        const Env env = info.Env();
        if (info.Length()<1 || !info[0].IsFunction())
          {
            Napi::Error::New(env, "Callback not provided").ThrowAsJavaScriptException();
          }
        messageCallbackSet=true;
        tsMessageCallback=TypedThreadSafeFunction<void,AddOnMinsky,messageCallback>::New
          (env,info[0].As<Function>(), "message",0,2,nullptr);
        
        return env.Null();
      }

      
      void message(const std::string& msg) override
      {
        if (!messageCallbackSet) return;
        theMessage=msg;
        messageButtons.clear(); // empty buttons imply a single OK button
        userResponse={}; //reset the promise
        tsMessageCallback.BlockingCall(const_cast<AddOnMinsky*>(this));
      }

      MemCheckResult checkMemAllocation(std::size_t bytes) const override {
        // Electron restricts heap size to 4GiB, regardless of how much physical memory is present
        if (messageCallbackSet && bytes>/*physicalMem()*/4ULL*1024*1024*1024)
          {
            theMessage="Allocation will use more than 50% of available memory. Do you want to proceed?";
            messageButtons={"No","Yes"};
            userResponse={}; //reset the promise
            tsMessageCallback.BlockingCall(const_cast<AddOnMinsky*>(this));
            return userResponse.get_future().get()? proceed: abort;
          }
        return OK;
      }

      // signature of last param must be non-const
      static void busyCursorCallback(Napi::Env env, Napi::Function fn, void*, bool* busy)
      {
        fn({Boolean::New(env,*busy)});
      }
      
      bool busyCursorCallbackSet=false;
      TypedThreadSafeFunction<void,bool,busyCursorCallback> tsBusyCursorCallback;
      Value setBusyCursorCallback(const Napi::CallbackInfo& info)
      {
        const Env env = info.Env();
        if (info.Length()<1 || !info[0].IsFunction())
          {
            Napi::Error::New(env, "Callback not provided").ThrowAsJavaScriptException();
          }
        busyCursorCallbackSet=true;
        tsBusyCursorCallback=TypedThreadSafeFunction<void,bool,busyCursorCallback>::New
          (env,info[0].As<Function>(), "setBusyCursor",0,2,nullptr);
        return env.Null();
      }

      bool busyCursor=false;
      void doBusyCursor(bool bc)
      {
        if (!busyCursorCallbackSet) return;
        busyCursor=bc;
        tsBusyCursorCallback.BlockingCall(&busyCursor);
      }
      void setBusyCursor() override
      {doBusyCursor(true);}
      void clearBusyCursor() override
      {doBusyCursor(false);}

      static void progressCallback(Napi::Env env, Napi::Function fn, void*, AddOnMinsky* addon)
      {
        fn({String::New(env,addon->progressTitle), Number::New(env,addon->progressValue)});
      }
      
      int progressValue=0;
      string progressTitle;
      void progress(const string& title, int x) override
      {
        if (!progressCallbackSet) return;
        progressValue=x;
        progressTitle=title;
        tsProgressCallback.BlockingCall(this);
      }
      bool progressCallbackSet=false;
      TypedThreadSafeFunction<void,AddOnMinsky,progressCallback> tsProgressCallback;
      Value setProgressCallback(const Napi::CallbackInfo& info)
      {
        const Env env = info.Env();
        if (info.Length()<1 || !info[0].IsFunction())
          {
            Napi::Error::New(env, "Callback not provided").ThrowAsJavaScriptException();
          }
        progressCallbackSet=true;
        tsProgressCallback=TypedThreadSafeFunction<void,AddOnMinsky,progressCallback>::New
          (env,info[0].As<Function>(), "progress",0,2,nullptr);
        return env.Null();
      }
      
      static void bookmarkRefreshCallback(Napi::Env env, Napi::Function fn, void*, AddOnMinsky* addon)
      {
        fn({});
      }
      
      void bookmarkRefresh() override
      {
        if (!bookmarkRefreshSet) return;
        tsBookmarkRefreshCallback.BlockingCall(this);
      }
      bool bookmarkRefreshSet=false;
      TypedThreadSafeFunction<void,AddOnMinsky,bookmarkRefreshCallback> tsBookmarkRefreshCallback;
      Value setBookmarkRefreshCallback(const Napi::CallbackInfo& info)
      {
        const Env env = info.Env();
        if (info.Length()<1 || !info[0].IsFunction())
          {
            Napi::Error::New(env, "Callback not provided").ThrowAsJavaScriptException();
          }
        bookmarkRefreshSet=true;
        tsBookmarkRefreshCallback=TypedThreadSafeFunction<void,AddOnMinsky,bookmarkRefreshCallback>::New
          (env,info[0].As<Function>(), "refreshBookmark",0,2,nullptr);
        return env.Null();
      }
    
      static void resetScrollCallback(Napi::Env env, Napi::Function fn, void*, AddOnMinsky* addon)
      {
        fn({});
      }
  
      void resetScroll() override
      {
        if (!resetScrollSet) return;
        tsResetScrollCallback.BlockingCall(this);
      }
      bool resetScrollSet=false;
      TypedThreadSafeFunction<void,AddOnMinsky,resetScrollCallback> tsResetScrollCallback;
      Value setResetScrollCallback(const Napi::CallbackInfo& info)
      {
        const Env env = info.Env();
        if (info.Length()<1 || !info[0].IsFunction())
          {
            Napi::Error::New(env, "Callback not provided").ThrowAsJavaScriptException();
          }
        resetScrollSet=true;
        tsResetScrollCallback=TypedThreadSafeFunction<void,AddOnMinsky,resetScrollCallback>::New
          (env,info[0].As<Function>(), "resetScroll",0,2,nullptr);
        return env.Null();
      }
    };
    
    Minsky* l_minsky=NULL;
  }

  Minsky& minsky()
  {
    static Minsky s_minsky;
    if (l_minsky)
      return *l_minsky;
    return s_minsky;
  }

  LocalMinsky::LocalMinsky(Minsky& minsky) {l_minsky=&minsky;}
  LocalMinsky::~LocalMinsky() {l_minsky=NULL;}

  // GUI callback needed only to solve linkage problems
  void doOneEvent(bool idleTasksOnly) {}

}

struct MinskyAddon: public Addon<MinskyAddon>
{
  minsky::AddOnMinsky addOnMinsky;
  minsky::LocalMinsky lm{addOnMinsky};

  MinskyAddon(Env env, Object exports)
  {
    tsPromiseResolver=TypedThreadSafeFunction<void,PromiseResolver,resolvePromise>::
      New(env,"TSResolver", 0, 2, nullptr);
    addOnMinsky.tsDrawNativeWindows_=
      TypedThreadSafeFunction<minsky::AddOnMinsky,void,minsky::AddOnMinsky::tsDrawNativeWindows>::
      New(env,"TSDrawNativeWindows",0, 2,&addOnMinsky);
    
    DefineAddon(exports, {
        InstanceMethod("call", &MinskyAddon::call),
        InstanceMethod("setMessageCallback", &MinskyAddon::setMessageCallback),
        InstanceMethod("setBusyCursorCallback", &MinskyAddon::setBusyCursorCallback),
        InstanceMethod("setProgressCallback", &MinskyAddon::setProgressCallback),
        InstanceMethod("setBookmarkRefreshCallback", &MinskyAddon::setBookmarkRefreshCallback),
        InstanceMethod("setResetScrollCallback", &MinskyAddon::setResetScrollCallback),
        InstanceMethod("cancelProgress", &MinskyAddon::cancelProgress)
      });
  }

  Value setMessageCallback(const Napi::CallbackInfo& info) {return addOnMinsky.setMessageCallback(info);}
  Value setBusyCursorCallback(const Napi::CallbackInfo& info) {return addOnMinsky.setBusyCursorCallback(info);}
  Value setProgressCallback(const Napi::CallbackInfo& info) {return addOnMinsky.setProgressCallback(info);}
  Value setBookmarkRefreshCallback(const Napi::CallbackInfo& info) {return addOnMinsky.setBookmarkRefreshCallback(info);}
  Value setResetScrollCallback(const Napi::CallbackInfo& info) {return addOnMinsky.setResetScrollCallback(info);}
  Value cancelProgress(const Napi::CallbackInfo& info) {
    *addOnMinsky.progressState.cancel=true;
    civita::ITensor::cancel(true);
    return info.Env().Null();
  }
    

  Value call(const Napi::CallbackInfo& info)
  {
    const Env env = info.Env();
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
        json_pack_t arguments(json5_parser::mValue::null);
        if (info.Length()>1)
          {
            const string jsonArguments=info[1].ToString();
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
