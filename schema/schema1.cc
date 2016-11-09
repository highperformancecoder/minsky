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
          if (!items.insert(i->id).second) 
            return false;
        return true;
      }
    };

    // internal type for associating layout data with model data
    class Combine
    {
      typedef map<int, UnionLayout> Layouts;
      Layouts layout;
    public:
      Combine(const vector<shared_ptr<Layout> >& l) {
        for (size_t i=0; i<l.size(); ++i)
          layout.insert(make_pair(l[i]->id, UnionLayout(*l[i])));
      }

      /// combine model and layout data
      void combine(minsky::Item&, const Item&) const;
      void combine(minsky::Wire& x, const Wire& y) const;
      void combine(minsky::OperationBase& x, const Operation& y) const;
      void combine(minsky::VariableBase& x, const Variable& y) const;
      void combine(minsky::PlotWidget& x, const Plot& y) const;
      void combine(minsky::GodleyIcon& x, const Godley& y) const;
      void combine(minsky::Group& x, const Group& y) const;

      /// populate the usual integer-based map from a vector of schema data
      template <class M, class U>
      void populate(M& m, const vector<U>& v) const
      {
        for (typename vector<U>::const_iterator i=v.begin(); i!=v.end(); ++i)
          combine(m[i->id], *i);
      }


      /// populate the GroupItem  from a vector of schema data
      void populate
      (map<int, minsky::Group>& m, const vector<Group>& v) const;

    };

    void Combine::combine(minsky::Item& x, const Item& y) const
    {
      x.detailedText=y.detailedText;
      x.tooltip=y.tooltip;
      auto l=layout.find(y.id);
      if (l!=layout.end())
        {
          //          x.moveTo(l->second.x,l->second.y); 
          x.m_x=l->second.x;
          x.m_y=l->second.y;
          x.m_visible=l->second.visible;
          x.rotation=l->second.rotation;
        }
    }

    void Combine::combine(minsky::Wire& x, const Wire& y) const
    {
      Layouts::const_iterator l=layout.find(y.id);
      if (l!=layout.end())
          x.coords(l->second.coords);
    }
    
    void Combine::combine(minsky::OperationBase& x, const Operation& y) const
    {
      combine(static_cast<minsky::Item&>(x), y);
      if (auto c=dynamic_cast<minsky::Constant*>(&x))
        c->value=y.value;
      if (auto d=dynamic_cast<minsky::DataOp*>(&x))
        {
          d->data=y.data;
          d->description=y.name;
        }
    }

    void Combine::combine(minsky::VariableBase& x, const Variable& y) const
    {
      combine(static_cast<minsky::Item&>(x), y);
      x.name(y.name);
      x.init(y.init);
      auto l=layout.find(y.id);
      if (l!=layout.end())
        {
          x.sliderVisible=l->second.sliderVisible;
          x.sliderBoundsSet=l->second.sliderBoundsSet;
          x.sliderStepRel=l->second.sliderStepRel;
          x.sliderMin=l->second.sliderMin;
          x.sliderMax=l->second.sliderMax;
          x.sliderStep=l->second.sliderStep;
        }
    }

    void Combine::combine(minsky::PlotWidget& x, const Plot& y) const
    {
      combine(static_cast<minsky::Item&>(x), y);
      x.legend=y.legend.get();
      if (y.legend)
        x.legendSide=*y.legend;
      x.title=y.title;
      x.xlabel=y.xlabel;
      x.ylabel=y.ylabel;
      x.y1label=y.y1label;
          x.logx=y.logx;
          x.logy=y.logy;
      auto l=layout.find(y.id);
      if (l!=layout.end())
        {
          x.width=l->second.width;
          x.height=l->second.height;
        }
    }

    void Combine::combine(minsky::GodleyIcon& x, const Godley& y) const
    {
      combine(static_cast<minsky::Item&>(x), y);
      x.table.setDEmode(y.doubleEntryCompliant);
      x.table.title=y.name;
      SchemaHelper::setPrivates(x.table, y.data, y.assetClasses);
      x.zoomFactor=y.zoomFactor;
    }

    void Combine::combine(minsky::Group& x, const Group& y) const
    {
      combine(static_cast<minsky::Item&>(x), y);
      x.title=y.name;
//        for (auto cv=y.model.createdVars)
//          x.createdVars.push_back(imap[cv]);
      auto l=layout.find(y.id);
      if (l!=layout.end())
        {
          x.width=l->second.width;
          x.height=l->second.height;
          x.displayZoom=l->second.displayZoom;
        }
    }

    template <class T> shared_ptr<Layout> layoutFactory(int id, const T&);

    template <class K, class V> 
    shared_ptr<Layout> layoutFactory(int id, const minsky::IntrusiveWrap<K,V>& x)
    {return layoutFactory(id, static_cast<const V&>(x));}

//    template <> shared_ptr<Layout> layoutFactory
//    (int id, const minsky::Wire& w)
//    {return shared_ptr<Layout>(new WireLayout(id,w));}

//    template <> shared_ptr<Layout> layoutFactory
//    (int id, const minsky::VariablePtr& v)
//    {return shared_ptr<Layout>(new SliderLayout(id,*v));}

//    template <> shared_ptr<Layout> layoutFactory
//    (int id, const minsky::OperationBase& o)
//    {
//      if (const minsky::Constant* c=dynamic_cast<const minsky::Constant*>(&o))
//        return shared_ptr<Layout>(new SliderLayout(id,*c));
//      else
//        return shared_ptr<Layout>(new ItemLayout(id,o));
//    }

//    template <> shared_ptr<Layout> layoutFactory
//    (int id, const minsky::GodleyIcon& g)
//    {return shared_ptr<Layout>(new PositionLayout(id,g));}

//    template <> shared_ptr<Layout> layoutFactory
//    (int id, const minsky::PlotWidget& p)
//    {return shared_ptr<Layout>(new PlotLayout(id,p));}

//    template <> shared_ptr<Layout> layoutFactory
//    (int id, const minsky::Group& g)
//    {return shared_ptr<Layout>(new GroupLayout(id,g));}
  }

  Operation::Operation(int id, const minsky::OperationBase& op): 
    Item(id,op), type(op.type()), value(0), intVar(-1) 
  {
    if (const minsky::Constant* c=dynamic_cast<const minsky::Constant*>(&op))
       value=c->value;
    else if (const minsky::IntOp* i=dynamic_cast<const minsky::IntOp*>(&op))
      {
        name=i->intVar->fqName();
        // intvar is populated at a higher level
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
    Combine combine(layout);
    for (auto& i: model.notes)
      {
        auto it=imap.addItem(new minsky::Item, i);
        combine.combine(*it,i);
      }
    for (auto& i: model.variables)
      {
        auto v=imap.addItem(minsky::VariableBase::create(i.type), i);
        combine.combine(*v,i);
        pmap.asgPorts(v->ports, i.ports);
      }
    // operations need to be after variables to allow integration
    // variables to be attached
    for (auto& i: model.operations)
      {
        auto o=imap.addItem(minsky::OperationBase::create(i.type), i);
        combine.combine(*o,i);
        if (auto d=dynamic_cast<minsky::DataOp*>(o))
          d->data=i.data;
        else if (auto integ=dynamic_cast<minsky::IntOp*>(o))
          {
            // this ensures that the output port refers to this item,
            // and not the variable that is deleted in the following
            // statement
            if (integ->coupled()) integ->toggleCoupled();
            g.removeItem(*integ->intVar);
            integ->intVar=imap[i.intVar];
          }
        pmap.asgPorts(o->ports, i.ports);
#ifndef NDEBUG
        // check the output port connects to correct item
        if (auto integ=dynamic_cast<minsky::IntOp*>(o))
          assert(integ->coupled() && &integ->ports[0]->item==integ->intVar.get() ||
                 !integ->coupled() && &integ->ports[0]->item==integ);
        else
          assert(o == &o->ports[0]->item);
#endif
      }
    for (auto& i: model.plots)
      {
        auto p=imap.addItem(new minsky::PlotWidget, i);
        combine.combine(*p,i);
        pmap.asgPorts(p->ports, i.ports);
      }
    for (auto& i: model.switches)
      {
        auto s=imap.addItem(new minsky::SwitchIcon, i);
        combine.combine(*s,i);
        s->setNumCases(i.ports.size()-2);
        pmap.asgPorts(s->ports, i.ports);
      }
    for (auto& i: model.godleys)
      {
        auto s=imap.addItem(new minsky::GodleyIcon, i);
        combine.combine(*s,i);
        //        pmap.asgPorts(s->ports, i.ports);
        for (auto p: i.ports)
          {
            if (auto v=dynamic_pointer_cast<minsky::VariableBase>
                (g.findItem(pmap[p]->item)))
              {
                if (v->isStock())
                  s->stockVars.push_back(v);
                else
                  s->flowVars.push_back(v);
              }
          }
        s->update();
      }
    for (auto& i: model.groups)
        {
          auto g=imap.addItem(new minsky::Group, i);
          combine.combine(*g, i);
        }

    /// process wires after all the items have been defined
    for (auto& i: model.wires)
      combine.combine(*g.addWire(new minsky::Wire(pmap[i.from],pmap[i.to])), i);

    /// groups needs to be processed last so that all references are defined
    for (auto& i: model.groups)
      {
        if (auto gg=dynamic_cast<minsky::Group*>(imap[i.id].get()))
          {
            for (auto id: i.items)
              if (imap.count(id))
                {
                  // item will be moved to new group, and wires
                  // adjusted. Position needs to be adjusted by group
                  // origin, though, because the schema saved values
                  // are group relative
                  auto it=imap[id];
                  it->moveTo(it->x()+gg->x(), it->y()+gg->y());
                  gg->addItem(it);
                  assert(gg->uniqueItems());
                }
            // process ports list to populate io variables
            for (int p: i.ports)
              if (pmap[p])
                if (minsky::VariablePtr v=gg->findItem(pmap[p]->item))
                  {
                    if (pmap[p]->input())
                      gg->inVariables.push_back(v);
                    else
                      gg->outVariables.push_back(v);
                  }
          }
      }
  }


  Minsky::operator minsky::Minsky() const
  {
    if (!model.validate())
      throw ecolab::error("inconsistent Minsky model");

    minsky::Minsky m;
    minsky::LocalMinsky lm(m);
    populateGroup(*m.model);
    
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

//    void addOffset(vector<Port>& v, int offs)
//    {
//      for (vector<Port>::iterator i=v.begin(); i!=v.end(); ++i)
//        i->id+=offs;
//    }

//    void addOffset(vector<Wire>& v, int offs)
//    {
//      for (vector<Wire>::iterator i=v.begin(); i!=v.end(); ++i)
//        {
//          i->id+=offs;
//          i->from+=offs;
//          i->to+=offs;
//        }
//    }
//    void addOffset(vector<Variable>& v, int offs)
//    {
//      for (vector<Variable>::iterator i=v.begin(); i!=v.end(); ++i)
//        {
//          i->id+=offs;
//          for (size_t j=0; j<i->ports.size(); ++j)
//            i->ports[j]+=offs;
//          // if variable is scoped, then offset its scope also
//          int scope;
//          vector<char> uqName(i->name.size()+1);
//          if (sscanf(i->name.c_str(), "[%d]:%s", &scope, &uqName[0])==2)
//            i->name="["+to_string(scope+offs)+"]:"+&uqName[0];
//        }
//    }

//    void addOffset(vector<Operation>& v, int offs)
//    {
//      for (vector<Operation>::iterator i=v.begin(); i!=v.end(); ++i)
//        {
//          i->id+=offs;
//          for (size_t j=0; j<i->ports.size(); ++j)
//            i->ports[j]+=offs;
//          if (i->intVar>=0) i->intVar+=offs;
//        }
//    }

//    void addOffset(vector<Group>& v, int offs)
//    {
//      for (vector<Group>::iterator i=v.begin(); i!=v.end(); ++i)
//        {
//          i->id+=offs;
//          for (size_t j=0; j<i->items.size(); ++j)
//            i->items[j]+=offs;
//          for (size_t j=0; j<i->ports.size(); ++j)
//            i->ports[j]+=offs;
//          for (size_t j=0; j<i->createdVars.size(); ++j)
//            i->createdVars[j]+=offs;
//        }
//    }

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
      Combine combine;
      int nextId=0;
      vector<shared_ptr<Layout> >& l;
      MinskyModel& m;
      PopulateMinsky(Minsky& m): combine(m.layout), l(m.layout), m(m.model) {}
      
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
        const char* what() const noexcept override 
        {return "duplicate item processed";}
      };

      void processGroup(Group& s1g, const minsky::Group& g)
      {
        vector<pair<size_t,minsky::Item*>> integralItemsToAssociate;
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
                if (auto i=dynamic_cast<minsky::IntOp*>(x))
                  integralItemsToAssociate.emplace_back(m.operations.size()-1, i->intVar.get());
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

        // fix up integral variable associations
        for (auto& i: integralItemsToAssociate)
          {
            assert(m.operations[i.first].type==minsky::OperationType::integrate);
            m.operations[i.first].intVar=itemMap[i.second];
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

        // I/O variable information is stashed as "ports" in the schema
        for (auto& v: g.inVariables)
          for (auto& p: v->ports)
            if (p->input())
              s1g.ports.push_back(portId(p.get()));
        for (auto& v: g.outVariables)
          for (auto& p: v->ports)
            if (!p->input())
              s1g.ports.push_back(portId(p.get()));
        // TODO - created variables??
//        for (auto& v: g.createdVariables)
//          s1g.createdVariables.push_back(itemMap[v.get()]);
        s1g.name=g.title;
       }
    };
  }

  Minsky::Minsky(const minsky::Group& g)
  {
    Group topLevel;
    // need to reserve enough memory on the group vector to prevent groups being moved
    size_t cnt=0;
    g.recursiveDo(&minsky::GroupItems::groups, 
                  [&](const minsky::Groups&,minsky::Groups::const_iterator) {
                    cnt++; return false;
                  });
    model.groups.reserve(cnt);

    PopulateMinsky(*this).processGroup(topLevel,g);
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


  template <> unique_ptr<Item> factory<Item>(const string& name)
  {
    return unique_ptr<Item>(itemFactory.create(name));
  }
  template <> unique_ptr<Layout> factory<Layout>(const string& name)
  {
    return unique_ptr<Layout>(factoryForLayout.create(name));
  }

  Layout* Layout::create(const string& name)
  {
    return factoryForLayout.create(name);
  }
}

