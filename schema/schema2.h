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

/**
   @file schema 2 is a defined and published Minsky schema. It is
expected to grow over time, deprecated attributes are also allowed,
but any renamed attributes require bumping the schema number.  
*/
#ifndef SCHEMA_2_H
#define SCHEMA_2_H

#include "model/minsky.h"
#include "schema/schema1.h"
#include "schemaHelper.h"
#include "classdesc.h"
#include "polyXMLBase.h"
#include "polyJsonBase.h"

#include <xsd_generate_base.h>
#include <vector>
#include <string>

namespace schema2
{

  using minsky::SchemaHelper;
  using namespace std;
  using namespace classdesc;
  using classdesc::shared_ptr;


  // see discussion in Stroustrup 4th ed 28.4.4
  template <class T> struct has_empty
  {
    template <class X>
    static auto check(X x)->decltype(x.empty());
    static std::false_type check(...);
    
    static constexpr const bool value = std::is_integral<decltype(check(std::declval<T>()))>::value;
  };
   
  /// convenience class to omit writing XML records when data absent or empty
  template <class T>
  struct Optional: shared_ptr<T>
  {
    Optional() {}
    Optional(const T& x) {assign(x);}
    template <class U>
    typename classdesc::enable_if<has_empty<U>,void>::T
    assign(const U& x, classdesc::dummy<0> d=0) {
      if (!x.empty()) this->reset(new U(x));
    }
    template <class U>
    typename classdesc::enable_if<Not<has_empty<U>>,void>::T
    assign(const U& x, classdesc::dummy<1> d=0) {this->reset(new U(x));}

    // if we access an optional, then create its target
    T& operator*() {if (!this->get()) this->reset(new T); return *this->get();}
    const T& operator*() const {return *this->get();}
    T* operator->() {return &**this;}
    const T* operator->() const {return &**this;}

    template <class U> Optional& operator=(const U& x) {**this=x; return *this;}
  };

 
  struct Note
  {
    Optional<std::string> detailedText, tooltip;
    Note() {}
    template <class T>
    Note(const T& x): detailedText(x.detailedText), tooltip(x.tooltip) {}
  };

  // attribute common to all items
  struct ItemBase: public Note
  {
    int id=-1;
    std::string type;
    float x=0, y=0; ///< position in canvas, or within group
    float zoomFactor=1;
    double rotation=0; ///< rotation of icon, in degrees
    std::vector<int> ports;
    ItemBase() {}
    ItemBase(int id, const minsky::Item& it, const std::vector<int>& ports): 
      Note(it), id(id), type(it.classType()),
      x(it.m_x), y(it.m_y), zoomFactor(it.zoomFactor), rotation(it.rotation),
      ports(ports) {}
    ItemBase(const schema1::Item& it, const std::string& type="Item"):
      Note(it), id(it.id), type(type) {}
  };

  struct Slider
  {
    bool visible=true, stepRel=false;
    double min, max, step;
    Slider() {}
    Slider(bool v, bool stepRel, double min, double max, double step):
      visible(v), stepRel(stepRel), min(min), max(max), step(step) {}
  };
    
  struct Item: public ItemBase
  {
    Optional<float> width, height;
    Optional<std::string> name; //name, description or title
    Optional<std::string> init;
    Optional<Slider> slider;
    std::shared_ptr<int> intVar;
    Optional<std::map<double,double>> dataOpData;
    Optional<std::string> filename;
    Optional<std::string> ravelDef;
    // Godley Icon specific fields
    Optional<std::vector<std::vector<std::string>>> data;
    Optional<std::vector<minsky::GodleyAssetClass::AssetClass>> assetClasses;
    Optional<float> iconScale; // for handling legacy schemas
    // Plot specific fields
    Optional<bool> logx, logy;
    Optional<std::string> xlabel, ylabel, y1label;
    std::shared_ptr<ecolab::Plot::Side> legend;

    Item() {}
    Item(int id, const minsky::Item& it, const std::vector<int>& ports): ItemBase(id,it,ports) {}
    // minsky object importers
    Item(const schema1::Item& it): ItemBase(it) {}
    Item(int id, const minsky::VariableBase& v, const std::vector<int>& ports):
      ItemBase(id,static_cast<const minsky::Item&>(v),ports),
      name(v.rawName()), init(v.init()) {
      if (v.sliderBoundsSet)
        slider.reset(new Slider(v.sliderVisible(),v.sliderStepRel,v.sliderMin,v.sliderMax,v.sliderStep));
    }
    Item(int id, const minsky::GodleyIcon& g, const std::vector<int>& ports):
      ItemBase(id,static_cast<const minsky::Item&>(g),ports),
      width(g.width()/g.zoomFactor), height(g.height()/g.zoomFactor), name(g.table.title), data(g.table.getData()),
      assetClasses(g.table._assetClass()) {}
    Item(int id, const minsky::PlotWidget& p, const std::vector<int>& ports):
      ItemBase(id,static_cast<const minsky::Item&>(p),ports),
      width(p.width), height(p.height), name(p.title), logx(p.logx), logy(p.logy),
      xlabel(p.xlabel), ylabel(p.ylabel), y1label(p.y1label),
      legend(p.legend? new ecolab::Plot::Side(p.legendSide): nullptr) {}
    Item(int id, const minsky::SwitchIcon& s, const std::vector<int>& ports):
      ItemBase(id, static_cast<const minsky::Item&>(s),ports) 
    {if (s.flipped) rotation=180;}
    Item(int id, const minsky::Group& g, const std::vector<int>& ports):
      ItemBase(id, static_cast<const minsky::Item&>(g),ports),
      width(g.width), height(g.height), name(g.title) {} 

    // schema1 importers
    Item(const schema1::Operation& it):
      ItemBase(it, "Operation:"+minsky::OperationType::typeName(it.type)),
      name(it.name), intVar(it.intVar>-1? new int(it.intVar): nullptr),
      dataOpData(it.data) {
      ports=it.ports;
      // rewrite deprecated constants as variables
      if (it.type==minsky::OperationType::constant)
        {
          type="Variable:constant";
          init.reset(new std::string(std::to_string(it.value)));
        }
    }
    Item(const schema1::Variable& it):
      ItemBase(it, "Variable:"+minsky::VariableType::typeName(it.type)),
      name(it.name), init(it.init)  {ports=it.ports;}
    Item(const schema1::Godley& it);
    Item(const schema1::Plot& it):
      ItemBase(it, "PlotWidget"),
      name(it.title), logx(it.logx), logy(it.logy), xlabel(it.xlabel), ylabel(it.ylabel), y1label(it.y1label),
      legend(it.legend) {ports=it.ports;}
    Item(const schema1::Group& it): ItemBase(it,"Group"), name(it.name) {} 
    Item(const schema1::Switch& it): ItemBase(it,"SwitchIcon") {ports=it.ports;} 

    void addLayout(const schema1::UnionLayout& layout) {
      x=layout.x;
      y=layout.y;
      rotation=layout.rotation;
      width.reset(new float(layout.width));
      height.reset(new float(layout.height));
      if (layout.sliderBoundsSet)
        slider.reset(new Slider(layout.sliderVisible,layout.sliderStepRel,
                                layout.sliderMin,layout.sliderMax,layout.sliderStep));
    }
  };


  struct Wire: public Note
  {
    int id=-1;
    int from=-1, to=-1;
    Optional<std::vector<float>> coords;
    Wire() {}
    Wire(int id, const minsky::Wire& w): Note(w), id(id) {
      if (w.coords().size()>4)
        coords.reset(new std::vector<float>(w.coords()));
     }
    Wire(const schema1::Wire& w): Note(w), id(w.id), from(w.from), to(w.to) {}
    void addLayout(const schema1::UnionLayout& layout) {
      if (layout.coords.size()>4)
        coords.reset(new std::vector<float>(layout.coords));
    }
  };

  struct Group: public Item
  {
    vector<int> items;
    Optional<vector<int>> inVariables, outVariables;
    Group() {}
    Group(int id, const minsky::Group& g): Item(id,g,std::vector<int>()) {}

    /// note this assumes that ids have been uniquified prior to this call
    Group(const schema1::Group& g):
      Item(g), items(g.items) {}
  };


  struct RungeKutta
  {
    double stepMin{0}, stepMax{0.01};
    int nSteps{1};
    double epsRel{1e-2}, epsAbs{1e-3};
    int order{4};
    bool implicit{false};
    int simulationDelay{0};
    RungeKutta() {}
    RungeKutta(const minsky::Minsky& m):
      stepMin(m.stepMin), stepMax(m.stepMax), nSteps(m.nSteps),
      epsRel(m.epsRel), epsAbs(m.epsAbs), order(m.order), 
      implicit(m.implicit), simulationDelay(m.simulationDelay) {}
    RungeKutta(const schema1::RungeKutta& m):
      stepMin(m.stepMin), stepMax(m.stepMax), nSteps(m.nSteps),
      epsRel(m.epsRel), epsAbs(m.epsAbs), order(m.order), 
      implicit(m.implicit), simulationDelay(m.simulationDelay) {}
    
  };

  struct Minsky
  {
    static const int version=2;
    int schemaVersion=Minsky::version;
    vector<Wire> wires;
    vector<Item> items;
    vector<Group> groups;
    RungeKutta rungeKutta;
    double zoomFactor=1;

    /// checks that all items are uniquely identified.
    //bool validate() const;
    Minsky(): schemaVersion(0) {} // schemaVersion defined on read in
    Minsky(const minsky::Group& g);
    Minsky(const minsky::Minsky& m): Minsky(*m.model) {
      rungeKutta=RungeKutta(m);
      zoomFactor=m.model->zoomFactor;
      //assert(validate());
    }

    Minsky(const schema1::Minsky& m);
    
    /// create a Minsky model from this
    operator minsky::Minsky() const;
    /// populate a group object from this. This mutates the ids in a
    /// consistent way into the free id space of the global minsky
    /// object
    void populateGroup(minsky::Group& g) const;
    /// move locations such that minx, miny lies at (0,0) on canvas
    //    void relocateCanvas();

  };


}

  /*
    This code ensure optional fields are not exported when empty This
    is really clunky, as partial function specialisations are not
    valid C++, so we need explicit specialisations for all types used
    in this schema.

    TODO: fix classdesc code to explicitly allow optional XML fields.
  */

namespace classdesc
{

#ifdef _CLASSDESC
#pragma omit xsd_generate schema2::Optional
#pragma omit xml_pack schema2::Optional
#endif

  template <class T>
  void xsd_generate(xsd_generate_t& g, const string& d, const schema2::Optional<T>& a) 
  {
    xsd_generate_t::Optional o(g,true); 
    xsd_generate(g,d,*a);
  }

  template <class T> void xpack(xml_pack_t& t,const string& d,T& a) 
  {if (a) ::xml_pack(t,d,*a);}


  inline void xml_pack(xml_pack_t& t,const string& d,schema2::Optional<std::string>& a) {xpack(t,d,a);}
  inline void xml_pack(xml_pack_t& t,const string& d,schema2::Optional<float>& a) {xpack(t,d,a);}
  inline void xml_pack(xml_pack_t& t,const string& d,schema2::Optional<bool>& a) {xpack(t,d,a);}
  inline void xml_pack(xml_pack_t& t,const string& d,std::shared_ptr<int>& a) {xpack(t,d,a);}
  inline void xml_pack(xml_pack_t& t,const string& d,
                       schema2::Optional<std::map<double,double>>& a) {xpack(t,d,a);}
  inline void xml_pack(xml_pack_t& t,const string& d,
                       schema2::Optional<std::vector<std::vector<std::string>>>& a) {xpack(t,d,a);}
  inline void xml_pack(xml_pack_t& t,const string& d,
                       schema2::Optional<std::vector<float>>& a) {xpack(t,d,a);}
  inline void xml_pack(xml_pack_t& t,const string& d,
                       schema2::Optional<std::vector<int>>& a) {xpack(t,d,a);}
  inline void xml_pack(xml_pack_t& t,const string& d,
                       schema2::Optional<std::vector<minsky::GodleyAssetClass::AssetClass>>& a) {xpack(t,d,a);}
  inline void xml_pack(xml_pack_t& t,const string& d,
                       schema2::Optional<schema2::Slider>& a) {xpack(t,d,a);}
  inline void xml_pack(xml_pack_t& t,const string& d,
                       std::shared_ptr<ecolab::Plot::Side>& a) {xpack(t,d,a);}
}

using classdesc::xsd_generate;
using classdesc::xml_pack;

#include "schema2.cd"
#include "schema2.xcd"

#endif
