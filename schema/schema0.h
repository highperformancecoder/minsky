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

#include <arrays.h>
#include <plot.h>

//TODO - remove this dependency
#include "../GUI/minsky.h"

namespace schema0
{
  struct Port
  {
    bool input; //true if input port
    double x,y;
    Port(float x=0, float y=0, bool input=false): input(input), x(x), y(y) {}
    operator minsky::Port () const {return minsky::Port(x,y,input);}
  };

  struct Wire
  {
    /// ports this wire connects
    int from, to;
    bool visible; ///<whether wire is visible on Canvas 
    /// display coordinates
    ecolab::array<float> coords;
    Wire(int from=0, int to=0): from(from), to(to), visible(true) {}
    operator minsky::Wire() const {
      return minsky::Wire(from, to, coords, visible);
    }
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

    operator minsky::OperationPtr() const;
  };

  struct VariablePtr
  {
    VariableType::Type m_type;

    double x{0}, y{0}; ///< position in canvas
    string init{"0"}; ///< initial value of variable
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

    operator minsky::VariablePtr() const;
  };

  // used to support an older schema
  struct Variable: public VariablePtr {};

  struct VariableValue
  {
    VariableType::Type m_type;
    int m_idx{-1}; /// index into value vector

    string init;
    bool godleyOverridden{false};

    VariableValue(VariableType::Type type=VariableType::undefined, 
                  const string& init="0"): m_type(type), init(init) {}
    operator minsky::VariableValue() const;
  };


  struct VariableManager: public std::map<int,Variable>
  {
    typedef std::map<int,Variable> Variables;
    typedef std::map<std::string, VariableValue> VariableValues;
  
    std::set<std::string> wiredVariables; /// variables whose input port is wired
    std::map<int, int> portToVariable; /// map of ports to variables
    VariableValues values; 

    /// scans variable, wire & port definitions to correct any inconsistencies
    /// - useful after a load to correct corrupt xml files
    //    void makeConsistent();
    operator minsky::VariableManager() const;
  };

  struct GodleyTable
  {
    std::vector<std::vector<std::string> > data;
    /// class of each column (used in DE compliant mode)
    std::vector<minsky::GodleyTable::AssetClass> m_assetClass;
    bool doubleEntryCompliant;
    std::string title;
  
    GodleyTable(): doubleEntryCompliant(false)  {}
    size_t rows() const {return data.size();}
    operator minsky::GodleyTable() const;
  };

  struct GodleyIcon
  {
    double adjustHoriz, adjustVert; // difference between where variables are displayed and screen coordinates
    double x, y; ///< position of Godley icon
    double scale; ///< scale factor of the XGL image
    std::vector<VariablePtr> flowVars, stockVars;
    GodleyTable table;
    operator minsky::GodleyIcon() const;
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
    operator minsky::GroupIcon() const;

    void updateEdgeVariables(const VariableManager& vm);

  };



  // a place to put helper classes to manage XML evolution
  namespace xml_conversions
  {
    struct GodleyIcon
    {
      std::set<Variable> flowVars, stockVars;
    };
  }

  struct PlotWidget: public ecolab::Plot
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
    operator minsky::PlotWidget() const;
  };


  struct Plots
  {
    typedef std::map<std::string, PlotWidget> Map;
    Map plots;
    operator minsky::Plots() const; 
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
    
    Minsky();

    /// Runge-Kutta parameters
    double stepMin; ///< minimum step size
    double stepMax; ///< maximum step size
    int nSteps;     ///< number of steps per GUI update
    double epsAbs;     ///< absolute error
    double epsRel;     ///< relative error

    /// load from a file
    void load(const char* filename);

    operator minsky::Minsky() const;

    /** See ticket #329 and references within. At some stage, IntOp had
        no destructor, which leads to an orphaned, invisible integral
        variable, with invalid output port. This bit of code deals with
        corrupt schema files that have been caused by this problem.

        Apparently schema0 files suffered from this problem too!
    */
    void removeIntVarOrphans();

  };

}

#include "schema0.xcd"
#endif
