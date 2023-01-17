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

 @file Schema 0 is the original (not really well-defined) schema
 used by Minsky in the Aristotle branch. This file contains the
 classes used to read in legacy Minsky files created in Aristotle
 version of Minsky.  

*/

#ifndef SCHEMA_SCHEMA_0
#define SCHEMA_SCHEMA_0

#include "operationType.h"
#include "variableType.h"
#include "assetClass.h"
#include <arrays.h>
#include <plot.h>

namespace schema0
{
  struct Port
  {
    bool input; //true if input port
    double x,y;
    Port(float x=0, float y=0, bool input=false): input(input), x(x), y(y) {}
  };

  struct Wire
  {
    /// ports this wire connects
    int from, to;
    bool visible; ///<whether wire is visible on Canvas 
    /// display coordinates
    ecolab::array<float> coords;
    Wire(int from=0, int to=0): from(from), to(to), visible(true) {}
  };


  struct Operation
  {
    double x{10}, y{10};
    double value{0}; /// for constants
    double rotation{0}; /// rotation if icon, in degrees

    bool visible{true}; ///< whether operation is visible on Canvas 
    minsky::OperationType::Type m_type{minsky::OperationType::numOps};

    bool sliderVisible{false}, sliderBoundsSet{false}, sliderStepRel{false};
    double sliderMin, sliderMax, sliderStep;

    std::string m_description; ///name of constant, variable
    std::string description; /// older version of name
    std::vector<int> m_ports;
    ///integration variable associated with this op. -1 if not used
    int intVar{-1}; 
  };

  struct VariablePtr
  {
    minsky::VariableType::Type m_type;

    double x{0}, y{0}; ///< position in canvas
    std::string init{"0"}; ///< initial value of variable
    std::string name; ///< variable name
    double rotation{0}; ///< rotation if icon, in degrees

    /**
       whether variable is visible on Canvas (note godley variables are
       never visible, as they appear as part of the godley icon 
    */
    bool visible{true};
    int m_outPort, m_inPort; ///< where wires connect to
    bool m_godley{false}; ///< variable is in a Godley table

    bool operator<(const VariablePtr& x) const
    {return name<x.name;}
  };

  // used to support an older schema
  struct Variable: public VariablePtr {};

  struct VariableValue
  {
    minsky::VariableType::Type m_type;
    int m_idx{-1}; /// index into value vector

    std::string init;
    bool godleyOverridden{false};

    VariableValue(minsky::VariableType::Type type=minsky::VariableType::undefined, 
                  const std::string& init="0"): m_type(type), init(init) {}
  };


  struct VariableManager: public std::map<int,Variable>
  {
    typedef std::map<int,Variable> Variables;
    typedef std::map<std::string, VariableValue> VariableValues;
  
    std::set<std::string> wiredVariables; /// variables whose input port is wired
    std::map<int, int> portToVariable; /// map of ports to variables
    VariableValues values; 
  };

  struct GodleyTable
  {
    std::vector<std::vector<std::string> > data;
    /// class of each column (used in DE compliant mode)
    std::vector<minsky::GodleyAssetClass::AssetClass> m_assetClass;
    bool doubleEntryCompliant;
    std::string title;
  
    GodleyTable(): doubleEntryCompliant(false)  {}
    std::size_t rows() const {return data.size();}
  };

  struct GodleyIcon
  {
    double adjustHoriz, adjustVert; // difference between where variables are displayed and screen coordinates
    double x, y; ///< position of Godley icon
    double scale; ///< scale factor of the XGL image
    std::vector<VariablePtr> flowVars, stockVars;
    GodleyTable table;
  };

  class GroupIcon
  {
    std::vector<int> inVariables, outVariables;
    CLASSDESC_ACCESS(GroupIcon);
  public:
    std::vector<int> operations;
    std::vector<int> variables;
    std::vector<int> wires;
    ecolab::array<int> m_ports;

    std::string name;
    double x,y; //icon position
    double width, height; // size of icon
    double rotation; // orientation of icon

    GroupIcon(): width(100), height(100), rotation(0) {}
  };



  // a place to put helper classes to manage XML evolution
  namespace xml_conversions
  {
    struct GodleyIcon
    {
      std::set<Variable> flowVars, stockVars;
    };
  }

  struct PlotWidget 
  {
    int nxTicks, nyTicks; ///< number of x/y-axis ticks
    double fontScale; ///< scale tick labels
    double offx, offy; ///< origin of plot within image
    bool logx, logy; ///< logarithmic plots (x/y axes)
    bool grid; ///< draw grid
    bool leadingMarker; ///< draw a leading marker on the curve (full draw only)
    bool autoscale; ///< autoscale plot to data
    ecolab::Plot::PlotType plotType;
    double minx, maxx, miny, maxy;
    ecolab::array<int> ports;
    std::vector<VariableValue> yvars;
    std::vector<VariableValue> xvars;
    VariableValue xminVar, xmaxVar, yminVar, ymaxVar;
  
    std::vector<std::string> images;
 
    double x,y;
  };


  struct Plots
  {
    typedef std::map<std::string, PlotWidget> Map;
    Map plots;
  };

  struct Minsky
  {
    typedef std::map<int, Port> Ports;
    typedef std::map<int, Wire> Wires;
    Ports ports;
    Wires wires;

    GodleyTable godley; // deprecated - needed for Minsky.1 capability
    typedef std::map<int, GodleyIcon> GodleyItems;
    GodleyItems godleyItems;

    typedef std::map<int, Operation> Operations;
    Operations operations;
    VariableManager variables;

    typedef std::map<int, GroupIcon> GroupIcons;
    GroupIcons groupItems;
    
    Plots plots;
    
    Minsky() {}

    /// Runge-Kutta parameters
    double stepMin=0; ///< minimum step size
    double stepMax=1; ///< maximum step size
    int nSteps=1;     ///< number of steps per GUI update
    double epsAbs=1e-3;     ///< absolute error
    double epsRel=1e-2;     ///< relative error

    Minsky(classdesc::xml_unpack_t& data)
    {
      ::xml_unpack(data,"root",*this);
      removeIntVarOrphans();
    }
    
    /** See ticket #329 and references within. At some stage, IntOp had
        no destructor, which leads to an orphaned, invisible integral
        variable, with invalid output port. This bit of code deals with
        corrupt schema files that have been caused by this problem.

        Apparently schema0 files suffered from this problem too!
    */
    void removeIntVarOrphans();
  };

}

namespace classdesc
{
  template <class T>
  void xml_unpackp(xml_unpack_t& t, const string& d, ecolab::array<T>& a)
  {
    string x; xml_unpack(t,d,x);
    std::istringstream is(x);
    is>>a;
  }
}

#include "schema0.xcd"
#endif
