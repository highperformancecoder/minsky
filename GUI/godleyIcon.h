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
#ifndef GODLEYICON_H
#define GODLEYICON_H

#include "variable.h"
#include "godley.h"
#include "zoom.h"
#include "intrusiveMap.h"
#include "classdesc_access.h"
#include "note.h"
#include "variableManager.h"
#include "SVGItem.h"

#include <map>
#include <cairo.h>
#include <cairo_base.h>

namespace minsky
{
  class Minsky;

  class GodleyIcon: public Note
  {
    float m_x, m_y; ///< position of Godley icon
    /// for placement of bank icon within complex
    float flowMargin, stockMargin, iconSize;
    CLASSDESC_ACCESS(GodleyIcon);
    friend class SchemaHelper;
  public:
    virtual int id() const {return -1;}

    static SVGRenderer svgRenderer;
    /// resource name (usually filename) for icon graphics
    void iconResource(const std::string& resource)
    {svgRenderer.setResource(resource);}
        
    /// this icon is never rotated, but this attributed is required
    /// for some generic algorithms
    static constexpr float rotation=0;

    bool mouseFocus; ///<true if target of a mouseover
    bool selected; ///<true if selected for cut, copy or group operation

    /// width of Godley icon in screen coordinates
    float width() const {return (flowMargin+iconSize)*zoomFactor;}
    /// height of Godley icon in screen coordinates
    float height() const {return (stockMargin+iconSize)*zoomFactor;}
    /// left margin of bank icon with Godley icon
    float leftMargin() const {return flowMargin*zoomFactor;}
    /// bottom margin of bank icon with Godley icon
    float bottomMargin() const {return stockMargin*zoomFactor;}

    GodleyIcon(): m_x(0), m_y(0), 
                  mouseFocus(false), selected(false), zoomFactor(1) {}

    /// set cell(row,col) with contents val
    void setCell(int row, int col, const string& val);
    /// delete row before \a row
    void deleteRow(unsigned row);
    /// move the contents of cell at (srcRow, srcCol) to (destRow, destCol).
    void moveCell(int srcRow, int srcCol, int destRow, int destCol);
    /// flows, along with multipliers, appearing in \a col
    std::map<string,double> flowSignature(int col) const;

    /// @{ position of Godley icon
    float x() const {return m_x;}
    float y() const {return m_y;}
    /// @}
    //float scale; ///< scale factor of the XGL image
    typedef std::vector<VariablePtr> Variables;
    Variables flowVars, stockVars;
    GodleyTable table;
    /// updates the variable lists with the Godley table
    void update();

    size_t numPorts() const;
    std::vector<int> ports() const;
    void moveTo(float x1, float x2);

    /// returns the name of a variable if point (x,y) is within a
    /// variable icon, "@" otherwise, indicating that the Godley table
    /// has been selected.
    int select(float x, float y);

    /// zoom by \a factor, scaling all widget's coordinates, using (\a
    /// xOrigin, \a yOrigin) as the origin of the zoom transformation
    void zoom(float xOrigin, float yOrigin,float factor);
    float zoomFactor;

    /// draw icon to \a context
    void draw(cairo_t* context) const;
    /// returns the clicktype given a mouse click at \a x, \a y.
    ClickType::Type clickType(float x, float y) const {
      return minsky::clickType(*this,x,y);
    }


    /**
     * Drawing routine for Wt.
     */
    void wtDraw(ecolab::cairo::Surface& cairoSurface);

    /**
     * the Wt UI changes back the zoom factor to 1 after rendering,
     * this refreshes the ports locations.
     */
    void updatePortLocation();
    /// returns valueid for variable reference in table
    // TODO: this should be refactored to a more central location
    std::string valueId(const std::string& x) const {
      if (x.find(':')==std::string::npos)
        return VariableManager::valueId(-1, x);
      else
        return VariableManager::valueId(x);
    }
  };

  typedef TrackedIntrusiveMap<int, GodleyIcon> GodleyIcons;
}

#include "godleyIcon.cd"
#endif
