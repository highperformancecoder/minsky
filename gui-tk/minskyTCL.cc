/*
  @copyright Steve Keen 2013
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

#include "cairoItems.h"
#include "minskyTCL.h"
#include "minskyTCLObj.h"
#include <ecolab.h>
#include <ecolab_epilogue.h>

#include <unistd.h>

#ifdef _WIN32
#undef Realloc
#include <windows.h>
#endif

namespace minsky
{
  namespace
  {
    Minsky* l_minsky=NULL;
  }

  Minsky& minsky()
  {
    static MinskyTCL s_minsky;
    if (l_minsky)
      return *l_minsky;
    else
      return s_minsky;
  }

  LocalMinsky::LocalMinsky(Minsky& minsky) {l_minsky=&minsky;}
  LocalMinsky::~LocalMinsky() {l_minsky=NULL;}

  cmd_data* getCommandData(const string& name)
  {
    Tcl_CmdInfo info;
    if (Tcl_GetCommandInfo(interp(),name.c_str(),&info))
      {
        if (info.isNativeObjectProc)
          return (cmd_data*)info.objClientData;
        else
          return (cmd_data*)info.clientData;
      }
    return nullptr;
  }
  
#if 0
  // useful structure for for figuring what commands are being called.
  struct CmdHist: public map<string,unsigned>
  {
    ~CmdHist() {
      for (auto& i: *this)
        cout << i.first<<" "<<i.second<<endl;
    }
  } cmdHist;
#endif

  // Add any additional post TCL_obj processing commands here
  void setTCL_objAttributes()
  {
    // setting this helps a lot in avoiding unnecessary callbacks, and
    // also avoiding annoying "do you want to save the model message
    // when closing Minsky
    if (auto t=getCommandData("minsky.wire.coords"))
      t->is_setterGetter=true;
    if (auto t=getCommandData("minsky.var.name"))
      t->is_setterGetter=true;
    if (auto t=getCommandData("minsky.var.init"))
      t->is_setterGetter=true;
    if (auto t=getCommandData("minsky.var.value"))
      t->is_setterGetter=true;
    if (auto t=getCommandData("minsky.integral.description"))
      t->is_setterGetter=true;
    if (auto t=getCommandData("minsky.resetEdited"))
      t->is_const=true;
    if (auto t=getCommandData("minsky.initGroupList"))
      t->is_const=true;
    if (auto t=getCommandData("minsky.godley.mouseFocus"))
      t->is_const=true;
    if (auto t=getCommandData("minsky.godley.table.setDEmode"))
      t->is_const=true;
     if (auto t=getCommandData("minsky.resetNotNeeded"))
      t->is_const=true;
 }


  tclvar TCL_obj_lib("ecolab_library",ECOLAB_LIB);
  int TCL_obj_minsky=
    (
     TCL_obj_init(minsky()),
     ::TCL_obj(minskyTCL_obj(),"minsky",static_cast<MinskyTCL&>(minsky())),
     setTCL_objAttributes(),
     1
     );

    void MinskyTCL::putClipboard(const string& s) const
    {
#ifdef MAC_OSX_TK
      int p[2];
      pipe(p);
      if (fork()==0)
        {
          dup2(p[0],0);
          close(p[1]);
          execl("/usr/bin/pbcopy","pbcopy",nullptr);
        }
      else 
        {
          close(p[0]);
          write(p[1],s.c_str(),s.length());
          close(p[1]);
        }
      int status;
      wait(&status);
#elif defined(_WIN32)
      OpenClipboard(nullptr);
      EmptyClipboard();
      HGLOBAL h=GlobalAlloc(GMEM_MOVEABLE, s.length()+1);
      LPTSTR hh=static_cast<LPTSTR>(GlobalLock(h));
      if (hh)
        {
          strcpy(hh,s.c_str());
          GlobalUnlock(h);
          if (SetClipboardData(CF_TEXT, h)==nullptr)
            GlobalFree(h);
        }
      CloseClipboard();
#else
      Tk_Window mainWin=Tk_MainWindow(interp());
      Tk_ClipboardClear(interp(), mainWin);
      Atom utf8string=Tk_InternAtom(mainWin,"UTF8_STRING");
      Tk_ClipboardAppend(interp(), mainWin, utf8string, utf8string, 
                         const_cast<char*>(s.c_str()));
#endif
    }

    string MinskyTCL::getClipboard() const
    {
#ifdef MAC_OSX_TK
      int p[2];
      pipe(p);
      string r;
      if (fork()==0)
        {
          dup2(p[1],1);
          close(p[0]);
          execl("/usr/bin/pbpaste","pbpaste",nullptr);
        }
      else 
        {
          close(p[1]);
          char c;
          while (read(p[0],&c,1)>0)
            r+=c;
          close(p[0]);
        }
      int status;
      wait(&status);
      return r;
#elif defined(_WIN32)
      string r;
      OpenClipboard(nullptr);
      if (HANDLE h=GetClipboardData(CF_TEXT))
        r=static_cast<const char*>(h);
      CloseClipboard();
      return r;
#else
      return (tclcmd()<<"clipboard get -type UTF8_STRING\n").result;
#endif
    }

  void MinskyTCL::latex(const char* filename, bool wrapLaTeXLines) 
  {
    if (cycleCheck()) throw error("cyclic network detected");
    ofstream f(filename);

    f<<"\\documentclass{article}\n";
    if (wrapLaTeXLines)
      {
        f<<"\\usepackage{breqn}\n\\begin{document}\n";
        MathDAG::SystemOfEquations(*this).latexWrapped(f);
      }
    else
      {
        f<<"\\begin{document}\n";
          MathDAG::SystemOfEquations(*this).latex(f);
      }
    f<<"\\end{document}\n";
  }

  namespace
  {
    template <class T>
    struct IconBase: public ecolab::cairo::CairoImage, public T
    {
      template <class... U> 
      IconBase(const char* imageName, U... x): T(std::forward<U>(x)...)
      {
        Tk_PhotoHandle photo = Tk_FindPhoto(interp(), imageName);
        if (photo)
          cairoSurface.reset(new cairo::TkPhotoSurface(photo));        
      }
      void draw()
      {
        initMatrix();
        cairo_translate(cairoSurface->cairo(), 0.5*cairoSurface->width(), 0.5*cairoSurface->height());
        cairo_select_font_face(cairoSurface->cairo(), "sans-serif", 
                  CAIRO_FONT_SLANT_ITALIC,CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cairoSurface->cairo(),12);
        cairo_set_line_width(cairoSurface->cairo(),1);
        T::draw(cairoSurface->cairo());
        cairoSurface->blit();
      }
    };

    struct OperationIcon
    {
      OperationIcon(const char* opName):
        op(OperationType::Type(enumKey<OperationType::Type>(opName))) 
      {}
      OperationPtr op;
      void draw(cairo_t* cairo)
      {
        RenderOperation(*op, cairo).draw();
      }
    };
  }

  void MinskyTCL::operationIcon(const char* imageName, const char* opName) const
  {
    if (string(opName)=="switch")
      IconBase<SwitchIcon>(imageName).draw();
    else
      IconBase<OperationIcon>(imageName, opName).draw();
  }

  int TclExtend<std::shared_ptr<minsky::IntOp>>::getIntVar() 
  {
    auto& m=dynamic_cast<MinskyTCL&>(minsky());
    m.item.setRef(ref->intVar,"wiringGroup.item");
    for (auto& i: m.items)
      if (dynamic_cast<VariableBase*>(i.get())==ref->intVar.get())
        return i.id();
    return -1;
  }

  bool TclExtend<std::shared_ptr<minsky::IntOp>>::toggleCoupled()
  {
    auto& m=dynamic_cast<MinskyTCL&>(minsky());
    int wireId=-1; // stash id to wire if uncoupled
    if (!ref->coupled())
      {
        auto& wires=ref->ports[0]->wires;
        if (!wires.empty())
          for (auto& w: m.wires)
            if (find(wires.begin(), wires.end(), w.get())!=wires.end())
              {
                wireId=w.id();
                break;
              }
      }

    bool coupled=ref->toggleCoupled();
    for (auto& i: m.items)
      if (dynamic_cast<VariableBase*>(i.get())==ref->intVar.get())
        {
          if (coupled)
            tclcmd() | ".wiring.canvas delete item"|i.id()|"\n";
          else
            tclcmd() << "newItem"<< i.id()<<"\n";
        }

    if (wireId>=0)
      {
        tclcmd() | ".wiring.canvas delete wire"|wireId|"\n";
        m.wires.erase(wireId);
      }
    else
      {
        assert(ref->ports.size() && ref->ports[0]->wires.size());
        if (auto g=ref->group.lock())
          {
            int newWire=m.getNewId();
            m.wires[newWire]=g->findWire(*ref->ports[0]->wires[0]);
            tclcmd() << "newWire"<< newWire<<"\n";
          }
      }
    return ref->coupled();
  }

  void MinskyTCL::makeVariableConsistentWithValue(int id) 
  {
    clearAllGetterSetters();
    auto i=items.find(id);
    if (i!=items.end())
      if (auto v=dynamic_cast<VariableBase*>(i->get()))
        {
          auto& value=variableValues[v->valueId()];
          if (value.type()!=v->type())
            {
              VariablePtr v(*i);
              v.makeConsistentWithValue();
              // now need to fix both the TCL items entry and minsky's
              if (auto g=(*i)->group.lock())
                {
                  g->removeItem(**i);
                  g->addItem(v);
                  *i=v;
                }
            }
        }
  }


}
