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


namespace schema2
{
  // binary serialisation used to serialise the tensorInit field of
  // variableValues into the minsky schema, fixed off here, rather
  // than classdesc generated to ensure backward compatibility
  void pack(classdesc::pack_t& b, const civita::XVector& a)
  {
    b<<a.name<<a.dimension<<a.size();
    for (auto& i: a)
      b<<civita::str(i);
  }

  void unpack(classdesc::pack_t& b, civita::XVector& a)
  {
    size_t size;
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
    // redundant dims field encoded in schema 2
    b<<a.hypercube().dims()<<a.size();
    for (size_t i=0; i<a.size(); ++i)
      b<<a[i];
    b<<a.hypercube().xvectors.size();
    for (auto& i: a.hypercube().xvectors)
      pack(b, i);
  }


  void unpack(classdesc::pack_t& b, civita::TensorVal& a)
  {
    vector<unsigned> dims; // ignored, because info carried with xvectors
    vector<double> data;
    b>>dims>>data;
    
    civita::Hypercube hc;
    size_t sz;
    b>>sz;
    for (size_t i=0; i<sz; ++i)
      {
        civita::XVector xv;
        unpack(b,xv);
        hc.xvectors.push_back(xv);
      }
    
    a.hypercube(hc); //dimension data
    assert(a.size()==data.size());
    memcpy(a.begin(),&data[0],data.size());
  }

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
              items.back().name=d->description();
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
  
//  void Item::packTensorInit(const minsky::VariableBase& v)
//  {
//    if (auto val=v.vValue())
//      if (val->tensorInit.size())
//        {
//          pack_t buf;
//          pack(buf,val->tensorInit);
//          tensorData=minsky::encode(buf);
//        }
//  }

  
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

  
//  void populateNote(minsky::NoteBase& x, const Note& y)
//  {
//    if (y.detailedText) x.detailedText=*y.detailedText;
//    if (y.tooltip) x.tooltip=*y.tooltip;
//  }
//
//  void populateItem(minsky::Item& x, const Item& y)
//  {
//    populateNote(x,y);
//    x.m_x=y.x;
//    x.m_y=y.y;
//    x.rotation(y.rotation);
//    if (auto x1=dynamic_cast<minsky::DataOp*>(&x))
//      {
//        if (y.name)
//          x1->description(*y.name);
//        if (y.dataOpData)
//          x1->data=*y.dataOpData;
//      }
//    if (auto x1=dynamic_cast<minsky::Ravel*>(&x))
//      {
//        if (y.filename)
//          try
//            {
//              x1->loadFile(*y.filename);
//            }
//          catch (...) {}
//        if (y.ravelState)
//          {
//            x1->applyState(*y.ravelState);
//            SchemaHelper::initHandleState(*x1,*y.ravelState);
//          }
//        
//        if (y.dimensions)
//          x1->axisDimensions=*y.dimensions;
//      }
//    if (auto x1=dynamic_cast<minsky::VariableBase*>(&x))
//      {
//        if (y.name)
//          x1->name(*y.name);
//        if (y.init)
//          x1->init(*y.init);
//        if (y.units)
//          x1->setUnits(*y.units);
//        if (y.slider)
//          {
//            x1->sliderBoundsSet=true;
//            x1->sliderVisible(y.slider->visible);
//            x1->sliderStepRel=y.slider->stepRel;
//            x1->sliderMin=y.slider->min;
//            x1->sliderMax=y.slider->max;
//            x1->sliderStep=y.slider->step;
//          }
//        if (y.tensorData)
//          if (auto val=x1->vValue())
//            {
//              auto buf=minsky::decode(*y.tensorData);
//              try
//                {
//                  unpack(buf, val->tensorInit);
//                  val->hypercube(val->tensorInit.hypercube());
//                }
//              catch (...) {} // absorb for now - maybe log later
//            }
//      }
//    if (auto x1=dynamic_cast<minsky::OperationBase*>(&x))
//      {
//        if (y.axis) x1->axis=*y.axis;
//        if (y.arg) x1->arg=*y.arg;
//      }
//   if (auto x1=dynamic_cast<minsky::GodleyIcon*>(&x))
//      {
//        std::vector<std::vector<std::string>> data;
//        std::vector<minsky::GodleyAssetClass::AssetClass> assetClasses;
//        if (y.data) data=*y.data;
//        if (y.assetClasses) assetClasses=*y.assetClasses;
//        if (y.name) x1->table.title=*y.name;
//        SchemaHelper::setPrivates(x1->table,data,assetClasses);
//        try
//          {
//            x1->table.orderAssetClasses();
//          }
//        catch (const std::exception&) {}
//      }
//    if (auto x1=dynamic_cast<minsky::PlotWidget*>(&x))
//      {
//        if (y.width) x1->width=*y.width;
//        if (y.height) x1->height=*y.height;
//        x1->bb.update(*x1);        
//        if (y.name) x1->title=*y.name;
//        if (y.logx) x1->logx=*y.logx;
//        if (y.logy) x1->logy=*y.logy;
//        if (y.ypercent) x1->percent=*y.ypercent;
//        if (y.plotType) x1->plotType=*y.plotType;
//        if (y.xlabel) x1->xlabel=*y.xlabel;
//        if (y.ylabel) x1->ylabel=*y.ylabel;
//        if (y.y1label) x1->y1label=*y.y1label;
//        if (y.nxTicks) x1->nxTicks=*y.nxTicks;
//        if (y.nyTicks) x1->nyTicks=*y.nyTicks;
//        if (y.xtickAngle) x1->xtickAngle=*y.xtickAngle;
//        if (y.exp_threshold) x1->exp_threshold=*y.exp_threshold;
//        if (y.legend)
//          {
//            x1->legend=true;
//            x1->legendSide=*y.legend;
//          }
//        if (y.palette) x1->palette=*y.palette;
//      }
//    if (auto x1=dynamic_cast<minsky::SwitchIcon*>(&x))
//      {
//        auto r=fmod(y.rotation,360);
//        x1->flipped=r>90 && r<270;
//        if (y.ports.size()>=2)
//          x1->setNumCases(y.ports.size()-2);
//      }
//    if (auto x1=dynamic_cast<minsky::Sheet*>(&x))
//      {
//        if (y.width) x1->m_width=*y.width;
//        if (y.height) x1->m_height=*y.height;
//      }
//    if (auto x1=dynamic_cast<minsky::Group*>(&x))
//      {
//        if (y.width) x1->iconWidth=*y.width;
//        if (y.height) x1->iconHeight=*y.height;
//        x1->bb.update(*x1);
//        if (y.name) x1->title=*y.name;
//        if (y.bookmarks) x1->bookmarks=*y.bookmarks;
//      }
//  }
//
//  void populateWire(minsky::Wire& x, const Wire& y)
//  {
//    populateNote(x,y);
//    if (y.coords)
//      x.coords(*y.coords);
//  }
//
//  struct LockGroupFactory: public shared_ptr<minsky::RavelLockGroup>
//  {
//    LockGroupFactory(): shared_ptr<minsky::RavelLockGroup>(new minsky::RavelLockGroup) {}
//  };
  

}


