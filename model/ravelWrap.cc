/*
  @copyright Steve Keen 2018
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

#include "ravelWrap.h"
#include "selection.h"
#include <ecolab_epilogue.h>
static const int ravelVersion=1;

#include <string>
using namespace std;

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace
{
  struct InvalidSym {};
  
#ifdef WIN32
  typedef HINSTANCE libHandle;
  libHandle loadLibrary(const string& lib)
  {return LoadLibraryA((lib+".dll").c_str());}

  FARPROC WINAPI dlsym(HMODULE lib, const char* name)
  {return GetProcAddress(lib,name);}

  void dlclose(HINSTANCE) {}

  const string dlerror() {
    char msg[1024];
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,nullptr,GetLastError(),0,msg,sizeof(msg),nullptr);
    return msg;
  }
#else
  typedef void* libHandle;
  libHandle loadLibrary(const string& lib)
  {return dlopen((lib+".so").c_str(),RTLD_NOW);}
#endif
  
  template <class F>
  void asgFnPointer(F& f, libHandle lib, const char* name)
  {
    f=(F)dlsym(lib,name);
    if (!f) throw InvalidSym();
  }
#define ASG_FN_PTR(f,lib) asgFnPointer(f,lib,#f)

  void* (*ravel_new)(size_t rank)=nullptr;
  void (*ravel_delete)(void* ravel)=nullptr;
  void (*ravel_render)(void* ravel, cairo_t* cairo)=nullptr;
  void (*ravel_onMouseDown)(void* ravel, double x, double y)=nullptr;
  void (*ravel_onMouseUp)(void* ravel, double x, double y)=nullptr;
  bool (*ravel_onMouseMotion)(void* ravel, double x, double y)=nullptr;
  bool (*ravel_onMouseOver)(void* ravel, double x, double y)=nullptr;
  void (*ravel_onMouseLeave)(void* ravel)=nullptr;
  void (*ravel_rescale)(void* ravel, double radius);

  struct RavelLib
  {
    libHandle lib;
    RavelLib(): lib(loadLibrary("libRavel"))
    {
#ifndef NDEBUG
      if (!lib)
        cerr << dlerror() << endl;
#endif
      
    auto version=(int (*)())dlsym(lib,"ravel_version");
    if (!version || ravelVersion!=version())
      { // incompatible API
        dlclose(lib);
        lib=nullptr;
      }

    if (lib)
        try
          {
            ASG_FN_PTR(ravel_new,lib);
            ASG_FN_PTR(ravel_delete,lib);
            ASG_FN_PTR(ravel_render,lib);
            ASG_FN_PTR(ravel_onMouseDown,lib);
            ASG_FN_PTR(ravel_onMouseMotion,lib);
            ASG_FN_PTR(ravel_onMouseOver,lib);
            ASG_FN_PTR(ravel_onMouseLeave,lib);
            ASG_FN_PTR(ravel_rescale,lib);
          }
        catch (InvalidSym)
          {
            dlclose(lib);
            lib=nullptr;
          }
    }
    ~RavelLib() {if (lib) dlclose(lib);}
  };

  RavelLib ravelLib;
}

namespace minsky
{
  bool ravelAvailable() {return !ravelLib.lib;}
  
  RavelWrap::RavelWrap()
  {
    ravel=ravel_new(1); // rank 1 for now
    ravel_rescale(ravel,100);
  }

  RavelWrap::~RavelWrap()
  {
    if (ravel && ravel_delete) ravel_delete(ravel);
  }

  void RavelWrap::noRavelSetup()
  {
    tooltip="https://ravelation.hpcoders.com.au";
  }

  void RavelWrap::draw(cairo_t* cairo) const
  {
    if (ravel)
      ravel_render(ravel,cairo);
    else
      DataOp::draw(cairo);
  }
}

  
