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
#include "itemTab.h"
#include "latexMarkup.h"
#include "group.h"
#include <pango.h>
#include "minsky.h"
#include "equations.h"
#include "itemTab.rcd"
#include "itemTab.xcd"
#include "pannableTab.rcd"
#include "pannableTab.xcd"
#include "pango.xcd"
#include "minsky_epilogue.h"
using namespace std;
using ecolab::cairo::Surface;
using ecolab::Pango;
using ecolab::cairo::CairoSave;

namespace minsky
{
  void ItemTab::populateItemVector() {
    itemVector.clear();	
    minsky().canvas.model->recursiveDo(&GroupItems::items,
                                       [&](Items&, Items::iterator i) {                                 
                                         if (itemSelector(*i)) 
                                           {		                                 
                                             itemVector.emplace_back(*i);
                                             if (!(*i)->itemTabInitialised)
                                               {
                                                 // NANs used to indicate the position on tab has not yet been initialised
                                                 (*i)->itemTabX=(*i)->x();
                                                 (*i)->itemTabY=(*i)->y();
                                                 (*i)->itemTabInitialised=true;
                                               }
                                           }
                                         return false;
                                       });   	
  }

  void ItemTab::moveItemTo(float x, float y)
  {   
    if (itemFocus) {    
      itemFocus->itemTabX=x-offsx;	                               
      itemFocus->itemTabY=y-offsy;
    }
  }  
  
  ItemTab::ClickType ItemTab::clickType(double x, double y) const
  {
    return (colX(x)>=0 && rowY(y)>=0)? internal: background;
  }
      
  void ItemTab::mouseDown(float x, float y)
  {
    switch (clickType(x-offsx,y-offsy))
      {
      case background:
        itemFocus.reset();
        break;
      case internal:
        if ((itemFocus=itemAt(x-offsx,y-offsy)))
          {
            moveOffsX=x-itemFocus->itemTabX;
            moveOffsY=y-itemFocus->itemTabY;
          }
        break;
      }
           
  }  
  
  void ItemTab::mouseUp(float x, float y)
  {
    mouseMove(x,y);
    itemFocus.reset();
  }
  
  void ItemTab::mouseMove(float x, float y)
  {
        if (itemFocus)
          {
            moveItemTo(x-moveOffsX,y-moveOffsY);
            requestRedraw();
          }
  }
  
  void ItemTab::displayDelayedTooltip(float x, float y)
  {
    if (auto item=itemAt(x,y))
      {
        item->displayDelayedTooltip(x,y);
        requestRedraw();
      }
  } 
  
  ItemPtr ItemTab::itemAt(float x, float y)
  {
    ItemPtr item;                    
    auto minD=numeric_limits<float>::max();
    for (auto& i: itemVector)
      {
        float xx=i->itemTabX+offsx, yy=i->itemTabY+offsy;  
        float d=sqr(xx-x)+sqr(yy-y);
        float z=i->zoomFactor();
        float w=0.5*i->iWidth()*z,h=0.5*i->iHeight()*z;
        if (d<minD && fabs(xx-x)<w && fabs(yy-y)<h)
          {
            minD=d;
            item=i;
          }
      }
    
    return item;
  }
  
  ecolab::Pango& ItemTab::cell(unsigned row, unsigned col) {
    cellPtr.reset(surface->cairo());
    if (row==0)
      cellPtr->setText(varAttrib[col]);
    else if (row-1<itemVector.size())
      if (auto* v=itemVector[row-1]->variableCast())
        switch (col)
          {
          case 0:
            cellPtr->setMarkup(latexToPango(v->name()));
            break;
          case 1:
            cellPtr->setMarkup(v->definition());
            break;
          case 2:
            cellPtr->setMarkup(latexToPango(v->init()));
            break;
          case 3:
            cellPtr->setMarkup(latexToPango(v->tooltip));
            break;
          case 4:
            cellPtr->setMarkup(latexToPango(v->detailedText));
            break;
          case 5:
            cellPtr->setText(str(v->sliderStep));
            break;
          case 6:
            cellPtr->setText(str(v->sliderMin));
            break;
          case 7:
            cellPtr->setText(str(v->sliderMax));
            break;
          case 8:
            if (!v->dims().empty())
              cellPtr->setText("<tensor>");
            else
              cellPtr->setText(str(v->value()));
            break;
          default:
            break;
          }
    assert(varAttrib.size()==9);
    return *cellPtr;
  }

  
  
  void ItemTab::draw(cairo_t* cairo)
  {
    Grid<ecolab::Pango>::draw();
    {
      cairo::CairoSave cs(cairo);
      cairo_set_source_rgba(cairo,0,0,0,0.2);
      cairo_rectangle(cairo,0,0,rightColMargin.back(),bottomRowMargin[0]);
      cairo_fill(cairo);
    }
    {
      cairo::CairoSave cs(cairo);
      cairo_set_source_rgba(cairo,0,0,0,0.5);
      // horizontal grid lines
      cairo_move_to(cairo,0,0);
      cairo_line_to(cairo,rightColMargin.back(),0);
      for (auto r: bottomRowMargin)
        {
          cairo_move_to(cairo,0,r);
          cairo_line_to(cairo,rightColMargin.back(),r);
        }
      // vertical grid lines
      cairo_move_to(cairo,0,0);
      cairo_line_to(cairo,0,bottomRowMargin.back());
      for (auto r: rightColMargin)
        {
          cairo_move_to(cairo,r,0);
          cairo_line_to(cairo,r,bottomRowMargin.back());
        }
      cairo_stroke(cairo);
    }
  }

  bool ItemTab::redraw(int, int, int, int)
  {
    if (surface.get()) {
      populateItemVector();			               
      cairo_t* cairo=surface->cairo();  
      cairo_translate(cairo,offsx,offsy); 
      draw(cairo); 
      ecolab::cairo::Surface surf
        (cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,NULL));            
      draw(surf.cairo());      
      m_width=surf.width();
      m_height=surf.height();
    }
    return surface.get();
  }    

}
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::ItemTab);
