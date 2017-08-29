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
#include <fstream>

// TCL specific definitions for global minsky object

namespace minsky
{
  cmd_data* getCommandData(const string& name);

  int deleteTclItem(ClientData, Tcl_Interp*, int, const char **);

  /// a TCL_obj_t that provides a hook for detecting model edits
  ecolab::TCL_obj_t& minskyTCL_obj();
  void setTCL_objAttributes();

  struct MinskyTCL: public Minsky
  {
    bool rebuildTCLcommands=false;

    /// generate a TCL_obj referring to variableValues[valueId]
    void getValue(const std::string& valueId);
    
    std::unique_ptr<ostream> eventRecord;
    void startRecording(const char* file) {
      eventRecord.reset(new std::ofstream(file));
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

//    void displayErrorItem(float x, float y) const override
//    {
//      tclcmd() << "catch {indicateCanvasItemInError"<<x<<y<<"}\n";
//      Tcl_ResetResult(interp());
//    }
    
    void putClipboard(const std::string& s) const override; 
    std::string getClipboard() const override; 

//    std::set<string> matchingTableColumns(int currTable, GodleyAssetClass::AssetClass ac) {
//      auto it=items.find(currTable);
//      if (it!=items.end())
//        if (auto g=dynamic_cast<GodleyIcon*>(it->get()))
//          return Minsky::matchingTableColumns(g->table, ac);
//      return std::set<string>();
//    }

    /// register item as a TCL obj under name if it exists, otherwise deregister it
    void registerRef(ItemPtr& item, const std::string& name)
    {
      if (item)
        item->TCL_obj(minskyTCL_obj(),name);
      else
        TCL_obj_deregister(name);
    }
    
    bool getItemAt(float x, float y)
    {
      // deregister any old definitions, as item is polymorphic
      TCL_obj_deregister("minsky.canvas.item");
      canvas.getItemAt(x,y);
      registerRef(canvas.item,"minsky.canvas.item");
      return canvas.item.get();
    }

    bool getWireAt(float x, float y)
    {
      canvas.getWireAt(x,y);
      if (canvas.wire)
        TCL_obj(minskyTCL_obj(),"minsky.canvas.wire",*canvas.wire);
      else
        TCL_obj_deregister("minsky.canvas.wire");
      return canvas.wire.get();
    }

    void addVariable(const std::string& name, VariableType::Type type) {
      canvas.addVariable(name,type);
      registerRef(canvas.itemFocus,"minsky.canvas.itemFocus");
    }
    void addOperation(OperationType::Type op) {
      canvas.addOperation(op);
      registerRef(canvas.itemFocus,"minsky.canvas.itemFocus");
    }
    void addNote(const std::string& text) {
      canvas.addNote(text);
      registerRef(canvas.itemFocus,"minsky.canvas.itemFocus");
    }
    void addPlot() {
      canvas.addPlot();
      registerRef(canvas.itemFocus,"minsky.canvas.itemFocus");
    }
    void addGodley() {
      canvas.addGodley();
      registerRef(canvas.itemFocus,"minsky.canvas.itemFocus");
    }
    void addGroup() {
      canvas.addGroup();
      registerRef(canvas.itemFocus,"minsky.canvas.itemFocus");
    }
    void addSwitch() {
      canvas.addSwitch();
      registerRef(canvas.itemFocus,"minsky.canvas.itemFocus");
    }
    void createGroup() {Minsky::createGroup();}

    bool selectVar(float x, float y) {
      bool r=canvas.selectVar(x,y);
      if (r)
        registerRef(canvas.item,"minsky.canvas.item");
      return r;
    }
    
    /// create a TCL controlled object that is a reference to item
    /// @return a unique TCL command lead in sequence, or empty if no
    /// such object is created
    std::string TCLItem() {
      std::string name="item"+std::to_string(size_t(canvas.item.get()));
      if (canvas.item && !TCL_obj_properties().count(name)) {
        canvas.item->TCL_obj(minskyTCL_obj(),name);
        // create a reference to manage object's lifetime
        Tcl_CreateCommand
          (ecolab::interp(), (name+".delete").c_str(),
           (Tcl_CmdProc*)deleteTclItem,
           (ClientData)(new ItemPtr(canvas.item)),NULL);
      }
      return canvas.item? name: "";
    }

    /// find first object of given \a type
    bool findObject(const std::string& type)
    {
      model->recursiveDo
        (&GroupItems::items, [&](const Items&, Items::const_iterator i)
                         {
                           if ((*i)->classType()==type)
                             {
                               canvas.item=*i;
                               return true;
                             }
                           return false;
                         });
      registerRef(canvas.item,"minsky.canvas.item");
      return canvas.item.get();
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

    /// load from a file
    void load(const std::string& filename) {
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
      if (args.count) Minsky::undo(args);
      else Minsky::undo();
    }

    string valueId(const string& x) {return VariableValue::valueId(x);}

    struct TkPhotoCanvas: public cairo::TkPhotoSurface
    {
      Canvas& canvas;
      TkPhotoCanvas(const char* photo, Canvas& canvas):
        cairo::TkPhotoSurface(Tk_FindPhoto(interp(),photo)), canvas(canvas) {}
      void requestRedraw() override {
        clear();
        canvas.redraw();
        blit();
      }
    };
    
    void addPhotoSurfaceToCanvas(const char* photo) 
    {canvas.surface.reset(new TkPhotoCanvas(photo, canvas));}

    bool hasSurface() const {return canvas.surface.get();}
  };
}

#include "minskyTCL.cd"
#endif
