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
      minsky::Group& vm;
    public:
      Combine(const vector<shared_ptr<Layout> >& l, minsky::Group& vm): vm(vm) {
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

//      template <class T, class U>
//      void populate(minsky::PMMap<T>& m, const vector<U>& v) const
//      {
//        for (typename vector<U>::const_iterator i=v.begin(); i!=v.end(); ++i)
//          {
//            typename minsky::PMMap<T>::value_type v(i->id);
//            combine(v, *i);
//            SchemaHelper::insert(m,v);
//          }
//      }

      

      /// populate the GroupItem  from a vector of schema data
      void populate
      (map<int, minsky::Group>& m, const vector<Group>& v) const;

//      /// populate the variable manager from a vector of schema data
//      void populate(minsky::VariableManager& vm, const vector<Variable>& v) const;

//      /// populate the plots manager from a vector of schema data
//      void populate(minsky::Plots& plots, const vector<Plot>& v) const;
    };

//    template <>
//    minsky::Port Combine::combine(minsky::Port& p, const Port& pp) const
//    {
//      Layouts::const_iterator l=layout.find(pp.id);
////      if (l!=layout.end())
////        p=minsky::Port(l->second.x, l->second.y, pp.input);
////      else
////        p=minsky::Port(0,0,pp.input);
//      return p;
//    }

    template <>
    minsky::Wire Combine::combine(minsky::Wire& w, const Wire& w1) const
    {
      Layouts::const_iterator l=layout.find(w1.id);
//      if (l!=layout.end())
//        {
//          w.from=w1.from;
//          w.to=w1.to;
//          w.visible=l->second.visible;
//          if (l->second.coords.size()>=4)
//            w.coords(toArray(l->second.coords));
//          else //add straight wire
//            {
//              ecolab::array<float> coords;
//              l=layout.find(w.from);
//              if (l!=layout.end())
//                {
//                  (coords<<=l->second.x)<<=l->second.y;
//                  l=layout.find(w.to);
//                  if (l!=layout.end())
//                    {
//                      (coords<<=l->second.x)<<=l->second.y;
//                      w.coords(coords);
//                    }
//                }
//            }
//        }
      return w;
    }
     
//    void asgNote(minsky::Note& n, const Item& i)
//    {
//      n.detailedText = i.detailedText;
//      n.tooltip = i.tooltip;
//    }     

//    template <> minsky::OpVarBaseAttributes 
//    Combine::combine(minsky::OpVarBaseAttributes& o, const Item& o1) const
//    {
//      asgNote(o, o1);
//      Layouts::const_iterator li=layout.find(o1.id);
//      if (li!=layout.end())
//        {
//          const UnionLayout& l=li->second;
//          o.rotation=l.rotation;
//          o.visible=l.visible;
//          o.m_x=l.x;
//          o.m_y=l.y;
//        }
//      return o;
//    }

    template <> minsky::OperationPtr 
    Combine::combine(minsky::OperationPtr& o, const Operation& o1) const
    {
//      Layouts::const_iterator li=layout.find(o1.id);
//      o=minsky::OperationPtr(o1.type, o1.ports);
//      combine(static_cast<minsky::OpVarBaseAttributes&>(*o), 
//              static_cast<const Item&>(o1));
//      switch (o1.type)
//        {
//        case minsky::OperationType::integrate:
//          SchemaHelper::setPrivates
//            (dynamic_cast<minsky::IntOp&>(*o), o1.name, o1.intVar);
//          break;
//        case minsky::OperationType::add: case minsky::OperationType::subtract: 
//        case minsky::OperationType::multiply: case minsky::OperationType::divide: 
//          SchemaHelper::makePortMultiWired(minsky::minsky().ports[o1.ports[1]]);
//          SchemaHelper::makePortMultiWired(minsky::minsky().ports[o1.ports[2]]);
//          break;
//        case minsky::OperationType::constant:
//          if (minsky::Constant* c=dynamic_cast<minsky::Constant*>(o.get()))
//            {
//              c->value=o1.value;
//              c->description=o1.name;
//            }
//          break;
//        case minsky::OperationType::data:
//          if (minsky::DataOp* d=dynamic_cast<minsky::DataOp*>(o.get()))
//            {
//              d->data=o1.data;
//              d->description=o1.name;
//            }
//          break;
//        default: break;
//        }
//
//      if (li!=layout.end())
//        if (minsky::Constant* c=dynamic_cast<minsky::Constant*>(o.get()))
//          {
//            const UnionLayout& l=li->second;
//            c->sliderVisible=l.sliderVisible;
//            c->sliderBoundsSet=l.sliderBoundsSet;
//            c->sliderStepRel=l.sliderStepRel;
//            c->sliderMin=l.sliderMin;
//            c->sliderMax=l.sliderMax;
//            c->sliderStep=l.sliderStep;
//          }
//      return o;
    }
   
    template <> minsky::VariablePtr 
    Combine::combine(minsky::VariablePtr& v, const Variable& v1) const
    {
//      assert(v);
//      combine(static_cast<minsky::OpVarBaseAttributes&>(*v), 
//              static_cast<const Item&>(v1));
//      Layouts::const_iterator li=layout.find(v1.id);
//      if (li!=layout.end())
//        {
//          const UnionLayout& l=li->second;
//          v->name(v1.name);
//          // any unscoped variables are considered global
//          if (v1.name.find(":")==string::npos)
//            v->setScope(-1);
//          v->init(v1.init);
//          v->sliderVisible=l.sliderVisible;
//          v->sliderBoundsSet=l.sliderBoundsSet;
//          v->sliderStepRel=l.sliderStepRel;
//          v->sliderMin=l.sliderMin;
//          v->sliderMax=l.sliderMax;
//          v->sliderStep=l.sliderStep;
//          int out=v1.ports.size()>0? v1.ports[0]: -1;
//          int in=v1.ports.size()>1? v1.ports[1]: -1;
//          SchemaHelper::setPrivates
//            (dynamic_cast<minsky::VariablePorts&>(*v), out, in);
//        }
//      return v;
    }

//    template <> minsky::SwitchIconPtr 
//    Combine::combine(minsky::SwitchIconPtr& sw, const Switch& s) const
//    {
//      if (!sw) sw.reset(new minsky::SwitchIcon);
//      combine(static_cast<minsky::OpVarBaseAttributes&>(*sw), 
//              static_cast<const Item&>(s));
//      SchemaHelper::setPrivates(*sw,s.ports);
//      return sw;
//    }

    template <> minsky::PlotWidget 
    Combine::combine(minsky::PlotWidget& p, const Plot& p1) const
    {
//      asgNote(p, p1);
//      Layouts::const_iterator li=layout.find(p1.id);
//      if (li!=layout.end())
//        {
//          const UnionLayout& l=li->second;
//          //p.ports().resize(22);
//          vector<int> pports(22);
//          // schema migration fix - handle old 12 port case
//          if (p1.ports.size()==12)
//            {
//              size_t i=0;
//              for (; i<4; ++i)
//                pports[i]=p1.ports[i];
//              // skip the two new control ports
//              for (; i<8; ++i)
//                pports[i+2]=p1.ports[i];
//              // skip the new RHS input ports
//              for (; i<12; ++i)
//                pports[i+6]=p1.ports[i];              
//              // add new RHS scale control ports
//              for (i=4; i<6; ++i)
//                pports[i]=minsky::minsky().addInputPort(); 
//              // add new RHS input ports
//              for (i=10; i<14; ++i)
//                pports[i]=minsky::minsky().addInputPort();
//              // add x input ports for the RHS inputs
//              for (i=18; i<22; ++i)
//                pports[i]=minsky::minsky().addInputPort();
//            }
//          else
//            {
//              assert(pports.size()==p1.ports.size());
//              for (size_t i=0; i<p1.ports.size(); ++i)
//                pports[i]=p1.ports[i];
//            }
//          SchemaHelper::setPrivates(p,pports);
//          p.legend=bool(p1.legend);
//          if (p1.legend)
//            p.legendSide=*p1.legend;
//          p.title=p1.title;
//          p.xlabel=p1.xlabel;
//          p.ylabel=p1.ylabel;
//          p.y1label=p1.y1label;
//          SchemaHelper::setXY(p,l.x,l.y);
//          p.width=l.width;
//          p.height=l.height;
//        }
//      return p;
    }

//    template <> minsky::GroupIcon
//    Combine::combine(minsky::GroupIcon& g, const Group& g1) const
//    {
//      asgNote(g, g1);
//      Layouts::const_iterator li=layout.find(g1.id);
//      if (li!=layout.end())
//        {
//          const UnionLayout& l=li->second;
//          // installing these has to happen later, as at this point,
//          // we just have a list of anonymous items, not what type
//          // they are
//          //          SchemaHelper::setPrivates
//          //            (g, g1.operations, g1.variables, g1.wires, g1.ports);
//          g.setName(g1.name);
//          minsky::SchemaHelper::setXY(g, l.x, l.y);
//          g.width=l.width;
//          g.height=l.height;
//          g.displayZoom=l.displayZoom;
//          g.rotation=l.rotation;
//          g.visible=l.visible;
//          g.createdVars=g1.createdVars;
//        }
//      return g;
//    }

    template <> minsky::GodleyIcon
    Combine::combine(minsky::GodleyIcon& g, const Godley& g1) const
    {
//      asgNote(g, g1);
//      Layouts::const_iterator li=layout.find(g1.id);
//      if (li!=layout.end())
//        {
//          const UnionLayout& l=li->second;
//
//          vector<vector<string> > data(g1.data);
//
//          SchemaHelper::setPrivates(g.table, data, g1.assetClasses);
//          g.table.doubleEntryCompliant=g1.doubleEntryCompliant;
//          g.table.title=g1.name;
//          g.zoomFactor=g1.zoomFactor;
//          // add in variables from the port list. Duplications don't
//          // matter, as update() will fix this.
//          for (size_t i=0; i<g1.ports.size(); ++i)
//            {
//              minsky::VariablePtr v = vm.getVariableFromPort(g1.ports[i]);
//              v->visible=false;
//              switch (v->type())
//                {
//                case VariableType::flow:
//                  g.flowVars.push_back(v);
//                  break;
//                case VariableType::stock:
//                  g.stockVars.push_back(v);
//                  break;
//                default: break;
//                }
//            }
//          try 
//            {
//              g.update();
//            }
//          catch (const std::exception& ex)
//            {
//              // if something is inconsistent about the variables in the table, this can throw,
//              // so ignore it here. The problem should be flagged elsewhere.
//            }
//          g.moveTo(l.x, l.y);
//        }
      return g;
    }
  
//    void Combine::populate
//    (map<int, minsky::GroupIcon>& m, const vector<Group>& v) const
//    {
//      for (vector<Group>::const_iterator i=v.begin(); i!=v.end(); ++i)
//        {
//          minsky::GroupIcon& g = m[i->id];
//          combine(g, *i);
//        }
//    }
//     
//    void Combine::populate(minsky::Plots& plots, const vector<Plot>& v) const
//    {
//      for (vector<Plot>::const_iterator i=v.begin(); i!=v.end(); ++i)
//          combine(plots[i->id], *i);
//    }
//
//    template <> 
//    void Combine::populate(minsky::SwitchIcons& switches, const vector<Switch>& v) const
//    {
//      for (auto& i: v)
//        combine(switches[i.id], i);
//    }
//
//    void Combine::populate(minsky::VariableManager& vm, const vector<Variable>& v) const
//    {
//      for (vector<Variable>::const_iterator i=v.begin(); i!=v.end(); ++i)
//        {
//          auto type=i->type;
//          // old constants are now called parameters
//          if (type==VariableType::constant && i->name!=i->init)
//            type=VariableType::parameter;
//          minsky::VariablePtr v1(type);
//          vm.addVariable(combine(v1, *i), i->id);
//          //vm.setInit(minsky::stripActive(i->name), i->init);
//        }
//    }

//    struct ItemMap: public map<int, int>
//    {
//      void remap(vector<int>& x) {
//        for (size_t i=0; i<x.size(); ++i) x[i]=(*this)[x[i]];
//      }
//    };

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
    (int id, const minsky::Group& g)
    {return shared_ptr<Layout>(new GroupLayout(id,g));}
  }

  Operation::Operation(int id, const minsky::OperationBase& op): 
    Item(id,op), type(op.type()), value(0), intVar(-1) 
  {
    if (const minsky::Constant* c=dynamic_cast<const minsky::Constant*>(&op))
       value=c->value;
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
    for (auto& i: operations)
      for (auto j:i.ports)
        portIds.insert(j);
    for (auto& i: variables)
      for (auto j:i.ports)
        portIds.insert(j);
    for (auto& i: plots)
      for (auto j:i.ports)
        portIds.insert(j);
    for (auto& i: groups)
      for (auto j:i.ports)
        portIds.insert(j);
    for (auto& i: switches)
      for (auto j:i.ports)
        portIds.insert(j);

    for (auto& w: wires)
      if (portIds.find(w.from)==portIds.end() || portIds.find(w.to)==portIds.end())
        return false;

    // check that ids are unique
    Validate v;
    return v.check(wires) && v.check(operations) && 
      v.check(variables) && v.check(groups) && v.check(godleys);
  }

  namespace
  {
    void asgItem(minsky::Item& x, const Item& y)
    {
      x.detailedText=y.detailedText;
      x.tooltip=y.tooltip;
    }

    void asgOp(minsky::OperationBase& x, const Operation& y)
    {
      asgItem(x,y);
      if (auto c=dynamic_cast<minsky::Constant*>(&x))
        c->value=y.value;
      if (auto d=dynamic_cast<minsky::DataOp*>(&x))
        d->data=y.data;
    }

    void asgVar(minsky::VariableBase& x, const Variable& y)
    {
      asgItem(x,y);
      x.name(y.name);
      x.init(y.init);
    }
   
    void asgPlot(minsky::PlotWidget& x, const Plot& y)
    {
      asgItem(x,y);
      x.legend=y.legend.get();
      if (y.legend)
        x.legendSide=*y.legend;
      x.title=y.title;
      x.xlabel=y.xlabel;
      x.ylabel=y.ylabel;
      x.y1label=y.y1label;
    }

    void asgGodley(minsky::GodleyIcon& x, const Godley& y)
    {
      asgItem(x,y);
      x.table.setDEmode(y.doubleEntryCompliant);
      x.table.title=y.name;
      SchemaHelper::setPrivates(x.table, y.data, y.assetClasses);
    }

    struct Portmap: public map<int, shared_ptr<minsky::Port> >
    {
      typedef map<int, shared_ptr<minsky::Port> > Super;
      shared_ptr<minsky::Port> operator[](int i) {
        auto r=Super::operator[](i);
        if (!r)
          throw error("undefined port referenced");
        return r;
      }

      void asgPorts(minsky::ItemPortVector& x, const vector<int>& pids)
      {
        for (size_t i=0; i<min(x.size(), pids.size()); ++i)
          {
            auto p=emplace(pids[i],x[i]);
            if (!p.second)
              x[i]=p.first->second;
          }
      }
    };

    struct ItemMap: public map<int,minsky::ItemPtr>
    {
      minsky::Group& g;
      ItemMap(minsky::Group& g): g(g) {}
      template <class T, class SI>
      T* addItem(T* x, const SI& i) {
        if (!emplace(i.id, g.addItem(x)).second)
          throw error("duplicate item ids found");
        return x;
      }
    };
  }

    // TODO combine in layout information
  void Minsky::populateGroup(minsky::Group& g) const
  {
    Portmap pmap;
    ItemMap imap(g);
    for (auto& i: model.notes)
      {
        auto it=imap.addItem(new minsky::Item, i);
        asgItem(*it,i);
      }
    for (auto& i: model.variables)
      {
        auto v=imap.addItem(minsky::VariableBase::create(i.type), i);
        asgVar(*v,i);
        pmap.asgPorts(v->ports, i.ports);
      }
    // operations need to be after variables to allow integration
    // variables to be attached
    for (auto& i: model.operations)
      {
        auto o=imap.addItem(minsky::OperationBase::create(i.type), i);
        asgOp(*o,i);
        pmap.asgPorts(o->ports, i.ports);
        if (auto d=dynamic_cast<minsky::DataOp*>(o))
          d->data=i.data;
        else if (auto integ=dynamic_cast<minsky::IntOp*>(o))
          integ->intVar=imap[i.intVar];
      }
    for (auto& i: model.plots)
      {
        auto p=imap.addItem(new minsky::PlotWidget, i);
        asgPlot(*p,i);
        pmap.asgPorts(p->ports, i.ports);
      }
    for (auto& i: model.switches)
      {
        auto s=imap.addItem(new minsky::SwitchIcon, i);
        asgItem(*s,i);
        pmap.asgPorts(s->ports, i.ports);
      }
    for (auto& i: model.switches)
      {
        auto s=imap.addItem(new minsky::SwitchIcon, i);
        asgItem(*s,i);
        pmap.asgPorts(s->ports, i.ports);
      }
    for (auto& i: model.groups)
        {
        auto g=imap.addItem(new minsky::Group, i);
        //g->name=i.name;
//        for (auto cv=i:model.createdVars)
//          g->createdVars.push_back(imap[cv]);
      }

    /// process wires after all the items have been defined
    for (auto& i: model.wires)
      g.addWire(new minsky::Wire(pmap[i.from],pmap[i.to]));

    /// groups needs to be processed last so that all references are defined
    for (auto& i: model.groups)
      {
        if (auto gg=dynamic_cast<minsky::Group*>(imap[i.id].get()))
          for (auto id: i.items)
            // a bit of a clumsy way of extracting the new id for this item.
            if (auto it=g.findItem(*imap[id]))
              // item will be moved to new group, and wires adjusted
              gg->addItem(it);
      }
  }


  Minsky::operator minsky::Minsky() const
  {
    if (!model.validate())
      throw ecolab::error("inconsistent Minsky model");

    minsky::Minsky m;
    minsky::LocalMinsky lm(m);
    populateGroup(*m.model);
    
//    minsky::Minsky m;
//    Combine c(layout, m.variables);
//    // override default minsky object for this method
//    minsky::LocalMinsky lm(m);
//
    m.model->setZoom(zoomFactor);
    
    m.stepMin=model.rungeKutta.stepMin; 
    m.stepMax=model.rungeKutta.stepMax; 
    m.nSteps=model.rungeKutta.nSteps;   
    m.epsAbs=model.rungeKutta.epsAbs;   
    m.epsRel=model.rungeKutta.epsRel;   
    m.order=model.rungeKutta.order;
    m.simulationDelay=model.rungeKutta.simulationDelay;
    m.implicit=model.rungeKutta.implicit;
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

  namespace
  {
    struct PopulateMinsky
    {
      map<const minsky::Item*,int> itemMap;
      map<const minsky::Port*,int> portMap;
      map<const minsky::Group*,int> groupMap;
      map<const minsky::Wire*,int> wireMap;
      int nextId=0;
      vector<shared_ptr<Layout> >& l;
      MinskyModel& m;
      PopulateMinsky(Minsky& m): m(m.model), l(m.layout) {}
      
      int portId(const minsky::Port* p) {
        auto pmi=portMap.find(p);
        if (pmi==portMap.end())
          return portMap.emplace(p, nextId++).first->second;
        else
          return pmi->second;
      }

      vector<int> ports(const minsky::Item& it) {
        vector<int> r;
        for (auto& p: it.ports)
          r.push_back(portId(p.get()));
        return r;
      }

      struct DuplicateError: public std::exception
      {
        const char* what() {return "duplicate item processed";}
      };

      void processGroup(Group& s1g, const minsky::Group& g)
      {
        for (auto& i: g.items)
          {
            if (itemMap.count(i.get()))
              throw DuplicateError();
            int id=nextId++;
            itemMap[i.get()]=id;
            if (auto x=dynamic_cast<minsky::OperationBase*>(i.get()))
              {
                m.operations.emplace_back(id, *x);
                m.operations.back().ports=ports(*x);
                l.emplace_back(new ItemLayout(id,*x));
              }
            else if (auto x=dynamic_cast<minsky::VariableBase*>(i.get()))
              {
                m.variables.emplace_back(id, *x);
                m.variables.back().ports=ports(*x);
                l.emplace_back(new SliderLayout(id,*x));
              }
            else if (auto x=dynamic_cast<minsky::PlotWidget*>(i.get()))
              {
                m.plots.emplace_back(id, *x);
                m.plots.back().ports=ports(*x);
                l.emplace_back(new PlotLayout(id,*x));
              }
            else if (auto x=dynamic_cast<minsky::SwitchIcon*>(i.get()))
              {
                m.switches.emplace_back(id, *x);
                m.switches.back().ports=ports(*x);
                l.emplace_back(new ItemLayout(id,*x));
              }
            else if (auto x=dynamic_cast<minsky::GodleyIcon*>(i.get()))
              {
                m.godleys.emplace_back(id, *x);
                m.godleys.back().ports=ports(*x);
                l.emplace_back(new ItemLayout(id,*x));
              }
            else
              m.notes.emplace_back(id, *i);
            s1g.items.push_back(id);
          }

        for (auto& i: g.groups)
          {
            if (groupMap.count(&*i))
              throw DuplicateError();
            int id=nextId++;
            groupMap[&*i]=id;
            m.groups.emplace_back(id, *i);
            processGroup(m.groups.back(), *i);
            s1g.items.push_back(id);
            l.emplace_back(new GroupLayout(id,*i));
            //TODO ports and createdVars??
          }
        for (auto& i: g.wires)
          {
            if (wireMap.count(i.get()))
              throw DuplicateError();
            int id=nextId++;
            wireMap[i.get()]=id;
            m.wires.emplace_back(id, *i);
            m.wires.back().from=portId(i->from().get());
            m.wires.back().to=portId(i->to().get());
            s1g.items.push_back(id);
            l.emplace_back(new WireLayout(id,*i));
          }
      }
    };
  }

  Minsky::Minsky(const minsky::Group& g)
  {
    Group topLevel;
    PopulateMinsky(*this).processGroup(topLevel,g);
  }    


//    schemaVersion = version;
//    zoomFactor=m.zoomFactor();
//
//    int id=0; // we're renumbering items
//    ItemMap portMap; // map old ports ids to the new ones
//    set<int> portsUsed; 
//    ItemMap wireMap;
//    ItemMap opMap;
//    ItemMap varMap;
//    ItemMap groupMap;
//    ItemMap noteMap;
//    
//    for (minsky::Minsky::Ports::const_iterator p=m.ports.begin(); 
//         p!=m.ports.end(); ++p, ++id)
//      {
//        model.ports.push_back(Port(id, *p));
//        //        layout.push_back(layoutFactory(id, *p));
//        portMap[p->id()]=id;
//      }
//
//    for (minsky::Minsky::Wires::const_iterator w=m.wires.begin(); 
//         w!=m.wires.end(); ++w, ++id)
//      {
//        model.wires.push_back(Wire(id, *w));
//        model.wires.back().from=portMap[w->from];
//        model.wires.back().to=portMap[w->to];
//        portsUsed.insert(model.wires.back().from);
//        portsUsed.insert(model.wires.back().to);
//        layout.push_back(layoutFactory(id, *w));
//        wireMap[w->id()]=id;
//      }
//
//    for (minsky::VariableManager::const_iterator v=m.variables.begin(); 
//         v!=m.variables.end(); ++v, ++id)
//      {
//        model.variables.push_back(Variable(id, **v));
//        Variable& var=model.variables.back();
//        portMap.remap(var.ports);
//        portsUsed.insert(var.ports.begin(), var.ports.end());
//        layout.push_back(layoutFactory(id, *v));
//        auto valIt=m.variables.values.find((*v)->valueId());
//        if (valIt!=m.variables.values.end())
//          var.init=valIt->second.init;
//        varMap[v->id()]=id;
//      }
//
//    for (minsky::Operations::const_iterator o=m.operations.begin(); 
//         o!=m.operations.end(); ++o, ++id)
//      {
//        model.operations.push_back(Operation(id, **o));
//        Operation& op=model.operations.back();
//        portMap.remap(op.ports);
//        portsUsed.insert(op.ports.begin(), op.ports.end());
//        model.operations.back().intVar=varMap[op.intVar];
//        layout.push_back(layoutFactory(id, **o));
//        opMap[o->id()]=id;
//      }
//
//    for (auto& s: m.switchItems)
//      {
//        model.switches.push_back(Switch(id,*s));
//        layout.emplace_back(new ItemLayout(id,*s));
//        Switch& sw=model.switches.back();
//        portMap.remap(sw.ports);
//        portsUsed.insert(sw.ports.begin(), sw.ports.end());
//        ++id;
//      }
//
//    for (minsky::Minsky::GodleyItems::const_iterator g=m.godleyItems.begin(); 
//         g!=m.godleyItems.end(); ++g, ++id)
//      {
//        model.godleys.push_back(Godley(id, *g));
//        Godley& godley=model.godleys.back();
//        portMap.remap(godley.ports);
//        portsUsed.insert(godley.ports.begin(), godley.ports.end());
//        layout.push_back(layoutFactory(id, *g));
//      }
//
//    for (minsky::GroupIcons::const_iterator g=m.groupItems.begin(); 
//         g!=m.groupItems.end(); ++g, ++id)
//      {
//        model.groups.push_back(Group(id, *g));
//        Group& group=model.groups.back();
//
//        // add in reamapped items 
//        group.items.clear(); // in case this was set during construction
//        const minsky::GroupIcon& g1=*g;
//        for (size_t i=0; i<g1.ports().size(); ++i)
//          group.items.push_back(portMap[g1.ports()[i]]);
//        for (size_t i=0; i<g1.wires().size(); ++i)
//          group.items.push_back(wireMap[g1.wires()[i]]);
//        for (size_t i=0; i<g1.operations().size(); ++i)
//          group.items.push_back(opMap[g1.operations()[i]]);
//        for (size_t i=0; i<g1.variables().size(); ++i)
//          group.items.push_back(varMap[g1.variables()[i]]);
//        for (size_t i=0; i<g1.createdVars.size(); ++i)
//          group.createdVars.push_back(varMap[g1.createdVars[i]]);
//
//        groupMap[g->id()]=id;
//
//        layout.push_back(layoutFactory(id, *g));
//      }
//
//    // loop over variables and remap group references in the var names
//    boost::regex fqNameSpec(R"(\[(\d+)\]:(.*))");
//    boost::smatch match;
//    for (Variable& v: model.variables)
//      if (regex_search(v.name, match, fqNameSpec))
//        {
//          assert(match.size()>=3);
//          v.name="["+minsky::str(groupMap[atoi(match[1].str().c_str())])+"]:"+
//            match[2].str();
//        }
//
//    // second pass to add in the child group ids, now that they're renumbered
//    int j=0;
//    for (const minsky::GroupIcon& g1: m.groupItems)
//      {
//        Group& group=model.groups[j];
//        for (size_t i=0; i<g1.groups().size(); ++i)
//          group.items.push_back(groupMap[g1.groups()[i]]);
//        ++j;
//      }
//
//    for (auto note=m.notes.begin(); note!=m.notes.end(); ++note, ++id)
//      {
//        model.notes.push_back(Item(id, *note));
//        layout.push_back(make_shared<ItemLayout>(id, *note));
//      }
//
//    for (minsky::Plots::const_iterator p=m.plots.begin(); 
//         p!=m.plots.end(); ++p, ++id)
//      {
//        model.plots.push_back(Plot(id, *p));
//        Plot& plot=model.plots.back();
//        portMap.remap(plot.ports);        
//        portsUsed.insert(plot.ports.begin(), plot.ports.end());
//        layout.push_back(layoutFactory(id, *p));
//      }
//
//    // remove unused ports
//    vector<Port> ports;
//    for (vector<Port>::iterator i=model.ports.begin(); 
//         i!=model.ports.end(); ++i)
//      if (portsUsed.count(i->id))
//        ports.push_back(*i);
//    ports.swap(model.ports);
//
//    model.rungeKutta=RungeKutta(m);
//      
//  }

//  Minsky::Minsky(const minsky::Minsky& m, const minsky::Selection& s)
//  {
//    schemaVersion = version;
//    zoomFactor=m.zoomFactor();
//    for (int i: s.wires)
//      {
//        const auto& w=m.wires[i];
//        model.wires.push_back(schema1::Wire(i, w));
//        layout.push_back(make_shared<schema1::WireLayout>(i, w));
//      }
//    for (int i: s.operations)
//      {
//        const auto& o=m.operations[i];
//        model.operations.push_back(schema1::Operation(i, *o));
//        layout.push_back(make_shared<schema1::ItemLayout>(i, *o));
//        for (int pi: o->ports())
//          {
//            const auto& p=m.ports[pi];
//            model.ports.push_back(schema1::Port(pi, p));
//            layout.push_back(make_shared<schema1::PositionLayout>(pi, p));
//          }
//      }
//    for (int i: s.variables)
//      {
//        const auto& v=m.variables[i];
//        model.variables.push_back(schema1::Variable(i, *v));
//        layout.push_back(make_shared<schema1::ItemLayout>(i, *v));
//        for (int pi: v->ports())
//          {
//            const auto& p=m.ports[pi];
//            model.ports.push_back(schema1::Port(pi, p));
//            layout.push_back(make_shared<schema1::PositionLayout>(pi, p));
//          }
//      }
//    for (int i: s.groups)
//      { 
//        const auto& g=m.groupItems[i];
//        model.groups.push_back(schema1::Group(i, g));
//        model.groups.back().addItems(g);
//        layout.push_back(make_shared<schema1::GroupLayout>(i, g));
//        populateWith(m,g,g.displayContents());
//      }
//     for (int i: s.godleys)
//      {
//        const auto& g=m.godleyItems[i];
//        model.godleys.push_back(schema1::Godley(i, g));
//        layout.push_back(make_shared<schema1::PositionLayout>(i, g));
//        // godley ports are just the ports of the underlying variables
//      }
//     for (int i: s.plots)
//      {
//        const auto& p=m.plots[i];
//        model.plots.push_back(schema1::Plot(i, p));
//        layout.push_back(make_shared<schema1::PositionLayout>(i, p));
//        for (int pi: p.ports())
//          {
//            const auto& p=m.ports[pi];
//            model.ports.push_back(schema1::Port(pi, p));
//            layout.push_back(make_shared<schema1::PositionLayout>(pi, p));
//          }
//      }
//
//  }

//  void Minsky::populateWith(const minsky::Minsky& m, const minsky::GroupIcon& g, bool visible)
//  {
//    schemaVersion = version;
//    zoomFactor=m.zoomFactor();
//    for (int i: g.wires())
//      {
//        const auto& w=m.wires[i];
//        model.wires.push_back(schema1::Wire(i, w));
//        layout.push_back(make_shared<schema1::WireLayout>(i, w));
//        dynamic_pointer_cast<VisibilityLayout>(layout.back())->visible=visible;
//      }
//    for (int i: g.operations())
//      {
//        const auto& o=m.operations[i];
//        model.operations.push_back(schema1::Operation(i, *o));
//        layout.push_back(make_shared<schema1::ItemLayout>(i, *o));
//        dynamic_pointer_cast<VisibilityLayout>(layout.back())->visible=visible;
//        for (int pi: o->ports())
//          {
//            const auto& p=m.ports[pi];
//            model.ports.push_back(schema1::Port(pi, p));
//          }
//      }
//    for (int i: g.variables())
//      {
//        const auto& v=m.variables[i];
//        model.variables.push_back(schema1::Variable(i, *v));
//        layout.push_back(make_shared<schema1::ItemLayout>(i, *v));
//        dynamic_pointer_cast<VisibilityLayout>(layout.back())->visible=visible;
//        for (int pi: v->ports())
//          {
//            const auto& p=m.ports[pi];
//            model.ports.push_back(schema1::Port(pi, p));
//          }
//      }
//    for (int i: g.groups())
//      { 
//        const auto& g=m.groupItems[i];
//        model.groups.push_back(schema1::Group(i, g));
//        model.groups.back().addItems(g);
//        layout.push_back(make_shared<schema1::GroupLayout>(i, g));
//        dynamic_pointer_cast<VisibilityLayout>(layout.back())->visible=visible;
//        populateWith(m,g,g.displayContents());
//      }
//    // TODO: godley and plots when they're added to groups
//  }


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

