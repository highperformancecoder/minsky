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
#include "schema2.h"
#include "sheet.h"
#include "minsky_epilogue.h"

#include "a85.h"
#include <zlib.h>

namespace classdesc {template <> Factory<minsky::Item,string>::Factory() {}}

namespace schema2
{

  // map of object to ID, that allocates a new ID on objects not seen before
  struct IdMap: public map<void*,int>
  {
    int nextId=0;
    int at(void* o) {
      auto i=find(o);
      if (i==end())
        return emplace(o,nextId++).first->second;
      else
        return i->second;
    }
    int operator[](void* o) {return at(o);}
    vector<int> at(const minsky::ItemPortVector& v) {
      vector<int> r;
      for (auto& i: v)
        r.push_back(at(i.get()));
      return r;
    }
    
    template <class T>
    bool emplaceIf(vector<Item>& items, minsky::Item* i)
    {
      auto j=dynamic_cast<T*>(i);
      if (j)
        {
          items.emplace_back(at(i), *j, at(j->ports));
          if (auto g=dynamic_cast<minsky::GodleyIcon*>(i))
            {
              // insert port references from flow/stock vars
              items.back().ports.clear();
              for (auto& v: g->flowVars())
                items.back().ports.push_back(at(v->ports[1].get()));
              for (auto& v: g->stockVars())
                items.back().ports.push_back(at(v->ports[0].get()));
            }
          if (auto d=dynamic_cast<minsky::DataOp*>(i))
            {
              items.back().dataOpData=d->data;
              items.back().name=d->description;
            }
          if (auto r=dynamic_cast<minsky::Ravel*>(i))
            {
              items.back().filename=r->filename();
              if (r->lockGroup)
                items.back().lockGroup=at(r->lockGroup.get());
              auto s=r->getState();
              if (!s.handleStates.empty())
                {
                  items.back().ravelState=s;
                  items.back().dimensions=r->axisDimensions;
                }
            }
        }
      return j;
    }
  };

  namespace
  {
    inline bool matchesStart(const string& x, const string& y)
    {
      size_t n=min(x.length(),y.length());
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
      registerClassType<minsky::Ravel>();
      registerClassType<minsky::Sheet>();
      registerClassType<minsky::VarConstant>();
      registerClassType<minsky::GodleyIcon>();
      registerClassType<minsky::PlotWidget>();
      registerClassType<minsky::SwitchIcon>();
    }

    // override here for special handling of Operations and Variables
    minsky::Item* create(const string& t) const
    {
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
      for (auto& i: x)
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

  Item::Item(const schema1::Godley& it):
    ItemBase(it, "GodleyIcon"),
    name(it.name), data(it.data), assetClasses(it.assetClasses)
  {
    typedef minsky::GodleyAssetClass::AssetClass AssetClass;
    ports=it.ports;
    // strip off leading :, since in schema 1, all godley tables are global
    for (auto& i: *data)
      for (auto& j: i)
        {
          minsky::FlowCoef fc(j);
          if (fc.coef==0)
            j="";
          else
            {
              if (fc.name.length()>0 && fc.name[0]==':')
                fc.name=fc.name.substr(1);
              j=fc.str();
            }
        }

    // in doubleEntry mode, schema1 got the accounting equation wrong.
    if (it.doubleEntryCompliant)
      {
        for (size_t r=1; r<data->size(); ++r)
          for (size_t c=1; c<(*data)[r].size(); ++c)
            if (!(*data)[r][c].empty() &&
                ((*assetClasses)[c]==AssetClass::liability || (*assetClasses)[c]==AssetClass::equity))
              {
                minsky::FlowCoef fc((*data)[r][c]);
                fc.coef*=-1;
                (*data)[r][c]=fc.str();
              }
      }
    else
      {
        // make all columns assets
        assetClasses->resize((*data)[0].size());
        for (auto& i: *assetClasses) i=AssetClass::asset;
      }
  }

  namespace
  {
    // nice RAII wrappers around zlib's data structures
    struct ZStream: public z_stream
    {
      ZStream(Bytef* input, size_t inputSize, Bytef* output, size_t outputSize)
      {
        next_in=input;
        avail_in=inputSize;
        next_out=output;
        avail_out=outputSize;
        zfree=Z_NULL;
        zalloc=Z_NULL;
      }
      void throwError() const {
        throw runtime_error(string("compression failure: ")+(msg? msg:""));
      }
    };
    
    struct DeflateZStream: public ZStream
    {
      template <class I, class O>
      DeflateZStream(const I& input, O& output):
        ZStream((Bytef*)input.data(), input.size(),
                (Bytef*)output.data(), output.size())
      {
        if (deflateInit(this,9)!=Z_OK) throwError();
      }
      ~DeflateZStream() {deflateEnd(this);}
      void deflate() {
        if (::deflate(this,Z_FINISH)!=Z_STREAM_END) throwError();
      }
     };
    
    struct InflateZStream: public ZStream
    {
      classdesc::pack_t output{256};
      Bytef* inputData;
      size_t inputSize;
      
      template <class I>
      InflateZStream(const I& input):
        ZStream((Bytef*)input.data(), input.size(), 0,0),
        inputData((Bytef*)input.data()),inputSize(input.size())
      {
        next_out=(Bytef*)output.data();
        avail_out=output.size();
        if (inflateInit(this)!=Z_OK) throwError();
      }
      ~InflateZStream() {inflateEnd(this);}

      void inflate() {
        int err;
        while ((err=::inflate(this,Z_SYNC_FLUSH))==Z_OK)
          {
            // try doubling size
            output.resize(2*output.size());
            next_out=(Bytef*)(output.data())+total_out;
            avail_out=output.size()-total_out;
            next_in=inputData+total_in;
            avail_in=inputSize-total_in;
          }
        if (err!=Z_STREAM_END) throwError();
      }
      void throwError() {
        throw runtime_error(string("compression failure: ")+(msg? msg:""));
      }
    };
 }
  
  void Item::packTensorInit(const minsky::VariableBase& v)
  {
    if (auto val=v.vValue())
      if (!val->tensorInit.data.empty())
        {
          pack_t buf;
          buf<<val->tensorInit<<val->hypercube().xvectors;
          
          vector<unsigned char> zbuf(buf.size());
          DeflateZStream zs(buf, zbuf);
          zs.deflate();
          
          vector<char> cbuf(a85::size_for_a85(zs.total_out,false));
          a85::to_a85(&zbuf[0],zs.total_out, &cbuf[0], false);
          // this ensures that the escape sequence ']]>' never appears in the data
          replace(cbuf.begin(),cbuf.end(),']','~');
          tensorData.reset(new CDATA(cbuf.begin(),cbuf.end()));
        }
  }

  
  Minsky::Minsky(const schema1::Minsky& m)
  {
    // read in layout into a map indexed by id
    Schema1Layout layout(m.layout);
    
    for (auto& i: m.model.wires)
      layout.addItem(wires,i);
    for (auto& i: m.model.notes)
      layout.addItem(items,i);    
    for (auto& i: m.model.operations)
      layout.addItem(items,i);    

    // save a mapping of ports to variables for later use by group
    map<int,pair<int,bool>> portToVar;
    for (auto& i: m.model.variables)
      {
        layout.addItem(items,i);
        for (size_t j=0; j<i.ports.size(); ++j)
          portToVar[i.ports[j]]=make_pair(i.id,j>0);
      }
    
    for (auto& i: m.model.plots)
      layout.addItem(items,i);    
    for (auto& i: m.model.switches)
      layout.addItem(items,i);
    
    for (auto& i: m.model.godleys)
      {
        layout.addItem(items,i);
        // override any height specifcation with a legacy iconScale parameter
        items.back().height.reset();
        items.back().iconScale=i.zoomFactor/m.zoomFactor;
      }
    for (auto& i: m.model.groups)
      {
        layout.addItem(groups,i);
        for (auto p: i.ports)
          {
            auto v=portToVar.find(p);
            if (v!=portToVar.end())
              {
                if (v->second.second)
                  groups.back().inVariables->push_back(v->second.first);
                else
                  groups.back().outVariables->push_back(v->second.first);
              }
          }
      }
    
    rungeKutta=m.model.rungeKutta;
    zoomFactor=m.zoomFactor;
  }

  
  Minsky::Minsky(const minsky::Group& g)
  {
    IdMap itemMap;

    g.recursiveDo(&minsky::GroupItems::items,[&](const minsky::Items&,minsky::Items::const_iterator i) {
        itemMap.emplaceIf<minsky::Ravel>(items, i->get()) ||
        itemMap.emplaceIf<minsky::OperationBase>(items, i->get()) ||
          itemMap.emplaceIf<minsky::VariableBase>(items, i->get()) ||
          itemMap.emplaceIf<minsky::GodleyIcon>(items, i->get()) ||
          itemMap.emplaceIf<minsky::PlotWidget>(items, i->get()) ||
          itemMap.emplaceIf<minsky::SwitchIcon>(items, i->get()) ||
          itemMap.emplaceIf<minsky::Sheet>(items, i->get()) ||
          itemMap.emplaceIf<minsky::Item>(items, i->get());
        return false;
      });
    
    // search for and link up integrals to their variables, and Godley table ports
    g.recursiveDo(&minsky::GroupItems::items,[&](const minsky::Items&,minsky::Items::const_iterator i) {
        if (auto integ=dynamic_cast<minsky::IntOp*>(i->get()))
          {
            int id=itemMap[i->get()];
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
                        assert(itemMap.count(j.get()));
                        groups.back().items.push_back(itemMap[j.get()]);
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
  }
      
  Minsky::operator minsky::Minsky() const
  {
    minsky::Minsky m;
    minsky::LocalMinsky lm(m);
    populateGroup(*m.model);
    m.model->setZoom(zoomFactor);
    m.model->bookmarks=bookmarks;
    m.dimensions=dimensions;
    m.conversions=conversions;

    static_cast<minsky::RungeKutta&>(m)=rungeKutta;
    return m;
  }

  void populateNote(minsky::NoteBase& x, const Note& y)
  {
    if (y.detailedText) x.detailedText=*y.detailedText;
    if (y.tooltip) x.tooltip=*y.tooltip;
  }

  void populateItem(minsky::Item& x, const Item& y)
  {
    populateNote(x,y);
    x.m_x=y.x;
    x.m_y=y.y;
    x.rotation=y.rotation;
    if (auto x1=dynamic_cast<minsky::DataOp*>(&x))
      {
        if (y.name)
          x1->description=*y.name;
        if (y.dataOpData)
          x1->data=*y.dataOpData;
      }
    if (auto x1=dynamic_cast<minsky::Ravel*>(&x))
      {
        if (y.filename)
          try
            {
              x1->loadFile(*y.filename);
            }
          catch (...) {}
        if (y.ravelState)
          {
            x1->applyState(*y.ravelState);
            SchemaHelper::initHandleState(*x1,*y.ravelState);
          }
        
        if (y.dimensions)
          x1->axisDimensions=*y.dimensions;
      }
    if (auto x1=dynamic_cast<minsky::VariableBase*>(&x))
      {
        if (y.name)
          x1->name(*y.name);
        if (y.init)
          x1->init(*y.init);
        if (y.units)
          x1->setUnits(*y.units);
        if (y.slider)
          {
            x1->sliderBoundsSet=true;
            x1->sliderVisible(y.slider->visible);
            x1->sliderStepRel=y.slider->stepRel;
            x1->sliderMin=y.slider->min;
            x1->sliderMax=y.slider->max;
            x1->sliderStep=y.slider->step;
          }
        if (y.tensorData)
          if (auto val=x1->vValue())
            {
              string trimmed; //trim whitespace
              for (auto c: *y.tensorData)
                if (!isspace(c)) trimmed+=c;

              vector<unsigned char> zbuf(a85::size_for_bin(trimmed.size()));
              // reverse transformation required to avoid the escape sequence ']]>'
              replace(trimmed.begin(),trimmed.end(),'~',']'); 
              a85::from_a85(trimmed.data(), trimmed.size(),zbuf.data());
              
              InflateZStream zs(zbuf);
              zs.inflate();
              
              vector<minsky::XVector> xv;
              try
                {
                  zs.output>>val->tensorInit>>xv;
                  val->hypercube(move(xv));
                }
              catch (...) {} // absorb for now - maybe log later
            }
      }
    if (auto x1=dynamic_cast<minsky::OperationBase*>(&x))
      {
        if (y.axis) x1->axis=*y.axis;
        if (y.arg) x1->arg=*y.arg;
      }
   if (auto x1=dynamic_cast<minsky::GodleyIcon*>(&x))
      {
        std::vector<std::vector<std::string>> data;
        std::vector<minsky::GodleyAssetClass::AssetClass> assetClasses;
        if (y.data) data=*y.data;
        if (y.assetClasses) assetClasses=*y.assetClasses;
        if (y.name) x1->table.title=*y.name;
        SchemaHelper::setPrivates(x1->table,data,assetClasses);
        try
          {
            x1->table.orderAssetClasses();
          }
        catch (const std::exception&) {}
      }
    if (auto x1=dynamic_cast<minsky::PlotWidget*>(&x))
      {
        if (y.width) x1->width=*y.width;
        if (y.height) x1->height=*y.height;
        if (y.name) x1->title=*y.name;
        if (y.logx) x1->logx=*y.logx;
        if (y.logy) x1->logy=*y.logy;
        if (y.ypercent) x1->percent=*y.ypercent;
        if (y.plotType) x1->plotType=*y.plotType;
        if (y.xlabel) x1->xlabel=*y.xlabel;
        if (y.ylabel) x1->ylabel=*y.ylabel;
        if (y.y1label) x1->y1label=*y.y1label;
        if (y.nxTicks) x1->nxTicks=*y.nxTicks;
        if (y.nyTicks) x1->nyTicks=*y.nyTicks;
        if (y.xtickAngle) x1->xtickAngle=*y.xtickAngle;
        if (y.exp_threshold) x1->exp_threshold=*y.exp_threshold;
        if (y.legend)
          {
            x1->legend=true;
            x1->legendSide=*y.legend;
          }
        if (y.palette) x1->palette=*y.palette;
      }
    if (auto x1=dynamic_cast<minsky::SwitchIcon*>(&x))
      {
        auto r=fmod(y.rotation,360);
        x1->flipped=r>90 && r<270;
        if (y.ports.size()>=2)
          x1->setNumCases(y.ports.size()-2);
      }
    if (auto x1=dynamic_cast<minsky::Sheet*>(&x))
      {
        if (y.width) x1->m_width=*y.width;
        if (y.height) x1->m_height=*y.height;
      }
    if (auto x1=dynamic_cast<minsky::Group*>(&x))
      {
        if (y.width) x1->width=*y.width;
        if (y.height) x1->height=*y.height;
        if (y.name) x1->title=*y.name;
        if (y.bookmarks) x1->bookmarks=*y.bookmarks;
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
  
  void Minsky::populateGroup(minsky::Group& g) const {
    map<int, minsky::ItemPtr> itemMap;
    map<int, shared_ptr<minsky::Port>> portMap;
    map<int, schema2::Item> schema2VarMap;
    MinskyItemFactory factory;
    map<int,LockGroupFactory> lockGroups;
    
    for (auto& i: items)
      if (auto newItem=itemMap[i.id]=g.addItem(factory.create(i.type)))
        {
          populateItem(*newItem,i);
          for (size_t j=0; j<min(newItem->ports.size(), i.ports.size()); ++j)
            portMap[i.ports[j]]=newItem->ports[j];
          if (matchesStart(i.type,"Variable:"))
            schema2VarMap[i.id]=i;
        }
    // second loop over items to wire up integrals, and populate Godley table variables
    for (auto& i: items)
      {
        if ((i.type=="IntOp" || i.type=="Operation:integrate") && i.intVar)
          {
            assert(itemMap.count(i.id));
            assert(dynamic_pointer_cast<minsky::IntOp>(itemMap[i.id]));
            if (auto integ=dynamic_cast<minsky::IntOp*>(itemMap[i.id].get()))
              {
                if (itemMap.count(*i.intVar))
                  {
                    if (integ->coupled()) integ->toggleCoupled();
                    g.removeItem(*integ->intVar);
                    integ->intVar=itemMap[*i.intVar];
                  }
                auto iv=schema2VarMap.find(*i.intVar);
                if (iv!=schema2VarMap.end())
                  if ((!i.ports.empty() && i.ports[0]==iv->second.ports[0]) != integ->coupled())
                    integ->toggleCoupled();
                // ensure that the correct port is inserted (may have been the deleted intVar)
                if (!i.ports.empty())
                  portMap[i.ports[0]]=integ->ports[0];
              }
          }
        if (i.type=="GodleyIcon")
          {
            assert(itemMap.count(i.id));
            if (auto godley=dynamic_cast<minsky::GodleyIcon*>(itemMap[i.id].get()))
              {
                minsky::GodleyIcon::Variables flowVars, stockVars;
                for (auto p: i.ports)
                  {
                    auto newP=portMap.find(p);
                    if (newP!=portMap.end())
                      if (auto ip=g.findItem(newP->second->item()))
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
                    if (i.height)
                      godley->scaleIconForHeight(*i.height*godley->zoomFactor());
                    else if (i.iconScale) //legacy schema handling
                      godley->scaleIconForHeight(*i.iconScale * godley->height());
                  }
                catch (...) {} //ignore exceptions: ticket #1045
              }
          }
        if (i.type=="Ravel" && i.lockGroup)
          if (auto r=dynamic_pointer_cast<minsky::Ravel>(itemMap[i.id]))
            {
              r->lockGroup=lockGroups[*i.lockGroup];
              r->lockGroup->ravels.push_back(r);
            }
      }
        
    for (auto& w: wires)
      if (portMap.count(w.to) && portMap.count(w.from))
        {
          assert(portMap[w.from].use_count()>1 && portMap[w.to].use_count()>1);
          populateWire
            (*g.addWire(new minsky::Wire(portMap[w.from],portMap[w.to])),w);
        }
          
    for (auto& i: groups)
      populateItem(*(itemMap[i.id]=g.addGroup(new minsky::Group)),i);

    // second loop over groups, because groups can contain other groups
    for (auto& i: groups)
      {
        assert(itemMap.count(i.id));
        auto newG=dynamic_pointer_cast<minsky::Group>(itemMap[i.id]);
        if (newG)
          {
            for (auto j: i.items)
              {
                auto it=itemMap.find(j);
                if (it!=itemMap.end())
                  newG->addItem(it->second, true/*inSchema*/);
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
  }
}


