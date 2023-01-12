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
#include "model/ravelWrap.h"
#include "model/sheet.h"
#include "schema/schema1.h"
#include "schemaHelper.h"
#include "classdesc.h"
#include "polyXMLBase.h"
#include "polyJsonBase.h"
#include "simulation.h"
#include "ravelState-schema2.h"

#include "optional.h"

#include <xsd_generate_base.h>
#include "xml_common.xcd"
//#include "plot.xcd"
#include <vector>
#include <string>


namespace schema2
{

  using minsky::SchemaHelper;
  using namespace std;
  using namespace classdesc;
  using classdesc::shared_ptr;
  using minsky::Optional;

  /// unpack a TensorVal from a pack_t buffer. Schema2 encoding.
  void unpack(classdesc::pack_t&, civita::TensorVal&);

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
    Optional<std::string> units;
    Optional<Slider> slider;
    Optional<int> intVar;
    Optional<std::map<double,double>> dataOpData;
    Optional<std::string> filename;
    Optional<RavelState> ravelState;
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
    Optional<ecolab::Plot::PlotType> plotType;
    Optional<std::string> xlabel, ylabel, y1label;
    Optional<int> nxTicks, nyTicks;
    Optional<double> xtickAngle, exp_threshold;
    Optional<ecolab::Plot::Side> legend;
    // group specific fields
    Optional<std::vector<minsky::Bookmark>> bookmarks;
    Optional<classdesc::CDATA> tensorData; // used for saving tensor data attached to parameters
    Optional<std::vector<ecolab::Plot::LineStyle>> palette;

    Item() {}
    // minsky object importers
    Item(const schema1::Item& it): ItemBase(it) {}

    // schema1 importers
    Item(const schema1::Operation& it):
      ItemBase(it, "Operation:"+minsky::OperationType::typeName(it.type)),
      name(it.name), dataOpData(it.data) {
      if (it.intVar>-1) intVar=it.intVar;
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
      name(it.title), logx(it.logx), logy(it.logy), xlabel(it.xlabel), ylabel(it.ylabel), y1label(it.y1label)
    {
      if (it.legend) legend=*it.legend;
      ports=it.ports;
    }
    Item(const schema1::Group& it): ItemBase(it,"Group"), name(it.name) {} 
    Item(const schema1::Switch& it): ItemBase(it,"SwitchIcon") {ports=it.ports;} 

    void addLayout(const schema1::UnionLayout& layout) {
      x=layout.x;
      y=layout.y;
      rotation=layout.rotation;
      if (layout.width>=0)
        width.reset(new float(layout.width));
      else if (type=="PlotWidget")
        width.reset(new float(150));
      if (layout.height>=0)
        height.reset(new float(layout.height));
      else if (type=="PlotWidget")
        height.reset(new float(150));
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
    /// note this assumes that ids have been uniquified prior to this call
    Group(const schema1::Group& g):
      Item(g), items(g.items) {}
  };


  struct Minsky
  {
    static const int version=2;
    int schemaVersion=Minsky::version;
    vector<Wire> wires;
    vector<Item> items;
    vector<Group> groups;
    minsky::Simulation rungeKutta;
    double zoomFactor=1;
    vector<minsky::Bookmark> bookmarks;
    minsky::Dimensions dimensions;
    minsky::ConversionsMap conversions;
    
    Minsky(): schemaVersion(0) {} // schemaVersion defined on read in
    Minsky(const schema1::Minsky& m);

    /// populate schema from XML data
    Minsky(classdesc::xml_unpack_t& data): schemaVersion(0)
    {minsky::loadSchema<schema1::Minsky>(*this,data,"Minsky");}
  };


}


#include "schema2.cd"
#include "schema2.xcd"

#endif
