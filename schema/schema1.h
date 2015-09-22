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

#include "GUI/minsky.h"
#include "schemaHelper.h"
#include "classdesc.h"
#include "polyXMLBase.h"
#include "polyJsonBase.h"
#include "plot.xcd"

#include <xsd_generate_base.h>
#include <vector>
#include <string>

// see ticket #461. This potentially masks a serious bug
#pragma GCC diagnostic ignored "-Wvirtual-move-assign"

namespace schema1
{

  using minsky::SchemaHelper;
  using namespace std;
  using namespace classdesc;
  using classdesc::shared_ptr;

  // refine poly templates for current usage
  struct SPolyBase: 
    virtual public PolyBase<string>,
    virtual public PolyPackBase,
    virtual public PolyJsonBase, 
    virtual public PolyXMLBase
  {
    virtual string json() const=0;
    virtual string json(const string& s)=0;
  };

  template <class T, class B1, class B2=PolyBase<string> >
  struct SPoly: virtual public B1, virtual public B2
  {
    // clone() has to return an SPoly* to satsify covariance
    SPoly* clone() const {return new T(static_cast<const T&>(*this));}
    string type() const {return classdesc::typeName<T>();}

    void pack(pack_t& x, const string& d) const
    {::pack(x,d,static_cast<const T&>(*this));}
      
    void unpack(unpack_t& x, const string& d)
    {::unpack(x,d,static_cast<T&>(*this));}

    void xml_pack(xml_pack_t& x, const string& d) const
    {::xml_pack(x,d,static_cast<const T&>(*this));}
      
    void xml_unpack(xml_unpack_t& x, const string& d)
    {::xml_unpack(x,d,static_cast<T&>(*this));}

    void json_pack(json_pack_t& x, const string& d) const
    {::json_pack(x,d,static_cast<const T&>(*this));}
      
    void json_unpack(json_unpack_t& x, const string& d)
    {::json_unpack(x,d,static_cast<T&>(*this));}

    string json() const {
      json_pack_t j;
      this->json_pack(j,"");
      return write(j);
    }
    string json(const string& s) {
      json_pack_t j;
      read(s, j);
      this->json_unpack(j,"");
      return s;
    }
  };

  template <class T, class U>
  struct Join: virtual public T, virtual public U 
  {
    Join* clone() const {return new Join(*this);}
    string type() const {return "";}
    void pack(pack_t& x, const string& d) const {}
    void unpack(unpack_t& x, const string& d) {}
    void xml_pack(xml_pack_t& x, const string& d) const {}
    void xml_unpack(xml_unpack_t& x, const string& d) {}
    void json_pack(json_pack_t& x, const string& d) const {}
    void json_unpack(json_unpack_t& x, const string& d) {}
    string json() const {return "";}
    string json(const string& s) {return "";}
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
    Port(int id, const minsky::Port& p): Item(id), input(p.input()) {}
  };

  struct Wire: public SPoly<Wire,Item>
  {
    int from, to;
    Wire(): from(-1), to(-1) {}
    Wire(int id, const minsky::Wire& w): Item(id,w), from(w.from), to(w.to) {}
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
    Operation(int id, const minsky::OperationBase& op); 
  };

  struct Variable: public SPoly<Variable,Item>
  {
    VariableType::Type type;
    string init;
    vector<int> ports;
    string name;
    Variable(): type(VariableType::undefined), init("0") {}
    Variable(int id, const minsky::VariableBase& v): 
      Item(id,v), type(v.type()), init(v.init()), ports(v.ports()), 
      name(v.fqName()) {}
  };

  // why is the schema1 qualifier needed here?
  struct Plot: public SPoly<schema1::Plot,Item>
  {
    typedef ecolab::Plot::Side Side;
    vector<int> ports;
    shared_ptr<Side> legend;
    string title, xlabel, ylabel, y1label;
    Plot() {}
    Plot(int id, const minsky::PlotWidget& p): 
      Item(id,p), ports(toVector(p.ports())), 
      legend(p.legend? new Side(p.legendSide): NULL),
      title(p.title), xlabel(p.xlabel), ylabel(p.ylabel), y1label(p.y1label) {}
  };

  struct Group: public SPoly<Group,Item>
  {
    vector<int> items;
    vector<int> ports;
    vector<int> createdVars;
    string name;
    Group() {}
    Group(int id, const minsky::GroupIcon& g): 
      Item(id,g), ports(g.ports()), name(g.name()) {}
    // not called from constructor, as we may want to renumber items
    void addItems(const minsky::GroupIcon& g);
  };

  struct Switch: public SPoly<Switch,Item>
  {
    vector<int> ports;
    Switch() {}
    Switch(int id, const minsky::SwitchIcon& s):
      Item(id,s), ports(s.ports()) {}
  };

  struct Godley: public SPoly<Godley,Item>
  {
    vector<int> ports;
    bool doubleEntryCompliant;
    string name;
    vector<vector<string> > data;
    vector<minsky::GodleyTable::AssetClass> assetClasses;
    double zoomFactor;
    Godley(): doubleEntryCompliant(true), zoomFactor(1) {}
    Godley(int id, const minsky::GodleyIcon& g):
      Item(id,g), ports(g.ports()), 
      doubleEntryCompliant(g.table.doubleEntryCompliant),
      name(g.table.title), data(g.table.getData()), 
      assetClasses(g.table._assetClass()),
      zoomFactor(g.zoomFactor) {}
  };

  struct Layout: public SPoly<Layout, SPolyBase>
  {
    int id;
    Layout(int id=-1): id(id) {}
    virtual ~Layout() {}
    static Layout* create(const string&);
  };

  /// represent objects whose layouts just have a position (ports,
  /// plots, godleyIcons)
  struct PositionLayout: public SPoly<PositionLayout, Layout>
  {
    double x,y;

    PositionLayout(): x(0), y(0) {}
    template <class T> PositionLayout(int id, const T& item): 
      Layout(id), x(SchemaHelper::x(item)), y(SchemaHelper::y(item)) {}
  };

  /// represents items with a visibility attribute
  struct VisibilityLayout
  {
    bool visible;
    VisibilityLayout(): visible(true) {}
    template <class T> VisibilityLayout(const T& item):
      visible(item.visible) {}
  };

  struct SizeLayout
  {
    double width, height;
    SizeLayout() {}
    template <class T>
    SizeLayout(const T& x): width(x.width), height(x.height) {}
  };

  /// represents layouts of wires
  struct WireLayout: public SPoly<WireLayout, Layout, VisibilityLayout>
  {
    vector<float> coords;
    
    WireLayout() {}
    WireLayout(int id, const minsky::Wire& wire): 
      Layout(id), VisibilityLayout(wire), 
      coords(toVector(SchemaHelper::coords(wire))) {}
  };

  /// represents layouts of objects like variables and operators
struct ItemLayout: public SPoly<ItemLayout, Layout, 
                                Join<PositionLayout, VisibilityLayout> >
  {
    double rotation;

    ItemLayout() {}
    template <class T> ItemLayout(int id, const T& item): 
      Layout(id), PositionLayout(id, item), VisibilityLayout(item),
      rotation(item.rotation) {}
  };


  /// group layouts also have a width & height
  struct GroupLayout: public SPoly<GroupLayout, ItemLayout, SizeLayout>
  {
    double displayZoom;
    GroupLayout(): displayZoom(1) {}
    GroupLayout(int id, const minsky::GroupIcon& g):
      Layout(id), PositionLayout(id, g), VisibilityLayout(g),
      ItemLayout(id, g), SizeLayout(g), 
      displayZoom(g.displayZoom) {}
  };

  struct PlotLayout: public SPoly<PlotLayout, PositionLayout, SizeLayout>
  {
    PlotLayout() {width=150; height=150;}
    PlotLayout(int id, const minsky::PlotWidget& p):
      Layout(id), PositionLayout(id, p), SizeLayout(p) {}
  };

  /// describes item with sliders - currently just constants
  struct SliderLayout: public SPoly<SliderLayout, ItemLayout>
  {
    bool sliderVisible, sliderBoundsSet, sliderStepRel;
    double sliderMin, sliderMax, sliderStep;
    SliderLayout(): sliderVisible(false), sliderBoundsSet(false), sliderStepRel(false) {}
    template <class T>
    SliderLayout(int id, const T& item):
      Layout(id), PositionLayout(id, item), VisibilityLayout(item), 
      ItemLayout(id, item), sliderVisible(item.sliderVisible),
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
  };

  struct MinskyModel
  {
    vector<Port> ports;
    vector<Wire> wires;
    vector<Item> notes; ///< descriptive notes
    vector<Operation> operations;
    vector<Variable> variables;
    vector<Plot> plots;
    vector<Group> groups;
    vector<Switch> switches;
    vector<Godley> godleys;
    RungeKutta rungeKutta;

    /// checks that all items are uniquely identified.
    bool validate() const;
  };

  struct Minsky
  {
    static const int version=1;
    int schemaVersion;
    MinskyModel model;
    vector<shared_ptr<Layout> > layout;
    double zoomFactor;
    Minsky(): schemaVersion(-1), zoomFactor(1) {} // schemaVersion defined on read in
    Minsky(const minsky::Minsky& m);
    /// construct a schema object containing contents of a selection
    Minsky(const minsky::Minsky& m, const minsky::Selection&);
    Minsky(const minsky::Minsky& m, const minsky::GroupIcon& g):
      schemaVersion(version), zoomFactor(m.zoomFactor())
    {populateWith(m,g,true);}
      
    /// fills a schema object from the contents of \a g. \a
    /// visible refers to whether the contents of \a g will be visible
    void populateWith(const minsky::Minsky& m, const minsky::GroupIcon& g, bool visible=true);

    /// create a Minsky model from this
    operator minsky::Minsky() const;
    /// populate a group object from this. This mutates the ids in a
    /// consistent way into the free id space of the global minsky
    /// object
    void populateGroup(minsky::GroupIcon& g);
    /// move locations such that minx, miny lies at (0,0) on canvas
    void relocateCanvas();

    /** See ticket #329 and references within. At some stage, IntOp had
        no destructor, which leads to an orphaned, invisible integral
        variable, with invalid output port. This bit of code deals with
        corrupt schema files that have been caused by this problem
    */
    void removeIntVarOrphans();


  };

  // Item and Layout factory
  template <class T> std::auto_ptr<T> factory(const std::string&);

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
  template <> struct access_xml_pack<shared_ptr<schema1::Layout> >
  {
    template <class U>
    void operator()(cd::xml_pack_t& x, const cd::string& d, U& a)
    {a->xml_pack(x,d);}
  };

  /// unpack into a UnionLayout structure, so everything's at hand 
  template <>struct access_xml_unpack<shared_ptr<schema1::Layout> >
  {
    template <class U>
    void operator()(cd::xml_unpack_t& x, const cd::string& d, U& a)
    {
      a.reset(new schema1::UnionLayout);
      ::xml_unpack(x, d, dynamic_cast<schema1::UnionLayout&>(*a));
    }
  };

  template <>struct access_json_pack<shared_ptr<schema1::Layout> >
  {
    template <class U>
    void operator()(json_pack_t& x, const string& d, U& a)
    {a->json_pack(x,d);}
  };

  /// unpack into a UnionLayout structure, so everything's at hand 
  template <>struct access_json_unpack<shared_ptr<schema1::Layout> >
  {
    template <class U>
    void operator()(json_unpack_t& x, const string& d, U& a)
    {
      a.reset(new schema1::UnionLayout);
      ::json_unpack(x, d, dynamic_cast<schema1::UnionLayout&>(*a));
    }
  };
  
}

using classdesc::xsd_generate;


inline void json_pack(classdesc::json_pack_t& j, const std::string& d, const std::vector<float>& x)
{
  std::vector<double> dx(x.begin(), x.end());
  json_pack(j,d,dx);
}

inline void json_unpack(classdesc::json_pack_t& j, const std::string& d, std::vector<float>& x)
{
  std::vector<double> dx;
  json_unpack(j,d,dx);
  x.clear();
  x.insert(x.end(), dx.begin(), dx.end());
}

#ifdef _CLASSDESC
#pragma omit xsd_generate schema1::SPolyBase
#pragma omit xsd_generate schema1::SPoly
#endif

inline void xsd_generate(classdesc::xsd_generate_t&,const string&,const schema1::SPolyBase&) {}
template <class T, class B1, class B2>
void xsd_generate(classdesc::xsd_generate_t& x,const string& d, 
                  const schema1::SPoly<T,B1,B2>& a) 
{xsd_generate(x,d,static_cast<const B1&>(a));}

// Layout is end of the line, no need to process further
inline void xsd_generate(classdesc::xsd_generate_t& x,const string& d, 
                  const schema1::SPoly<schema1::Layout,schema1::SPolyBase>& a) 
{}


#include "schema1.cd"
#include "schema1.xcd"
#include "enumerateSchema1.h"


#endif
