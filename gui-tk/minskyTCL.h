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
#include "godleyTableWindow.h"
#include <fstream>

// TCL specific definitions for global minsky object

namespace minsky
{
  cmd_data* getCommandData(const string& name);

  template <class O>
  int deleteTclObject(ClientData, Tcl_Interp*, int, const char **);

  /// a TCL_obj_t that provides a hook for detecting model edits
  ecolab::TCL_obj_t& minskyTCL_obj();
  void setTCL_objAttributes();

  size_t physicalMem();
  
  struct MinskyTCL: public Minsky
  {
    bool rebuildTCLcommands=false;

    /// generate a TCL_obj referring to variableValues[valueId]
    void getValue(const std::string& valueId);
    
    std::unique_ptr<ostream> eventRecord;
    void startRecording(const char* file) {
      eventRecord.reset(new std::ofstream(file));
      *eventRecord<<"checkRecordingVersion "<<minskyVersion<<endl;
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

    std::set<string> matchingTableColumns(const std::string& currTable, GodleyAssetClass::AssetClass ac) {
      auto it=TCL_obj_properties().find(currTable);
      if (it!=TCL_obj_properties().end())
        if (auto g=it->second->memberPtrCasted<GodleyIcon>())
          return Minsky::matchingTableColumns(*g, ac);
      return std::set<string>();
    }

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

    bool getItemAtFocus()
    {
      // deregister any old definitions, as item is polymorphic
      TCL_obj_deregister("minsky.canvas.item");
      canvas.item=canvas.itemFocus;
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
    void addIntegral() {
      Minsky::addIntegral();
      registerRef(canvas.item,"minsky.canvas.item");
    }
    void addRavel() {
      canvas.addRavel();
      registerRef(canvas.item,"minsky.canvas.item");
    }
    void addSheet() {
      canvas.addSheet();
      registerRef(canvas.item,"minsky.canvas.item");
    }

    void createGroup() {
      canvas.groupSelection();
      registerRef(canvas.itemFocus,"minsky.canvas.itemFocus");
    }
    void convertVarType(const std::string& name, VariableType::Type type) {
      Minsky::convertVarType(name,type);
      registerRef(canvas.item,"minsky.canvas.item");
    }
    
    void groupOfItem() {
      if (canvas.item) {
          canvas.item=canvas.item->group.lock();
          registerRef(canvas.item,"minsky.canvas.item");
        }
    }

    void resetItem() {
      canvas.item.reset();
      registerRef(canvas.item,"minsky.canvas.item");
    }
    
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
           (Tcl_CmdProc*)deleteTclObject<Item>,
           (ClientData)(new ItemPtr(canvas.item)),NULL);
      }
      return canvas.item? name: "";
    }
  
    /// sets the colour of palette item i to Tk acceptable colour name
    void setColour(size_t i, const char* name);
    
    /// find first object of given \a type
    bool findObject(const std::string& type)
    {
      canvas.item.reset();
      if (type=="Group" && !canvas.model->groups.empty())
        canvas.item=canvas.model->groups.front();
      else
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
    
    bool findVariable(const std::string& name)
    {
      canvas.item.reset();
      model->recursiveDo
        (&GroupItems::items, [&](const Items&, Items::const_iterator i)
                         {
                           if (auto v=dynamic_cast<VariableBase*>(i->get()))
                             if (v->name()==name)
                               {
                                 canvas.item=*i;
                                 return true;
                               }
                           return false;
                         });
      registerRef(canvas.item,"minsky.canvas.item");
      return canvas.item.get();
    }
    
    /// reinitialises canvas to the group located in item
    void openGroupInCanvas() {
      canvas.openGroupInCanvas(canvas.item);
      TCL_obj(minskyTCL_obj(),"minsky.canvas.model", *canvas.model);
    }
    void openModelInCanvas() {
      canvas.openGroupInCanvas(model);
      TCL_obj(minskyTCL_obj(),"minsky.canvas.model", *canvas.model);
    }

    std::string openGodley() {
      if (auto gi=dynamic_pointer_cast<GodleyIcon>(canvas.item))
        {
          std::string name="godleyWindow"+to_string(size_t(canvas.item.get()));
          if (TCL_obj_properties().count(name)==0)
            {
              auto godley=new GodleyTableWindow(gi);
              // pass ownership of object to TCL interpreter
              Tcl_CreateCommand
                (ecolab::interp(), (name+".delete").c_str(),
                 (Tcl_CmdProc*)deleteTclObject<GodleyTableWindow>,
                 (ClientData)godley,NULL);
              TCL_obj(minskyTCL_obj(),name,*godley);
            }
          return name;
        }
      return "";
    }

    void loadVariableFromCSV(const std::string& specVar, const std::string& filename)
    {
      auto i=TCL_obj_properties().find(specVar);
      if (i!=TCL_obj_properties().end())
        if (auto spec=dynamic_cast<member_entry<DataSpec>*>(i->second.get()))
          if (auto v=dynamic_cast<VariableBase*>(canvas.item.get()))
            v->importFromCSV(filename, *spec->memberptr);
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

    vector<string> listFonts() const {
      PangoFontFamily **families;
      int num;
      pango_font_map_list_families(pango_cairo_font_map_get_default(),
                                   &families,&num);
      vector<string> r;
      for (int i=0; i<num; ++i)
        r.push_back(pango_font_family_get_name(families[i]));
      g_free(families);
      return r;
    }

    ecolab::Accessor<std::string> defaultFont{
      [this]() {return _defaultFont? _defaultFont.get(): "";},
      [this](const std::string& x) {
        _defaultFont.reset(new char[x.length()+1]);
        strcpy(_defaultFont.get(),x.c_str());
        ecolab::Pango::defaultFamily=_defaultFont.get();
        return x;
      }};

    void setBusyCursor() override
    {tclcmd()<<"setCursor watch\n";}
    void clearBusyCursor() override
    {tclcmd()<<"setCursor {}\n";}

    bool checkMemAllocation(size_t bytes) const override {
      bool r=true;
      if (bytes>0.2*physicalMem())
        {
          tclcmd cmd;
          cmd<<"tk_messageBox -message {Allocation will use more than 50% of available memory. Do you want to proceed?} -type yesno\n";
          r=cmd.result=="yes";
        }
      return r;
    }
    
    int numOpArgs(OperationType::Type o) const;
    OperationType::Group classifyOp(OperationType::Type o) const {return OperationType::classify(o);}
  private:
    std::unique_ptr<char[]> _defaultFont;

  };
}

#include "minskyTCL.cd"
#endif
