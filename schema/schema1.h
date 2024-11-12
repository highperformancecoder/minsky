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

/**
   @file schema 1 is a defined and published Minsky schema. It is
expected to grow over time, deprecated attributes are also allowed,
but any renamed attributes require bumping the schema number.  
*/
#ifndef SCHEMA_1_H
#define SCHEMA_1_H

#include "model/minsky.h"
#include "schema/schema0.h"
#include "schemaHelper.h"
#include "classdesc.h"
#include "polyXMLBase.h"
#include "polyJsonBase.h"
#include "plot.xcd"
#include "simulation.h"

#include <xsd_generate_base.h>
#include <vector>
#include <string>

// Note the explicit declaration of default assignment operators is
// required to disable warnings about virtual move assigns. See ticket
// #461.

namespace schema1
{

  using minsky::SchemaHelper;
  using namespace std;
  using namespace classdesc;
  using classdesc::shared_ptr;

  // refine poly templates for current usage
  struct SPolyBase: 
    virtual public PolyBase<string>,
    virtual public PolyJsonBase, 
    virtual public PolyXMLBase
  {
  };

  template <class T, class B1, class B2=PolyBase<string> >
  struct SPoly: virtual public B1, virtual public B2
  {
    SPoly& operator=(const SPoly&)=default;
    // clone() has to return an SPoly* to satisfy covariance
    SPoly* clone() const override {return new T(static_cast<const T&>(*this));}
    string type() const override {return classdesc::typeName<T>();}

    void xml_pack(xml_pack_t& x, const string& d) const override
    {::xml_pack(x,d,static_cast<const T&>(*this));}
      
    void xml_unpack(xml_unpack_t& x, const string& d) override
    {::xml_unpack(x,d,static_cast<T&>(*this));}

    void json_pack(json_pack_t& x, const string& d) const override
    {::json_pack(x,d,static_cast<const T&>(*this));}
      
    void json_unpack(json_unpack_t& x, const string& d) override
    {::json_unpack(x,d,static_cast<T&>(*this));}
  };

  template <class T, class U>
  struct Join: virtual public T, virtual public U 
  {
    Join& operator=(const Join&)=default;
    Join* clone() const override {return new Join(*this);}
    string type() const override {return "";}
    void xml_pack(xml_pack_t& x, const string& d) const override {}
    void xml_unpack(xml_unpack_t& x, const string& d) override {}
    void json_pack(json_pack_t& x, const string& d) const override {}
    void json_unpack(json_unpack_t& x, const string& d) override {}
  };

  struct Item: public SPoly<Item, SPolyBase>
  {
    int id;
    /// commentary and short commentary on this item
    string detailedText, tooltip;
    Item(int id=-1): id(id) {}
    template <class T>
    Item(int id, const T& it): 
      id(id), detailedText(it.detailedText), tooltip(it.tooltip) {}
    // just because Operation and Variable overload "type"
    string typeName() const {return type();}
  };


  struct Port: public SPoly<Port,Item>
  {
    bool input;
    Port(): input(false) {}
  };

  struct Wire: public SPoly<Wire,Item>
  {
    int from, to;
    Wire(): from(-1), to(-1) {}
    Wire(int id, int from, int to): Item(id), from(from), to(to) {}
  };

  struct Operation: public SPoly<Operation,Item>
  {
    minsky::OperationType::Type type;
    double value;
    vector<int> ports;
    map<double,double> data; //for data operations
    string name;
    int intVar;
    Operation(): type(minsky::OperationType::numOps), value(0) {}
    Operation(int id, const schema0::Operation& op):
      Item(id), type(op.m_type), value(op.value),
      ports(op.m_ports),
      name(op.m_description.empty()? op.description: op.m_description),
      intVar(op.intVar) {}
  };

  struct Variable: public SPoly<Variable,Item>
  {
    minsky::VariableType::Type type;
    string init;
    vector<int> ports;
    string name;
    Variable(): type(minsky::VariableType::undefined), init("0") {}
    Variable(int id, const schema0::VariablePtr& v):
      Item(id), type(v.m_type), init(v.init), ports{v.m_outPort,v.m_inPort},
      name(v.name) {}
  };

  // why is the schema1 qualifier needed here?
  struct Plot: public SPoly<schema1::Plot,Item>
  {
    typedef ecolab::Plot::Side Side;
    vector<int> ports;
    shared_ptr<Side> legend;
    bool logx{0}, logy{0};
    string title, xlabel, ylabel, y1label;
    Plot() {}
    Plot(int id, const schema0::PlotWidget& p): 
      Item(id), ports(p.ports.begin(),p.ports.end()), logx(p.logx), logy(p.logy) {}
  };

  struct Group: public SPoly<Group,Item>
  {
    //the following field is left commented out here to indicate this
    //deprecated field is part of the version 1 spec
    //vector<int> ports; 
    vector<int> items;
    vector<int> ports;
    vector<int> createdVars;
    string name;
    Group() {}
    /// note this assumes that ids have been uniquified prior to this call
    Group(int id, const schema0::GroupIcon& g):
      Item(id), items(g.operations), ports(g.m_ports.begin(),g.m_ports.end()) {
      items.insert(items.end(), g.variables.begin(), g.variables.end());
    }
  };

  struct Switch: public SPoly<Switch,Item>
  {
    vector<int> ports;
    Switch() {}
  };

  struct Godley: public SPoly<Godley,Item>
  {
    vector<int> ports;
    bool doubleEntryCompliant=true;
    string name;
    vector<vector<string> > data;
    vector<minsky::GodleyTable::AssetClass> assetClasses;
    double zoomFactor=1;
    Godley() {}
    Godley(int id, const schema0::GodleyIcon& g):
      Item(id),
      doubleEntryCompliant(g.table.doubleEntryCompliant),
      name(g.table.title), data(g.table.data),
      assetClasses(g.table.m_assetClass)
    {
      for (auto& i: g.flowVars) ports.push_back(i.m_inPort);
      for (auto& i: g.stockVars) ports.push_back(i.m_outPort);
    }
  };

  struct Layout: public SPoly<Layout, SPolyBase>
  {
    int id;
    Layout(int id=-1): id(id) {}
    virtual ~Layout() {}
  };

  /// represent objects whose layouts just have a position (ports,
  /// plots, godleyIcons)
  struct PositionLayout: public SPoly<PositionLayout, Layout>
  {
    double x=0, y=0;

    PositionLayout() {}
    PositionLayout(int id, double x, double y): Layout(id), x(x), y(y) {}
    template <class T> PositionLayout(int id, const T& item): 
      Layout(id), x(item.m_x), y(item.m_y) {}
    PositionLayout(int id, const schema0::Operation& o):
      Layout(id), x(o.x), y(o.y) {}
    PositionLayout(int id, const schema0::Variable& v):
      Layout(id), x(v.x), y(v.y) {}
    PositionLayout(int id, const schema0::GroupIcon& g):
      Layout(id), x(g.x), y(g.y) {}
    PositionLayout(int id, const schema0::PlotWidget& p):
      Layout(id), x(p.x), y(p.y) {}
  };

  /// represents items with a visibility attribute
  struct VisibilityLayout
  {
    bool visible;
    VisibilityLayout(bool visible=true): visible(visible) {}
    template <class T> VisibilityLayout(const T& item):
      visible(item.visible()) {}
    VisibilityLayout(const schema0::Operation& item):
      visible(item.visible) {}
    VisibilityLayout(const schema0::Variable& item):
      visible(item.visible) {}
    VisibilityLayout(const schema0::GroupIcon& item):
      visible(true) {}
  };

  struct SizeLayout
  {
    double width=-1, height=-1; // -ve indicates schema file does not supply sizelayout info
    SizeLayout() {}
    template <class T>
    SizeLayout(const T& x): width(x.width), height(x.height) {}
    SizeLayout(const schema0::PlotWidget&) {}
  };

  /// represents layouts of wires
  struct WireLayout: public SPoly<WireLayout, Layout, VisibilityLayout>
  {
    vector<float> coords;
    
    WireLayout() {}
    WireLayout(int id, const schema0::Wire& wire): 
      Layout(id), VisibilityLayout(wire.visible), 
      coords(wire.coords.begin(),wire.coords.end()) {}
  };

  /// represents layouts of objects like variables and operators
  struct ItemLayout: public SPoly<ItemLayout, Layout, 
                                Join<PositionLayout, VisibilityLayout> >
  {
    double rotation=0;

    ItemLayout() {}
    template <class T> ItemLayout(int id, const T& item): 
      Layout(id), PositionLayout(id, item), VisibilityLayout(item),
      rotation(item.rotation) {}
  };


  /// group layouts also have a width & height
  struct GroupLayout: public SPoly<GroupLayout, ItemLayout, SizeLayout>
  {
    double displayZoom=1;
    GroupLayout(): displayZoom(1) {}
    GroupLayout(int id, const schema0::GroupIcon& g):
      Layout(id), PositionLayout(id, g.x, g.y), ItemLayout(id, g), SizeLayout(g) {}
  };

  struct PlotLayout: public SPoly<PlotLayout, PositionLayout, SizeLayout>
  {
    PlotLayout() {}
    PlotLayout(int id, const schema0::PlotWidget& p):
      Layout(id), PositionLayout(id, p), SizeLayout(p) {}
  };

  /// describes item with sliders - currently just constants
  struct SliderLayout: public SPoly<SliderLayout, ItemLayout>
  {
    bool sliderVisible=false, sliderBoundsSet=false, sliderStepRel=false;
    double sliderMin=0, sliderMax=0, sliderStep=0;
    SliderLayout() {}
    template <class T>
    SliderLayout(int id, const T& item):
      Layout(id), PositionLayout(id, item), VisibilityLayout(item), 
      ItemLayout(id, item), sliderVisible(item.sliderVisible()),
      sliderBoundsSet(item.sliderBoundsSet), sliderStepRel(item.sliderStepRel),
      sliderMin(item.sliderMin), sliderMax(item.sliderMax), 
      sliderStep(item.sliderStep) {}
  };

  /// structure representing a union of all of the above Layout
  /// classes, for xml_unpack
  struct UnionLayout: public SPoly<UnionLayout, 
                                   Join<SliderLayout, GroupLayout>, 
                                   Join<PlotLayout, WireLayout> >
  {
    UnionLayout() {}
    UnionLayout(const Layout&);
  };

  struct MinskyModel
  {
    //the following field is left commented out here to indicate this
    //deprecated field is part of the version 1 spec
    //    vector<Port> ports;
    vector<Wire> wires;
    vector<Item> notes; ///< descriptive notes
    vector<Operation> operations;
    vector<Variable> variables;
    vector<Plot> plots;
    vector<Group> groups;
    vector<Switch> switches;
    vector<Godley> godleys;
    minsky::Simulation rungeKutta;
  };

  struct Minsky
  {
    static const int version=1;
    int schemaVersion=Minsky::version;
    MinskyModel model;
    vector<shared_ptr<Layout> > layout;
    double zoomFactor=1;
    Minsky(): schemaVersion(-1) {} // schemaVersion defined on read in
    Minsky(const schema0::Minsky& m);
    
    /// populate schema from XML data
    Minsky(classdesc::xml_unpack_t& data): schemaVersion(0)
    {
      minsky::loadSchema<schema0::Minsky>(*this,data,"Minsky");
      removeIntVarOrphans();
    }

    /** See ticket #329 and references within. At some stage, IntOp had
        no destructor, which leads to an orphaned, invisible integral
        variable, with invalid output port. This bit of code deals with
        corrupt schema files that have been caused by this problem
    */
    void removeIntVarOrphans();


  };

  // Item and Layout factory
  template <class T> std::unique_ptr<T> factory(const std::string&);

}

namespace classdesc
{
  // we provide a specialisation here, to ensure our intended schema
  // is as a "vector of Layouts"
  template <> inline std::string typeName<shared_ptr<schema1::Layout> >() 
  {return "schema1::Layout";}
}

namespace classdesc_access
{
  namespace cd=classdesc;
  template <> struct access_xml_pack<std::shared_ptr<schema1::Layout> >
  {
    template <class U>
    void operator()(cd::xml_pack_t& x, const cd::string& d, U& a)
    {a->xml_pack(x,d);}
  };

  /// unpack into a UnionLayout structure, so everything's at hand 
  template <>struct access_xml_unpack<std::shared_ptr<schema1::Layout> >
  {
    template <class U>
    void operator()(cd::xml_unpack_t& x, const cd::string& d, U& a)
    {
      a.reset(new schema1::UnionLayout);
      ::xml_unpack(x, d, dynamic_cast<schema1::UnionLayout&>(*a));
    }
  };

}

using classdesc::xsd_generate;


#ifdef _CLASSDESC
#pragma omit xsd_generate schema1::SPolyBase
#pragma omit xsd_generate schema1::SPoly
#endif

inline void xsd_generate(classdesc::xsd_generate_t&,const std::string&,const schema1::SPolyBase&) {}
template <class T, class B1, class B2>
void xsd_generate(classdesc::xsd_generate_t& x,const std::string& d, 
                  const schema1::SPoly<T,B1,B2>& a) 
{xsd_generate(x,d,static_cast<const B1&>(a));}

// Layout is end of the line, no need to process further
inline void xsd_generate(classdesc::xsd_generate_t& x,const std::string& d, 
                  const schema1::SPoly<schema1::Layout,schema1::SPolyBase>& a) 
{}


#include "schema1.cd"
#include "schema1.xcd"
#include "enumerateSchema1.h"


#endif
