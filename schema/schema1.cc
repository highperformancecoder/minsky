/*
  @copyright Steve Keen 2012
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

#include "schema1.h"
#include "schemaHelper.h"
#include "factory.h"
#include "str.h"
#include <ecolab_epilogue.h>
#include <boost/regex.hpp>

namespace schema1
{
  using minsky::SchemaHelper;
  const int Minsky::version;


  namespace
  {
    // internal type used in MinskyModel::validate()
    struct Validate  {
      set<int> items;
      template <class T> bool check(const T& x) {
        for (typename T::const_iterator i=x.begin(); i!=x.end(); ++i)
          if (!items.insert(i->id).second) return false;
        return true;
      }
    };

    // internal type for associating layout data with model data
    class Combine
    {
      typedef map<int, UnionLayout> Layouts;
      Layouts layout;
      minsky::VariableManager& vm;
    public:
      Combine(const vector<shared_ptr<Layout> >& l, minsky::VariableManager& vm): vm(vm) {
        for (size_t i=0; i<l.size(); ++i)
          layout.insert(make_pair(l[i]->id, UnionLayout(*l[i])));
      }

      /// combine model and layout data
      template <class T, class U>
      T combine(T&, const U&) const;

      template <class K, class V, class U>
      V combine(minsky::IntrusiveWrap<K,V>& x, const U& y) const
      {return combine(static_cast<V&>(x), y);}


      template <class T, class U>
      T combine(const T& t, const U& u) const
      {T t1(t); return combine(t1,u);}

      /// populate the usual integer-based map from a vector of schema data
      template <class M, class U>
      void populate(M& m, const vector<U>& v) const
      {
        for (typename vector<U>::const_iterator i=v.begin(); i!=v.end(); ++i)
          combine(m[i->id], *i);
      }

      template <class T, class U>
      void populate(minsky::PMMap<T>& m, const vector<U>& v) const
      {
        for (typename vector<U>::const_iterator i=v.begin(); i!=v.end(); ++i)
          {
            typename minsky::PMMap<T>::value_type v(i->id);
            combine(v, *i);
            SchemaHelper::insert(m,v);
          }
      }

      

      /// populate the GroupItem  from a vector of schema data
      void populate
      (map<int, minsky::GroupIcon>& m, const vector<Group>& v) const;

      /// populate the variable manager from a vector of schema data
      void populate(minsky::VariableManager& vm, const vector<Variable>& v) const;

      /// populate the plots manager from a vector of schema data
      void populate(minsky::Plots& plots, const vector<Plot>& v) const;
    };

    template <>
    minsky::Port Combine::combine(minsky::Port& p, const Port& pp) const
    {
      Layouts::const_iterator l=layout.find(pp.id);
      if (l!=layout.end())
        p=minsky::Port(l->second.x, l->second.y, pp.input);
      else
        p=minsky::Port(0,0,pp.input);
      return p;
    }

    template <>
    minsky::Wire Combine::combine(minsky::Wire& w, const Wire& w1) const
    {
      Layouts::const_iterator l=layout.find(w1.id);
      if (l!=layout.end())
        {
          w.from=w1.from;
          w.to=w1.to;
          w.visible=l->second.visible;
          if (l->second.coords.size()>=4)
            w.coords(toArray(l->second.coords));
          else //add straight wire
            {
              ecolab::array<float> coords;
              l=layout.find(w.from);
              if (l!=layout.end())
                {
                  (coords<<=l->second.x)<<=l->second.y;
                  l=layout.find(w.to);
                  if (l!=layout.end())
                    {
                      (coords<<=l->second.x)<<=l->second.y;
                      w.coords(coords);
                    }
                }
            }
        }
      return w;
    }
     
    void asgNote(minsky::Note& n, const Item& i)
    {
      n.detailedText = i.detailedText;
      n.tooltip = i.tooltip;
    }     

    template <> minsky::OpVarBaseAttributes 
    Combine::combine(minsky::OpVarBaseAttributes& o, const Item& o1) const
    {
      asgNote(o, o1);
      Layouts::const_iterator li=layout.find(o1.id);
      if (li!=layout.end())
        {
          const UnionLayout& l=li->second;
          o.rotation=l.rotation;
          o.visible=l.visible;
          o.m_x=l.x;
          o.m_y=l.y;
        }
      return o;
    }

    template <> minsky::OperationPtr 
    Combine::combine(minsky::OperationPtr& o, const Operation& o1) const
    {
      Layouts::const_iterator li=layout.find(o1.id);
      o=minsky::OperationPtr(o1.type, o1.ports);
      combine(static_cast<minsky::OpVarBaseAttributes&>(*o), 
              static_cast<const Item&>(o1));
      switch (o1.type)
        {
        case minsky::OperationType::integrate:
          SchemaHelper::setPrivates
            (dynamic_cast<minsky::IntOp&>(*o), o1.name, o1.intVar);
          break;
        case minsky::OperationType::add: case minsky::OperationType::subtract: 
        case minsky::OperationType::multiply: case minsky::OperationType::divide: 
          SchemaHelper::makePortMultiWired(minsky::minsky().ports[o1.ports[1]]);
          SchemaHelper::makePortMultiWired(minsky::minsky().ports[o1.ports[2]]);
          break;
        case minsky::OperationType::constant:
          if (minsky::Constant* c=dynamic_cast<minsky::Constant*>(o.get()))
            {
              c->value=o1.value;
              c->description=o1.name;
            }
          break;
        case minsky::OperationType::data:
          if (minsky::DataOp* d=dynamic_cast<minsky::DataOp*>(o.get()))
            {
              d->data=o1.data;
              d->description=o1.name;
            }
          break;
        default: break;
        }

      if (li!=layout.end())
        if (minsky::Constant* c=dynamic_cast<minsky::Constant*>(o.get()))
          {
            const UnionLayout& l=li->second;
            c->sliderVisible=l.sliderVisible;
            c->sliderBoundsSet=l.sliderBoundsSet;
            c->sliderStepRel=l.sliderStepRel;
            c->sliderMin=l.sliderMin;
            c->sliderMax=l.sliderMax;
            c->sliderStep=l.sliderStep;
          }
      return o;
    }
   
    template <> minsky::VariablePtr 
    Combine::combine(minsky::VariablePtr& v, const Variable& v1) const
    {
      assert(v);
      combine(static_cast<minsky::OpVarBaseAttributes&>(*v), 
              static_cast<const Item&>(v1));
      Layouts::const_iterator li=layout.find(v1.id);
      if (li!=layout.end())
        {
          const UnionLayout& l=li->second;
          v->name(v1.name);
          // any unscoped variables are considered global
          if (v1.name.find(":")==string::npos)
            v->setScope(-1);
          v->init(v1.init);
          v->sliderVisible=l.sliderVisible;
          v->sliderBoundsSet=l.sliderBoundsSet;
          v->sliderStepRel=l.sliderStepRel;
          v->sliderMin=l.sliderMin;
          v->sliderMax=l.sliderMax;
          v->sliderStep=l.sliderStep;
          int out=v1.ports.size()>0? v1.ports[0]: -1;
          int in=v1.ports.size()>1? v1.ports[1]: -1;
          SchemaHelper::setPrivates
            (dynamic_cast<minsky::VariablePorts&>(*v), out, in);
        }
      return v;
    }

    template <> minsky::SwitchIconPtr 
    Combine::combine(minsky::SwitchIconPtr& sw, const Switch& s) const
    {
      if (!sw) sw.reset(new minsky::SwitchIcon);
      combine(static_cast<minsky::OpVarBaseAttributes&>(*sw), 
              static_cast<const Item&>(s));
      SchemaHelper::setPrivates(*sw,s.ports);
      return sw;
    }

    template <> minsky::PlotWidget 
    Combine::combine(minsky::PlotWidget& p, const Plot& p1) const
    {
      asgNote(p, p1);
      Layouts::const_iterator li=layout.find(p1.id);
      if (li!=layout.end())
        {
          const UnionLayout& l=li->second;
          //p.ports().resize(22);
          vector<int> pports(22);
          // schema migration fix - handle old 12 port case
          if (p1.ports.size()==12)
            {
              size_t i=0;
              for (; i<4; ++i)
                pports[i]=p1.ports[i];
              // skip the two new control ports
              for (; i<8; ++i)
                pports[i+2]=p1.ports[i];
              // skip the new RHS input ports
              for (; i<12; ++i)
                pports[i+6]=p1.ports[i];              
              // add new RHS scale control ports
              for (i=4; i<6; ++i)
                pports[i]=minsky::minsky().addInputPort(); 
              // add new RHS input ports
              for (i=10; i<14; ++i)
                pports[i]=minsky::minsky().addInputPort();
              // add x input ports for the RHS inputs
              for (i=18; i<22; ++i)
                pports[i]=minsky::minsky().addInputPort();
            }
          else
            {
              assert(pports.size()==p1.ports.size());
              for (size_t i=0; i<p1.ports.size(); ++i)
                pports[i]=p1.ports[i];
            }
          SchemaHelper::setPrivates(p,pports);
          p.legend=bool(p1.legend);
          if (p1.legend)
            p.legendSide=*p1.legend;
          p.title=p1.title;
          p.xlabel=p1.xlabel;
          p.ylabel=p1.ylabel;
          p.y1label=p1.y1label;
          SchemaHelper::setXY(p,l.x,l.y);
          p.width=l.width;
          p.height=l.height;
        }
      return p;
    }

    template <> minsky::GroupIcon
    Combine::combine(minsky::GroupIcon& g, const Group& g1) const
    {
      asgNote(g, g1);
      Layouts::const_iterator li=layout.find(g1.id);
      if (li!=layout.end())
        {
          const UnionLayout& l=li->second;
          // installing these has to happen later, as at this point,
          // we just have a list of anonymous items, not what type
          // they are
          //          SchemaHelper::setPrivates
          //            (g, g1.operations, g1.variables, g1.wires, g1.ports);
          g.setName(g1.name);
          minsky::SchemaHelper::setXY(g, l.x, l.y);
          g.width=l.width;
          g.height=l.height;
          g.displayZoom=l.displayZoom;
          g.rotation=l.rotation;
          g.visible=l.visible;
          g.createdVars=g1.createdVars;
        }
      return g;
    }

    template <> minsky::GodleyIcon
    Combine::combine(minsky::GodleyIcon& g, const Godley& g1) const
    {
      asgNote(g, g1);
      Layouts::const_iterator li=layout.find(g1.id);
      if (li!=layout.end())
        {
          const UnionLayout& l=li->second;

          vector<vector<string> > data(g1.data);

          SchemaHelper::setPrivates(g.table, data, g1.assetClasses);
          g.table.doubleEntryCompliant=g1.doubleEntryCompliant;
          g.table.title=g1.name;
          g.zoomFactor=g1.zoomFactor;
          // add in variables from the port list. Duplications don't
          // matter, as update() will fix this.
          for (size_t i=0; i<g1.ports.size(); ++i)
            {
              minsky::VariablePtr v = vm.getVariableFromPort(g1.ports[i]);
              v->visible=false;
              switch (v->type())
                {
                case VariableType::flow:
                  g.flowVars.push_back(v);
                  break;
                case VariableType::stock:
                  g.stockVars.push_back(v);
                  break;
                default: break;
                }
            }
          try 
            {
              g.update();
            }
          catch (const std::exception& ex)
            {
              // if something is inconsistent about the variables in the table, this can throw,
              // so ignore it here. The problem should be flagged elsewhere.
            }
          g.moveTo(l.x, l.y);
        }
      return g;
    }
    
    void Combine::populate
    (map<int, minsky::GroupIcon>& m, const vector<Group>& v) const
    {
      for (vector<Group>::const_iterator i=v.begin(); i!=v.end(); ++i)
        {
          minsky::GroupIcon& g = m[i->id];
          combine(g, *i);
        }
    }
     
    void Combine::populate(minsky::Plots& plots, const vector<Plot>& v) const
    {
      for (vector<Plot>::const_iterator i=v.begin(); i!=v.end(); ++i)
          combine(plots[i->id], *i);
    }

    template <> 
    void Combine::populate(minsky::SwitchIcons& switches, const vector<Switch>& v) const
    {
      for (auto& i: v)
        combine(switches[i.id], i);
    }

    void Combine::populate(minsky::VariableManager& vm, const vector<Variable>& v) const
    {
      for (vector<Variable>::const_iterator i=v.begin(); i!=v.end(); ++i)
        {
          auto type=i->type;
          // old constants are now called parameters
          if (type==VariableType::constant && i->name!=i->init)
            type=VariableType::parameter;
          minsky::VariablePtr v1(type);
          vm.addVariable(combine(v1, *i), i->id);
          //vm.setInit(minsky::stripActive(i->name), i->init);
        }
    }

    struct ItemMap: public map<int, int>
    {
      void remap(vector<int>& x) {
        for (size_t i=0; i<x.size(); ++i) x[i]=(*this)[x[i]];
      }
    };

    template <class T> shared_ptr<Layout> layoutFactory(int id, const T&);

    template <class K, class V> 
    shared_ptr<Layout> layoutFactory(int id, const minsky::IntrusiveWrap<K,V>& x)
    {return layoutFactory(id, static_cast<const V&>(x));}

    template <> shared_ptr<Layout> layoutFactory
    (int id, const minsky::Port& p)
    {return shared_ptr<Layout>(new PositionLayout(id, p));}

    template <> shared_ptr<Layout> layoutFactory
    (int id, const minsky::Wire& w)
    {return shared_ptr<Layout>(new WireLayout(id,w));}

    template <> shared_ptr<Layout> layoutFactory
    (int id, const minsky::VariablePtr& v)
    {return shared_ptr<Layout>(new SliderLayout(id,*v));}

    template <> shared_ptr<Layout> layoutFactory
    (int id, const minsky::OperationBase& o)
    {
      if (const minsky::Constant* c=dynamic_cast<const minsky::Constant*>(&o))
        return shared_ptr<Layout>(new SliderLayout(id,*c));
      else
        return shared_ptr<Layout>(new ItemLayout(id,o));
    }

    template <> shared_ptr<Layout> layoutFactory
    (int id, const minsky::GodleyIcon& g)
    {return shared_ptr<Layout>(new PositionLayout(id,g));}

    template <> shared_ptr<Layout> layoutFactory
    (int id, const minsky::PlotWidget& p)
    {return shared_ptr<Layout>(new PlotLayout(id,p));}

    template <> shared_ptr<Layout> layoutFactory
    (int id, const minsky::GroupIcon& g)
    {return shared_ptr<Layout>(new GroupLayout(id,g));}
  }

  Operation::Operation(int id, const minsky::OperationBase& op): 
    Item(id,op), type(op.type()), value(0), ports(op.ports()), intVar(-1) 
  {
    if (const minsky::Constant* c=dynamic_cast<const minsky::Constant*>(&op))
      {
        name=c->description;
        value=c->value;
      }
    else if (const minsky::IntOp* i=dynamic_cast<const minsky::IntOp*>(&op))
      {
        name=i->getIntVar()->fqName();
        intVar=i->intVarID();
      }
    else if (const minsky::DataOp* d=dynamic_cast<const minsky::DataOp*>(&op))
      {
        name=d->description;
        data=d->data;
      }
  }

  namespace
  {
    template <class T>
    bool assignIf(UnionLayout& u, const Layout& l)
    {
      const T* p;
      if ((p=dynamic_cast<const T*>(&l)))
        static_cast<T&>(u)=*p;
      return p;
    }
  }

  UnionLayout::UnionLayout(const Layout& l)
  {
    assignIf<UnionLayout>(*this, l) ||
      assignIf<SliderLayout>(*this, l) ||
      assignIf<GroupLayout>(*this, l) ||
      assignIf<PlotLayout>(*this, l) ||
      assignIf<WireLayout>(*this, l) ||
      assignIf<ItemLayout>(*this, l) ||
      assignIf<PositionLayout>(*this, l);
  }


  bool MinskyModel::validate() const
  {
    // check that wire from/to labels refer to ports
    set<int> portIds;
    for (vector<Port>::const_iterator p=ports.begin(); p!=ports.end(); ++p)
      portIds.insert(p->id);
    for (vector<Wire>::const_iterator w=wires.begin(); w!=wires.end(); ++w)
      if (portIds.find(w->from)==portIds.end() || portIds.find(w->to)==portIds.end())
        return false;

    // check that ids are unique
    Validate v;
    return v.check(ports) && v.check(wires) && v.check(operations) && 
      v.check(variables) && v.check(groups) && v.check(godleys);
  }

  

  Minsky::operator minsky::Minsky() const
  {
    if (!model.validate())
      throw ecolab::error("inconsistent Minsky model");

    minsky::Minsky m;
    Combine c(layout, m.variables);
    // override default minsky object for this method
    minsky::LocalMinsky lm(m);

    // convert the old constant operation into the new constant variables
    vector<Operation> newOpList;
    vector<Variable> newVarList=model.variables;
    // we need to back out of this change if there is a preexisting
    // variable the same name as the constant
    set<string> existingVarNames;
    for (vector<Variable>::iterator v=newVarList.begin(); 
         v!=newVarList.end(); ++v)
      existingVarNames.insert(v->name);
    for (vector<Operation>::const_iterator o=model.operations.begin();
         o!=model.operations.end(); ++o)
      if (o->type==minsky::OperationType::constant && !existingVarNames.count(o->name))
        {
          Variable v;
          v.id=o->id;
          v.init=minsky::str(o->value);
          v.name=o->name;
          v.type=v.name==v.init? VariableType::constant: VariableType::parameter;
          v.ports=o->ports;
          newVarList.push_back(v);
        }
      else
        newOpList.push_back(*o);

    c.populate(m.ports, model.ports);
    c.populate(m.groupItems, model.groups);
    c.populate(m.switchItems, model.switches);
    c.populate(m.wires, model.wires);
    //    m.resetNextId();
    c.populate(m.operations, newOpList);
    c.populate(m.variables, newVarList);

    m.variables.makeConsistent();

    m.resetNextId();
    c.populate(m.godleyItems, model.godleys);
    //    c.populate(m.groupItems, model.groups);
    c.populate(m.plots, model.plots);
    c.populate(m.notes, model.notes);

    // separate the group item list into ports, wires, operations and
    // variables. Then set the Minsky model group item list to these
    // appropriate sublists.
    for (vector<Group>::const_iterator g=model.groups.begin(); 
         g!=model.groups.end(); ++g)
      {
        vector<int> ports, wires, ops, vars, groups;
        for (vector<int>::const_iterator item=g->items.begin(); 
             item!=g->items.end(); ++item)
          if (m.wires.count(*item))
            wires.push_back(*item);
          else if (m.operations.count(*item))
            ops.push_back(*item);
          else if (m.variables.count(*item))
            vars.push_back(*item);
          else if (m.groupItems.count(*item))
            groups.push_back(*item);
        // because variables and operator create ports, duplicate
        // entries in the port map may already exist, so we must place
        // the port check last
          else if (m.ports.count(*item))
            ports.push_back(*item);
        minsky::GroupIcon& gi=m.groupItems[g->id];

        vector<int> inVars, outVars;
        for (size_t i=0; i<ports.size(); ++i)
          {
            int varId=m.variables.getVariableIDFromPort(ports[i]);
            if (varId>=0)
              {
                if (m.ports[ports[i]].input())
                  inVars.push_back(varId);
                else
                  outVars.push_back(varId);
              }
          }
        SchemaHelper::setPrivates(gi, ops, vars, wires, groups, inVars, outVars);
        // set the parent attribute of all child groups
        for (vector<int>::const_iterator i=groups.begin(); i!=groups.end(); ++i)
          SchemaHelper::setParent(m.groupItems[*i], g->id);

        for (vector<int>::const_iterator o=ops.begin(); o!=ops.end(); ++o)
          m.operations[*o]->group=g->id;
        for (vector<int>::const_iterator v=vars.begin(); v!=vars.end(); ++v)
          m.variables[*v]->group=g->id;
        for (vector<int>::const_iterator w=wires.begin(); w!=wires.end(); ++w)
          m.wires[*w].group=g->id;
      }

    m.setZoom(zoomFactor);

    m.stepMin=model.rungeKutta.stepMin; 
    m.stepMax=model.rungeKutta.stepMax; 
    m.nSteps=model.rungeKutta.nSteps;   
    m.epsAbs=model.rungeKutta.epsAbs;   
    m.epsRel=model.rungeKutta.epsRel;   
    m.order=model.rungeKutta.order;
    m.simulationDelay=model.rungeKutta.simulationDelay;
    m.implicit=model.rungeKutta.implicit;
    m.resetNextId();
    return m;
  }

  namespace
  {
    template <class C>
    int minRange(int minId, const C& v) {
      for (typename C::const_iterator i=v.begin(); i!=v.end(); ++i) 
        if (minId>i->id) minId=i->id;
      return minId;
    }

    void addOffset(vector<Port>& v, int offs)
    {
      for (vector<Port>::iterator i=v.begin(); i!=v.end(); ++i)
        i->id+=offs;
    }

    void addOffset(vector<Wire>& v, int offs)
    {
      for (vector<Wire>::iterator i=v.begin(); i!=v.end(); ++i)
        {
          i->id+=offs;
          i->from+=offs;
          i->to+=offs;
        }
    }
    void addOffset(vector<Variable>& v, int offs)
    {
      for (vector<Variable>::iterator i=v.begin(); i!=v.end(); ++i)
        {
          i->id+=offs;
          for (size_t j=0; j<i->ports.size(); ++j)
            i->ports[j]+=offs;
          // if variable is scoped, then offset its scope also
          int scope;
          vector<char> uqName(i->name.size()+1);
          if (sscanf(i->name.c_str(), "[%d]:%s", &scope, &uqName[0])==2)
            i->name="["+to_string(scope+offs)+"]:"+&uqName[0];
        }
    }

    void addOffset(vector<Operation>& v, int offs)
    {
      for (vector<Operation>::iterator i=v.begin(); i!=v.end(); ++i)
        {
          i->id+=offs;
          for (size_t j=0; j<i->ports.size(); ++j)
            i->ports[j]+=offs;
          if (i->intVar>=0) i->intVar+=offs;
        }
    }

    void addOffset(vector<Group>& v, int offs)
    {
      for (vector<Group>::iterator i=v.begin(); i!=v.end(); ++i)
        {
          i->id+=offs;
          for (size_t j=0; j<i->items.size(); ++j)
            i->items[j]+=offs;
          for (size_t j=0; j<i->ports.size(); ++j)
            i->ports[j]+=offs;
          for (size_t j=0; j<i->createdVars.size(); ++j)
            i->createdVars[j]+=offs;
        }
    }

    // return a vector ids from an item vector
    template <class Item>
    vector<int> ids(const vector<Item>& v)
    {
      vector<int> r;
      for (typename vector<Item>::const_iterator i=v.begin(); i!=v.end(); ++i)
        r.push_back(i->id);
      return r;
    }

    template <class M, class Item>
    bool exists(const M& m, const Item& i)
    {return m.find(i.id)!=m.end();}

  }

  using minsky::minsky;

  void Minsky::populateGroup(minsky::GroupIcon& g)
  {
    // We must renumber objects into free space, before inserting into the group
    // first, compute the maximum item id within Minsky
    int maxExistingId=minsky().ports.empty()? 0: minsky().ports.rbegin()->id();
    if (!minsky().wires.empty())
      maxExistingId=max(maxExistingId, minsky().wires.rbegin()->id());
    if (!minsky().variables.empty())
      maxExistingId=max(maxExistingId, minsky().variables.rbegin()->id());
    if (!minsky().operations.empty())
      maxExistingId=max(maxExistingId, minsky().operations.rbegin()->id());
    if (!minsky().groupItems.empty())
      maxExistingId=max(maxExistingId, minsky().groupItems.rbegin()->id());

    // now compute min and max item ID within this import fragment
    int minId=numeric_limits<int>::max();
    minId=minRange(minId, model.ports);
    minId=minRange(minId, model.wires);
    minId=minRange(minId, model.operations);
    minId=minRange(minId, model.variables);
    minId=minRange(minId, model.groups);

    // by adding offset, all item ids in this are in the free range
    int offset=maxExistingId-minId+1;

    if (offset>0)
      {
        addOffset(model.ports, offset);
        addOffset(model.wires, offset);
        addOffset(model.operations, offset);
        addOffset(model.variables, offset);
        addOffset(model.groups, offset);
        for (size_t i=0; i<layout.size(); ++i)
          layout[i]->id+=offset;
      }
    
    // winnow ports and wires to just the relevant ones
    set<int> pset;
    for (vector<Operation>::const_iterator o=model.operations.begin(); 
         o!=model.operations.end(); ++o)
      for (size_t j=0; j<o->ports.size(); ++j)
        pset.insert(o->ports[j]);
    for (vector<Variable>::const_iterator v=model.variables.begin(); 
         v!=model.variables.end(); ++v)
      for (size_t j=0; j<v->ports.size(); ++j)
        pset.insert(v->ports[j]);
    for (vector<Group>::const_iterator gi=model.groups.begin(); 
         gi!=model.groups.end(); ++gi)
      for (size_t j=0; j<gi->ports.size(); ++j)
        pset.insert(gi->ports[j]);
    vector<Port> ports;
    for (vector<Port>::const_iterator p=model.ports.begin(); 
         p!=model.ports.end(); ++p)
      if (pset.count(p->id))
        ports.push_back(*p);

    vector<Wire> wires;
    for (vector<Wire>::const_iterator w=model.wires.begin();
         w!=model.wires.end(); ++w)
      if (pset.count(w->from) && pset.count(w->to))
        wires.push_back(*w);

    // now add the items to the global Minsky object, and build the group object
    Combine c(layout, minsky().variables);
    c.populate(minsky().ports, ports);
    c.populate(minsky().groupItems, model.groups);
    c.populate(minsky().wires, wires);
    c.populate(minsky().operations, model.operations);
    c.populate(minsky().variables, model.variables);
    minsky().variables.makeConsistent();
    c.populate(minsky().notes, model.notes);

    // dummy a very large group size to prevent added variables being
    // placed on the interface
    g.width=1000000;
    g.moveTo(0,0);

    minsky().resetNextId();

    // set of items already contained within a group, so not needed to
    // be added to overall group
    set<int> alreadyGroupedIds;
    for (auto& g: model.groups)
      for (int i: g.items)
        alreadyGroupedIds.insert(i);

    vector<int> wiresToAdd;
    for (Wire w: wires)
      if (!alreadyGroupedIds.count(w.id))
        wiresToAdd.push_back(w.id);
    g.addWires(wiresToAdd);
    for (const Operation& i: model.operations)
      {
        minsky::Operations::iterator op = minsky().operations.find(i.id);
        if (op!=minsky().operations.end() && !alreadyGroupedIds.count(i.id))
          {
            g.addOperation(*op);
            g.addAnyWires((*op)->ports());
          }
      }

    for (const Variable& i: model.variables)
      {
        minsky::VariableManager::iterator v = minsky().variables.find(i.id);
        if (v!=minsky().variables.end()  && !alreadyGroupedIds.count(i.id))
          {
            g.addVariable(*v);
            g.addAnyWires((*v)->ports());
          }
      }

    for (auto& i: model.groups)
      {
        minsky::GroupIcons::iterator gi=minsky().groupItems.find(i.id);
        if (gi!=minsky().groupItems.end()  && !alreadyGroupedIds.count(i.id))
          {
            g.addGroup(*gi);
            g.addAnyWires(gi->ports());

            // need to populate the groupIcon with the contained items
            vector<int> ports, wires, ops, vars, groups;
            for (int item: i.items)
              if (minsky().wires.count(item))
                wires.push_back(item);
              else if (minsky().operations.count(item))
                ops.push_back(item);
              else if (minsky().variables.count(item))
                vars.push_back(item);
              else if (minsky().groupItems.count(item))
                groups.push_back(item);
            // because variables and operator create ports, duplicate
            // entries in the port map may already exist, so we must place
            // the port check last
              else if (minsky().ports.count(item))
                ports.push_back(item);

            vector<int> inVars, outVars;

            for (int p: ports)
              {
                int varId=minsky().variables.getVariableIDFromPort(p);
                if (varId>=0)
                  {
                    if (minsky().ports[p].input())
                      inVars.push_back(varId);
                    else
                      outVars.push_back(varId);
                  }
              }
            SchemaHelper::setPrivates(*gi, ops, vars, wires, groups, inVars, outVars);
            // set the parent attribute of all child groups
            for (int j: groups)
              SchemaHelper::setParent(minsky().groupItems[j], i.id);
            for (int j: ops)
              minsky().operations[j]->group=i.id;
            for (int j: vars)
              minsky().variables[j]->group=i.id;
            for (int j: wires)
              minsky().wires[j].group=i.id;

          }
      }

    float x0, x1, y0, y1;
    g.contentBounds(x0,y0,x1,y1);
    // centre group on centroid of contents
    g.moveContents(g.x()-0.5f*(x0+x1), g.y()-0.5f*(y0+y1));
    g.height=100, g.width=100;
    g.computeDisplayZoom();
  }

  void Group::addItems(const minsky::GroupIcon& g)
  {
    for (auto i: g.ports())
      items.push_back(i);
    for (auto i: g.wires())
      items.push_back(i);
    for (auto i: g.operations())
      items.push_back(i);
    for (auto i: g.variables())
      items.push_back(i);
    for (auto i: g.createdVars)
      createdVars.push_back(i);
  }


  Minsky::Minsky(const minsky::Minsky& m)
  {
    schemaVersion = version;
    zoomFactor=m.zoomFactor();

    int id=0; // we're renumbering items
    ItemMap portMap; // map old ports ids to the new ones
    set<int> portsUsed; 
    ItemMap wireMap;
    ItemMap opMap;
    ItemMap varMap;
    ItemMap groupMap;
    ItemMap noteMap;
    
    for (minsky::Minsky::Ports::const_iterator p=m.ports.begin(); 
         p!=m.ports.end(); ++p, ++id)
      {
        model.ports.push_back(Port(id, *p));
        //        layout.push_back(layoutFactory(id, *p));
        portMap[p->id()]=id;
      }

    for (minsky::Minsky::Wires::const_iterator w=m.wires.begin(); 
         w!=m.wires.end(); ++w, ++id)
      {
        model.wires.push_back(Wire(id, *w));
        model.wires.back().from=portMap[w->from];
        model.wires.back().to=portMap[w->to];
        portsUsed.insert(model.wires.back().from);
        portsUsed.insert(model.wires.back().to);
        layout.push_back(layoutFactory(id, *w));
        wireMap[w->id()]=id;
      }

    for (minsky::VariableManager::const_iterator v=m.variables.begin(); 
         v!=m.variables.end(); ++v, ++id)
      {
        model.variables.push_back(Variable(id, **v));
        Variable& var=model.variables.back();
        portMap.remap(var.ports);
        portsUsed.insert(var.ports.begin(), var.ports.end());
        layout.push_back(layoutFactory(id, *v));
        auto valIt=m.variables.values.find((*v)->valueId());
        if (valIt!=m.variables.values.end())
          var.init=valIt->second.init;
        varMap[v->id()]=id;
      }

    for (minsky::Operations::const_iterator o=m.operations.begin(); 
         o!=m.operations.end(); ++o, ++id)
      {
        model.operations.push_back(Operation(id, **o));
        Operation& op=model.operations.back();
        portMap.remap(op.ports);
        portsUsed.insert(op.ports.begin(), op.ports.end());
        model.operations.back().intVar=varMap[op.intVar];
        layout.push_back(layoutFactory(id, **o));
        opMap[o->id()]=id;
      }

    for (auto& s: m.switchItems)
      {
        model.switches.push_back(Switch(id,*s));
        layout.emplace_back(new ItemLayout(id,*s));
        Switch& sw=model.switches.back();
        portMap.remap(sw.ports);
        portsUsed.insert(sw.ports.begin(), sw.ports.end());
        ++id;
      }

    for (minsky::Minsky::GodleyItems::const_iterator g=m.godleyItems.begin(); 
         g!=m.godleyItems.end(); ++g, ++id)
      {
        model.godleys.push_back(Godley(id, *g));
        Godley& godley=model.godleys.back();
        portMap.remap(godley.ports);
        portsUsed.insert(godley.ports.begin(), godley.ports.end());
        layout.push_back(layoutFactory(id, *g));
      }

    for (minsky::GroupIcons::const_iterator g=m.groupItems.begin(); 
         g!=m.groupItems.end(); ++g, ++id)
      {
        model.groups.push_back(Group(id, *g));
        Group& group=model.groups.back();

        // add in reamapped items 
        group.items.clear(); // in case this was set during construction
        const minsky::GroupIcon& g1=*g;
        for (size_t i=0; i<g1.ports().size(); ++i)
          group.items.push_back(portMap[g1.ports()[i]]);
        for (size_t i=0; i<g1.wires().size(); ++i)
          group.items.push_back(wireMap[g1.wires()[i]]);
        for (size_t i=0; i<g1.operations().size(); ++i)
          group.items.push_back(opMap[g1.operations()[i]]);
        for (size_t i=0; i<g1.variables().size(); ++i)
          group.items.push_back(varMap[g1.variables()[i]]);
        for (size_t i=0; i<g1.createdVars.size(); ++i)
          group.createdVars.push_back(varMap[g1.createdVars[i]]);

        groupMap[g->id()]=id;

        layout.push_back(layoutFactory(id, *g));
      }

    // loop over variables and remap group references in the var names
    boost::regex fqNameSpec(R"(\[(\d+)\]:(.*))");
    boost::smatch match;
    for (Variable& v: model.variables)
      if (regex_search(v.name, match, fqNameSpec))
        {
          assert(match.size()>=3);
          v.name="["+minsky::str(groupMap[atoi(match[1].str().c_str())])+"]:"+
            match[2].str();
        }

    // second pass to add in the child group ids, now that they're renumbered
    int j=0;
    for (const minsky::GroupIcon& g1: m.groupItems)
      {
        Group& group=model.groups[j];
        for (size_t i=0; i<g1.groups().size(); ++i)
          group.items.push_back(groupMap[g1.groups()[i]]);
        ++j;
      }

    for (auto note=m.notes.begin(); note!=m.notes.end(); ++note, ++id)
      {
        model.notes.push_back(Item(id, *note));
        layout.push_back(make_shared<ItemLayout>(id, *note));
      }

    for (minsky::Plots::const_iterator p=m.plots.begin(); 
         p!=m.plots.end(); ++p, ++id)
      {
        model.plots.push_back(Plot(id, *p));
        Plot& plot=model.plots.back();
        portMap.remap(plot.ports);        
        portsUsed.insert(plot.ports.begin(), plot.ports.end());
        layout.push_back(layoutFactory(id, *p));
      }

    // remove unused ports
    vector<Port> ports;
    for (vector<Port>::iterator i=model.ports.begin(); 
         i!=model.ports.end(); ++i)
      if (portsUsed.count(i->id))
        ports.push_back(*i);
    ports.swap(model.ports);

    model.rungeKutta=RungeKutta(m);
      
  }

  Minsky::Minsky(const minsky::Minsky& m, const minsky::Selection& s)
  {
    schemaVersion = version;
    zoomFactor=m.zoomFactor();
    for (int i: s.wires)
      {
        const auto& w=m.wires[i];
        model.wires.push_back(schema1::Wire(i, w));
        layout.push_back(make_shared<schema1::WireLayout>(i, w));
      }
    for (int i: s.operations)
      {
        const auto& o=m.operations[i];
        model.operations.push_back(schema1::Operation(i, *o));
        layout.push_back(make_shared<schema1::ItemLayout>(i, *o));
        for (int pi: o->ports())
          {
            const auto& p=m.ports[pi];
            model.ports.push_back(schema1::Port(pi, p));
            layout.push_back(make_shared<schema1::PositionLayout>(pi, p));
          }
      }
    for (int i: s.variables)
      {
        const auto& v=m.variables[i];
        model.variables.push_back(schema1::Variable(i, *v));
        layout.push_back(make_shared<schema1::ItemLayout>(i, *v));
        for (int pi: v->ports())
          {
            const auto& p=m.ports[pi];
            model.ports.push_back(schema1::Port(pi, p));
            layout.push_back(make_shared<schema1::PositionLayout>(pi, p));
          }
      }
    for (int i: s.groups)
      { 
        const auto& g=m.groupItems[i];
        model.groups.push_back(schema1::Group(i, g));
        model.groups.back().addItems(g);
        layout.push_back(make_shared<schema1::GroupLayout>(i, g));
        populateWith(m,g,g.displayContents());
      }
     for (int i: s.godleys)
      {
        const auto& g=m.godleyItems[i];
        model.godleys.push_back(schema1::Godley(i, g));
        layout.push_back(make_shared<schema1::PositionLayout>(i, g));
        // godley ports are just the ports of the underlying variables
      }
     for (int i: s.plots)
      {
        const auto& p=m.plots[i];
        model.plots.push_back(schema1::Plot(i, p));
        layout.push_back(make_shared<schema1::PositionLayout>(i, p));
        for (int pi: p.ports())
          {
            const auto& p=m.ports[pi];
            model.ports.push_back(schema1::Port(pi, p));
            layout.push_back(make_shared<schema1::PositionLayout>(pi, p));
          }
      }

  }

  void Minsky::populateWith(const minsky::Minsky& m, const minsky::GroupIcon& g, bool visible)
  {
    schemaVersion = version;
    zoomFactor=m.zoomFactor();
    for (int i: g.wires())
      {
        const auto& w=m.wires[i];
        model.wires.push_back(schema1::Wire(i, w));
        layout.push_back(make_shared<schema1::WireLayout>(i, w));
        dynamic_pointer_cast<VisibilityLayout>(layout.back())->visible=visible;
      }
    for (int i: g.operations())
      {
        const auto& o=m.operations[i];
        model.operations.push_back(schema1::Operation(i, *o));
        layout.push_back(make_shared<schema1::ItemLayout>(i, *o));
        dynamic_pointer_cast<VisibilityLayout>(layout.back())->visible=visible;
        for (int pi: o->ports())
          {
            const auto& p=m.ports[pi];
            model.ports.push_back(schema1::Port(pi, p));
          }
      }
    for (int i: g.variables())
      {
        const auto& v=m.variables[i];
        model.variables.push_back(schema1::Variable(i, *v));
        layout.push_back(make_shared<schema1::ItemLayout>(i, *v));
        dynamic_pointer_cast<VisibilityLayout>(layout.back())->visible=visible;
        for (int pi: v->ports())
          {
            const auto& p=m.ports[pi];
            model.ports.push_back(schema1::Port(pi, p));
          }
      }
    for (int i: g.groups())
      { 
        const auto& g=m.groupItems[i];
        model.groups.push_back(schema1::Group(i, g));
        model.groups.back().addItems(g);
        layout.push_back(make_shared<schema1::GroupLayout>(i, g));
        dynamic_pointer_cast<VisibilityLayout>(layout.back())->visible=visible;
        populateWith(m,g,g.displayContents());
      }
    // TODO: godley and plots when they're added to groups
  }


  void Minsky::relocateCanvas()
  {
    // ignore group items, as these coordinates are relative to group centre
    set<int> groupItems;
    for (vector<Group>::iterator i=model.groups.begin(); 
         i<model.groups.end(); ++i)
      groupItems.insert(i->items.begin(), i->items.end());

    // compute min values of coordinates
    float xmin=numeric_limits<float>::max(), 
      ymin=numeric_limits<float>::max();
    for (size_t i=0; i<layout.size(); ++i)
      {
        if (groupItems.count(layout[i]->id)) continue;
        if (PositionLayout* p=dynamic_cast<PositionLayout*>(layout[i].get()))
          {
            if (p->x<xmin) xmin=p->x;
            if (p->y<ymin) ymin=p->y;
          }
        else if (WireLayout* w=dynamic_cast<WireLayout*>(layout[i].get()))
          {
            for (size_t i=0; i<w->coords.size(); i+=2)
              if (w->coords[i]<xmin) xmin=w->coords[i];
            for (size_t i=1; i<w->coords.size(); i+=2)
              if (w->coords[i]<ymin) ymin=w->coords[i];
          }
      }
    // move all items be (-xmin, -ymin)
    for (size_t i=0; i<layout.size(); ++i)
      {
        if (groupItems.count(layout[i]->id)) continue;
        if (PositionLayout* p=dynamic_cast<PositionLayout*>(layout[i].get()))
          {
            p->x-=xmin;
            p->y-=ymin;
          }
        else if (WireLayout* w=dynamic_cast<WireLayout*>(layout[i].get()))
          {
            for (size_t i=0; i<w->coords.size(); i+=2)
              w->coords[i]-=xmin;
            for (size_t i=1; i<w->coords.size(); i+=2)
              w->coords[i]-=ymin;
          }
      }
  }

  namespace
  {
    struct IsOrphan
    {
      set<int> ids;
      void insert(int id) {ids.insert(id);}
      bool operator()(const Variable& v) {
        return ids.count(v.id);
      }
      bool operator()(const shared_ptr<Layout>& l) {
        return l && ids.count(l->id);
      }
    };
  }

  void Minsky::removeIntVarOrphans()
  {
    set<string> intNames;
    for (vector<Operation>::const_iterator o=model.operations.begin(); 
         o!=model.operations.end(); ++o)
      if (o->type==minsky::OperationType::integrate)
        intNames.insert(o->name);

    IsOrphan isOrphan;
    for (vector<Variable>::const_iterator v=model.variables.begin();
         v!=model.variables.end(); ++v)
      // an orphaned variable is an integral variable not attached to an integral and without
      if (v->type==VariableType::integral && intNames.count(v->name)==0)
        isOrphan.insert(v->id);

    model.variables.erase
      (remove_if(model.variables.begin(), model.variables.end(), isOrphan), 
       model.variables.end());
    layout.erase
      (remove_if(layout.begin(), layout.end(), isOrphan), layout.end());
  }

}

namespace classdesc
{
  template<> Factory<schema1::Item,string>::Factory() {}
  template<> Factory<schema1::Layout,string>::Factory() 
  {enumerateRegisterLayout(*this);}
}

namespace schema1
{
  using classdesc::Factory;

  struct ItemFactory: public Factory<schema1::Item,string>
  {
    ItemFactory() {enumerateRegisterItems(*this);}
    using Factory<schema1::Item,string>::registerType;
    // define our own registerType that avoids calling type()
    template <class T> void registerType()
    {registerType<T>(classdesc::typeName<T>());}
  };

  ItemFactory itemFactory;
  Factory<schema1::Layout,string> factoryForLayout;


  template <> std::auto_ptr<Item> factory<Item>(const std::string& name)
  {
    return auto_ptr<Item>(itemFactory.create(name));
  }
  template <> std::auto_ptr<Layout> factory<Layout>(const std::string& name)
  {
    return auto_ptr<Layout>(factoryForLayout.create(name));
  }

  Layout* Layout::create(const string& name)
  {
    return factoryForLayout.create(name);
  }
}

