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
#ifndef GROUPICON_H
#define GROUPICON_H

#include <vector>
#include <algorithm>
#include <string>
#include "wire.h"
#include "variable.h"
#include "operation.h"
#include "geometry.h"
#include "selection.h"
#include "note.h"
#include "SVGItem.h"

#include "classdesc_access.h"
#include <TCL_obj_base.h>
#include <arrays.h>

#include <cairo/cairo.h>
#include <cairo_base.h>

namespace minsky
{
  using namespace ecolab;
  class Minsky;

  class GroupIcon: public Note
  {
  public:
  private:
    CLASSDESC_ACCESS(GroupIcon);
    std::vector<int> m_operations;
    std::vector<int> m_variables;
    std::vector<int> m_wires;
    std::vector<int> m_groups;
    /// input and output port variables of this group
    std::set<int> inVariables, outVariables;
    /// used for ensuring that only one reference to a given variable
    /// is included
    std::set<std::string> inVarNames, outVarNames;
    float m_x{0}, m_y{0}; ///< icon position
    int m_parent{-1};
    /// icon's name
    std::string m_name;

    friend struct SchemaHelper;

    /// add variable to one of the edge lists, connected to port \a
    /// port. If the operation results in addition wires being
    /// created, these are returned in \a additionalWires
    void addEdgeVariable(std::set<int>& vars, std::set<string>& varNames,
                         std::vector<Wire>& additionalWires, int port);

    void drawVar(cairo_t*, const VariablePtr&) const;

  public:

    static SVGRenderer svgRenderer;
    /// resource name (usually filename) for icon graphics
    std::string iconResource(const std::string& resource)
    {svgRenderer.setResource(resource); return resource;}

    const std::string& name() const {return m_name;}
    /// sets the name attribute, and updates contained variable's
    /// fully qualified names
    void setName(const std::string&);

    float width{100}, height{100}; // size of icon
    float rotation{0}; // orientation of icon
    bool visible{true};
    /// variables created during the grouping process - to be deleted
    /// when ungrouped
    std::vector<int> createdVars;

    bool mouseFocus{false}; ///<true if target of a mouseover
    bool selected{false}; ///<true if selected for cut, copy or group operation

    std::vector<int> ports() const;
    size_t numPorts() const {return inVariables.size()+outVariables.size();}

    /// returns a list of edgeVariables
    std::vector<VariablePtr> edgeVariables() const;
    /// return the set of variable ideas, suitable for membership testing
    std::set<int> edgeSet() const {
      std::set<int> r(inVariables.begin(), inVariables.end());
      r.insert(outVariables.begin(), outVariables.end());
      return r;
    }
    /// eliminate any duplicate I/O variable references
    void eliminateIOduplicates();

    const std::vector<int>& operations() const {return m_operations;}
    const std::vector<int>& variables() const {return m_variables;}
    const std::vector<int>& wires() const {return m_wires;}
    const std::vector<int>& groups() const {return m_groups;}
    
    /// @{ coordinates of this icon on canvas
    float x() const;
    float y() const;
    /// @}

    int parent() const {return m_parent;}
    /// synonym for parent(), for TCL scripting purposes
    int group() const {return parent();}
    
    /// id of this group
    virtual int id() const {return -1;}
    int groupId() const {return id();}

    /// @return true if gid is a parent, or parent of a parent, etc
    bool isAncestor(int gid) const;

    /// x-coordinate of the vertical centre line of the icon
    float iconCentre() const {
      float left, right;
      margins(left,right);
      return x()+zoomFactor*0.5*(left-right);
    }

    // scaling factor to allow a rotated icon to fit on the bitmap
    float rotFactor() const;
                                   
    /// ungroup all icons
    void ungroup();

    bool empty() {return m_variables.empty() && m_operations.empty() && 
        m_wires.empty() && m_groups.empty();}

    /// populates this with a copy of src (with all internal objects
    /// registered with minsky).
    void copy(const GroupIcon& src);

    /// update port locations to current geometry and rotation.  Return
    /// the relative locations of the ports in unrotated coordinates as
    /// a vector of (x,y) pairs
    array<float> updatePortLocation() const;

    /// draw representations of edge variables around group icon
    void drawEdgeVariables(cairo_t*/*, float xScale, float yScale*/) const;
    /// draw notches in the I/O region to indicate docability of variables there
    void drawIORegion(cairo_t* cairo) const;

    /// margin sizes to allow space for edge variables. 
    void margins(float& left, float& right) const;

    void moveTo(float x1, float y1); ///< absolute move
    void move(float dx, float dy) {moveTo(x()+dx, y()+dy);}

    /// return bounding box coordinates for all variables, operators
    /// etc in this group. Returns the zoom scale (aka local zoom) of
    /// the contained items, or 1 if the group is empty.
    float contentBounds(float& x0, float& y0, float& x1, float& y1) const;

    /// move contents by dx, dy
    void moveContents(float dx, float dy);

    /// for TCL debugging
    array<float> cBounds() const {
      array<float> r(4);
      contentBounds(r[0],r[1],r[2],r[3]);
      return r;
    }

    /// zoom by \a factor, scaling all widget's coordinates, using (\a
    /// xOrigin, \a yOrigin) as the origin of the zoom transformation
    void zoom(float xOrigin, float yOrigin,float factor);
    float zoomFactor{1};
    /// sets the zoomFactor, and the appropriate zoom factors for all
    /// contained items
    void setZoom(float factor);

    /// delete contents, leaving an empty group
    void deleteContents();

    /// computes the zoom at which to show contents, given current
    /// contentBounds and width
    float displayZoom{1}; ///< zoom at which contents are displayed
    float computeDisplayZoom();
    float localZoom() const {
      return (displayZoom>0 && zoomFactor>displayZoom)
        ? zoomFactor/displayZoom: 1;
    }
    /// scale used to render io variables. Smoothly interpolates
    /// between the scale at which internal items are displayed, and
    /// the outside zoom factor
    float edgeScale() const {
      return zoomFactor>1? localZoom(): zoomFactor;
    }


    /// returns 1 if x,y is located in the in margin, 2 if in the out
    /// margin, 0 otherwise
    int inIORegion(float x, float y) const;

    /// returns whether contents should be displayed
    bool displayContents() const {return zoomFactor>displayZoom;}

    /// add a variable to to the group. \a checkIOregions overrides
    /// check of whether variables exists in the IO regions
    void addVariable(const VariableManager::value_type&, bool checkIOregions=true);
    /// @{ remove variable from group
    void removeVariable(const VariableManager::value_type&);
    // slightly different name to allow for access from TCL
    void removeVariableById(int id);
    /// @}

    /// add a operation to to the group
    void addOperation(Operations::value_type&);
    /// remove operation from group
    void removeOperation(Operations::value_type&);
    /// make group a child of this group
    /// @return true if successful
    bool addGroup(IntrusiveWrap<int, GroupIcon>&);
    /// remove child group from this group, and add it to the parent
    void removeGroup(IntrusiveWrap<int, GroupIcon>&);
    
    /// see if any attached wires should also be moved into the group
    /// \a ports is a sequence (vector or array) of ports
    template <class S> void addAnyWires(const S& ports);
    /// remove any attached wires that belong to the group
    template <class S> void removeAnyWires(const S& ports);
    /// add wire to group. Only added if both end points belong to the
    /// group, and if one end point is outside group, and the other
    /// in, an appropriate edge variable is also added to break the
    /// wire
    void addWire(int id);
    /// remove wire from group
    void delWire(int id);
    /// adds the list of wires, regardless of ports
    void addWires(const vector<int>& wires); 

    /// rotate icon and all its contents
    void rotate(float angle);
    /// rotate icon to a given rotation value
    void rotateTo(float angle) {rotate(angle-rotation);}

    /// flip contents left to right (in rotated frame of reference)
    void flipContents();

    /// returns the name of a variable if point (x,y) is within a
    /// variable icon, "@" otherwise, indicating that the GroupIcon itself
    /// has been clicked on.
    int selectVariable(float x, float y) const;

    /// geometry of scaled, rotated icon
    Polygon geom() const;

    /// draw the icon on \a context
    void draw(cairo_t* context) const;
    /// returns the clicktype given a mouse click at \a x, \a y.
    ClickType::Type clickType(float x, float y) const {
      return minsky::clickType(*this,x,y);
    }

    void wtDraw(ecolab::cairo::Surface& cairoSurface);

    /// handle lasso selection within a group. Doesn't clear current
    /// selection, just adds to it.
    void select(Selection& sel, float x0, float y0, float x1, float y1);

    /// centre icon on contents
    void centreIconOnContents();

    /// sets the scope of all global variables to \a id. Applied recursively to contained groups
    void rehostGlobalVars(int id);

  };

  struct GroupIcons: public TrackedIntrusiveMap<int, GroupIcon>
  {
    std::vector<int> visibleGroups() const;
    // ensures that contained variables, operations, wires and groups
    // only belong to a single group within this container. This is a
    // useful invariant to test in asserts
    bool uniqueGroupMembership() const;
  };

}

#include "groupIcon.cd"
#endif
