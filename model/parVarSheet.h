/*
  @copyright Steve Keen 2020
  @author Russell Standish
  @author Wynand Dednam
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

#ifndef PARVARSHEET_H
#define PARVARSHEET_H
#include <variable.h>
#include <cairoSurfaceImage.h>
#include "classdesc_access.h"

namespace minsky
{
	 
  class ParVarSheet: public ecolab::CairoSurface
  {
    CLASSDESC_ACCESS(ParVarSheet);         
  public: 
    ParVarSheet() {}
  
    double xoffs=80;
    double rowHeight=0;
    double colWidth=50; 
    float offsx=0, offsy=0;       
    float m_width=600, m_height=800;
    virtual float width() const {return m_width;}
    virtual float height() const {return m_height;}
    Items itemVector;     

    /// computed positions of the table columns
    std::vector<double> colLeftMargin;                              

    void populateItemVector();
    virtual bool variableSelector(ItemPtr i) = 0;
    void toggleVarDisplay(int i) const {if (i>=0 && i<int(2*itemVector.size())) (itemVector[i/2])->variableCast()->toggleVarTabDisplay(); else return;}
    std::string getVarName(int i) const {if (i>=0 && i<int(2*itemVector.size())) return (itemVector[i/2])->variableCast()->name(); else return "";}
    std::vector<std::string> varAttrib{"Name","Initial Value","Short Description", "Long Description","Slider Step","Slider Min","Slider Max","Value"};       
    std::vector<std::string> varAttribVals;
    /// column at \a x in unzoomed coordinates
    int colX(double x) const;
    /// row at \a y in unzoomed coordinates
    int rowY(double y) const;    
    enum ClickType {background, internal};    
    ClickType clickType(double x, double y) const;         
    void draw(cairo_t* cairo); 
    void redraw(int, int, int width, int height) override;
    void requestRedraw() {if (surface.get()) surface->requestRedraw();}         
       
    ~ParVarSheet() {}
  };
  
}

#include "parVarSheet.cd"
#endif
