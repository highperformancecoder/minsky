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
    memcpy(a.begin(),data.data(),data.size()*sizeof(data[0]));
  }

  struct Schema1Layout
  {
    map<int,schema1::UnionLayout> layout;
    Schema1Layout(const vector<shared_ptr<schema1::Layout>>& x) {
      for (auto& i: x)
        {
          if (auto ul=dynamic_cast<schema1::UnionLayout*>(i.get()))
            layout[i->id]=*ul;
          else
            {
              // TODO: I don't think this code will ever be executed?
              // serialise to XML, then deserialise to a UnionLayout
              ostringstream is;
              xml_pack_t xmlPackBuf(is);
              i->xml_pack(xmlPackBuf,"");
              istringstream os(is.str());
              xml_unpack_t xmlUnpackBuf(os);
              xml_unpack(xmlUnpackBuf,"",layout[i->id]);
            }
        }
    }
    template <class V, class O>
    void addItem(V& vec, const O& item) {
      vec.emplace_back(item);
      if (layout.contains(item.id))
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

}


