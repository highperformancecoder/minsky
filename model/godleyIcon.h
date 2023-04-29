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
#include "godleyTableWindow.h"
#include "zoom.h"
#include "intrusiveMap.h"
#include "classdesc_access.h"
#include "SVGItem.h"
#include "group.h"
#include "valueId.h"
#include "variableValue.h"

#include <map>
#include <cairo.h>
#include <cairo_base.h>

namespace minsky
{
  class GodleyTableEditor;

  class GodleyIcon: public ItemT<GodleyIcon>
  {
    
    /// for placement of bank icon within complex
    float flowMargin=0, stockMargin=0;
    bool m_editorMode=false;
    CLASSDESC_ACCESS(GodleyIcon);
    friend struct SchemaHelper;


    void updateBB() {
      auto wasSelected=selected;
      selected=true; // ensure bounding box is set to the entire icon
      bb.update(*this); 
      selected=wasSelected;
    }

    double titleOffs() const {return !table.title.empty()? 15*zoomFactor(): 0;}
    
  public:
    static SVGRenderer svgRenderer; ///< SVG icon to display when not in editor mode
    
    GodleyIcon() {iWidth(150); iHeight(150); editor.adjustWidgets();}
    GodleyIcon(const GodleyIcon&)=default;
    ~GodleyIcon() {Item::removeControlledItems();}

    /// indicate whether icon is in editor mode or icon mode
    bool editorMode() const {return m_editorMode;}
    void toggleEditorMode();

    /// enable/disable drawing buttons in table on canvas display
    bool buttonDisplay() const;
    void toggleButtons(); 

    bool variableDisplay=false;
    void toggleVariableDisplay() {variableDisplay=!variableDisplay; update();}

    /// table data. Must be declared before editor
    GodleyTable table;
    /// rendering as a godley table
    GodleyTableEditor editor{*this};
    /// for rendering the popup window
    GodleyTableWindow popup{*this};
    void adjustPopupWidgets() {popup.adjustWidgets();}
    
    /// scale icon until it's height or width matches \a h or \a w depending on which is minimum             
    void scaleIcon(float w, float h);         
    
    /// left margin of bank icon with Godley icon
    float leftMargin() const {return variableDisplay? flowMargin*scaleFactor()*zoomFactor(): 0;}
    /// bottom margin of bank icon with Godley icon
    float bottomMargin() const {return variableDisplay? stockMargin*scaleFactor()*zoomFactor(): 0;}

    void resize(const LassoBox&) override;
    void removeControlledItems(GroupItems&) const override;
 
    /// set cell(row,col) with contents val
    void setCell(int row, int col, const string& val);
    /// delete row before \a row
    void deleteRow(unsigned row);
    /// move the contents of cell at (srcRow, srcCol) to (destRow, destCol).
    struct MoveCellArgs
    {
      int srcRow, srcCol, destRow, destCol;
    };
    void moveCell(const GodleyIcon::MoveCellArgs&);
    /// flows, along with multipliers, appearing in \a col
    std::map<string,double> flowSignature(unsigned col) const;

    typedef std::vector<VariablePtr> Variables;
    const Variables& flowVars() const {return m_flowVars;}
    const Variables& stockVars() const {return m_stockVars;}

    /// updates the variable lists with the Godley table
    void update();
    
    GodleyIcon* clone() const override {
      auto r=new GodleyIcon(*this);  
	  r->update();       
	  r->group.reset();
	  return r;
    }      

    /// returns the variable if point (x,y) is within a
    /// variable icon, null otherwise, indicating that the Godley table
    /// has been selected.
    ItemPtr select(float x, float y) const override;
    ClickType::Type clickType(float x, float y) const override;

    /// draw icon to \a context
    void draw(cairo_t* cairo) const override;

    /// return the A-L-E row sum for \a row
    std::string rowSum(int row) const;
    
    /// returns valueid for variable reference in table
    // TODO: this should be refactored to a more central location
    std::string valueId(const std::string& x) const {return minsky::valueId(group.lock(), x);}
    /// performs dimensional analysis on stock var column \a stockName
    /// @param check indicates whether a consistency check is applied
    Units stockVarUnits(const std::string& stockName, bool check) const;

    void setCurrency(const std::string& currency) 
    {for (auto& i: m_stockVars) i->setUnits(currency);}
      
    void insertControlled(Selection& selection) override;

    void onMouseDown(float, float) override;
    void onMouseUp(float, float) override;
    bool onMouseMotion(float, float) override;
    bool onMouseOver(float, float) override;
    void onMouseLeave() override;
    bool onKeyPress(int, const std::string&, int) override;
    bool inItem(float, float) const override;

    /// clean up popup window structures on window close
    void destroyFrame() override {popup.destroyFrame();}

  private:
    void updateVars(Variables& vars, 
                    const vector<string>& varNames, 
                    VariableBase::Type varType);
    /// move contained variables to correct locations within icon
    void positionVariables() const;
    Variables m_flowVars, m_stockVars;

    /// @{ convert mouse coordinates into editor coords
    float toEditorX(float) const;
    float toEditorY(float) const;
  };
}

#ifdef CLASSDESC
// omit these, because weak/shared pointers cause problems, and its
// not needed anyway
#pragma omit pack minsky::GodleyIcon
#pragma omit unpack minsky::GodleyIcon
#endif
namespace classdesc_access
{
  template <> struct access_pack<minsky::GodleyIcon>: 
    public classdesc::NullDescriptor<classdesc::pack_t> {};
  template <> struct access_unpack<minsky::GodleyIcon>: 
    public classdesc::NullDescriptor<classdesc::unpack_t> {};
}

#include "godleyIcon.cd"
#include "godleyIcon.xcd"
#endif

