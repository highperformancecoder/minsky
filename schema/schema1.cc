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
#include "minsky_epilogue.h"
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
          {
            bool ok=items.insert(i->id).second;
            assert(ok);
            if (!ok) 
              return false;
          }
        return true;
      }
    };

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
      {
        assert(portIds.find(w.from)!=portIds.end() && portIds.find(w.to)!=portIds.end());
        if (portIds.find(w.from)==portIds.end() || portIds.find(w.to)==portIds.end())
          return false;
      }

    // check that ids are unique
    Validate v;
    return v.check(wires) && v.check(operations) && 
      v.check(variables) && v.check(groups) && v.check(godleys);
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
      if (v->type==minsky::VariableType::integral && intNames.count(v->name)==0)
        isOrphan.insert(v->id);

    model.variables.erase
      (remove_if(model.variables.begin(), model.variables.end(), isOrphan), 
       model.variables.end());
    layout.erase
      (remove_if(layout.begin(), layout.end(), isOrphan), layout.end());
  }

  Minsky::Minsky(const schema0::Minsky& m)
  {
    // uniquify all IDs
    int nextId=0;
    map<int,int> newPortIds;
    map<int,int> newOpIds;
    map<int,int> newVarIds;

    for (auto& i: m.ports)
      newPortIds[i.first]=nextId++;
    for (auto& i: m.wires)
      {
        model.wires.emplace_back(nextId, newPortIds[i.second.from], newPortIds[i.second.to]);
        layout.emplace_back(new WireLayout(nextId++, i.second));
      }
    for (auto v: m.variables)
      {
        newVarIds[v.first]=nextId;
        v.second.m_outPort=newPortIds[v.second.m_outPort];
        if (v.second.m_inPort>=0)
          v.second.m_inPort=newPortIds[v.second.m_inPort];
        // values init field overrides that of the variable's
        auto value=m.variables.values.find(v.second.name);
        if (value!=m.variables.values.end())
          v.second.init=value->second.init;
        model.variables.emplace_back(nextId, v.second);
        layout.emplace_back(new ItemLayout(nextId++, v.second));
      }
    for (auto i: m.operations)
      {
        newOpIds[i.first]=nextId;
        for (auto& j: i.second.m_ports)
          j=newPortIds[j];
        if (i.second.intVar>-1) i.second.intVar=newVarIds[i.second.intVar];
        model.operations.emplace_back(nextId, i.second);
        layout.emplace_back(new ItemLayout(nextId++, i.second));
      }

    for (auto i: m.plots.plots)
      {
        for (auto& j: i.second.ports)
          j=newPortIds[j];
        // insert another 6 ports for axis ranges that weren't there in schema 0
        // move first 4 ports (x ports) to end, and add anothe 4 x ports
        i.second.ports=(pcoord(6)+nextId)<<
          i.second.ports[pcoord(8)+4]<<
          i.second.ports[pcoord(4)]<<
          (pcoord(4)+nextId+6);
        nextId+=10;
        model.plots.emplace_back(nextId, i.second);
        layout.emplace_back(new PlotLayout(nextId++, i.second));
      }

    for (auto g: m.groupItems)
      {
        for (auto& i: g.second.operations)
          i=newOpIds[i];
        for (auto& i: g.second.variables)
          i=newVarIds[i];
        for (auto& j: g.second.m_ports)
          j=newPortIds[j];
        // wires not used in new schema, no need to renumber
        model.groups.emplace_back(nextId, g.second);
        layout.emplace_back(new GroupLayout(nextId++, g.second));
      }

    for (auto g: m.godleyItems)
      {
        // need to renumber ports in variables
        for (auto& v: g.second.flowVars)
          v.m_inPort=newPortIds[v.m_inPort];
        for (auto& v: g.second.stockVars)
          v.m_outPort=newPortIds[v.m_outPort];
        model.godleys.emplace_back(nextId, g.second);
        layout.emplace_back(new PositionLayout(nextId++, g.second.x, g.second.y));
      }
    
    model.rungeKutta.stepMin=m.stepMin;
    model.rungeKutta.stepMax=m.stepMax;
    model.rungeKutta.nSteps=m.nSteps;
    model.rungeKutta.epsAbs=m.epsAbs;
    model.rungeKutta.epsRel=m.epsRel;
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

