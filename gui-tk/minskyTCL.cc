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

#undef CLASSDESC_ARITIES
#define CLASSDESC_ARITIES 0x3F 
#include "cairoItems.h"
#include "minskyTCL.h"
#include "minskyTCLObj.h"
#include "callableFunction.h"
#include "userFunction.h"
#include "CSVDialog.h"
#include <ecolab.h>
#include "minsky_epilogue.h"
#ifdef _WIN32
#undef Realloc
#include <windows.h>
#include <tkPlatDecls.h>
#endif

#if defined(__linux__)
#include <sys/sysinfo.h>
#endif

#include <stdexcept>
using namespace std;

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
        return (cmd_data*)info.clientData;
      }
    return nullptr;
  }

  template <class T>
  int deleteTclObject(ClientData cd, Tcl_Interp *interp,
                    int argc, const char **argv)
  {
    assert( strcmp(argv[0]+strlen(argv[0])-strlen(".delete"),
                   ".delete")==0);
    std::string s(argv[0]);
    ecolab::TCL_obj_deregister(s.substr(0,s.length()-strlen(".delete")));
    delete (T*)cd;
    return TCL_OK; 
  }

  template int deleteTclObject<Item>(ClientData cd, Tcl_Interp *interp,
                                   int argc, const char **argv);
  template int deleteTclObject<GodleyTableWindow>(ClientData cd, Tcl_Interp *interp,
                                   int argc, const char **argv);

  
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

  void MinskyTCL::getValue(const std::string& valueId)
  {
    auto value=variableValues.find(valueId);
    if (value!=variableValues.end())
      TCL_obj(minskyTCL_obj(),"minsky.value",value->second);
    else
      TCL_obj_deregister("minsky.value");
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
        op->draw(cairo);
      }
    };
  }

  void MinskyTCL::operationIcon(const char* imageName, const char* opName) const
  {
    if (string(opName)=="switch")
      IconBase<SwitchIcon>(imageName).draw();
    else if (string(opName)=="userFunction")
      {
        IconBase<UserFunction> uf(imageName);
        uf.description("f(x)");
        uf.draw();
      }
    else
      IconBase<OperationIcon>(imageName, opName).draw();
  }

  int MinskyTCL::numOpArgs(OperationType::Type o)
  {
    OperationPtr op(o);
    return op->numPorts()-1;
  }

  TCLTYPE(CSVDialog);
  
  namespace
  {
    // type independent scaling to [0..1]
    template <class T> double scale(T x) {
      return double(x)/numeric_limits<T>::max();
    }
  }
  
  void MinskyTCL::setColour(size_t i, const char* name)
  {
    if (auto p=canvas.item->plotWidgetCast())
      if (i<p->palette.size())
        if (auto c=Tk_GetColor(interp(),Tk_MainWindow(interp()),name))
          {
            auto& pi=p->palette[i];
            pi.colour.r=scale(c->red);
            pi.colour.g=scale(c->green);
            pi.colour.b=scale(c->blue);
            pi.colour.a=1;
          }
        else
          throw runtime_error(string("Colour ")+name+" not understood");
      else
        throw runtime_error("invalid palette index");
    else
      throw runtime_error("Not a plot");
  }
}
