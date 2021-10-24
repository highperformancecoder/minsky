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

#include <exception>
#include <iostream>

using namespace Napi;


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
      return info[0];
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
  std::cout << "Init"<<std::endl;
  exports.Set(String::New(env, "call"), Function::New(env, RESTCall));
  return exports;
}

NODE_API_MODULE(addon, Init)

