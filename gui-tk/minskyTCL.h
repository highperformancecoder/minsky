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

#ifndef MINSKYTCL_H
#define MINSKYTCL_H
#include "minsky.h"
#include "groupTCL.h"
#include <fstream>

// TCL specific definitions for global minsky object

namespace minsky
{
  cmd_data* getCommandData(const string& name);

  struct MinskyTCL: public GroupTCL<Minsky>
  {
    bool rebuildTCLcommands=false;
    
    GetterSetter<VariableValues> value;
    MinskyTCL(): value(variableValues) {}
    
    void clearAllGetterSetters() {
      GroupTCL::clearAllGetterSetters();
      value.clear();
    }
    
    void clearAll() {
      GroupTCL::clearAll();
      clearAllMaps();
      value.clear();
    }
    
    
    std::unique_ptr<ostream> eventRecord;
    void startRecording(const char* file) {
      eventRecord.reset(new std::ofstream(file));
      resetNextId(); //ensures consistent IDs are allocated
      if (nextId==0)
        // empty system
        (*eventRecord)<<"newSystem\n";
      else
        // save current system state to a mkytmp file
        {
          string savedState=file;
          auto ext=savedState.rfind('.');
          if (ext!=string::npos)
            savedState.erase(ext);
          savedState+=".mkytmp";
          save(savedState);
          tclcmd() << "newSystem\n";
          load(savedState); // ensures ids are same as from a fresh load
          resetNextId();
          (*eventRecord)<<"newSystem\n";
          (*eventRecord)<<"minsky.load {"<<savedState<<"}\n";
          (*eventRecord)<<"minsky.resetNextId\n";
          (*eventRecord)<<"recentreCanvas\n";
        }
    }
    void stopRecording() {
      eventRecord.reset();
    }

    /// flag to indicate whether a TCL should be pushed onto the
    /// history stack, or logged in a recording. This is used to avoid
    /// movements being added to recordings and undo history
    bool doPushHistory=true;

    

    /// fill in a Tk image with the icon for a specific operation
    /// @param Tk imageName
    /// @param operationName
    void operationIcon(const char* imageName, const char* opName) const;

    void displayErrorItem(float x, float y) const override
    {
      tclcmd() << "catch {indicateCanvasItemInError"<<x<<y<<"}\n";
      Tcl_ResetResult(interp());
    }
    
    void cut()
    {
      // need to clear getters/setters before calling Minsky::cut(),
      // as well as minskTCL's maps to remove extraneous references
      clearAllGetterSetters();
      items.clear();
      wires.clear();
      Minsky::cut();
      buildMaps();
    }

    int paste()
    {
      auto g=Minsky::paste();
      buildMaps();
      // find the newly added group
      for (auto& i: items)
        if (i==g) return i.id();
      return -1;
    }
      
    void putClipboard(const std::string& s) const override; 
    std::string getClipboard() const override; 

    std::set<string> matchingTableColumns(int currTable, GodleyAssetClass::AssetClass ac) {
      auto it=items.find(currTable);
      if (it!=items.end())
        if (auto g=dynamic_cast<GodleyIcon*>(it->get()))
          return Minsky::matchingTableColumns(g->table, ac);
      return std::set<string>();
    }


//   void inGroupSelect(int gid, float x0, float y0, float x1, float y1)
//    {
//      clearSelection();
//      if (groupItems.count(gid))
//        groupItems[gid].select(currentSelection,x0,y0,x1,y1);
//    }
//
    void initGroupList(TCL_args args) {
      //      groupTest.initGroupList(groupItems, (args.count? args: -1));
    }

    /// @returns true if (x,y) is over an I/O variable within a group id or a variable within at (x,y), false otherwise. var is set to the variable if it exists
    bool selectVar(int id, float x, float y);

    int insertGroupFromFile(const char* file) {
      int r=-1;
      if (auto g=Minsky::insertGroupFromFile(file))
        items[r=getNewId()]=g;
      return r;
    }


    /// load from a file
    void load(const std::string& filename) {
      clearAll();
      Minsky::load(filename);
      // load resets the model pointer, etc, so request rebuilding of
      // TCL commands
      rebuildTCLcommands=true;
    }
    void exportSchema(TCL_args args) {
      const char* filename=args;
      int schemaLevel=1;
      if (args.count) schemaLevel=args;
      Minsky::exportSchema(filename, schemaLevel);
    }
    void latex(const char* filename, bool wrapLaTeXLines);

    void matlab(const char* filename) {
      if (cycleCheck()) throw error("cyclic network detected");
      ofstream f(filename);
      MathDAG::SystemOfEquations(*this).matlab(f);
    }

    // for testing purposes
    string latex2pango(const char* x) {return latexToPango(x);}

    /// restore model to state \a changes ago 
    void undo(TCL_args args) {
      clearAllGetterSetters();
      if (args.count) Minsky::undo(args);
      else Minsky::undo();
    }

    string valueId(const string& x) {return VariableValue::valueId(x);}

    /// make variable\a id's type consistent with its value
    /// as a side effect, sets the var reference to the new variable
    void makeVariableConsistentWithValue(int id);

  };
}

#include "minskyTCL.cd"
#endif
