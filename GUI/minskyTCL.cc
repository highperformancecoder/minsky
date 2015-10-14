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

#include "minskyTCL.h"
#include "cairoItems.h"
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

  string to_string(CONST84 char* x) {return x;}
  string to_string(Tcl_Obj* x) {
    return Tcl_GetString(x);
  }

  // a hook for recording when the minsky model's state changes
  template <class AV>
  void member_entry_hook(int argc, AV argv)
  {
    string argv0=to_string(argv[0]);
    MinskyTCL& m=static_cast<MinskyTCL&>(minsky());
    if (m.doPushHistory && argv0!="minsky.doPushHistory" && 
        argv0.find(".get")==string::npos  && 
        m.pushHistoryIfDifferent())
      {
        if (argv0!="minsky.load") m.markEdited();
        if (m.eventRecord.get() && argv0=="minsky.startRecording")
          {
            for (int i=0; i<argc; ++i)
              (*m.eventRecord) << "{"<<to_string(argv[i]) <<"} ";
            (*m.eventRecord)<<endl;
          }
      }
  }

  

  TCL_obj_t& minskyTCL_obj() 
  {
    static TCL_obj_t t;
    static int dum=(
                    t.member_entry_hook=member_entry_hook<CONST84 char**>,
                    t.member_entry_thook=member_entry_hook<Tcl_Obj* const *>,
                    1);
    return t;
  }

  tclvar TCL_obj_lib("ecolab_library",ECOLAB_LIB);
  int TCL_obj_minsky=
    (
     TCL_obj_init(minsky()),
     ::TCL_obj(minskyTCL_obj(),"minsky",static_cast<MinskyTCL&>(minsky())),
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


}
