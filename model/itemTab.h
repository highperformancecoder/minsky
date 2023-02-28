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

#ifndef ITEMTAB_H
#define ITEMTAB_H
#include "variable.h"
#include "grid.h"
#include <pango.h>
#include "eventInterface.h"
#include "renderNativeWindow.h"
#include "classdesc_access.h"

namespace minsky
{
  class DisplayVariable {
      public:
      std::string name;
      std::string definition;
      std::string dimensions;
      std::string type;
      std::string units;
      std::string init;
      double value;
      bool isTensor;
  };

  class ItemTabCellPtr: public std::shared_ptr<ecolab::Pango>
  {
    cairo_t* cachedCairo=nullptr;
  public:
    void reset(cairo_t* cairo) {
      if (cairo!=cachedCairo) {
        cachedCairo=cairo;
        std::shared_ptr<ecolab::Pango>::operator=(std::make_shared<ecolab::Pango>(cairo));
      }
    }
  };
		 
  class ItemTab: public RenderNativeWindow, public Grid<ecolab::Pango>
  {
    CLASSDESC_ACCESS(ItemTab);         
    bool redraw(int, int, int width, int height) override;
  protected:
    classdesc::Exclude<ItemTabCellPtr> cellPtr;
  public:
    ItemTab() {} 
  
    double xoffs=80;
    float offsx=0, offsy=0;
    float m_width=600, m_height=800;
    virtual float width() const {return m_width;}
    virtual float height() const {return m_height;}
    Items itemVector;     

    unsigned numRows() const override {return itemVector.size()+1;}
    unsigned numCols() const override {return varAttrib.size();}

    ecolab::Pango& cell(unsigned row, unsigned col) override;

    void moveCursorTo(double x, double y) override {
      if (surface.get())
        cairo_move_to(surface->cairo(),x,y);
    }

    virtual std::vector<DisplayVariable> getDisplayVariables();
    virtual void populateItemVector();
    virtual bool itemSelector(const ItemPtr& i) = 0;
    void toggleVarDisplay(int i) const {if (i>0 && i-1<int(itemVector.size())) (itemVector[i-1])->variableCast()->toggleVarTabDisplay(); else return;}    
    std::string getVarName(int i) const {if (i>0 && i-1<int(itemVector.size())) return (itemVector[i-1])->variableCast()->name(); else return "";}
    std::vector<std::string> varAttrib{"Name","Definition","Initial Value","Short Description", "Long Description","Slider Step","Slider Min","Slider Max","Value"};       
    void moveItemTo(float x, float y);  
         
    float moveOffsX, moveOffsY,xItem,yItem;
    ItemPtr itemFocus;      
    bool getItemAt(float x, float y) override {return (item=itemAt(x,y)).get();}  
    enum ClickType {background, internal};    
    virtual ClickType clickType(double x, double y) const;         
    virtual void draw(cairo_t* cairo); 
    void requestRedraw() {if (surface.get()) surface->requestRedraw();}         

    void mouseDown(float x, float y) override;
    void mouseUp(float x, float y) override;
    void mouseMove(float x, float y) override;    
    void moveTo(float x, float y) override
    {
      offsx=x;
      offsy=y;
      requestRedraw();
    }
    virtual ItemPtr itemAt(float x, float y);
    void displayDelayedTooltip(float x, float y);        
       
    virtual ~ItemTab() {}
  };
  
}

#include "itemTab.cd"
#endif
