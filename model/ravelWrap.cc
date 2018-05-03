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
#include "minsky.h"
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

#include "cairoRenderer.h"

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
    Ravel::RavelImpl* (*ravel_new)(size_t rank)=nullptr;
    void (*ravel_delete)(Ravel::RavelImpl* ravel)=nullptr;
    void (*ravel_render)(Ravel::RavelImpl* ravel, CAPIRenderer*)=nullptr;
    void (*ravel_onMouseDown)(Ravel::RavelImpl* ravel, double x, double y)=nullptr;
    void (*ravel_onMouseUp)(Ravel::RavelImpl* ravel, double x, double y)=nullptr;
    bool (*ravel_onMouseMotion)(Ravel::RavelImpl* ravel, double x, double y)=nullptr;
    bool (*ravel_onMouseOver)(Ravel::RavelImpl* ravel, double x, double y)=nullptr;
    void (*ravel_onMouseLeave)(Ravel::RavelImpl* ravel)=nullptr;
    void (*ravel_rescale)(Ravel::RavelImpl* ravel, double radius)=nullptr;
    double (*ravel_radius)(Ravel::RavelImpl* ravel)=nullptr;
    size_t (*ravel_rank)(Ravel::RavelImpl* ravel)=nullptr;
    void (*ravel_outputHandleIds)(Ravel::RavelImpl* ravel, size_t ids[])=nullptr;
    void (*ravel_setOutputHandleIds)(Ravel::RavelImpl* ravel, size_t rank, size_t ids[])=nullptr;
    unsigned (*ravel_numHandles)(Ravel::RavelImpl* ravel)=nullptr;
    const char* (*ravel_handleDescription)(Ravel::RavelImpl* ravel, size_t handle)=nullptr;
    void (*ravel_sliceLabels)(Ravel::RavelImpl* ravel, size_t axis, const char* labels[])=nullptr;
    void (*ravel_displayFilterCaliper)(Ravel::RavelImpl* ravel, size_t axis, bool display)=nullptr;
    const char* (*ravel_toXML)(Ravel::RavelImpl* ravel)=nullptr;
    int (*ravel_fromXML)(Ravel::RavelImpl* ravel, const char*)=nullptr;
    void (*ravel_getHandleState)(const Ravel::RavelImpl* ravel, size_t handle, Ravel::HandleState* handleState)=nullptr;
    void (*ravel_setHandleState)(Ravel::RavelImpl* ravel, size_t handle, const Ravel::HandleState* handleState)=nullptr;

    Ravel::Ravel::DataCube* (*ravelDC_new)()=nullptr;
    void (*ravelDC_delete)(Ravel::DataCube*)=nullptr;
    bool (*ravelDC_initRavel)(Ravel::DataCube* dc,Ravel::RavelImpl* ravel)=nullptr;
    bool (*ravelDC_openFile)(Ravel::DataCube* dc, const char* fileName, DataSpec spec)=nullptr;
    int (*ravelDC_hyperSlice)(Ravel::DataCube*, Ravel::RavelImpl*, size_t dims[], double**)=nullptr;


    struct RavelLib
    {
      libHandle lib;
      string errorMsg;
      string versionFound;
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
            try
              {
                ASG_FN_PTR(ravel_version,lib);
                versionFound=ravel_version();
              }
            catch (...) {}
            dlclose(lib);
            lib=nullptr;
          }

        if (lib)
          try
            {
              ASG_FN_PTR(ravel_version,lib);
              versionFound=ravel_version();
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
              ASG_FN_PTR(ravel_rank,lib);
              ASG_FN_PTR(ravel_outputHandleIds,lib);
              ASG_FN_PTR(ravel_setOutputHandleIds,lib);
              ASG_FN_PTR(ravel_numHandles,lib);
              ASG_FN_PTR(ravel_handleDescription,lib);
              ASG_FN_PTR(ravel_sliceLabels,lib);
              ASG_FN_PTR(ravel_displayFilterCaliper,lib);
              ASG_FN_PTR(ravel_toXML,lib);
              ASG_FN_PTR(ravel_fromXML,lib);
              ASG_FN_PTR(ravel_getHandleState,lib);
              ASG_FN_PTR(ravel_setHandleState,lib);
              ASG_FN_PTR(ravelDC_new,lib);
              ASG_FN_PTR(ravelDC_delete,lib);
              ASG_FN_PTR(ravelDC_initRavel,lib);
              ASG_FN_PTR(ravelDC_openFile,lib);
              ASG_FN_PTR(ravelDC_hyperSlice,lib);
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
      ~RavelLib() {
        if (lib)
          dlclose(lib);
        lib=nullptr;
      }
    };

    RavelLib ravelLib;

    inline double sqr(double x) {return x*x;} 
  }

  bool ravelAvailable() {return ravelLib.lib;}

  const char* Ravel::ravelVersion() const
  {return ravel_version? ravel_version(): "Ravel unavailable";}

  const char* Ravel::lastErr() const {
    if (ravelAvailable())
      return ravel_lastErr();
    return ravelLib.errorMsg.c_str();
  }
  
  Ravel::Ravel()
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

  Ravel::~Ravel()
  {
    if (ravelAvailable()) // NB during shutdown, ravel may be unloaded before getting here
      {
        if (ravel) ravel_delete(ravel);
        if (dataCube) ravelDC_delete(dataCube);
      }
  }

  void Ravel::noRavelSetup()
  {
    tooltip="https://ravelation.hpcoders.com.au";
    detailedText=ravelLib.errorMsg;
  }

  void Ravel::draw(cairo_t* cairo) const
  {
    double r=1.1*zoomFactor*ravel_radius(ravel);
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
    {
      cairo::CairoSave cs(cairo);
      cairo_rectangle(cairo,-r,-r,2*r,2*r);
      cairo_clip(cairo);
      cairo_scale(cairo,zoomFactor,zoomFactor);
      ravel::CairoRenderer cr(cairo);
      if (ravel)
        ravel_render(ravel,&cr);
    }        
    if (selected) drawSelected(cairo);
  }

  void Ravel::resize(const LassoBox& b)
  {
    if (ravel)
      ravel_rescale(ravel, 0.5*std::max(fabs(b.x0-b.x1),fabs(b.y0-b.y1))/(1.21*zoomFactor));
  }
  
  ClickType::Type Ravel::clickType(float xx, float yy)
  {
    for (auto& p: ports)
      if (hypot(xx-p->x(), yy-p->y()) < portRadius*zoomFactor)
        return ClickType::onPort;
    double r=1.1*zoomFactor*ravel_radius(ravel);
    if (std::abs(xx-x())>1.1*r || std::abs(yy-y())>1.1*r)
      return ClickType::outside;
    else if (std::abs(xx-x())<=r && std::abs(yy-y())<=r)
      return ClickType::onRavel;
    else
      return ClickType::onItem;
  }


  void Ravel::onMouseDown(float xx, float yy)
  {if (ravel) ravel_onMouseDown(ravel,(xx-x())/zoomFactor,(yy-y())/zoomFactor);}
  void Ravel::onMouseUp(float xx, float yy)
  {
    if (ravel)
      ravel_onMouseUp(ravel,(xx-x())/zoomFactor,(yy-y())/zoomFactor);
  }
  bool Ravel::onMouseMotion(float xx, float yy)
  {if (ravel) return ravel_onMouseMotion(ravel,(xx-x())/zoomFactor,(yy-y())/zoomFactor); return false;}
  bool Ravel::onMouseOver(float xx, float yy)
  {if (ravel) return ravel_onMouseOver(ravel,(xx-x())/zoomFactor,(yy-y())/zoomFactor); return false;}
  void Ravel::onMouseLeave()
  {if (ravel) ravel_onMouseLeave(ravel);}

  void Ravel::loadFile(const string& fileName)
  {
    m_filename=fileName;
    if (dataCube && ravel)
      {
        if (!ravelDC_openFile(dataCube, fileName.c_str(), DataSpec()))
          throw error(ravel_lastErr());
        else if (!ravelDC_initRavel(dataCube,ravel))
          throw error(ravel_lastErr());
        for (size_t i=0; i<ravel_numHandles(ravel); ++i)
          ravel_displayFilterCaliper(ravel,i,true);
      }
  }

  void Ravel::loadDataFromSlice(VariableValue& v)
  {
    if (ravel && dataCube)
      {
        assert(ravel_rank(ravel)==1);
        vector<size_t> dims(ravel_rank(ravel));
        double* tmp;
        ravelDC_hyperSlice(dataCube, ravel, &dims[0], &tmp);
        if (dims[0]==0)
          {
            if (v.idx()==-1) v.allocValue();
            return; // do nothing if ravel data is empty
          }
        if (tmp)
          {
            vector<size_t> outHandles(dims.size());
            ravel_outputHandleIds(ravel, &outHandles[0]);
            size_t prevNumElem=v.numElements();
            v.xVector.clear();
            for (auto h: outHandles)
              {
                vector<const char*> labels(dims[0]); 
                ravel_sliceLabels(ravel,h,&labels[0]);
                assert(all_of(labels.begin(), labels.end(),
                              [](const char* i){return bool(i);}));
                set<double> testNum;
                for (auto& i: labels)
                  testNum.insert(atof(i));
                //numerically converted labels are all distinct
                bool numerical=testNum.size()==labels.size(); 
                v.xVector.emplace_back
                  (ravel_handleDescription(ravel,h));
                for (size_t i=0; i<labels.size(); ++i)

                  v.xVector.back().emplace_back
                    (numerical? stod(labels[i]): double(i+1),labels[i]);
              }
            if (v.idx()==-1 || prevNumElem!=v.numElements())
              v.allocValue();
            //assert(v.numElements()==sizeof(*tmp))
            for (size_t i=0; i<v.numElements(); ++i)
              *(v.begin()+i)=tmp[i];
          }
        else
          throw error(ravel_lastErr());
      }
    if (v.idx()==-1) v.allocValue();
  }

//  const char* Ravel::toXML() const
//  {
//    if (ravel)
//      return ravel_toXML(ravel);
//    return "";
//  }
//
//  void Ravel::fromXML(const std::string& xml)
//  {
//    if (ravel)
//      ravel_fromXML(ravel, xml.c_str());
//  }

  Ravel::State Ravel::getState() const
  {
    State state;
    if (ravel)
      {
        for (size_t i=0; i<ravel_numHandles(ravel); ++i)
          ravel_getHandleState
            (ravel, i, &state.handleStates[ravel_handleDescription(ravel,i)]);
        vector<size_t> ids(ravel_rank(ravel));
        for (size_t i=0; i<ids.size(); ++i)
          state.outputHandles.push_back(ravel_handleDescription(ravel,ids[i]));
      }
    return state;
  }

  /// apply the \a state to the Ravel, leaving data, slicelabels etc unchanged
  void Ravel::applyState(const State& state)
  {
    if (ravel)
      {
        map<string,size_t> nameToIdx;
        for (size_t i=0; i<ravel_numHandles(ravel); ++i)
          {
            string name=ravel_handleDescription(ravel,i);
            auto hs=state.handleStates.find(name);
            if (hs!=state.handleStates.end())
              ravel_setHandleState(ravel,i,&hs->second);
            nameToIdx[name]=i;
          }
        vector<size_t> ids;
        for (auto& i: state.outputHandles)
          ids.push_back(nameToIdx[i]);
        ravel_setOutputHandleIds(ravel,ids.size(),&ids[0]);
      }
  }

  
  string Minsky::ravelVersion() const
  {
    if (ravel_version)
      return string(ravel_version())+(ravelLib.lib?"":" but incompatible");
    else if (ravelLib.versionFound.length())
      return ravelLib.versionFound+" but incompatible";
    else
      return "unavailable";
  }
 
}

  
