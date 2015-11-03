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
#include "schema0.h"
#include "schemaHelper.h"
#include "str.h"
#include <ecolab_epilogue.h>
#include <fstream>
using namespace ecolab;
using namespace classdesc;
using namespace std;

namespace schema0
{
  using minsky::SchemaHelper;

  template <class K, class V1, class V2>
  void asg(minsky::IntrusiveMap<K,V1>& m1, const std::map<K,V2>& m2)
  {
    for (const typename std::map<K,V2>::value_type& x: m2)
      m1.insert
        (typename minsky::IntrusiveMap<K,V1>::value_type(x.first, x.second));
  }
 
  void asg(minsky::Plots& x, const std::map<std::string, PlotWidget>& y)
  {
    for (auto i=y.begin(); i!=y.end(); ++i)
      x.insert(minsky::minsky().getNewId(), minsky::PlotWidget(i->second));
  }

  template <class T, class T1>
  void asg(minsky::PMMap<T>& x, const std::map<int, T1>& y) {
    SchemaHelper::asg(x,y);
  }


  // like assignment between containers, but value_types needn't be identical
  template <class C1, class C2>
  void asg(C1& c1, const C2& c2)
  {
    C1 tmp(c2.begin(), c2.end());
    tmp.swap(c1);
  }

  template <>
  void asg(minsky::GroupIcons& m1, const Minsky::GroupIcons& m2)
  {
    for (Minsky::GroupIcons::const_iterator i=m2.begin(); i!=m2.end(); ++i)
      m1[i->first]=i->second;
  }
 
  Operation::operator minsky::OperationPtr() const 
  {
    minsky::OperationPtr op(m_type, m_ports);
    op->m_x=x;
    op->m_y=y;
    // handle a previous schema change
    string desc=m_description.empty()? description: m_description;
    switch (m_type)
      {
      case minsky::OperationType::constant:
        if (minsky::Constant* c=dynamic_cast<minsky::Constant*>(op.get()))
          {
            c->description=desc;
            c->value=value;
            c->sliderVisible=sliderVisible;
            c->sliderBoundsSet=sliderBoundsSet;
            c->sliderStepRel=sliderStepRel;
            c->sliderMin=sliderMin;
            c->sliderMax=sliderMax;
            c->sliderStep=sliderStep;
          }
        break;
      case minsky::OperationType::integrate:
        if (minsky::IntOp* i=dynamic_cast<minsky::IntOp*>(op.get()))
          minsky::SchemaHelper::setPrivates(*i, desc, intVar);
        break;
      case minsky::OperationType::multiply: case minsky::OperationType::divide: 
        SchemaHelper::makePortMultiWired(minsky::minsky().ports[m_ports[1]]);
        SchemaHelper::makePortMultiWired(minsky::minsky().ports[m_ports[2]]);
        break;
      default: break;
      }

    op->rotation=rotation;
    op->visible=visible;
    // deal with an older schema
    return op;
  }

  VariablePtr::operator minsky::VariablePtr() const
  {
    minsky::VariablePtr v(m_type, name);
    v->setScope(-1);
    v->moveTo(x,y);
    v->init(init);
    v->rotation=rotation;
    v->visible=visible && !m_godley;
    minsky::SchemaHelper::setPrivates
      (dynamic_cast<minsky::VariablePorts&>(*v), m_outPort, m_inPort);
    return v;
  }

  VariableValue::operator minsky::VariableValue() const
  {
    minsky::VariableValue v(m_type, "", init);
    v.godleyOverridden=godleyOverridden;
    return v;
  }

  VariableManager::operator minsky::VariableManager() const
  {
    minsky::VariableManager vm;
    for (const value_type& v: *this)
      static_cast<minsky::IntrusiveMap<int,minsky::VariablePtr>&>(vm).
        insert(v.first, minsky::VariablePtr(v.second));
    // construct a translation table between variable descriptions and their valueId
    map<string,string> valueId;
    for (minsky::VariableManager::value_type i: vm)
      valueId[i->name()]=i->valueId();

    // rewrite values and wired variables
    set<string> wV;
    for (set<string>::iterator i=wiredVariables.begin(); i!=wiredVariables.end(); ++i)
      wV.insert(valueId[*i]);
    minsky::SchemaHelper::setPrivates(vm, wV, portToVariable);
    
    // rewrite and load values map
    for (VariableValues::const_iterator i=values.begin(); i!=values.end(); ++i)
      {
        vm.values[valueId[i->first]] = i->second;
        vm.values[valueId[i->first]].name = ":"+i->first;
      }
    return vm;
  }

  GodleyTable::operator minsky::GodleyTable() const
  {
    minsky::GodleyTable g;

    minsky::SchemaHelper::setPrivates(g, data, m_assetClass);
    g.doubleEntryCompliant=doubleEntryCompliant;
    g.title=title;
    return g;
  }

  GodleyIcon::operator minsky::GodleyIcon() const
  {
    minsky::GodleyIcon g;
    g.moveTo(x, y);
    //g.scale=scale;
    asg(g.flowVars, flowVars);
    asg(g.stockVars, stockVars);
    g.table=table;
    return g;
  }

  GroupIcon::operator minsky::GroupIcon() const
  {
    minsky::GroupIcon g;
    minsky::SchemaHelper::setPrivates
      (g, operations, variables, wires, vector<int>(), inVariables, outVariables);
    minsky::SchemaHelper::setXY(g, x, y);
    g.setName(name);
    g.width=width;
    g.height=height;
    g.rotation=rotation;
    return g;
  }

  void GroupIcon::updateEdgeVariables(const VariableManager& vm)
  {
    inVariables.clear();
    outVariables.clear();
    // for each edge port, we need to determine the edgeVariable
    // associated with it, and whether it is an input or output edge
    // variable
    for (size_t p=0; p<m_ports.size(); ++p)
      {
        map<int,int>::const_iterator vid=vm.portToVariable.find(m_ports[p]);
        if (vid!=vm.portToVariable.end())
          {
            VariableManager::Variables::const_iterator v=vm.find(vid->second);
            if (v!=vm.end())
              {
                if (v->second.m_inPort==m_ports[p])
                  inVariables.push_back(v->first);
                else if (v->second.m_outPort==m_ports[p])
                  outVariables.push_back(v->first);
              }
          }
      }
  }


  PlotWidget::operator minsky::PlotWidget() const
  {
    minsky::PlotWidget pw;
    pw.nxTicks=nxTicks;
    pw.nyTicks=nyTicks;
    pw.fontScale=fontScale;
    pw.offx=offx;
    pw.offy=offy;
    pw.logx=logx;
    pw.logy=logy;
    pw.grid=grid;
    pw.leadingMarker=leadingMarker;
    pw.autoscale=autoscale;
    pw.plotType=plotType;
    pw.minx=minx;
    pw.maxx=maxx;
    pw.miny=miny;
    pw.maxy=maxy;
    SchemaHelper::setPrivates(pw, ports);
    asg(pw.yvars,yvars);
    asg(pw.xvars,xvars);
    pw.xminVar=xminVar;
    pw.xmaxVar=xmaxVar;
    pw.yminVar=yminVar;
    pw.ymaxVar=ymaxVar;
    pw.moveTo(x,y);
    return pw;
  }

  Plots::operator minsky::Plots() const
  {
    minsky::Plots p;
    asg(p,plots);
    return p;
  }

  Minsky::Minsky(): stepMin(0), stepMax(1), nSteps(1),
                    epsAbs(1e-3), epsRel(1e-2) {}

  Minsky::operator minsky::Minsky() const
  {
    minsky::Minsky m;
    // override default minsky object for this method
    minsky::LocalMinsky lm(m);
    
    asg(m.ports, ports);
    asg(m.wires, wires);
    asg(m.godleyItems, godleyItems);
    m.variables=variables;
    for (const Operations::value_type& o: operations)
      m.operations.insert(o.first, minsky::OperationPtr(o.second));
    asg(m.groupItems, groupItems);
    // schema1 has old style plots - these need to be reinserted manually
    //    m.plots=plots;
    m.stepMin=stepMin;
    m.stepMax=stepMax;
    m.nSteps=nSteps;
    m.epsAbs=epsAbs;
    m.epsRel=epsRel;
    m.resetNextId();
    return m;
  }



  void Minsky::load(const char* filename)
  {
    ifstream inf(filename);
    xml_unpack_t saveFile(inf);
    saveFile >> *this;

    // if a godley table is present, and no godley icon present, copy
    // into godleyItems, to support XML migration
    if (godleyItems.empty() && godley.rows()>2)
      {
        godleyItems[0].table=godley;
        godleyItems[0].x=godleyItems[0].y=10;
      }
    
    map<int, xml_conversions::GodleyIcon> gItems;
    xml_unpack(saveFile,"root.godleyItems", gItems);
    
    for (GodleyItems::iterator g=godleyItems.begin(); g!=godleyItems.end(); ++g)
      if (g->second.flowVars.empty() && g->second.stockVars.empty())
        {
          xml_conversions::GodleyIcon& gicon=gItems[g->first];
          GodleyIcon& gi=g->second;
          asg(gi.flowVars, gicon.flowVars);
          asg(gi.stockVars, gicon.stockVars);
        }
    
    for (size_t i=0; i<groupItems.size(); ++i)
      groupItems[i].updateEdgeVariables(variables);

    removeIntVarOrphans();
  }

  namespace
  {
    struct IsOrphan
    {
      set<string> ids;
      void insert(const string& name) {ids.insert(name);}
      bool operator()(const VariableManager::value_type& v) {
        return ids.count(v.second.name);
      }
      bool operator()(const VariableManager::VariableValues::value_type& l) {
        return ids.count(l.first);
      }
    };
  }

  void Minsky::removeIntVarOrphans()
  {
    set<string> intNames;
    for (Operations::const_iterator o=operations.begin(); 
         o!=operations.end(); ++o)
      if (o->second.m_type==minsky::OperationType::integrate)
        {
          if (!o->second.m_description.empty())
            intNames.insert(o->second.m_description);
          else
            if (!o->second.description.empty())
              intNames.insert(o->second.description);
        }

    IsOrphan isOrphan;
    for (VariableManager::const_iterator v=variables.begin();
         v!=variables.end(); ++v)
      // an orphaned variable is an integral variable not attached to an integral and without
      if (v->second.m_type==VariableType::integral && 
          intNames.count(v->second.name)==0)
        isOrphan.insert(v->second.name);

    for (VariableManager::iterator i=variables.begin();
         i!=variables.end();)
      {
        VariableManager::iterator j=i++;
        if (isOrphan(*j)) variables.erase(j);
      }

    for (VariableManager::VariableValues::iterator i=variables.values.begin();
         i!=variables.values.end();)
      {
        VariableManager::VariableValues::iterator j=i++;
        if (isOrphan(*j)) variables.values.erase(j);
      }

  }


}
