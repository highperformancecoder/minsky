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

#include <dlfcn.h>

namespace
{
  struct InvalidSym {};
  
  template <class F>
  void asgFnPointer(F& f, void* lib, const char* name)
  {
    f=(F)dlsym(lib,name);
    if (!f) throw InvalidSym();
  }
#define ASG_FN_PTR(f,lib) asgFnPointer(f,lib,#f)
}

namespace minsky
{
  RavelWrap::RavelWrap()
  {
    auto lib=dlopen("libravel.so",RTLD_LAZY);
    if (!lib)
      {
#ifndef NDEBUG
        cerr << dlerror() << endl;
#endif
        noRavelSetup();
        return;
      }
    
    auto version=(int (*)())dlsym(lib,"ravel_version");
    if (!version || ravelVersion!=version())
      noRavelSetup(); // incompatible API
    else
      try
        {
          void* (*ravel_new)(size_t);
          ASG_FN_PTR(ravel_new,lib);
          ASG_FN_PTR(ravel_delete,lib);
          ASG_FN_PTR(ravel_render,lib);
          ASG_FN_PTR(ravel_onMouseDown,lib);
          ASG_FN_PTR(ravel_onMouseMotion,lib);
          ASG_FN_PTR(ravel_onMouseOver,lib);
          ASG_FN_PTR(ravel_onMouseLeave,lib);
          ASG_FN_PTR(ravel_rescale,lib);
          ravel=ravel_new(1); // rank 1 for now
          ravel_rescale(ravel,100);
        }
      catch (InvalidSym)
        {
          noRavelSetup();
        }
    dlclose(lib);
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

  
