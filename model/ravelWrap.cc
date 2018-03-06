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
#include <cmath>
using namespace std;

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif


namespace minsky
{
  namespace
  {
    struct InvalidSym {
      const string symbol;
      InvalidSym(const string& s): symbol(s) {}
    };

    struct DataSpec
    {
      int nRowAxes=-1; ///< No. rows describing axes
      int nColAxes=-1; ///< No. cols describing axes
      int nCommentLines=-1; ///< No. comment header lines
      char separator=','; ///< field separator character
    };

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
      if (!f) throw InvalidSym(name);
    }
#define ASG_FN_PTR(f,lib) asgFnPointer(f,lib,#f)

    const char* (*ravel_lastErr)()=nullptr;
    const char* (*ravel_version)()=nullptr;
    Ravel* (*ravel_new)(size_t rank)=nullptr;
    void (*ravel_delete)(Ravel* ravel)=nullptr;
    void (*ravel_render)(Ravel* ravel, cairo_t* cairo)=nullptr;
    void (*ravel_onMouseDown)(Ravel* ravel, double x, double y)=nullptr;
    void (*ravel_onMouseUp)(Ravel* ravel, double x, double y)=nullptr;
    bool (*ravel_onMouseMotion)(Ravel* ravel, double x, double y)=nullptr;
    bool (*ravel_onMouseOver)(Ravel* ravel, double x, double y)=nullptr;
    void (*ravel_onMouseLeave)(Ravel* ravel)=nullptr;
    void (*ravel_rescale)(Ravel* ravel, double radius)=nullptr;
    double (*ravel_radius)(Ravel* ravel)=nullptr;

    DataCube* (*ravelDC_new)()=nullptr;
    void (*ravelDC_delete)(DataCube*)=nullptr;
    bool (*ravelDC_initRavel)(DataCube* dc,Ravel* ravel)=nullptr;
    bool (*ravelDC_openFile)(DataCube* dc, const char* fileName, DataSpec spec)=nullptr;


    struct RavelLib
    {
      libHandle lib;
      string errorMsg;
      RavelLib(): lib(loadLibrary("libravel"))
      {
        if (!lib)
          {
            errorMsg=dlerror();
            return;
          }
      
        auto version=(int (*)())dlsym(lib,"ravel_capi_version");
        if (!version || ravelVersion!=version())
          { // incompatible API
            errorMsg="Incompatible libravel dynamic library found";
            dlclose(lib);
            lib=nullptr;
          }

        if (lib)
          try
            {
              ASG_FN_PTR(ravel_version,lib);
              ASG_FN_PTR(ravel_lastErr,lib);
              ASG_FN_PTR(ravel_new,lib);
              ASG_FN_PTR(ravel_delete,lib);
              ASG_FN_PTR(ravel_render,lib);
              ASG_FN_PTR(ravel_onMouseDown,lib);
              ASG_FN_PTR(ravel_onMouseUp,lib);
              ASG_FN_PTR(ravel_onMouseMotion,lib);
              ASG_FN_PTR(ravel_onMouseOver,lib);
              ASG_FN_PTR(ravel_onMouseLeave,lib);
              ASG_FN_PTR(ravel_rescale,lib);
              ASG_FN_PTR(ravel_radius,lib);
              ASG_FN_PTR(ravelDC_new,lib);
              ASG_FN_PTR(ravelDC_delete,lib);
              ASG_FN_PTR(ravelDC_initRavel,lib);
              ASG_FN_PTR(ravelDC_openFile,lib);
            }
          catch (const InvalidSym& err)
            {
              errorMsg=dlerror();
              errorMsg+="\nLooking for "+err.symbol;
              errorMsg+="\nProbably libravel dynamic library is too old";
              dlclose(lib);
              lib=nullptr;
            }
      }
      ~RavelLib() {if (lib) dlclose(lib);}
    };

    RavelLib ravelLib;

    inline double sqr(double x) {return x*x;} 
  }

  bool ravelAvailable() {return ravelLib.lib;}

  const char* RavelWrap::ravelVersion() const
  {return ravel_version? ravel_version(): "Ravel unavailable";}

  const char* RavelWrap::lastErr() const {
    if (ravelAvailable())
      return ravel_lastErr();
    return ravelLib.errorMsg.c_str();
  }
  
  RavelWrap::RavelWrap()
  {
    if (ravelAvailable())
      {
        ravel=ravel_new(1); // rank 1 for now
        ravel_rescale(ravel,100);
        dataCube=ravelDC_new();
      }
    else
      noRavelSetup();
  }

  RavelWrap::~RavelWrap()
  {
    if (ravel) ravel_delete(ravel);
    if (dataCube) ravelDC_delete(dataCube);
  }

  void RavelWrap::noRavelSetup()
  {
    tooltip="https://ravelation.hpcoders.com.au";
    detailedText=ravelLib.errorMsg;
  }

  void RavelWrap::draw(cairo_t* cairo) const
  {
    if (ravel)
      {
        ravel_render(ravel,cairo);
        double r=1.1*ravel_radius(ravel);
        ports[0]->moveTo(x()+1.1*r, y());
        ports[1]->moveTo(x()-1.1*r, y());
        if (mouseFocus)
          {
            drawPorts(cairo);
            displayTooltip(cairo);
          }
        cairo_rectangle(cairo,-r,-r,2*r,2*r);
        cairo_rectangle(cairo,-1.1*r,-1.1*r,2.2*r,2.2*r);
        cairo_stroke_preserve(cairo);
        cairo_clip(cairo);
        if (selected) drawSelected(cairo);
      }
    else
      DataOp::draw(cairo);
  }

  ClickType::Type RavelWrap::clickType(float xx, float yy)
  {
    if (ravel)
      {
        for (auto& p: ports)
          if (hypot(xx-p->x(), yy-p->y()) < portRadius*zoomFactor)
            return ClickType::onPort;
        double r=1.1*ravel_radius(ravel);
        if (std::abs(xx-x())>1.1*r || std::abs(yy-y())>1.1*r)
          return ClickType::outside;
        else if (std::abs(xx-x())<=r && std::abs(yy-y())<=r)
          return ClickType::onRavel;
        else
          return ClickType::onItem;
      }
    else
      return DataOp::clickType(xx,yy);
  }

  void RavelWrap::onMouseDown(float xx, float yy)
  {if (ravel) ravel_onMouseDown(ravel,xx-x(),yy-y());}
  void RavelWrap::onMouseUp(float xx, float yy)
  {if (ravel) ravel_onMouseUp(ravel,xx-x(),yy-y());}
  bool RavelWrap::onMouseMotion(float xx, float yy)
  {if (ravel) return ravel_onMouseMotion(ravel,xx-x(),yy-y());}
  bool RavelWrap::onMouseOver(float xx, float yy)
  {if (ravel) return ravel_onMouseOver(ravel,xx-x(),yy-y());}
  void RavelWrap::onMouseLeave()
  {if (ravel) ravel_onMouseLeave(ravel);}

  void RavelWrap::loadFile(const char* fileName)
  {
    if (dataCube)
      {
        if (!ravelDC_openFile(dataCube, fileName, DataSpec()))
          detailedText+=string("\n")+ravel_lastErr();
        else if (!ravelDC_initRavel(dataCube,ravel))
          detailedText+=string("\n")+ravel_lastErr();
      }
  }

  
}

  
