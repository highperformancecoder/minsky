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
#include "godleyTable.h"
#include "zoom.h"
#include "intrusiveMap.h"
#include "classdesc_access.h"
#include "SVGItem.h"

#include <map>
#include <cairo.h>
#include <cairo_base.h>

namespace minsky
{
  class GodleyIcon: public ItemT<GodleyIcon>
  {
    /// for placement of bank icon within complex
    float flowMargin=0, stockMargin=0, iconSize=100;
    /// icon scale is adjusted when Godley icon is resized
    float iconScale=1;
    CLASSDESC_ACCESS(GodleyIcon);
    friend class SchemaHelper;
  public:
    static SVGRenderer svgRenderer;
    
    /// width of Godley icon in screen coordinates
    float width() const {return (flowMargin+iconSize)*iconScale*zoomFactor;}
    /// height of Godley icon in screen coordinates
    float height() const {return (stockMargin+iconSize)*iconScale*zoomFactor;}
    /// scale icon until it's height matches \a h 
    void scaleIconForHeight(float h) {update(); iconScale*=h/height();}

    /// left margin of bank icon with Godley icon
    float leftMargin() const {return flowMargin*iconScale*zoomFactor;}
    /// bottom margin of bank icon with Godley icon
    float bottomMargin() const {return stockMargin*iconScale*zoomFactor;}

    /// helper for schema1
    double schema1ZoomFactor() const; 
    
    void resize(const LassoBox&) override;

    /// set cell(row,col) with contents val
    void setCell(int row, int col, const string& val);
    /// delete row before \a row
    void deleteRow(unsigned row);
    /// move the contents of cell at (srcRow, srcCol) to (destRow, destCol).
    void moveCell(int srcRow, int srcCol, int destRow, int destCol);
    /// flows, along with multipliers, appearing in \a col
    std::map<string,double> flowSignature(int col) const;

    //float scale; ///< scale factor of the XGL image
    typedef std::vector<VariablePtr> Variables;
    const Variables& flowVars() const {return m_flowVars;}
    const Variables& stockVars() const {return m_stockVars;}
    GodleyTable table;
    /// updates the variable lists with the Godley table
    void update();

    void zoom(float xOrigin, float yOrigin,float factor) override {
      Item::zoom(xOrigin,yOrigin,factor);
      update();
    }


    /// returns the variable if point (x,y) is within a
    /// variable icon, null otherwise, indicating that the Godley table
    /// has been selected.
    ItemPtr select(float x, float y) const override;

    /// draw icon to \a context
    void draw(cairo_t* context) const override;

    /// returns valueid for variable reference in table
    // TODO: this should be refactored to a more central location
    std::string valueId(const std::string& x) const {
      return VariableValue::valueId(group.lock(), x);
    }
  private:
    void updateVars(Variables& vars, 
                    const vector<string>& varNames, 
                    VariableBase::Type varType);
    /// move contained variables to correct locations within icon
    void positionVariables() const;
    Variables m_flowVars, m_stockVars;
  };
}

#include "godleyIcon.cd"
#endif
