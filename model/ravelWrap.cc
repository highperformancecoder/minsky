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
#include "minsky_epilogue.h"
static const int ravelVersion=2;

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
  unsigned RavelLockGroup::nextColour=1;

  namespace
  {
    struct InvalidSym {
      const string symbol;
      InvalidSym(const string& s): symbol(s) {}
    };

    struct RavelDataSpec
    {
      int nRowAxes=-1; ///< No. rows describing axes
      int nColAxes=-1; ///< No. cols describing axes
      int nCommentLines=-1; ///< No. comment header lines
      char separator=','; ///< field separator character
    };

    typedef RavelState::HandleState::ReductionOp ReductionOp;
    typedef RavelState::HandleState::HandleSort HandleSort;
    
    struct RavelHandleState
    {
      double x=0,y=0; ///< handle tip coordinates (only angle important, not length)
      size_t sliceIndex=0, sliceMin=0, sliceMax=0;
      bool collapsed=false, displayFilterCaliper=false;
      ReductionOp reductionOp=RavelState::HandleState::sum;
      HandleSort order=RavelState::HandleState::none;
      RavelHandleState() {}
      // NB: sliceIndex, sliceMin, sliceMax need to be dealt with separately
      RavelHandleState(const RavelState::HandleState& s):
        x(s.x), y(s.y), collapsed(s.collapsed), displayFilterCaliper(s.displayFilterCaliper),
        reductionOp(s.reductionOp), order(s.order) {}
      operator RavelState::HandleState() {
        RavelState::HandleState r;
        r.x=x; r.y=y;
        r.collapsed=collapsed; r.displayFilterCaliper=displayFilterCaliper;
        r.reductionOp=reductionOp; r.order=order;
        return r;
      }
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
  
    //#define ASG_FN_PTR(f,lib) asgFnPointer(f,lib,#f)

    struct RavelLib
    {
      libHandle lib;
      string errorMsg;
      string versionFound="unavailable";
      RavelLib(): lib(loadLibrary("libravel"))
      {
        if (!lib)
          {
            errorMsg=dlerror();
            return;
          }
      
        auto version=(const char* (*)())dlsym(lib,"ravel_version");
        auto capi_version=(int (*)())dlsym(lib,"ravel_capi_version");
        if (version) versionFound=version();
        if (!version || !capi_version || ravelVersion!=capi_version())
          { // incompatible API
            errorMsg="Incompatible libravel dynamic library found";
            dlclose(lib);
            lib=nullptr;
          }
      }
      ~RavelLib() {
        if (lib)
          dlclose(lib);
        lib=nullptr;
      }
      template <class F>
      void asgFnPointer(F& f, const char* name)
      {
        if (lib)
          {
            f=(F)dlsym(lib,name);
            if (!f)
              {
                errorMsg=dlerror();
                errorMsg+="\nLooking for ";
                errorMsg+=name;
                errorMsg+="\nProbably libravel dynamic library is too old";
                dlclose(lib);
                lib=nullptr;
              }
          }
      }
    };

    RavelLib ravelLib;

    template <class... T> struct RavelFn;
    
    template <class R>
    struct RavelFn<R>
    {
      R (*f)()=nullptr;
      RavelFn(const char*name, libHandle lib) {ravelLib.asgFnPointer(f,name);}
      R operator()() {return f? f(): R();}
    };
    template <>
    struct RavelFn<void>
    {
      void (*f)()=nullptr;
      RavelFn(const char*name, libHandle lib) {ravelLib.asgFnPointer(f,name);}
      void operator()() {if (f) f();}
    };
    template <class R, class A>
    struct RavelFn<R,A>
    {
      R (*f)(A)=nullptr;
      RavelFn(const char*name, libHandle lib) {ravelLib.asgFnPointer(f,name);}
      R operator()(A a) {return f? f(a): R{};}
    };
    template <class A>
    struct RavelFn<void,A>
    {
      void (*f)(A)=nullptr;
      RavelFn(const char*name, libHandle lib) {ravelLib.asgFnPointer(f,name);}
      void operator()(A a) {if (f) f(a);}
    };
    template <class R, class A0, class A1>
    struct RavelFn<R,A0,A1>
    {
      R (*f)(A0,A1)=nullptr;
      RavelFn(const char*name, libHandle lib) {ravelLib.asgFnPointer(f,name);}
      R operator()(A0 a0, A1 a1) {return f? f(a0,a1): R{};}
    };
    template <class A0, class A1>
    struct RavelFn<void,A0,A1>
    {
      void (*f)(A0,A1)=nullptr;
      RavelFn(const char*name, libHandle lib) {ravelLib.asgFnPointer(f,name);}
      void operator()(A0 a0, A1 a1) {if (f) f(a0,a1);}
    };
    template <class R, class A0, class A1, class A2>
    struct RavelFn<R,A0,A1,A2>
    {
      R (*f)(A0,A1,A2)=nullptr;
      RavelFn(const char*name, libHandle lib) {ravelLib.asgFnPointer(f,name);}
      R operator()(A0 a0, A1 a1,A2 a2) {return f? f(a0,a1,a2): R{};}
    };
    template <class A0, class A1, class A2>
    struct RavelFn<void,A0,A1,A2>
    {
      void (*f)(A0,A1,A2)=nullptr;
      RavelFn(const char*name, libHandle lib) {ravelLib.asgFnPointer(f,name);}
      void operator()(A0 a0, A1 a1, A2 a2) {if (f) f(a0,a1,a2);}
    };
    template <class R, class A0, class A1, class A2, class A3>
    struct RavelFn<R,A0,A1,A2,A3>
    {
      R (*f)(A0,A1,A2,A3)=nullptr;
      RavelFn(const char*name, libHandle lib) {ravelLib.asgFnPointer(f,name);}
      R operator()(A0 a0, A1 a1,A2 a2,A3 a3) {return f? f(a0,a1,a2,a3): R{};}
    };
    template <class A0, class A1, class A2,class A3>
    struct RavelFn<void,A0,A1,A2,A3>
    {
      void (*f)(A0,A1,A2,A3)=nullptr;
      RavelFn(const char*name, libHandle lib) {ravelLib.asgFnPointer(f,name);}
      void operator()(A0 a0, A1 a1, A2 a2,A3 a3) {if (f) f(a0,a1,a2,a3);}
    };
    
#define DEFFN(f,...) RavelFn<__VA_ARGS__> f(#f,ravelLib.lib);
    
    DEFFN(ravel_lastErr, const char*);
    DEFFN(ravel_version, const char*);
    DEFFN(ravel_new, Ravel::RavelImpl*,size_t);
    DEFFN(ravel_delete, void, Ravel::RavelImpl*);
    DEFFN(ravel_clear, void, Ravel::RavelImpl*);
    DEFFN(ravel_render, void, Ravel::RavelImpl*, CAPIRenderer*);
    DEFFN(ravel_onMouseDown, void, Ravel::RavelImpl*, double, double);
    DEFFN(ravel_onMouseUp,void, Ravel::RavelImpl*, double, double);
    DEFFN(ravel_onMouseMotion, bool, Ravel::RavelImpl*, double, double);
    DEFFN(ravel_onMouseOver, bool, Ravel::RavelImpl*, double, double);
    DEFFN(ravel_onMouseLeave, void, Ravel::RavelImpl*);
    DEFFN(ravel_rescale, void, Ravel::RavelImpl*, double);
    DEFFN(ravel_radius, double, Ravel::RavelImpl*);
    DEFFN(ravel_rank, size_t, Ravel::RavelImpl*);
    DEFFN(ravel_description, const char*, Ravel::RavelImpl*);
    DEFFN(ravel_explain, const char*, Ravel::RavelImpl*, double, double);
    DEFFN(ravel_outputHandleIds, void, Ravel::RavelImpl*, size_t*);
    DEFFN(ravel_setOutputHandleIds, void, Ravel::RavelImpl*, size_t, size_t*);
    DEFFN(ravel_addHandle, void, Ravel::RavelImpl*, const char*, size_t, const char**);
    DEFFN(ravel_numHandles, unsigned, Ravel::RavelImpl*);
    DEFFN(ravel_selectedHandle, int, Ravel::RavelImpl*);
    DEFFN(ravel_handleDescription, const char*, Ravel::RavelImpl*, int);
    DEFFN(ravel_setHandleDescription, void, Ravel::RavelImpl*, int, const char*);
    DEFFN(ravel_numSliceLabels, size_t, Ravel::RavelImpl*, size_t);
    DEFFN(ravel_sliceLabels, void, Ravel::RavelImpl*, size_t, const char**);
    DEFFN(ravel_displayFilterCaliper, void, Ravel::RavelImpl*, size_t, bool);
    DEFFN(ravel_setSlicer,void,Ravel::RavelImpl*,size_t,const char*);
    DEFFN(ravel_setCalipers,void,Ravel::RavelImpl*,size_t,const char*,const char*);
    DEFFN(ravel_orderLabels, void, Ravel::RavelImpl*, size_t,HandleSort);
    DEFFN(ravel_applyCustomPermutation, void, Ravel::RavelImpl*, size_t, size_t, const size_t*);
    DEFFN(ravel_currentPermutation, void, Ravel::RavelImpl*, size_t, size_t, size_t*);
    DEFFN(ravel_toXML, const char*, Ravel::RavelImpl*);
    DEFFN(ravel_fromXML, int, Ravel::RavelImpl*, const char*);
    DEFFN(ravel_getHandleState, void, const Ravel::RavelImpl*, size_t, RavelHandleState*);
    DEFFN(ravel_setHandleState, void, Ravel::RavelImpl*, size_t, const RavelHandleState*);
    DEFFN(ravel_adjustSlicer, void, Ravel::RavelImpl*, int);
    DEFFN(ravel_redistributeHandles,void, Ravel::RavelImpl*);
    
    DEFFN(ravelDC_new, Ravel::Ravel::DataCube*);
    DEFFN(ravelDC_delete, void, Ravel::DataCube*);
    DEFFN(ravelDC_initRavel, bool, Ravel::DataCube*, Ravel::RavelImpl*);
    DEFFN(ravelDC_openFile, bool, Ravel::DataCube*, const char*, RavelDataSpec);
    DEFFN(ravelDC_loadData, void, Ravel::DataCube*, const Ravel::RavelImpl*, const double*);
    DEFFN(ravelDC_hyperSlice, int, Ravel::DataCube*, Ravel::RavelImpl*, size_t*, double**);

    inline double sqr(double x) {return x*x;} 
  }

  bool ravelAvailable() {return ravelLib.lib;}

  string Ravel::ravelVersion() const
  {return ravelLib.versionFound;}

  const char* Ravel::lastErr() const {
    if (ravelAvailable())
      return ravel_lastErr();
    return ravelLib.errorMsg.c_str();
  }
  
  Ravel::Ravel()
  {
    if (ravelAvailable())
      {
        ravel=ravel_new(0); // rank 1 for now
        ravel_rescale(ravel,ravelDefaultRadius);
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
    double  z=zoomFactor(), r=ravelDefaultRadius*z;
    if (ravel) r=1.1*z*ravel_radius(ravel);
    ports[0]->moveTo(x()+1.1*r, y());
    ports[1]->moveTo(x()-1.1*r, y());
    if (mouseFocus)
      {
        drawPorts(cairo);
        displayTooltip(cairo,tooltip.empty()? explanation: tooltip);
      }
    if (onResizeHandles) drawResizeHandles(cairo);
    cairo_rectangle(cairo,-r,-r,2*r,2*r);
    cairo_rectangle(cairo,-1.1*r,-1.1*r,2.2*r,2.2*r);
    cairo_stroke_preserve(cairo);
    if (onBorder || lockGroup)
      { // shadow the border when mouse is over it
        cairo::CairoSave cs(cairo);
        cairo::Colour c{1,1,1,0};
        if (lockGroup)
          c=palette[ lockGroup->colour() % paletteSz ];
        c.r*=0.5; c.g*=0.5; c.b*=0.5;
        c.a=onBorder? 0.5:0.3;
        cairo_set_source_rgba(cairo,c.r,c.g,c.b,c.a);
        cairo_set_fill_rule(cairo,CAIRO_FILL_RULE_EVEN_ODD);
        cairo_fill_preserve(cairo);
      }
    
    cairo_clip(cairo);

    {
      cairo::CairoSave cs(cairo);
      cairo_rectangle(cairo,-r,-r,2*r,2*r);
      cairo_clip(cairo);
      cairo_scale(cairo,z,z);
      ravel::CairoRenderer cr(cairo);
      if (ravel)
        ravel_render(ravel,&cr);
    }        
    if (selected) drawSelected(cairo);
  }

  void Ravel::resize(const LassoBox& b)
  {
    if (ravel)
      ravel_rescale(ravel, 0.5*std::max(fabs(b.x0-b.x1),fabs(b.y0-b.y1))/(1.21*zoomFactor()));
    moveTo(0.5*(b.x0+b.x1), 0.5*(b.y0+b.y1));
    bb.update(*this);
  }

  double Ravel::radius() const {
    return ravel? ravel_radius(ravel): 0;
  }
  
  ClickType::Type Ravel::clickType(float xx, float yy)
  {
    double z=zoomFactor();
    for (auto& p: ports)
      if (hypot(xx-p->x(), yy-p->y()) < portRadius*z)
        return ClickType::onPort;
    double r=1.1*z*(ravel? ravel_radius(ravel): ravelDefaultRadius);
    double R=1.1*r;
    double dx=xx-x(), dy=yy-y();
    // check if (x,y) is within portradius of the 4 corners
    if (fabs(fabs(dx)-R) < portRadius*z &&
        fabs(fabs(dy)-R) < portRadius*z &&
        fabs(hypot(dx,dy)-std::sqrt(2)*R) < portRadius*z)
      return ClickType::onResize;
    else if (std::abs(xx-x())>R || std::abs(yy-y())>R)
      return ClickType::outside;    
    else if (std::abs(dx)<=r && std::abs(dy)<=r)
      return ClickType::onRavel;
    else
      return ClickType::onItem;
  }

  int Ravel::selectedHandle() const {
    if (ravel)
      return ravel_selectedHandle(ravel);
    else
      return -1;
  }


  void Ravel::onMouseDown(float xx, float yy)
  {
    double invZ=1/zoomFactor();
    if (ravel) ravel_onMouseDown(ravel,(xx-x())*invZ,(yy-y())*invZ);
  }
  
  void Ravel::onMouseUp(float xx, float yy)
  {
    if (ravel)
      {
        double invZ=1/zoomFactor();
        ravel_onMouseUp(ravel,(xx-x())*invZ,(yy-y())*invZ);
      }
  }
  bool Ravel::onMouseMotion(float xx, float yy)
  {
    double invZ=1/zoomFactor();
    if (ravel) return ravel_onMouseMotion(ravel,(xx-x())*invZ,(yy-y())*invZ);
    return false;
  }
  
  bool Ravel::onMouseOver(float xx, float yy)
  {
    double invZ=1/zoomFactor();
    if (ravel) return ravel_onMouseOver(ravel,(xx-x())*invZ,(yy-y())*invZ);
    return false;
  }
  void Ravel::onMouseLeave()
  {if (ravel) ravel_onMouseLeave(ravel);}

  void Ravel::loadFile(const string& fileName)
  {
    m_filename=fileName;
    if (dataCube && ravel)
      {
        if (!ravelDC_openFile(dataCube, fileName.c_str(), RavelDataSpec()))
          throw error(ravel_lastErr());
        else if (!ravelDC_initRavel(dataCube,ravel))
          throw error(ravel_lastErr());
        for (size_t i=0; i<ravel_numHandles(ravel); ++i)
          {
            ravel_displayFilterCaliper(ravel,i,false);
            ravel_orderLabels(ravel,i,HandleState::forward);
          }
        setRank(ravel_numHandles(ravel));
      }
  }

  void Ravel::loadDataFromSlice(ITensorVal& v) const
  {
    if (ravel && dataCube)
      {
        vector<size_t> dims(ravel_rank(ravel));
        double* tmp=nullptr;
        ravelDC_hyperSlice(dataCube, ravel, &dims[0], &tmp);
        if (dims.empty() || dims[0]==0)
          {
            v.hypercube({});
            if (dims.empty() && tmp) v[0]=tmp[0];
            return; // do nothing if ravel data is empty
          }
        if (tmp)
          {
            vector<size_t> outHandles(dims.size());
            ravel_outputHandleIds(ravel, &outHandles[0]);
            Hypercube hc;
            auto& xv=hc.xvectors;
            for (size_t j=0; j<outHandles.size(); ++j)
              {
                auto h=outHandles[j];
                vector<const char*> labels(ravel_numSliceLabels(ravel,h));
                assert(ravel_numSliceLabels(ravel,h)==dims[j]);
                ravel_sliceLabels(ravel,h,&labels[0]);
                assert(all_of(labels.begin(), labels.end(),
                              [](const char* i){return bool(i);}));
                xv.emplace_back
                  (ravel_handleDescription(ravel,h));
                auto dim=axisDimensions.find(xv.back().name);
                if (dim!=axisDimensions.end())
                  xv.back().dimension=dim->second;
                else
                  {
                    auto dim=cminsky().dimensions.find(xv.back().name);
                    if (dim!=cminsky().dimensions.end())
                      xv.back().dimension=dim->second;
                  }
                // else otherwise dimension is a string (default type)
                for (size_t i=0; i<labels.size(); ++i)
                  xv.back().push_back(labels[i]);
              }
            v.hypercube(move(hc));
            assert(vector<unsigned>(dims.begin(), dims.end())==v.hypercube().dims());

            for (size_t i=0; i< v.size(); ++i)
              *(v.begin()+i)=tmp[i];
          }
        else
          throw error(ravel_lastErr());
      }
    v.hypercube({}); // ensure scalar data space allocated
  }

  void Ravel::loadDataCubeFromVariable(const ITensor& v)
  {
    if (ravel && dataCube)
      {
        // this ensure that handles are restored correctly after loading a .mky file. 
        RavelState state=initState.empty()? getState(): initState;
        initState.clear();
        ravel_clear(ravel);
        for (auto& i: v.hypercube().xvectors)
          {
            vector<string> ss;
            for (auto& j: i) ss.push_back(str(j));
            // clear the format if time so that data will reload correctly
            if (i.dimension.type==Dimension::time)
              axisDimensions[i.name]=Dimension(Dimension::time,"");
            vector<const char*> sl;
            for (auto& j: ss)
              sl.push_back(j.c_str());
            ravel_addHandle(ravel, i.name.c_str(), i.size(), &sl[0]);
            size_t h=ravel_numHandles(ravel)-1;
            ravel_displayFilterCaliper(ravel,h,false);
            // set forward sort order
            ravel_orderLabels(ravel,h,HandleState::forward);
          }
        if (state.empty())
          setRank(v.hypercube().rank());
#ifndef NDEBUG
        if (state.empty())
          {
            auto d=v.hypercube().dims();
            assert(d.size()==ravel_rank(ravel));
            vector<size_t> outputHandles(d.size());
            ravel_outputHandleIds(ravel,&outputHandles[0]);
            for (size_t i=0; i<d.size(); ++i)
              assert(d[i]==ravel_numSliceLabels(ravel,outputHandles[i]));
          }
#endif
        vector<double> tmp(v.size());
        for (size_t i=0; i<v.size(); ++i) tmp[i]=v[i];
        ravelDC_loadData(dataCube, ravel, &tmp[0]);
        applyState(state);
      }
  }

  unsigned Ravel::maxRank() const
  {
    if (ravel) return ravel_numHandles(ravel);
    return 0;
  }
  unsigned Ravel::rank() const
  {
    if (ravel) return ravel_rank(ravel);
    return 0;
  }
  
  void Ravel::setRank(unsigned rank)
  {
    if (ravel)
      {
        vector<size_t> ids;
        for (size_t i=0; i<rank; ++i) ids.push_back(i);
        ravel_setOutputHandleIds(ravel,rank,&ids[0]);
      }
  }
  
  void Ravel::adjustSlicer(int n)
  {
    ravel_adjustSlicer(ravel,n);
    broadcastStateToLockGroup();
  }

  bool Ravel::handleArrows(int dir, bool modifier)
  {
    adjustSlicer(dir);
    if (modifier)
      minsky().reset();
    return true;
  }

  
  bool Ravel::displayFilterCaliper() const
  {
    int h=ravel_selectedHandle(ravel);
    if (h>=0)
      {
        RavelHandleState state;
        ravel_getHandleState(ravel,h,&state);
        return state.displayFilterCaliper;
      }
    else
      return false;
  }
    
  bool Ravel::setDisplayFilterCaliper(bool x)
  {
    int h=ravel_selectedHandle(ravel);
    if (h>=0)
      ravel_displayFilterCaliper(ravel,h,x);
    return x;
  }

  vector<string> Ravel::allSliceLabels() const
  {
      return allSliceLabelsImpl(ravel_selectedHandle(ravel),HandleState::forward);
  }

  vector<string> Ravel::allSliceLabelsImpl(int axis, HandleSort order) const
  {
    if (axis>=0 && axis<int(ravel_numHandles(ravel)))
      {
        assert(order!=HandleState::custom); //custom makes no sense here
        // grab the labels in sorted order, or forward order if a custom order is applied
        RavelHandleState state;
        ravel_getHandleState(ravel, axis, &state);
        // grab the ordering in case its custom
        vector<size_t> customOrdering(ravel_numSliceLabels(ravel,axis));
        ravel_currentPermutation(ravel,axis,customOrdering.size(),&customOrdering[0]);

        // set the order and reset calipers to full range
        auto modifiedState=state;
        modifiedState.order=order;
        modifiedState.sliceMin=0;
        modifiedState.sliceMax=~0UL;
        ravel_setHandleState(ravel,axis,&modifiedState);
        vector<const char*> sliceLabels(ravel_numSliceLabels(ravel,axis));
        ravel_sliceLabels(ravel,axis,&sliceLabels[0]);
        ravel_setHandleState(ravel,axis,&state); // return things to how they were
        if (state.order==HandleState::custom)
          ravel_applyCustomPermutation(ravel,axis,customOrdering.size(),&customOrdering[0]);
        return {sliceLabels.begin(), sliceLabels.end()};
      }
    return {};
  }

  vector<string> Ravel::pickedSliceLabels() const
  {
   int axis=ravel_selectedHandle(ravel);
   if (axis>=0)
     {
       vector<const char*> sliceLabels(ravel_numSliceLabels(ravel,axis));
       ravel_sliceLabels(ravel,axis,&sliceLabels[0]);
       return {sliceLabels.begin(), sliceLabels.end()};
     }
   return {};
  }

  void Ravel::pickSliceLabels(int axis, const vector<string>& pick) 
  {
    if (axis>=0 && axis<int(ravel_numHandles(ravel)))
      {
        // stash previous handle sort order
        RavelHandleState state;
        ravel_getHandleState(ravel,axis,&state);
        if (state.order!=HandleState::custom)
          previousOrder=state.order;

        size_t numSliceLabels=ravel_numSliceLabels(ravel,axis);
        if (pick.size()>=numSliceLabels)
          {
            // if all labels are selected, revert ordering to previous
            ravel_orderLabels(ravel,axis,previousOrder);
            return;
          }
        
        auto allLabels=allSliceLabelsImpl(axis, HandleState::none);
        map<string,size_t> idxMap; // map index positions
        for (size_t i=0; i<allLabels.size(); ++i)
          idxMap[allLabels[i]]=i;
        vector<size_t> customOrder;
        for (auto& i: pick)
          {
            auto j=idxMap.find(i);
            if (j!=idxMap.end())
              customOrder.push_back(j->second);
          }
        assert(!customOrder.empty());
        ravel_applyCustomPermutation(ravel,axis,customOrder.size(),&customOrder[0]);
      }
  }
  
  Ravel::HandleState::HandleSort Ravel::sortOrder() const
  {
    if (!ravel) return HandleState::none;
    int h=ravel_selectedHandle(ravel);
    if (h>=0)
      {
        RavelHandleState state;
        ravel_getHandleState(ravel,h,&state);
        return state.order;
      }
    else
      return HandleState::none;
  }
 
  Ravel::HandleState::HandleSort Ravel::setSortOrder(Ravel::HandleState::HandleSort x)
  {
    int h=ravel_selectedHandle(ravel);
    if (h>=0)
      ravel_orderLabels(ravel,h,x);
    return x;
  }

  string Ravel::description() const
  {
    return ravel_handleDescription(ravel,ravel_selectedHandle(ravel));
  }
  
  void Ravel::setDescription(const string& description)
  {
    ravel_setHandleDescription(ravel,ravel_selectedHandle(ravel),description.c_str());
  }

  Dimension::Type Ravel::dimensionType() const
  {
    auto descr=description();
    auto i=axisDimensions.find(descr);
    if (i!=axisDimensions.end())
      return i->second.type;
    else
      {
        auto i=cminsky().dimensions.find(descr);
        if (i!=cminsky().dimensions.end())
          return i->second.type;
        else
          return Dimension::string;
      }
  }
  
  std::string Ravel::dimensionUnitsFormat() const
  {
    auto descr=description();
    if (descr.empty()) return "";
    auto i=axisDimensions.find(descr);
    if (i!=axisDimensions.end())
      return i->second.units;
    i=cminsky().dimensions.find(descr);
    if (i!=cminsky().dimensions.end())
      return i->second.units;
    return "";
  }
      
  /// @throw if type does not match global dimension type
  void Ravel::setDimension(Dimension::Type type,const std::string& units)
  {
    auto descr=description();
    if (descr.empty()) return;
    auto i=cminsky().dimensions.find(descr);
    Dimension d{type,units};
    if (i!=cminsky().dimensions.end())
      {
        if (type!=i->second.type)
          throw error("type mismatch with global dimension");
      }
    else
      minsky().dimensions[descr]=d;
    axisDimensions[descr]=d;
  }

  
  RavelState Ravel::getState() const
  {
    RavelState state;
    if (ravel)
      {
        state.radius=ravel_radius(ravel);
        for (size_t i=0; i<ravel_numHandles(ravel); ++i)
          {
            RavelHandleState hs;
            ravel_getHandleState(ravel, i, &hs);
            auto& s=state.handleStates[ravel_handleDescription(ravel,i)];
            s=hs;
            vector<const char*> sliceLabels(ravel_numSliceLabels(ravel,i));
            if (!sliceLabels.empty())
              {
                ravel_sliceLabels(ravel,i,&sliceLabels[0]);
                s.minLabel=sliceLabels.front();
                s.maxLabel=sliceLabels.back();
                if (hs.sliceIndex<sliceLabels.size())
                  s.sliceLabel=sliceLabels[hs.sliceIndex];
                if (s.order==HandleState::custom)
                  {
                    s.customOrder={sliceLabels.begin(),sliceLabels.end()};
                  }
              }
          }
        vector<size_t> ids(ravel_rank(ravel));
        ravel_outputHandleIds(ravel,&ids[0]);
        for (size_t i=0; i<ids.size(); ++i)
          state.outputHandles.push_back(ravel_handleDescription(ravel,ids[i]));
      }
    return state;
  }

  /// apply the \a state to the Ravel, leaving data, slicelabels etc unchanged
  void Ravel::applyState(const RavelState& state)
  {
    if (ravel)
      {
        ravel_rescale(ravel,state.radius);

        vector<size_t> ids;
        for (auto& outName: state.outputHandles)
          {
            for (size_t handle=0; handle<ravel_numHandles(ravel); ++handle)
              if (ravel_handleDescription(ravel,handle)==outName)
                ids.push_back(handle);
          }
        ravel_setOutputHandleIds(ravel,ids.size(),&ids[0]);

        for (size_t i=0; i<ravel_numHandles(ravel); ++i)
          {
            string name=ravel_handleDescription(ravel,i);
            auto hs=state.handleStates.find(name);
            if (hs!=state.handleStates.end())
              {
                RavelHandleState state(hs->second);
                ravel_setHandleState(ravel,i,&state);
                if (hs->second.order==HandleState::custom)
                  pickSliceLabels(i,hs->second.customOrder);
                else if (state.displayFilterCaliper)
                  ravel_setCalipers(ravel,i,hs->second.minLabel.c_str(),
                                    hs->second.maxLabel.c_str());
                ravel_setSlicer(ravel,i,hs->second.sliceLabel.c_str());
              }
          }
        ravel_redistributeHandles(ravel);
      }
  }

  void Ravel::exportAsCSV(const string& filename) const
  {
    // TODO: add some comment lines
    VariableValue v(VariableType::flow);
    loadDataFromSlice(v);
    v.exportAsCSV(filename, m_filename+": "+ravel_description(ravel));
  }

  Units Ravel::units(bool check) const
  {
    Units inputUnits=ports[1]->units(check);
    if (inputUnits.empty() || !ravel) return inputUnits;
    size_t multiplier=1;
    // at this stage, gross up exponents by the handle size of each
    // reduced by product handles
    for (size_t h=0; h<ravel_numHandles(ravel); ++h)
      {
        RavelHandleState state;
        ravel_getHandleState(ravel,h,&state);
        if (state.collapsed && state.reductionOp==HandleState::prod)
          multiplier*=ravel_numSliceLabels(ravel,h);
      }
    if (multiplier>1)
      for (auto& u: inputUnits)
        u.second*=multiplier;
    return inputUnits;
  }
  
  void Ravel::displayDelayedTooltip(float xx, float yy)
  {
    if (ravel_rank(ravel)==0)
      explanation="load CSV data from\ncontext menu";
    else
      {
        explanation=ravel_explain(ravel,xx-x(),yy-y());
        // line break every 5 words
        int spCnt=0;
        for (auto& c: explanation)
          if (isspace(c) && ++spCnt % 5 == 0)
            c='\n';
      }
  }
    
  string Minsky::ravelVersion() const
  {
    return ravelLib.versionFound+
      ((ravelLib.lib || ravelLib.versionFound=="unavailable")?
        "":" but incompatible");
  }

  void Ravel::leaveLockGroup()
  {
    if (lockGroup)
      lockGroup->removeFromGroup(*this);
    lockGroup.reset();
  }

  void Ravel::broadcastStateToLockGroup() const
  {
    if (lockGroup)
      {
        auto state=getState();
        for (auto& rr: lockGroup->ravels)
          if (auto r=rr.lock())
            if (r.get()!=this)
              r->applyState(state/*,true*/);
      }
  }
 
  void RavelLockGroup::removeFromGroup(const Ravel& ravel)
  {
    vector<weak_ptr<Ravel>> newRavelList;
    for (auto& i: ravels)
      {
        auto r=i.lock();
        if (r && r.get()!=&ravel)
          newRavelList.push_back(move(i));
      }
    ravels.swap(newRavelList);
    if (ravels.size()==1)
      if (auto r=ravels[0].lock())
        r->lockGroup.reset(); // this may delete this, so should be last
  }
 
}

  
