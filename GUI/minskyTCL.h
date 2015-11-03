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
#include "TCL_extend.h"
#include <fstream>

// TCL specific definitions for global minsky object

namespace minsky
{
  /// a TCL_obj_t that provides a hook for detecting model edits
  ecolab::TCL_obj_t& minskyTCL_obj();


  /**
     convenience class for accessing elements of a map from TCL
  */
  template <class M>
  class GetterSetter
  {
    M& map;
    typedef typename M::key_type K;
    typedef typename M::mapped_type T;
    std::unique_ptr<TclExtend<T> > ref;
  public:
    K key; ///<last key gotten
    void get(TCL_args args) {
      string cmdPrefix=(char*)args[-1];
      cmdPrefix.erase(cmdPrefix.rfind(".get"));
      TCL_args tmp(args);
      tmp>>key;
      if (map.count(key)) {
        ref.reset(new TclExtend<T>(map[key]));
        TCL_obj(minskyTCL_obj(), cmdPrefix, *ref);
      }
      else
        throw error("%s not found",(char*)args[0]);
      tclcmd() << "scopedDisableEventProcessing\n";
    }
    void set(TCL_args args) {
    }
    GetterSetter(M& m): map(m) {}
    // assignment is do nothing, as reference member is created as part
    // of constructor
    void operator=(const GetterSetter&) {}
    void clear() {}
  };

  template <class M, class V=typename M::mapped_type::element_type>
  class GetterSetterPtr
  {
    typedef typename M::key_type K;
    typedef typename M::mapped_type T;
    M& map;
    std::unique_ptr<TclExtend<std::shared_ptr<V> > > ref;
    std::shared_ptr<V> val;
    string cmdPrefix;
  public:
    // nb, in spite of appearances, this approach does not work well
    // with non-shared_pointer value types
    void get(TCL_args args) {
      cmdPrefix=(char*)args[-1];
      cmdPrefix.erase(cmdPrefix.rfind(".get"));
      K key;
      TCL_args tmp(args);
      tmp>>key;
      typename M::iterator i=map.find(key);
      if (i!=map.end()) 
        {
          // register current object with TCL
          auto v=std::dynamic_pointer_cast<V>(*i);
          if (v)
            {
              if (v!=val)
              {
                // we keep another reference to value here so that we
                // never dereference an invalid object
                val=v;
                ref.reset(new TclExtend<std::shared_ptr<V>>(val));
                TCL_obj(minskyTCL_obj(), cmdPrefix, *ref);
              }
            }
          else
            {
              ostringstream s;
              s<<"gotten object "<<key<<" not of type "<<typeName<V>();
              throw error(s.str().c_str());
            }
        }
      else
        throw error("object not found: %s[%s]",(char*)args[-1],(char*)args[0]);
      tclcmd() << "scopedDisableEventProcessing\n";
    }
    // for backward compatibility
    void set(TCL_args args) {}
    //  release object, and also delete all TCL command references to the object
    void clear() {
      val.reset();
      if (!cmdPrefix.empty())
        {
          tclcmd() | "foreach cmd [info commands "|cmdPrefix|".*] {rename $cmd {}}\n";
          // reestablish default get/set command
          TCL_obj(minskyTCL_obj(), cmdPrefix, *this);
        }
      ref.reset();
    }
    GetterSetterPtr(M& m): map(m) {}
    // assignment is do nothing, as reference member is created as part
    // of constructor
    void operator=(const GetterSetterPtr&) {}
  };


  struct MinskyTCL: public Minsky
  {
    /// TCL accessors
    GetterSetter<Ports> port;
    GetterSetter<Wires> wire;
    GetterSetterPtr<Operations> op;
    GetterSetterPtr<Operations, Constant> constant;
    GetterSetterPtr<Operations, IntOp> integral;
    GetterSetterPtr<Operations, DataOp > data;
    GetterSetterPtr<VariableManager> var;
    GetterSetter<std::map<string, VariableValue> > value;
    GetterSetter<Plots> plot;
    GetterSetter<GodleyIcons> godley;
    GetterSetter<GroupIcons> group;
    GetterSetter<SwitchIcons> switchItem;
    GetterSetter<Notes> note;

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

    MinskyTCL(): port(ports), wire(wires), op(operations), 
                 constant(operations), integral(operations), 
                 data(operations), var(variables),
                 value(variables.values), plot(plots), 
                 godley(godleyItems), group(groupItems), 
                 switchItem(switchItems), note(notes) {}

    void clearAllGetterSetters() {
      // need also to clear the GetterSetterPtr variables, as these
      // potentially hold onto objects
      port.clear();
      wire.clear();
      op.clear();
      constant.clear();
      integral.clear();
      var.clear();
      value.clear();
      plot.clear();
      godley.clear();
      group.clear();
      switchItem.clear();
      note.clear();
    }

    void clearAll() {
      clearAllMaps();
      clearAllGetterSetters();
    }

    /// add a new wire connecting \a from port to \a to port with \a coordinates
    /// @return wireid, or -1 if wire is invalid
    int addWire(TCL_args args) {
      int from=args, to=args;
      ecolab::array<float> coords;
      if (args.count)
        args>>coords;
      return Minsky::addWire(from, to, coords);
    }

    void deleteOperation(int id) {
      // ticket #199, remove references held by getter/setter
      op.clear();
      integral.clear();
      constant.clear();
      Minsky::deleteOperation(id);
    }
    /// fill in a Tk image with the icon for a specific operation
    /// @param Tk imageName
    /// @param operationName
    void operationIcon(const char* imageName, const char* opName) const;

    void cut()
    {
      // need to clear getters/setters before calling Minsky::cut(),
      // to remove extraneous references
      clearAllGetterSetters();
      Minsky::cut();
    }
      
    void putClipboard(const std::string& s) const override; 
    std::string getClipboard() const override; 

    int newVariable(std::string name, VariableType::Type type)
    {return variables.newVariable(name,type);}

    void convertVarType(const std::string& name, VariableType::Type type)
    {variables.convertVarType(name, type);}
    
    void deleteVariable(int id) {
      var.clear(); //remove references held by getter/setter
      variables.erase(id);
    }

    void inGroupSelect(int gid, float x0, float y0, float x1, float y1)
    {
      clearSelection();
      if (groupItems.count(gid))
        groupItems[gid].select(currentSelection,x0,y0,x1,y1);
    }

    void initGroupList(TCL_args args) {
      groupTest.initGroupList(groupItems, (args.count? args: -1));}
    float localZoomFactor(std::string item, int id, float x, float y) const {
      int g=groupTest.containingGroup(x,y);
      float z=1;
      // godley tables can have a user overridden zoom
      if (item=="godley") 
        z=godleyItems[id].zoomFactor;
      if (g==-1 || (g==id && item=="groupItem"))
        return z*zoomFactor(); //global zoom factor
      else 
        return z*groupItems.find(g)->localZoom();
    }

    /// load from a file
    void load(const std::string& filename) {
      clearAllGetterSetters();
      Minsky::load(filename);
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

  };
}

#ifdef _CLASSDESC
 // we don't want to serialise this helper
#pragma omit pack minsky::GetterSetter
#pragma omit unpack minsky::GetterSetter
#pragma omit xml_pack minsky::GetterSetter
#pragma omit xml_unpack minsky::GetterSetter
#pragma omit xsd_generate minsky::GetterSetter
#pragma omit pack minsky::GetterSetterPtr
#pragma omit unpack minsky::GetterSetterPtr
#pragma omit xml_pack minsky::GetterSetterPtr
#pragma omit xml_unpack minsky::GetterSetterPtr
#pragma omit xsd_generate minsky::GetterSetterPtr
#endif

namespace classdesc_access
{
  namespace cd=classdesc;
  
  template <class M> 
  struct access_pack<minsky::GetterSetter<M> >:
    public cd::NullDescriptor<cd::pack_t> {};

  template <class M> 
  struct access_unpack<minsky::GetterSetter<M> >:
    public cd::NullDescriptor<cd::pack_t> {};

  template <class M, class V> 
  struct access_pack<minsky::GetterSetterPtr<M, V> >:
    public cd::NullDescriptor<cd::pack_t> {};

  template <class M, class V> 
  struct access_unpack<minsky::GetterSetterPtr<M, V> >:
    public cd::NullDescriptor<cd::pack_t> {};

}

#include "minskyTCL.cd"
#endif
