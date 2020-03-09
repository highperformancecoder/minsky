/*
  @copyright Steve Keen 2020
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
   @file schema 3 is a defined and published Minsky schema. It is
expected to grow over time, deprecated attributes are also allowed,
but any renamed attributes require bumping the schema number.  
*/
#ifndef SCHEMA_3_H
#define SCHEMA_3_H

#include "model/minsky.h"
#include "model/ravelWrap.h"
#include "model/sheet.h"
#include "schema/schema2.h"
#include "schemaHelper.h"
#include "classdesc.h"
#include "polyXMLBase.h"
#include "polyJsonBase.h"
#include "rungeKutta.h"

#include <xsd_generate_base.h>
#include <vector>
#include <string>


namespace schema3
{

  using minsky::SchemaHelper;
  using namespace std;
  using namespace classdesc;
  using classdesc::shared_ptr;
  using minsky::Optional;

 
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
      x(it.m_x), y(it.m_y), zoomFactor(it.zoomFactor()), rotation(it.rotation()),
      ports(ports) {}
    ItemBase(const schema2::Item& it, const std::string& type="Item"):
      Note(it), id(it.id), type(type), x(it.x), y(it.y), zoomFactor(it.zoomFactor),
      rotation(it.rotation), ports(it.ports) {}
  };

  struct Slider
  {
    bool visible=true, stepRel=false;
    double min, max, step;
    Slider() {}
    Slider(bool v, bool stepRel, double min, double max, double step):
      visible(v), stepRel(stepRel), min(min), max(max), step(step) {}
    Slider(const schema2::Slider& s):
      visible(s.visible), stepRel(s.stepRel), min(s.min), max(s.max), step(s.step) {}
  };
    
  struct Item: public ItemBase
  {
    Optional<float> width, height;
    Optional<std::string> name; //name, description or title
    Optional<std::string> init;
    Optional<std::string> units;
    Optional<Slider> slider;
    Optional<int> intVar;
    Optional<std::map<double,double>> dataOpData;
    Optional<std::string> filename;
    Optional<minsky::RavelState> ravelState;
    Optional<int> lockGroup;
    Optional<minsky::Dimensions> dimensions;
    // Operation tensor parameters
    Optional<std::string> axis;
    Optional<double> arg;
    // Godley Icon specific fields
    Optional<std::vector<std::vector<std::string>>> data;
    Optional<std::vector<minsky::GodleyAssetClass::AssetClass>> assetClasses;
    Optional<float> iconScale; // for handling legacy schemas
    // Plot specific fields
    Optional<bool> logx, logy, ypercent;
    Optional<Plot::PlotType> plotType;
    Optional<std::string> xlabel, ylabel, y1label;
    Optional<int> nxTicks, nyTicks;
    Optional<double> xtickAngle, exp_threshold;
    Optional<ecolab::Plot::Side> legend;
    // group specific fields
    Optional<std::vector<minsky::Bookmark>> bookmarks;
    Optional<classdesc::CDATA> tensorData; // used for saving tensor data attached to parameters
    Optional<std::vector<ecolab::Plot::LineStyle>> palette;

    void packTensorInit(const minsky::VariableBase&);

    Item() {}
    Item(int id, const minsky::Item& it, const std::vector<int>& ports): ItemBase(id,it,ports) {}
    // minsky object importers
    Item(int id, const minsky::VariableBase& v, const std::vector<int>& ports):
      ItemBase(id,static_cast<const minsky::Item&>(v),ports),
      name(v.rawName()), init(v.init()) {
      if (v.sliderBoundsSet)
        slider.reset(new Slider(v.sliderVisible(),v.sliderStepRel,v.sliderMin,v.sliderMax,v.sliderStep));
      if (auto vv=v.vValue())
        units=vv->units.str();
      packTensorInit(v);
    }
    Item(int id, const minsky::OperationBase& o, const std::vector<int>& ports):
      ItemBase(id,static_cast<const minsky::Item&>(o),ports),
      axis(o.axis), arg(o.arg) {}
    Item(int id, const minsky::GodleyIcon& g, const std::vector<int>& ports):
      ItemBase(id,static_cast<const minsky::Item&>(g),ports),
      width(g.width()/g.zoomFactor()), height(g.height()/g.zoomFactor()), name(g.table.title), data(g.table.getData()),
      assetClasses(g.table._assetClass()) {}
    Item(int id, const minsky::PlotWidget& p, const std::vector<int>& ports):
      ItemBase(id,static_cast<const minsky::Item&>(p),ports),
      width(p.width), height(p.height), name(p.title),
      logx(p.logx), logy(p.logy), ypercent(p.percent),
      plotType(p.plotType),
      xlabel(p.xlabel), ylabel(p.ylabel), y1label(p.y1label),
      nxTicks(p.nxTicks), nyTicks(p.nyTicks), xtickAngle(p.xtickAngle),
      exp_threshold(p.exp_threshold), palette(p.palette)
    {
      if (p.legend) legend=p.legendSide;
    }
    Item(int id, const minsky::Sheet& s, const std::vector<int>& ports):
      ItemBase(id,static_cast<const minsky::Item&>(s),ports),
      width(s.m_width), height(s.m_height) {}
    Item(int id, const minsky::SwitchIcon& s, const std::vector<int>& ports):
      ItemBase(id, static_cast<const minsky::Item&>(s),ports) 
    {if (s.flipped) rotation=180;}
    Item(int id, const minsky::Group& g, const std::vector<int>& ports):
      ItemBase(id, static_cast<const minsky::Item&>(g),ports),
      width(g.iconWidth), height(g.iconHeight), name(g.title), bookmarks(g.bookmarks) {} 

    static Optional<classdesc::CDATA> convertTensorDataFromSchema2(const Optional<classdesc::CDATA>&);  

    Item(const schema2::Item& it):
      ItemBase(it,it.type), width(it.width), height(it.height), name(it.name), init(it.init),
      units(it.units),
      slider(it.slider), intVar(it.intVar), dataOpData(it.dataOpData), filename(it.filename),
      ravelState(it.ravelState), lockGroup(it.lockGroup), dimensions(it.dimensions),
      axis(it.axis), arg(it.arg), data(it.data), assetClasses(it.assetClasses),
      iconScale(it.iconScale), logx(it.logx), logy(it.logy), ypercent(it.ypercent),
      plotType(it.plotType), xlabel(it.xlabel), ylabel(it.ylabel), y1label(it.y1label),
      nxTicks(it.nxTicks), nyTicks(it.nyTicks), xtickAngle(it.xtickAngle),
      exp_threshold(it.exp_threshold), legend(it.legend), bookmarks(it.bookmarks),
      tensorData(convertTensorDataFromSchema2(it.tensorData)), palette(it.palette)
    {}

                 
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
    Wire(const schema2::Wire& w): Note(w), id(w.id), from(w.from), to(w.to), coords(w.coords) {}
  };

  struct Group: public Item
  {
    vector<int> items;
    Optional<vector<int>> inVariables, outVariables;
    Group() {}
    Group(int id, const minsky::Group& g): Item(id,g,std::vector<int>()) {}

    Group(const schema2::Group& g):
      Item(g), items(g.items), inVariables(g.inVariables), outVariables(g.outVariables) {}
  };


  struct Minsky
  {
    static const int version=3;
    int schemaVersion=Minsky::version;
    vector<Wire> wires;
    vector<Item> items;
    vector<Group> groups;
    minsky::RungeKutta rungeKutta;
    double zoomFactor=1;
    vector<minsky::Bookmark> bookmarks;
    minsky::Dimensions dimensions;
    minsky::ConversionsMap conversions;
    
    Minsky(): schemaVersion(0) {} // schemaVersion defined on read in
    Minsky(const minsky::Group& g);
    Minsky(const minsky::Minsky& m): Minsky(*m.model) {
      rungeKutta=m;
      zoomFactor=m.model->zoomFactor();
      bookmarks=m.model->bookmarks;
      dimensions=m.dimensions;
      conversions=m.conversions;
    }

    /// populate schema from XML data
    Minsky(classdesc::xml_unpack_t& data): schemaVersion(0)
    {minsky::loadSchema<schema2::Minsky>(*this,data,"Minsky");}
    
    Minsky(const schema2::Minsky& m):
      schemaVersion(m.schemaVersion),
      wires(m.wires.begin(), m.wires.end()),
      items(m.items.begin(), m.items.end()),
      groups(m.groups.begin(), m.groups.end()), rungeKutta(m.rungeKutta),
      zoomFactor(m.zoomFactor), bookmarks(m.bookmarks), dimensions(m.dimensions),
      conversions(m.conversions) {}
    
    /// create a Minsky model from this
    operator minsky::Minsky() const;
    /// populate a group object from this. This mutates the ids in a
    /// consistent way into the free id space of the global minsky
    /// object
    void populateGroup(minsky::Group& g) const;
  };


}

using classdesc::xsd_generate;
using classdesc::xml_pack;

#include "schema3.cd"
#include "schema3.xcd"

#endif
