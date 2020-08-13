/*
  @copyright Steve Keen 2016
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

/// Creation and access to the minskyTCL_obj object, which has code to
/// record whenever Minsky's state changes

// Lazy instantiated pattern to avoid the indeterminate initialisation
// ordering problem of C++ (See Stroustrup 4th edition §15.4.1)

#ifndef MINSKYTCLOBJ_H
#define MINSKYTCLOBJ_H
#include "minskyTCL.h"
#include <string>

namespace minsky
{
  inline string to_string(CONST84 char* x) {return x;}
  inline string to_string(Tcl_Obj* x) {return Tcl_GetString(x);}

  // a hook for recording when the minsky model's state changes
  template <class AV>
  void member_entry_hook(int argc, AV argv)
  {
    string argv0=to_string(argv[0]);
    MinskyTCL& m=static_cast<MinskyTCL&>(minsky());
    if (argv0.find("godleyWindow")==0)
      {
        if (argv0.find(".undo")!=std::string::npos) return; // do not record undo in history
        if (argv0.find(".keyRelease")!=std::string::npos) return; 
        // support undo/redo in a Godley edit window
        auto i=argv0.find('.');
        if (i==std::string::npos) i=argv0.length();
        auto t=dynamic_cast<member_entry_base*>(getCommandData(argv0.substr(0,i)));
        if (!t || (!t->is_const && (!t->is_setterGetter || argc>1)))
            if (auto gtw=t->memberPtrCasted<GodleyTableWindow>())
              gtw->pushHistory();
        return;
      }
    if (m.doPushHistory &&
        argv0!="minsky.availableOperations" &&
        argv0!="minsky.canvas.select" &&
        argv0!="minsky.canvas.recentre" &&
        argv0!="minsky.canvas.mouseDown" &&
        argv0!="minsky.canvas.focusFollowsMouse" &&
        argv0!="minsky.canvas.requestRedraw" &&
        (argv0!="minsky.canvas.mouseMove"
         || m.eventRecord.get()) && /* ensure we record mouse movements, but filter from history */
        argv0!="minsky.clearAll" &&
        argv0!="minsky.doPushHistory" &&
        argv0!="minsky.model.moveTo" &&
        argv0!="minsky.canvas.model.moveTo" &&
        argv0!="minsky.canvas.model.zoom" &&
        argv0!="minsky.model.zoom" &&
        argv0!="minsky.newGlobalGroupTCL" &&
        argv0!="minsky.popFlags" &&
        argv0!="minsky.pushFlags" &&
        argv0!="minsky.select" &&
        argv0!="minsky.selectVar" &&
        argv0!="minsky.setGodleyIconResource" &&
        argv0!="minsky.setGroupIconResource" &&
        argv0!="minsky.step" &&
        argv0!="minsky.running" &&
        argv0!="minsky.multipleEquities" &&
        argv0.find("minsky.panopticon")==string::npos &&
        argv0.find("minsky.equationDisplay")==string::npos && 
        argv0.find(".get")==string::npos && 
        argv0.find(".@elem")==string::npos && 
        argv0.find(".mouseFocus")==string::npos
        )
      {
        auto t=getCommandData(argv0);
        if (!t || (!t->is_const && (!t->is_setterGetter || argc>1)))
          {
            //            cmdHist[argv0]++;
            bool modelChanged=m.pushHistory();
            if (modelChanged && argv0!="minsky.load" && argv0!="minsky.reverse") m.markEdited();
            if (m.eventRecord.get() && argv0!="minsky.startRecording" &&
                (modelChanged || argv0.find("minsky.canvas.mouse")!=string::npos))
              {
                for (int i=0; i<argc; ++i)
                  (*m.eventRecord) << "{"<<to_string(argv[i]) <<"} ";
                (*m.eventRecord)<<endl;
              }
            if (modelChanged && m.autoSaveFile.get())
              try
                {
                  m.save(*m.autoSaveFile);
                  m.markEdited(); // undo edited flag reset
                }
              catch(...)
                { // unable to autosave
                  m.autoSaveFile.reset();
                  throw std::runtime_error("Unable to autosave to this location");
                }
          }
      }
    if (m.rebuildTCLcommands)
      {
        TCL_obj(minskyTCL_obj(), "minsky", m);
        m.rebuildTCLcommands=false;
      }
  }

  inline TCL_obj_t& minskyTCL_obj() 
  {
    static TCL_obj_t t;
    static int dum=(
                    t.member_entry_hook=member_entry_hook<CONST84 char**>,
                    t.member_entry_thook=member_entry_hook<Tcl_Obj* const *>,
                    1);
    return t;
  }

}

#endif
