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

// An object to represent a Tk canvas around a group

#ifndef GROUPTCL_H
#define GROUPTCL_H
#include "minsky.h"
#include "TCL_extend.h"

namespace minsky
{
  /// a TCL_obj_t that provides a hook for detecting model edits
  ecolab::TCL_obj_t& minskyTCL_obj();
  void setTCL_objAttributes();

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
    K key;
  public:

    template <class T>
    void setRef(const std::shared_ptr<T>& v, const std::string& cmdPrefix) {
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

    const std::shared_ptr<V>& getRef() const {return val;}

    // nb, in spite of appearances, this approach does not work well
    // with non-shared_pointer value types
    void get(TCL_args args) {
      cmdPrefix=(char*)args[-1];
      cmdPrefix.erase(cmdPrefix.rfind(".get"));
      TCL_args tmp(args);
      tmp>>key;
      typename M::iterator i=map.find(key);
      if (i!=map.end()) 
        setRef(*i, cmdPrefix);
      else
        throw error("object not found: %s[%s]",(char*)args[-1],(char*)args[0]);
      TCL_obj(minskyTCL_obj(), cmdPrefix+".id", key);
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

  // add a vtbl to allow deleters to work with generic types
  struct DeleterBase
  {
    virtual ~DeleterBase() {}
  };
  
  // dummy model class needed to represent the group referenced by a pop up window
  struct Model
  {
    GroupPtr model;
    ~Model();
  };

  template <class Model> // Model must have a GroupPtr model member
  class GroupTCL: public DeleterBase, public Model
  {
  protected:
    int nextId=0;
    void resetNextId() {nextId=0;}
 
  public:

    /// TCL accessors
    typedef TrackedIntrusiveMap<int,WirePtr> Wires;
    Wires wires;
    GetterSetterPtr<Wires, Wire> wire;
   
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

    GroupTCL(): wire(wires), op(items), 
                 constant(items), integral(items), 
                 data(items), var(items),
                 plot(items), godley(items), group(items), 
                 switchItem(items), item(items) {}

    int getNewId() {return nextId++;}

    void clearAllGetterSetters() {
      // need also to clear the GetterSetterPtr variables, as these
      // potentially hold onto objects
      wire.clear();
      op.clear();
      constant.clear();
      integral.clear();
      var.clear();
      plot.clear();
      godley.clear();
      group.clear();
      switchItem.clear();
      item.clear();
    }

    void clearAll() {
      wires.clear();
      items.clear();
      clearAllGetterSetters();
      resetNextId();
    }

    void buildMaps() {
      wires.clear();
      items.clear();
      resetNextId();
      Model::model->recursiveDo
        (&Group::items,[&](const minsky::Items&, minsky::Items::const_iterator it)
         {
           items.emplace(getNewId(), *it);
           return false;
         });
      Model::model->recursiveDo
        (&Group::groups,[&](const Groups&, Groups::const_iterator it)
         {
           items.emplace(getNewId(), *it);
           return false;
         });
      Model::model->recursiveDo
        (&Group::wires,[&](const minsky::Wires&, minsky::Wires::const_iterator it)
         {
           wires.emplace(getNewId(), *it);
           return false;
         });
    }

    /// add a new wire connecting \a from port to \a to port with \a coordinates
    /// @return wireid, or -1 if wire is invalid
    int addWire(TCL_args args)
    {
      int from=args;
      float fromx=args, fromy=args, tox=args, toy=args;
      std::vector<float> coords;
      int r=-1;
      if (args.count)
        args>>coords;
      if (auto& fromItem=items[from])
        if (auto fromPort=fromItem->closestOutPort(fromx, fromy))
          {
            ClosestPort toPort(*Model::model, ClosestPort::in, tox, toy);
            if (toPort)
              for (unsigned i=0; i<toPort->item.ports.size(); ++i)
                if (toPort->item.ports[i].get()==toPort.get())
                  if (auto newWire=
                      Model::model->addWire(fromPort->item, toPort->item, i, coords))
                    wires[r=getNewId()]=newWire;
          }
      return r;
    }

    int newVariable(std::string name, VariableType::Type type) {
      int id=getNewId();
      items[id]=Model::model->addItem(VariablePtr(type,name));
      return id;
    }

    /// add an operation
    int addOperation(OperationType::Type op) {
      int id=getNewId();
      auto& it=items[id]=Model::model->addItem(OperationBase::create(op));
      if (IntOp* i=dynamic_cast<IntOp*>(it.get()))
        items[getNewId()]=i->intVar;
      return id;
    }
     
    int addGodleyTable() {
      int id=getNewId();
      items[id]=Model::model->addItem(new GodleyIcon);
      return id;
    }
     
    int newSwitch() {
      int id=getNewId();
      items[id]=Model::model->addItem(new SwitchIcon);
      return id;
    }
     
    int newPlot() {
      int id=getNewId();
      items[id]=Model::model->addItem(new PlotWidget);
      return id;
    }
     
    int newNote() {
      int id=getNewId();
      items[id]=Model::model->addItem(new Item);
      return id;
    }

    int createGroup()
    {
      int id=getNewId();
      items[id]=minsky().createGroup();
      Model::model->addItem(items[id]);
      return id;
    }

    void ungroup(int i)
    {
      if (Group* g=dynamic_cast<Group*>(items[i].get()))
        if (auto parent=g->group.lock())
          {
            parent->moveContents(*g);
            parent->removeGroup(*g);
            items.erase(i);
          }
    }
     
    /// create a new item that is a copy of item
    int copyItem() {
      int r=-1;
      items[r=getNewId()]=Model::model->addItem(item.getRef()->clone());
      return r;
    }

    void deleteItem(int id) {
      auto it=items.find(id);
      if (it!=items.end())
        {
          clearAllGetterSetters();
          Model::model->removeItem(**it);
          items.erase(id);
        }
    }
    
    void deleteWire(int id) {
      auto it=wires.find(id);
      if (it!=wires.end())
        {
          clearAllGetterSetters();
        Model::model->removeWire(**it);
        wires.erase(id);
        }
    }
    
    /// adjust wires after item \id moves
    void adjustWires(int id);

    /// returns the id of the group containing \a item
    int groupOf(int item);

    /// checks whether item \a id needs to be moved to a different
    /// group, and do the move if so
    void checkAddGroup(int id, float x, float y);

    void adjustItemWires(Item* it);

    /// create a new TCL C++ object of name \a name referring to group \a id
    /// Use name.delete to clean up.
    void newGroupTCL(const std::string& name, int id);

    /// essentially as above, but refer to this rather than create a new C++ object
    void newGlobalGroupTCL(const std::string& name);

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

#include "groupTCL.cd"
#endif
