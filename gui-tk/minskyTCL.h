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
  extern void setTCL_objAttributes();


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
        setTCL_objAttributes();
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

    template <class T>
    void setRef(const std::shared_ptr<T>& v) {
      auto tmp=std::dynamic_pointer_cast<V>(v);
      if (!tmp)
        throw error("object not of type %s",typeName<V>().c_str());
      if (tmp!=val)
        {
          // we keep another reference to value here so that we
          // never dereference an invalid object
          val=tmp;
          ref.reset(new TclExtend<std::shared_ptr<V>>(val));
          TCL_obj(minskyTCL_obj(), cmdPrefix, *ref);
          setTCL_objAttributes();
        }
    }

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
        setRef(*i);
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
          setTCL_objAttributes();
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
    //    GetterSetter<Ports> port;
    typedef TrackedIntrusiveMap<int,WirePtr> Wires;
    Wires wires;
    GetterSetterPtr<Wires, Wire> wire;
    GetterSetter<VariableValues> value;
    
    typedef TrackedIntrusiveMap<int,ItemPtr> Items;
    Items items;
    GetterSetterPtr<Items, OperationBase> op;
    GetterSetterPtr<Items, Constant> constant;
    GetterSetterPtr<Items, IntOp> integral;
    GetterSetterPtr<Items, DataOp > data;
    GetterSetterPtr<Items, VariableBase> var;
    GetterSetterPtr<Items, PlotWidget> plot;
    GetterSetterPtr<Items, GodleyIcon> godley;
    GetterSetterPtr<Items, Group> group;
    GetterSetterPtr<Items, SwitchIcon> switchItem;
    GetterSetterPtr<Items, Item> item; //< generic item

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

    int nextId=0;
    void resetNextId() {nextId=0;}
    int getNewId() {return nextId++;}
 
    /// flag to indicate whether a TCL should be pushed onto the
    /// history stack, or logged in a recording. This is used to avoid
    /// movements being added to recordings and undo history
    bool doPushHistory=true;

    MinskyTCL(): /*port(ports),*/ wire(wires), op(items), 
                 constant(items), integral(items), 
                 data(items), var(items),
                 value(variableValues), plot(items), 
                 godley(items), group(items), 
                 switchItem(items), item(items) {}

    void clearAllGetterSetters() {
      // need also to clear the GetterSetterPtr variables, as these
      // potentially hold onto objects
      //port.clear();
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
      item.clear();
    }

    void clearAll() {
      wires.clear();
      items.clear();
      clearAllMaps();
      clearAllGetterSetters();
      resetNextId();
    }

    void buildMaps() {
      wires.clear();
      items.clear();
      resetNextId();
      model->recursiveDo
        (&Group::items,[&](const minsky::Items&, minsky::Items::const_iterator it)
         {
           items.emplace(getNewId(), *it);
           return false;
         });
      model->recursiveDo
        (&Group::groups,[&](const Groups&, Groups::const_iterator it)
         {
           items.emplace(getNewId(), *it);
           return false;
         });
      model->recursiveDo
        (&Group::wires,[&](const minsky::Wires&, minsky::Wires::const_iterator it)
         {
           wires.emplace(getNewId(), *it);
           return false;
         });
    }

    /// add a new wire connecting \a from port to \a to port with \a coordinates
    /// @return wireid, or -1 if wire is invalid
    int addWire(TCL_args args) {
      int from=args;
      float fromx=args, fromy=args, tox=args, toy=args;
      std::vector<float> coords;
      int r=-1;
      if (args.count)
        args>>coords;
      if (auto& fromItem=items[from])
        if (auto fromPort=fromItem->closestOutPort(fromx, fromy))
          {
            ClosestPort toPort(*model, ClosestPort::in, tox, toy);
            if (toPort)
              wires[r=getNewId()]=model->addWire(new Wire(fromPort, toPort, coords));
          }
      return r;
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

    int newVariable(std::string name, VariableType::Type type) {
      int id=getNewId();
      items[id]=model->addItem(VariablePtr(type,name));
      return id;
    }

    /// add an operation
    int addOperation(OperationType::Type op) {
      int id=getNewId();
      items[id]=model->addItem(OperationBase::create(op));
      return id;
    }
     
    int addGodleyTable() {
      int id=getNewId();
      items[id]=model->addItem(new GodleyIcon);
      return id;
    }
     
    int newSwitch() {
      int id=getNewId();
      items[id]=model->addItem(new SwitchIcon);
      return id;
    }
     
    int newPlot() {
      int id=getNewId();
      items[id]=model->addItem(new PlotWidget);
      return id;
    }
     
    int newNote() {
      int id=getNewId();
      items[id]=model->addItem(new Item);
      return id;
    }

    int createGroup()
    {
      int id=getNewId();
      items[id]=Minsky::createGroup();
      return id;
    }
     
    /// create a new operation that is a copy of \a id
    int copyItem(int id) {
      int r=-1;
      auto it=items.find(id);
      if (it!=items.end())
        items[r=getNewId()]=model->addItem((*it)->clone());
      return id;
    }

  void deleteItem(int id) {
    auto it=items.find(id);
    if (it!=items.end())
      {
        clearAllGetterSetters();
        model->removeItem(**it);
        items.erase(id);
      }
  }

  void deleteWire(int id) {
    auto it=wires.find(id);
    if (it!=wires.end())
      {
        clearAllGetterSetters();
        model->removeWire(**it);
        wires.erase(id);
      }
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

    /// returns the local zoom factor to be applied to item \a id at \a x,y
    float localZoomFactor(int id, float x, float y) const;

    /// checks whether item \a id needs to be moved to a different
    /// group, and do the move if so
    /// @return true if the item moved between groups
    bool checkAddGroup(int id, float x, float y);

    void adjustItemWires(Item* it);

    /// adjust wires after item \id moves
    void adjustWires(int id);

    /// load from a file
    void load(const std::string& filename) {
      clearAll();
      Minsky::load(filename);
      buildMaps();
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
