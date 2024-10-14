 /*
  @copyright Steve Keen 2017
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
#include "dataOp.h"
#include "schema3.h"
#include "sheet.h"
#include "userFunction.h"
#include "minsky_epilogue.h"

using namespace std;

namespace classdesc {template <> Factory<minsky::Item,string>::Factory() {}}


namespace classdesc_access
{
}

namespace schema3
{
  
  // binary serialisation used to serialise the tensorInit field of
  // variableValues into the minsky schema, fixed off here, rather
  // than classdesc generated to ensure backward compatibility
  void pack(classdesc::pack_t& b, const civita::XVector& a)
  {
    b<<a.name<<a.dimension<<uint64_t(a.size());
    for (const auto& i: a)
      b<<civita::str(i,a.dimension.units);
  }

  void unpack(classdesc::pack_t& b, civita::XVector& a)
  {
    uint64_t size;
    std::string x;
    a.clear();
    b>>a.name>>a.dimension>>size;
    for (size_t i=0; i<size; ++i)
      {
        b>>x;
        a.push_back(x);
      }
  }
  
  void pack(classdesc::pack_t& b, const civita::TensorVal& a)
  {
    b<<uint64_t(a.size());
    for (auto i: a)
      b<<i;
    b<<uint64_t(a.index().size());
    for (auto i: a.index())
      b<<uint64_t(i);

    b<<uint64_t(a.hypercube().xvectors.size());
    for (const auto& i: a.hypercube().xvectors)
      pack(b, i);
  }


  void unpack(classdesc::pack_t& b, civita::TensorVal& a)
  {
    uint64_t sz;
    b>>sz;
    vector<double> data;
    for (size_t i=0; i<sz; ++i)
      {
        data.emplace_back();
        b>>data.back();
      }
    b>>sz;
    set<size_t> index;
    for (size_t i=0; i<sz; ++i)
      {
        uint64_t x;
        b>>x;
        index.insert(x);
      }

    b>>sz;
    civita::Hypercube hc;

    for (size_t i=0; i<sz; ++i)
      {
        civita::XVector xv;
        unpack(b,xv);
        hc.xvectors.push_back(xv);
      }
    assert(std::find_if(index.begin(),index.end(),[&](size_t i){return i>=hc.numElements();})==index.end());
    a.index(std::move(index)); //NOLINT
    a.hypercube(std::move(hc)); //dimension data
    assert(a.size()==data.size());
    memcpy(a.begin(),data.data(),data.size()*sizeof(data[0]));
  }

  Optional<classdesc::CDATA> Item::convertTensorDataFromSchema2(const Optional<classdesc::CDATA>& x)
  {
    Optional<classdesc::CDATA> r;
    if (x)
      {
        auto buf=minsky::decode(*x);
        civita::TensorVal tmp;
        schema2::unpack(buf,tmp);
        buf.reseti();
        pack(buf,tmp);
        r=minsky::encode(buf);
      }
    return r;
  }
  
  // map of object to ID, that allocates a new ID on objects not seen before
  struct IdMap: public map<const void*,int>
  {
    int nextId=0;
    set<minsky::RavelLockGroup*> lockGroups;
    int at(const void* o) {
      auto i=find(o);
      if (i==end())
        return emplace(o,nextId++).first->second;
      return i->second;
    }
    int operator[](const void* o) {return at(o);}
    vector<int> at(const minsky::Item& item) {
      vector<int> r;
      for (size_t i=0; i<item.portsSize(); ++i)
        r.push_back(at(item.ports(i).lock().get()));
      return r;
    }
  
    template <class T>
    bool emplaceIf(vector<Item>& items, const minsky::Item* i)
    {
      auto* j=dynamic_cast<const T*>(i);
      if (j)
        {
          // do not write invisible unwired GodleyIcon variables to the schema. For #1759.
          if (auto v=i->variableCast())
            if (auto g=dynamic_cast<minsky::GodleyIcon*>(v->controller.lock().get()))
              for (size_t i=0; !g->variableDisplay() && i<2; ++i)
                if (auto p=v->ports(i).lock())
                  if (p->wires().empty())
                    return j;
          
          items.emplace_back(at(i), *j, at(*j));
          if (auto* g=dynamic_cast<const minsky::GodleyIcon*>(i))
            {
              // insert port references from flow/stock vars
              items.back().ports.clear();
              for (const auto& v: g->flowVars())
                items.back().ports.push_back(at(v->ports(1).lock().get()));
              for (const auto& v: g->stockVars())
                items.back().ports.push_back(at(v->ports(0).lock().get()));
            }
          if (auto* d=dynamic_cast<const minsky::DataOp*>(i))
            {
              items.back().dataOpData=d->data;
              items.back().name=d->description();
            }
          if (auto* s=dynamic_cast<const minsky::Sheet*>(i))
            {
              items.back().showSlice=s->showRowSlice;
              items.back().showColSlice=s->showColSlice;
            }
          if (auto* d=dynamic_cast<const minsky::UserFunction*>(i))
            {
              items.back().expression=d->expression;
              items.back().name=d->description();
            }
          if (auto* r=dynamic_cast<const minsky::Ravel*>(i))
            {
              if (r->lockGroup)
                lockGroups.insert(r->lockGroup.get());
              auto s=r->getState();
              if (!s.handleStates.empty())
                {
                  items.back().ravelState=s;
                  items.back().dimensions=r->axisDimensions;
                  items.back().editorMode=r->editorMode();
                }
              if (r->flipped) items.back().rotation=180;
            }
          if (auto* l=dynamic_cast<const minsky::Lock*>(i))
            if (l->locked())
              items.back().ravelState=l->lockedState;
        }
      return j;
    }
  };

  namespace
  {
    inline bool matchesStart(const string& x, const string& y)
    {
      const size_t n=min(x.length(),y.length());
      return x.substr(0,n)==y.substr(0,n);
    }
  }
  
  struct MinskyItemFactory: public classdesc::Factory<minsky::Item,string>
  {
    template <class T>
    void registerClassType() {
      auto s=typeName<T>();
      // remove minsky namespace
      static const char* ns="minsky::";
      static const int eop=strlen(ns);
      if (s.substr(0,eop)==ns)
        s=s.substr(eop);
      registerType<T>(s);
    }
    MinskyItemFactory()
    {
      registerClassType<minsky::Item>();
      registerClassType<minsky::IntOp>();
      registerClassType<minsky::DataOp>();
      registerClassType<minsky::UserFunction>();
      registerClassType<minsky::Ravel>();
      registerClassType<minsky::Lock>();
      registerClassType<minsky::Sheet>();
      registerClassType<minsky::VarConstant>();
      registerClassType<minsky::GodleyIcon>();
      registerClassType<minsky::PlotWidget>();
      registerClassType<minsky::SwitchIcon>();
    }

    // override here for special handling of Operations and Variables
    minsky::Item* create(const string& t) const
    {
      if (t=="Operation:rho") // handle legacy names after operation rename
        return minsky::OperationBase::create(minsky::OperationType::correlation); 
      if (matchesStart(t,"Operation:"))
        return minsky::OperationBase::create
          (enum_keys<minsky::OperationType::Type>()
           (t.substr(t.find(':')+1)));
      if (matchesStart(t,"Variable:"))
        return minsky::VariableBase::create
          (enum_keys<minsky::VariableType::Type>()
           (t.substr(t.find(':')+1)));
      try
        {
          return classdesc::Factory<minsky::Item,string>::create("::minsky::"+t);
        }
      catch (...)
        {
          assert(!"item type not registered");
          return nullptr;
        }
    }
  };
  
  struct Schema1Layout
  {
    map<int,schema1::UnionLayout> layout;
    Schema1Layout(const vector<shared_ptr<schema1::Layout>>& x) {
      for (const auto& i: x)
        {
          // serialise to json, then deserialise to a UnionLayout
          json_pack_t jbuf;
          i->json_pack(jbuf,"");
          json_unpack(jbuf,"",layout[i->id]);
        }
    }
    template <class V, class O>
    void addItem(V& vec, const O& item) {
      vec.emplace_back(item);
      if (layout.count(item.id))
        vec.back().addLayout(layout[item.id]);
    }
  };

  void Item::packTensorInit(const minsky::VariableBase& v)
  {
    if (auto val=v.vValue())
      if (val->tensorInit.rank())
        {
          pack_t buf;
          pack(buf,val->tensorInit);
          tensorData=minsky::encode(buf);
        }
  }

  struct MinskyImpl
  {
    IdMap itemMap, pubItemMap;                            // for serialisation
    map<int, minsky::ItemPtr> reverseItemMap; // for deserialisation
  };

  void Minsky::makeImpl()
  {
    impl=make_shared<MinskyImpl>();
  }
  
  Minsky::~Minsky()=default; // required because of Pimpl pattern

  Minsky::Minsky(const minsky::Group& g, bool packTensorData)
  {
    makeImpl();
    auto& itemMap=impl->itemMap;

    g.recursiveDo(&minsky::GroupItems::items,[&](const minsky::Items&,minsky::Items::const_iterator i) {
        itemMap.emplaceIf<minsky::Ravel>(items, i->get()) ||
        itemMap.emplaceIf<minsky::OperationBase>(items, i->get()) ||
          itemMap.emplaceIf<minsky::VariableBase>(items, i->get()) ||
          itemMap.emplaceIf<minsky::GodleyIcon>(items, i->get()) ||
          itemMap.emplaceIf<minsky::PlotWidget>(items, i->get()) ||
          itemMap.emplaceIf<minsky::SwitchIcon>(items, i->get()) ||
          itemMap.emplaceIf<minsky::Sheet>(items, i->get()) ||
          itemMap.emplaceIf<minsky::Item>(items, i->get());
        if (packTensorData) //pack tensor data
          if (auto* v=(*i)->variableCast())
            if (!items.back().tensorData)
              items.back().packTensorInit(*v);

        return false;
      });

    // add any I/O variables
    vector<int> inVars, outVars;
    for (auto& i: g.inVariables)
      inVars.push_back(itemMap[i.get()]);
    for (auto& i: g.outVariables)
      outVars.push_back(itemMap[i.get()]);
    inVariables=inVars;
    outVariables=outVars;
    
    // search for and link up integrals to their variables, and Godley table ports
    g.recursiveDo(&minsky::GroupItems::items,[&](const minsky::Items&,minsky::Items::const_iterator i) {
        if (auto* integ=dynamic_cast<minsky::IntOp*>(i->get()))
          {
            const int id=itemMap[i->get()];
            for (auto& j: items)
              if (j.id==id)
                {
                  // nb conversion to Item* essential here, as
                  // conversion of derived class to void* may not be
                  // equivalent
                  assert(itemMap.count(static_cast<minsky::Item*>(integ->intVar.get())));
                  j.intVar.reset(new int(itemMap[static_cast<minsky::Item*>(integ->intVar.get())]));
                  break;
                }
          }
        return false;
      });
        
    g.recursiveDo(&minsky::GroupItems::wires,
                  [&](const minsky::Wires&,minsky::Wires::const_iterator i) {
                    wires.emplace_back(itemMap[i->get()], **i);
                    assert(itemMap.count((*i)->from().get()) && itemMap.count((*i)->to().get()));
                    wires.back().from=itemMap[(*i)->from().get()];
                    wires.back().to=itemMap[(*i)->to().get()];
                    return false;
      });
    
    g.recursiveDo(&minsky::GroupItems::groups,
                  [&](const minsky::Groups&,minsky::Groups::const_iterator i) {
                    groups.emplace_back(itemMap[i->get()], **i);
                    for (auto& j: (*i)->items)
                      {
                        // some invisible items are excluded from the schema 
                        assert(!j->visible() || itemMap.count(j.get()));
                        groups.back().items.push_back(itemMap[j.get()]);
                        if (j==(*i)->displayPlot)
                          groups.back().displayPlot=itemMap[j.get()];
                      }
                    for (auto& j: (*i)->groups)
                      groups.back().items.push_back(itemMap[j.get()]);
                   for (auto& v: (*i)->inVariables)
                      {
                        assert(itemMap.count(v.get()));
                        groups.back().inVariables->push_back(itemMap[v.get()]);
                      }
                    for (auto& v: (*i)->outVariables)
                      {
                        assert(itemMap.count(v.get()));
                        groups.back().outVariables->push_back(itemMap[v.get()]);
                      }
                    return false;
                  });

    
    
    // process lock groups
    for (auto lg: itemMap.lockGroups)
      {
        lockGroups.emplace_back();
        auto& slg=lockGroups.back();
        for (auto& wr: lg->ravels())
          if (auto r=wr.lock())
            slg.ravels.push_back(itemMap.at(static_cast<minsky::Item*>(r.get())));
          else
            slg.ravels.push_back(-1); // indicate invalidity
        for (auto& hli: lg->handleLockInfo)
          slg.handleLockInfo.emplace_back(hli);
      }
  }
      
  PhillipsDiagram::PhillipsDiagram(const minsky::PhillipsDiagram& pd)
  {
    IdMap itemMap;
    for (auto& [key,stock]: pd.stocks)
      itemMap.emplaceIf<minsky::VariableBase>(stocks, &stock);

    for (auto& [key,flow]: pd.flows)
      {
        flows.emplace_back(itemMap[&flow], flow);
        assert(itemMap.count(flow.from().get()) && itemMap.count(flow.to().get()));
        flows.back().from=itemMap[flow.from().get()];
        flows.back().to=itemMap[flow.to().get()];
        for (auto& term: flow.terms)
          flows.back().terms.emplace_back(term.first, Item(-1,term.second,{}));
      }
  }

  void Minsky::populateSchemaPublicationTabs(const std::vector<minsky::PubTab>& pubTabs)
  {
    assert(impl.get());
    auto& itemMap=impl->itemMap;
    publicationTabs.clear();
    if (pubTabs.size()==1 && pubTabs.front().items.empty()) return; // don't bother adding a single empty pub tab
    for (auto& i: pubTabs)
      {
        publicationTabs.emplace_back();
        publicationTabs.back().name=i.name;
        for (auto& j: i.items)
          {
            // add locally added notes on publication tab to items list in schema
            if (!itemMap.count(j.itemRef.get()))
              itemMap.emplaceIf<minsky::Item>(publicationItems,j.itemRef.get());
            publicationTabs.back().items.emplace_back(itemMap[j.itemRef.get()], j);
          }
      }
  }

  void Minsky::populateMinsky(minsky::Minsky& m) const
  {
    const minsky::LocalMinsky lm(m);
    m.model->clear();
    populateGroup(*m.model);
    m.canvas.model=m.model;

    m.model->setZoom(zoomFactor);
    m.model->bookmarks.insert(bookmarks.begin(), bookmarks.end());
    m.dimensions=dimensions;
    m.conversions=conversions;
    m.fileVersion=minskyVersion;
    
    static_cast<minsky::Simulation&>(m)=rungeKutta;

    phillipsDiagram.populatePhillipsDiagram(m.phillipsDiagram);
    populatePublicationTabs(m.publicationTabs);
  }

  void populateNote(minsky::NoteBase& x, const Note& y)
  {
    if (y.detailedText && !y.detailedText->empty())
      x.detailedText(*y.detailedText);
    if (y.tooltip && !y.tooltip->empty())
      x.tooltip(*y.tooltip);
  }

  void populateItem(minsky::Item& x, const Item& y)
  {
    populateNote(x,y);
    x.m_x=y.x;
    x.m_y=y.y;
    x.m_sf=y.scaleFactor;
    x.rotation(y.rotation);
    x.iWidth(y.width);
    x.iHeight(y.height);
    x.bookmark=y.bookmark;
    if (auto* x1=dynamic_cast<minsky::DataOp*>(&x))
      {
        if (y.name)
          x1->description(*y.name);
        if (y.dataOpData)
          x1->data=*y.dataOpData;
      }
    if (auto* x1=dynamic_cast<minsky::UserFunction*>(&x))
      {
        if (y.name)
          x1->description(*y.name);
        if (y.expression)
          x1->expression=*y.expression;
      }
    
    if (auto* x1=dynamic_cast<minsky::Ravel*>(&x))
      {
        if (y.ravelState)
          {
            x1->applyState(y.ravelState->toRavelRavelState());
            x1->redistributeHandles();
            SchemaHelper::initHandleState(*x1,y.ravelState->toRavelRavelState());
          }
        
        if (y.dimensions)
          x1->axisDimensions=*y.dimensions;
        if (y.editorMode && *y.editorMode!=x1->editorMode())
          x1->toggleEditorMode();
        x1->flipped=minsky::flipped(y.rotation);
      }
    if (auto* x1=dynamic_cast<minsky::Lock*>(&x))
      {
        if (y.ravelState)
          {
            x1->lockedState=y.ravelState->toRavelRavelState();
            x1->tooltip(ravel::Ravel::description(x1->lockedState));
          }
      }
    if (auto* x1=dynamic_cast<minsky::VariableBase*>(&x))
      {
        if (y.name)
          x1->name(*y.name);
        if (y.slider)
          {
            x1->sliderBoundsSet=true;
            x1->sliderStepRel=y.slider->stepRel;
            x1->enableSlider=y.slider->visible;
            x1->sliderMin=y.slider->min;
            x1->sliderMax=y.slider->max;
            x1->sliderStep=y.slider->step;
          }
        x1->miniPlotEnabled(y.miniPlot);
        // variableValue attributes populated later once variable is homed in its group
      }
    if (auto* x1=dynamic_cast<minsky::OperationBase*>(&x))
      {
        if (y.axis) x1->axis=*y.axis;
        if (y.arg) x1->arg=*y.arg;
      }
   if (auto* x1=dynamic_cast<minsky::GodleyIcon*>(&x))
      {	  
        std::vector<std::vector<std::string>> data;
        std::vector<minsky::GodleyAssetClass::AssetClass> assetClasses;
        if (y.data) data=*y.data;
        if (y.assetClasses) assetClasses=*y.assetClasses;
        SchemaHelper::setPrivates(*x1,data,assetClasses);
        try
          {
            x1->table.orderAssetClasses();
          }
        catch (const std::exception&) {}
        if (y.name) x1->table.title=*y.name;
        if (y.editorMode && *y.editorMode!=x1->editorMode())
          x1->toggleEditorMode();
        if (y.variableDisplay)
          SchemaHelper::setVariableDisplay(*x1, *y.variableDisplay);
        if (y.buttonDisplay && *y.buttonDisplay!=x1->buttonDisplay())
          x1->toggleButtons();
        if (y.currency) x1->currency=*y.currency;
      }
    if (auto* x1=dynamic_cast<minsky::PlotWidget*>(&x))
      {
        x1->bb.update(*x1);        
        if (y.name) x1->title=*y.name;
        y.applyPlotOptions(*x1);
        if (y.palette) x1->palette=*y.palette;
        if (y.ports.size()>x1->nBoundsPorts)
          x1->numLines((y.ports.size()-x1->nBoundsPorts)/4);
      }
    if (auto* x1=dynamic_cast<minsky::Sheet*>(&x))
      {
        if (y.showSlice)
          x1->showRowSlice=*y.showSlice;
        if (y.showColSlice)
          x1->showColSlice=*y.showColSlice;
      }
    if (auto* x1=dynamic_cast<minsky::SwitchIcon*>(&x))
      {
        x1->flipped=minsky::flipped(y.rotation);
        if (y.ports.size()>=2)
          x1->setNumCases(y.ports.size()-2);
      }
    if (auto* x1=dynamic_cast<minsky::Group*>(&x))
      {
        x1->bb.update(*x1);
        if (y.name) x1->title=*y.name;
        if (y.bookmarks)
          {
            x1->bookmarks.clear();
            x1->bookmarks.insert(y.bookmarks->begin(), y.bookmarks->end());
          }
      }
  }

  void populateWire(minsky::Wire& x, const Wire& y)
  {
    populateNote(x,y);
    if (y.coords)
      x.coords(*y.coords);
  }

  struct LockGroupFactory: public shared_ptr<minsky::RavelLockGroup>
  {
    LockGroupFactory(): shared_ptr<minsky::RavelLockGroup>(new minsky::RavelLockGroup) {}
  };
  
  void Minsky::populatePublicationTabs(std::vector<minsky::PubTab>& pubTabs) const
  {
    assert(impl.get());
    auto& itemMap=impl->reverseItemMap;

    // add in publication tab only items
    const MinskyItemFactory factory;
    for (auto& i: publicationItems)
      if (auto newItem=itemMap[i.id]=minsky::ItemPtr(factory.create(i.type)))
        populateItem(*newItem,i);
    
    pubTabs.clear();
    for (auto& pub: publicationTabs)
      {
        pubTabs.emplace_back(pub.name);
        pubTabs.back().offsx=pub.x;
        pubTabs.back().offsy=pub.y;
        pubTabs.back().m_zoomFactor=pub.zoomFactor;
           
        for (auto& item: pub.items)
          if (itemMap.contains(item.item))
            pubTabs.back().items.emplace_back(itemMap[item.item], item);
      }
    if (pubTabs.empty()) pubTabs.emplace_back("Publication");
  }
  
  void Minsky::populateGroup(minsky::Group& g) const {
    assert(impl.get());
    auto& itemMap=impl->reverseItemMap;
    map<int, weak_ptr<minsky::Port>> portMap;
    map<int, schema3::Item> schema3VarMap;
    const MinskyItemFactory factory;
    map<int,LockGroupFactory> lockGroups;
    
    for (const auto& i: items)
      if (auto newItem=itemMap[i.id]=g.addItem(factory.create(i.type)))
        {
          populateItem(*newItem,i);
          for (size_t j=0; j<min(newItem->portsSize(), i.ports.size()); ++j)
            portMap[i.ports[j]]=newItem->ports(j);
          if (newItem->variableCast())
            schema3VarMap[i.id]=i;
        }

    if (inVariables)
      for (auto i: *inVariables)
        g.inVariables.push_back(itemMap[i]);
    if (outVariables)
      for (auto i: *outVariables)
        g.outVariables.push_back(itemMap[i]);

    // second loop over items to wire up integrals, and populate Godley table variables
    for (const auto& i: items)
      {
        if ((i.type=="IntOp" || i.type=="Operation:integrate") && i.intVar)
          {
            assert(itemMap.count(i.id));
            assert(dynamic_pointer_cast<minsky::IntOp>(itemMap[i.id]));
            if (auto* integ=dynamic_cast<minsky::IntOp*>(itemMap[i.id].get()))
              {
                assert(integ->intVar);
                assert(integ->intVar->type()==minsky::VariableType::integral);
                if (itemMap.contains(*i.intVar))
                  {
                    if (integ->coupled()) integ->toggleCoupled();
                    g.removeItem(*integ->intVar);
                    integ->intVar=itemMap[*i.intVar];
                    if (!integ->intVar || integ->intVar->type()!=minsky::VariableType::integral)
                      // input mky file is corrupted at this point
                      integ->description(integ->description());
                  }
                auto iv=schema3VarMap.find(*i.intVar);
                if (iv!=schema3VarMap.end())
                  if ((!i.ports.empty() && !iv->second.ports.empty() && i.ports[0]==iv->second.ports[0]) != integ->coupled())
                    integ->toggleCoupled();
                // ensure that the correct port is inserted (may have been the deleted intVar)
                if (!i.ports.empty())
                  portMap[i.ports[0]]=integ->coupled()? integ->intVar->ports(0): integ->ports(0);
              }
          }
        if (i.type=="GodleyIcon")
          {
            assert(itemMap.count(i.id));
            if (auto* godley=dynamic_cast<minsky::GodleyIcon*>(itemMap[i.id].get()))
              {
                minsky::GodleyIcon::Variables flowVars, stockVars;
                for (auto p: i.ports)
                  {
                    auto newP=portMap.find(p);
                    if (newP!=portMap.end())
                      if (auto ip=g.findItem(newP->second.lock()->item()))
                        if (auto v=dynamic_pointer_cast<minsky::VariableBase>(ip))
                          switch (v->type())
                            {
                            case minsky::VariableType::stock:
                              stockVars.push_back(v);
                              break;
                            case minsky::VariableType::flow:
                              flowVars.push_back(v);
                              break;
                            default:
                              break;
                            }
                  }
                SchemaHelper::setStockAndFlow(*godley, flowVars, stockVars);
                try
                  {
                    godley->update();
                  }
                catch (...) {} //ignore exceptions: ticket #1045
              }
          }
        if (i.type=="Ravel" && i.lockGroup)
          if (auto r=dynamic_pointer_cast<minsky::Ravel>(itemMap[i.id]))
            {
              // note: legacy lock group format handling
              r->lockGroup=lockGroups[*i.lockGroup];
              r->lockGroup->addRavel(r);
              if (i.lockGroupHandles)
                r->lockGroup->setLockHandles({i.lockGroupHandles->begin(), i.lockGroupHandles->end()});
            }
      }
  
    // add in new lockGroup info
    for (auto& lgi: this->lockGroups)
      {
        auto lockGroup=make_shared<minsky::RavelLockGroup>();
        for (auto i: lgi.ravels)
          if (auto r=dynamic_pointer_cast<minsky::Ravel>(itemMap[i]))
            {
              lockGroup->addRavel(r);
              r->lockGroup=lockGroup;
            }
          else
            lockGroup->addRavel({});
        lockGroup->handleLockInfo=lgi.handleLockInfo;
      }
    
    for (const auto& w: wires)
      if (portMap.contains(w.to) && portMap.contains(w.from))
        {
          populateWire
            (*g.addWire(new minsky::Wire(portMap[w.from],portMap[w.to])),w);
        }
          
    for (const auto& i: groups)
      populateItem(*(itemMap[i.id]=g.addGroup(new minsky::Group)),i);

    // second loop over groups, because groups can contain other groups
    for (const auto& i: groups)
      {
        assert(itemMap.count(i.id));
        auto newG=dynamic_pointer_cast<minsky::Group>(itemMap[i.id]);
        if (newG)
          {
            for (auto j: i.items)
              {
                auto it=itemMap.find(j);
                if (it!=itemMap.end())
                  {
                    newG->addItem(it->second, true/*inSchema*/);
                  }
              }
            if (i.displayPlot>=0)
              {
                auto it=itemMap.find(i.displayPlot);
                if (it!=itemMap.end())
                  newG->displayPlot=dynamic_pointer_cast<minsky::PlotWidget>(it->second);
              }
            if (i.inVariables)
              for (auto j: *i.inVariables)
                {
                  auto it=itemMap.find(j);
                  if (it!=itemMap.end())
                    if (auto v=dynamic_pointer_cast<minsky::VariableBase>(it->second))
                      {
                        newG->addItem(it->second);
                        newG->inVariables.push_back(v);
                        v->controller=newG;
                      }
                }
            if (i.outVariables)
              for (auto j: *i.outVariables)
                {
                  auto it=itemMap.find(j);
                  if (it!=itemMap.end())
                    if (auto v=dynamic_pointer_cast<minsky::VariableBase>(it->second))
                      {
                        newG->addItem(it->second);
                        newG->outVariables.push_back(v);
                        v->controller=newG;
                      }
                }
          }
      }
    // now that variables have been homed in their groups, set the variableValue stuff
    for (auto& i: schema3VarMap)
      {
        auto it=itemMap.find(i.first);
        if (!it->second) continue;
        if (auto* v=it->second->variableCast())
          {
            if (i.second.init)
              v->init(*i.second.init);
            if (i.second.units)
              v->setUnits(*i.second.units);
            populateNote(*v,i.second);
            if (auto val=v->vValue())
              {
                if (i.second.csvDataSpec)
                  val->csvDialog.spec=*i.second.csvDataSpec;
                if (i.second.url)
                  val->csvDialog.url=*i.second.url;
                if (i.second.tensorData)
                  {
                    auto buf=minsky::decode(*i.second.tensorData);
                    try
                      {
                        civita::TensorVal tmp;
                        unpack(buf, tmp);
                        *val=tmp;
                        val->tensorInit=std::move(tmp);
                        assert(val->idxInRange());
                      }
                    catch (const std::exception& ex) {
                      val->tensorInit.hypercube({});
#if !defined(NDEBUG) || !defined(_WIN32)
                      cout<<ex.what()<<endl;
#endif
                    }
                    catch (...) {
                      val->tensorInit.hypercube({});
                      assert(val->idxInRange());
                    } // absorb for now - maybe log later
                  }
              }
          }
      }
  }

  void PhillipsDiagram::populatePhillipsDiagram(minsky::PhillipsDiagram& pd) const
  {
    static const MinskyItemFactory itemFactory;
    map<int, weak_ptr<minsky::Port>> portMap;
    for (auto& i: stocks)
      {
        minsky::PhillipsStock stock;
        populateItem(stock, i);
        auto& item=pd.stocks[stock.valueId()]=stock;
        for (size_t j=0; j<std::min(i.ports.size(), stock.numPorts()); ++j)
          portMap[i.ports[j]]=item.ports(j);
      }
    
    for (auto& i: flows)
      {
        assert(portMap[i.from].lock() && portMap[i.to].lock());
        minsky::PhillipsFlow flow(portMap[i.from],portMap[i.to]);
        populateWire(flow, i);
        for (auto& j: i.terms)
          {
            flow.terms.emplace_back(j.first, minsky::FlowVar{});
            populateItem(flow.terms.back().second, j.second);
          }
        auto fromId=flow.from()->item().variableCast()->valueId();
        auto toId=flow.to()->item().variableCast()->valueId();
        auto success=pd.flows.emplace(make_pair(fromId, toId), flow).second; //NOLINT
        assert(success);
      }
  }
  
}


