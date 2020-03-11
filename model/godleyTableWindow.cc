/*
  @copyright Steve Keen 2018
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

#include "godleyTableWindow.h"
#include "selection.h"
#include "latexMarkup.h"
#include "minsky.h"
#include <pango.h>
#include "minsky_epilogue.h"

using namespace std;
using namespace minsky;
using ecolab::Pango;
using namespace ecolab::cairo;

#include <cairo/cairo-ps.h>
#include <cairo/cairo-pdf.h>
#include <cairo/cairo-svg.h>

constexpr double GodleyTableWindow::leftTableOffset,
  GodleyTableWindow::topTableOffset, GodleyTableWindow::pulldownHot,
  GodleyTableWindow::columnButtonsOffset, GodleyTableWindow::minColumnWidth;

namespace
{
  string capitalise(string x)
  {
    if (!x.empty()) x[0]=toupper(x[0]);
    return x;
  }

  struct Colour
  {
    double r,g,b;
  } assetColour[] = {
    {0,0,0},
    {0,0,0},
    {1,0,0},
    {.6,.5,0}
  };

  void showAsset(Pango& pango, cairo_t* cairo, GodleyAssetClass::AssetClass assetClass)
  {
    CairoSave cs(cairo);
    auto& colour=assetColour[assetClass];
    cairo_set_source_rgb(cairo,colour.r,colour.g,colour.b);
    pango.show();
  }

  struct ZoomablePango: public Pango
  {
    static double zoomFactor;
    ZoomablePango(cairo_t* c): Pango(c) {setFontSize(10*zoomFactor);}
    double idxToPos(size_t i) const {return Pango::idxToPos(i)/zoomFactor;}
    double width() const {return Pango::width()/zoomFactor;}
    double height() const {return Pango::height()/zoomFactor;}
  };

  double ZoomablePango::zoomFactor=1;
}

namespace minsky
{
  template <>
  void ButtonWidget<ButtonWidgetEnums::row>::invoke(double x)
  {
    int button=x/buttonSpacing;
    switch (button)
      {
      case 0:
        godleyIcon.table.insertRow(idx+1);
        break;
      case 1:      
        if (pos!=first && pos!=firstAndLast) godleyIcon.deleteRow(idx+1); // Initial conditions row cannot be deleted, even when it is the only row in the table. For ticket 1064
        break;
      case 2:  
        if (pos==second)                                       // Third button of second row cannot swap initial conditions and second row. For ticket 1064
           godleyIcon.table.moveRow(idx,1); 
        else if (pos!=first && pos!=firstAndLast)               // Third button cannot swap column headings and initial conditions row values. For ticket 1064     
          godleyIcon.table.moveRow(idx,-1);   
        break;
      case 3:
        if (pos==middle)                                         // Fourth button on first and second row cannot move initial conditions row. For ticket 1064
          godleyIcon.table.moveRow(idx,1);
        break;
      }
    try {godleyIcon.update();}   // Update current Godley icon and table after button widget invoke. for ticket 1059.
    catch (...) {}
  }

  template <>
  void ButtonWidget<ButtonWidgetEnums::col>::invoke(double x)
  {
    int button=x/buttonSpacing;
    if (!cminsky().multipleEquities && godleyIcon.table.singleEquity()) {  // no column widgets on equity column in single equity column mode
      if (pos!=last)
        switch (button)
         {
         case 0:
           godleyIcon.table.insertCol(idx+1);
           break;
         case 1:
           godleyIcon.table.deleteCol(idx+1);
           break;
         case 2:
           if (pos==first)
             godleyIcon.table.moveCol(idx,1);
           else if (pos!=first)
             godleyIcon.table.moveCol(idx,-1);
           break;
         case 3:
           if (pos==middle)
             godleyIcon.table.moveCol(idx,1);
           break;
         }
	 } else {
        switch (button)
          {
          case 0:
            godleyIcon.table.insertCol(idx+1);
            break;
          case 1:
            godleyIcon.table.deleteCol(idx+1);
            break;
          case 2:
            if (pos==first)
              godleyIcon.table.moveCol(idx,1);
            else
              godleyIcon.table.moveCol(idx,-1);
            break;
          case 3:
            if (pos==middle)
              godleyIcon.table.moveCol(idx,1);
            break;
          }	 
     }
    try {godleyIcon.update();}   // Update current Godley icon and table after button widget invoke. for ticket 1059.
    catch (...) {}      
  }


  void GodleyTableWindow::redraw(int, int, int, int)
  {
    if (!godleyIcon) return;
    CairoSave cs(surface->cairo());
    cairo_scale(surface->cairo(),zoomFactor,zoomFactor);
    ZoomablePango::zoomFactor=zoomFactor;
    ZoomablePango pango(surface->cairo());
    pango.setMarkup("Flows ↓ / Stock Vars →");
    rowHeight=pango.height()+2;
    double tableHeight=(godleyIcon->table.rows()-scrollRowStart+1)*rowHeight;
    double x=leftTableOffset;
    double lastAssetBoundary=x;
    auto assetClass=GodleyAssetClass::noAssetClass;
    colLeftMargin.clear();
  
    for (unsigned col=0; col<godleyIcon->table.cols(); ++col)
      {
        // omit stock columns less than scrollColStart
        if (col>0 && col<scrollColStart) continue;
        // vertical lines & asset type tag
        if (assetClass!=godleyIcon->table._assetClass(col))
          {
            if (assetClass!=GodleyAssetClass::noAssetClass)
              {
                pango.setMarkup(capitalise(enumKey<GodleyAssetClass::AssetClass>(assetClass)));
                // increase column by enough to fit asset class label
                if (x < pango.width()+lastAssetBoundary+3)
                  x=pango.width()+lastAssetBoundary+3;
                cairo_move_to(surface->cairo(),0.5*(x+lastAssetBoundary-pango.width()),0);
                showAsset(pango, surface->cairo(), assetClass);
              }
            lastAssetBoundary=x;
          
            assetClass=godleyIcon->table._assetClass(col);
            cairo_move_to(surface->cairo(),x+3,topTableOffset);
            cairo_rel_line_to(surface->cairo(),0,tableHeight);
          }
        cairo_move_to(surface->cairo(),x,topTableOffset);
        cairo_rel_line_to(surface->cairo(),0,tableHeight);
        cairo_set_line_width(surface->cairo(),0.5);
        cairo_stroke(surface->cairo());
      
        if (col>0 && col<colWidgets.size())
          {
            CairoSave cs(surface->cairo());
            cairo_move_to(surface->cairo(), x, columnButtonsOffset);
        	colWidgets[col].draw(surface->cairo());   
          }
      
        if (col>1)
          {
            cairo_move_to(surface->cairo(),x-pulldownHot,topTableOffset);
            pango.setMarkup("▼");
            pango.show();
          }
      
        double y=topTableOffset;
        double colWidth=minColumnWidth;
        for (unsigned row=0; row<godleyIcon->table.rows(); ++row)
          {
            if (row>0 && row<scrollRowStart) continue;

            if (col==0 && row>0 && col<rowWidgets.size())
              {
                CairoSave cs(surface->cairo());
                cairo_move_to(surface->cairo(), 0, y);
                rowWidgets[row].draw(surface->cairo());
              }
            
            CairoSave cs(surface->cairo());
            if (row!=0 || col!=0)               
              {
                string text=godleyIcon->table.cell(row,col);
                if (!text.empty())
                  {
                    string value;
                    FlowCoef fc(text);
                    if (displayValues)
                      try
                        {
                          auto vv=cminsky().variableValues
                            [VariableValue::valueIdFromScope
                             (godleyIcon->group.lock(),fc.name)];
                          if (vv.idx()>=0)
                            {
                              double val=fc.coef*vv.value();
                              auto ee=engExp(val);
                              if (ee.engExp==-3) ee.engExp=0;
                              value=" = "+mantissa(val,ee)+expMultiplier(ee.engExp);
                            }
                        }
                      catch (const std::exception& ex)
                        {
                          value=string("= Err: ")+ex.what();
                          // highlight error in red
                          cairo_set_source_rgb(surface->cairo(),1,0,0);
                        }
                            
                    // the active cell renders as bare LaTeX code for
                    // editing, all other cells rendered as LaTeX
                    if ((int(row)!=selectedRow || int(col)!=selectedCol) && !godleyIcon->table.initialConditionRow(row))
                      {
                        if (row>0 && col>0)    
                          { // handle DR/CR mode and colouring of text
                            if (fc.coef<0)
                              cairo_set_source_rgb(surface->cairo(),1,0,0);
                            if (displayStyle==DRCR)
                              {
                                if (assetClass==GodleyAssetClass::asset ||
                                    assetClass==GodleyAssetClass::noAssetClass)
                                  text = (fc.coef<0)?"CR ":"DR ";
                                else
                                  text = (fc.coef<0)?"DR ":"CR ";
                                fc.coef=abs(fc.coef);
                                text+=latexToPango(fc.str());
                              }
                            else
                              text = latexToPango(text);
                          }
                        else // is flow tag, stock var or initial condition
                          text = latexToPango(text);
                        if (row!=1 || col!=0) text+=value;  // Do not add value "= 0.0" to Initial Condtions cell(1,0). For ticket 1064
                      }
                    else
                      text=defang(text);
                  }
                pango.setMarkup(text);
              }
            // allow extra space for the ▼ in row 0
            colWidth=max(colWidth,pango.width() + (row==0? pulldownHot:0));
            cairo_move_to(surface->cairo(),x+3,y);
            pango.show();
            y+=rowHeight;
          }
        y=topTableOffset;
        colWidth+=5;

        colLeftMargin.push_back(x);
        x+=colWidth;
      }
  
    pango.setMarkup
      (capitalise(enumKey<GodleyAssetClass::AssetClass>(assetClass)));
    // increase column by enough to fit asset class label
    if (x < pango.width()+lastAssetBoundary+3)
      x=pango.width()+lastAssetBoundary+3;
    cairo_move_to(surface->cairo(),0.5*(x+lastAssetBoundary-pango.width()),0);
    showAsset(pango, surface->cairo(), assetClass);
    // final column vertical line
    colLeftMargin.push_back(x);
    cairo_move_to(surface->cairo(),x,topTableOffset);
    cairo_rel_line_to(surface->cairo(),0,tableHeight);
    cairo_move_to(surface->cairo(),x+3,topTableOffset);
    cairo_rel_line_to(surface->cairo(),0,tableHeight);
    cairo_set_line_width(surface->cairo(),0.5);
    cairo_stroke(surface->cairo());

    cairo_move_to(surface->cairo(),x-pulldownHot,topTableOffset);

    // now row sum column
    x+=3;
    double y=topTableOffset;
    cairo_move_to(surface->cairo(),x,y);
    pango.setMarkup("A-L-E");
    pango.show();
    double colWidth=pango.width();
    y+=rowHeight;
  
    for (unsigned row=1; row<godleyIcon->table.rows(); ++row)
      {
        if (row>0 && row<scrollRowStart) continue;
        pango.setMarkup(latexToPango(godleyIcon->table.rowSum(row)));
        colWidth=max(colWidth,pango.width());
        cairo_move_to(surface->cairo(),x,y);
        pango.show();
        y+=rowHeight;
      }

    x+=colWidth;
    y=topTableOffset;
    for (unsigned row=0; row<=godleyIcon->table.rows(); ++row)
      {
        // horizontal lines
        if (row>0 && row<scrollRowStart) continue;
        cairo_move_to(surface->cairo(),leftTableOffset,y);
        cairo_line_to(surface->cairo(),x,y);
        cairo_set_line_width(surface->cairo(),0.5);
        cairo_stroke(surface->cairo());
        y+=rowHeight;
      }

    // final vertical line
    colLeftMargin.push_back(x);
    cairo_move_to(surface->cairo(),x,topTableOffset);
    cairo_rel_line_to(surface->cairo(),0,tableHeight);
    cairo_set_line_width(surface->cairo(),0.5);
    cairo_stroke(surface->cairo());

    // indicate cell mouse is hovering over
    if ((hoverRow>0 || hoverCol>0) &&                                
        size_t(hoverRow)<godleyIcon->table.rows() &&
        size_t(hoverCol)<godleyIcon->table.cols())
      {
        CairoSave cs(surface->cairo());
        cairo_rectangle(surface->cairo(),
                        colLeftMargin[hoverCol],hoverRow*rowHeight+topTableOffset,
                        colLeftMargin[hoverCol+1]-colLeftMargin[hoverCol],rowHeight);
        cairo_set_line_width(surface->cairo(),1);
        cairo_stroke(surface->cairo());
      }
          
    // indicate selected cells
    {
      CairoSave cs(surface->cairo());
      if (selectedRow==0 || (selectedRow>=int(scrollRowStart) && selectedRow<int(godleyIcon->table.rows())))
        {
          size_t i=0, j=0;
          if (selectedRow>=int(scrollRowStart)) j=selectedRow-scrollRowStart+1;
          double y=j*rowHeight+topTableOffset;

          if (motionCol>=0 && selectedRow==0 && selectedCol>0) // whole col being moved
            {
              highlightColumn(surface->cairo(),selectedCol);
              highlightColumn(surface->cairo(),motionCol);
            }
          else if (motionRow>=0 && selectedCol==0 && selectedRow>0) // whole Row being moved
            {
              highlightRow(surface->cairo(),selectedRow);
              highlightRow(surface->cairo(),motionRow);
            }
          else if (selectedCol==0 || /* selecting individual cell */
                   (selectedCol>=int(scrollColStart) && selectedCol<int(godleyIcon->table.cols())))   
            {
              if ((selectedRow>1 || selectedRow <0) || selectedCol!=0) // can't select flows/stockVars or Initial Conditions labels
                {
                  if (selectedCol>=int(scrollColStart)) i=selectedCol-scrollColStart+1;
                  double x=colLeftMargin[i];
                  cairo_set_source_rgba(surface->cairo(),1,1,1,1);
                  cairo_rectangle(surface->cairo(),x,y,colLeftMargin[i+1]-x,rowHeight);
                  cairo_fill(surface->cairo());
                  pango.setMarkup(defang(godleyIcon->table.cell(selectedRow,selectedCol)));
                  cairo_set_source_rgba(surface->cairo(),0,0,0,1);
                  cairo_move_to(surface->cairo(),x,y);
                  pango.show();
				  
                  // show insertion cursor
                  cairo_move_to(surface->cairo(),x+pango.idxToPos(insertIdx),y);
                  cairo_rel_line_to(surface->cairo(),0,rowHeight);
                  cairo_set_line_width(surface->cairo(),1);
                  cairo_stroke(surface->cairo());
                  if (motionRow>0 && motionCol>0)
                    highlightCell(surface->cairo(),motionRow,motionCol);
                  if (selectIdx!=insertIdx)
                    {
                      // indicate some text has been selected
                      cairo_rectangle(surface->cairo(),x+pango.idxToPos(insertIdx),y,
                                      pango.idxToPos(selectIdx)-pango.idxToPos(insertIdx),rowHeight);
                      cairo_set_source_rgba(surface->cairo(),0.5,0.5,0.5,0.5);
                      cairo_fill(surface->cairo());
                   }
                }
            }
        }
    }
  }

  int GodleyTableWindow::colX(double x) const
  {
    if (x<colLeftMargin[0]) return -1;
    if (x<colLeftMargin[1]) return 0;
    auto p=std::upper_bound(colLeftMargin.begin(), colLeftMargin.end(), x);
    size_t r=p-colLeftMargin.begin()-2+scrollColStart;
    if (r>godleyIcon->table.cols()) r=-1; // out of bounds, invalidate
    return r;
  }

  int GodleyTableWindow::rowY(double y) const
  {
    int c=(y-topTableOffset)/rowHeight;
    if (c>0) c+=scrollRowStart-1;
    if (c<0 || size_t(c)>godleyIcon->table.rows()) c=-1; // out of bounds, invalidate
    return c;
  }

  int GodleyTableWindow::textIdx(double x) const
  {
    cairo::Surface surf(cairo_recording_surface_create(CAIRO_CONTENT_COLOR,NULL));
    ZoomablePango pango(surf.cairo());
    if (selectedRow>=0 && size_t(selectedRow)<godleyIcon->table.rows() &&
        selectedCol>=0 && size_t(selectedCol)<godleyIcon->table.cols() && (selectedRow!=1 || selectedCol!=0)) // No text index needed for a cell that is immutable. For ticket 1064
      {
        auto& str=godleyIcon->table.cell(selectedRow,selectedCol);
        pango.setMarkup(defang(str));
        int j=0;
        if (selectedCol>=int(scrollColStart)) j=selectedCol-scrollColStart+1;
        x-=colLeftMargin[j]+2;
        x*=zoomFactor;
        return x>0 && str.length()>0?pango.posToIdx(x)+1: 0;  
      }
    return 0;
  }

  void GodleyTableWindow::mouseDown(double x, double y)
  {
    x/=zoomFactor;
    y/=zoomFactor;
    switch (clickType(x,y))
      {
      case rowWidget:
        {
          unsigned r=rowY(y);
          if (r<rowWidgets.size())
            {
              rowWidgets[r].invoke(x);
              adjustWidgets();
              selectedCol=selectedRow=-1;
              requestRedraw();
            }
          return;
        }
      case colWidget:
        {
          unsigned c=colX(x);
          unsigned visibleCol=c-scrollColStart+1;
          if (c<colWidgets.size() && visibleCol < colLeftMargin.size())
            {
              colWidgets[c].invoke(x-colLeftMargin[visibleCol]);
              adjustWidgets();
              selectedCol=selectedRow=-1;
              requestRedraw();
            }
          return;
        }
      default:
        // catch exception, as the intention here is to allow the user to fix a problem
        try {update();}
        catch (...) {}
        selectedCol=colX(x);
        selectedRow=rowY(y);
        if (selectedRow>=0 && selectedRow<int(godleyIcon->table.rows()) &&
            selectedCol>=0 && selectedCol<int(godleyIcon->table.cols()) && (selectedRow!=1 || selectedCol!=0)) // Cannot save text in cell(1,0). For ticket 1064
           {
             selectIdx=insertIdx = textIdx(x);
             auto& str=godleyIcon->table.cell(selectedRow,selectedCol);                         
             godleyIcon->table.savedText=godleyIcon->table.cell(selectedRow, selectedCol);
           }
        else
          selectIdx=insertIdx=0;
        requestRedraw();
        break;
      }
  }

  void GodleyTableWindow::mouseUp(double x, double y)
  {
    x/=zoomFactor;
    y/=zoomFactor;
    int c=colX(x), r=rowY(y);
    motionRow=motionCol=-1;
    // Cannot swap cell(1,0) with another. For ticket 1064. Also cannot move cells outside an existing Godley table to create new rows or columns. For ticket 1066. 
    if ((selectedCol==0 && selectedRow==1) || (c==0 && r==1) || size_t(selectedRow)>=(godleyIcon->table.rows()) || size_t(r)>=(godleyIcon->table.rows()) || size_t(c)>=(godleyIcon->table.cols()) || size_t(selectedCol)>=(godleyIcon->table.cols()))
      return;  
    else if (selectedRow==0)
      {  
		// Disallow moving flow labels column and prevent columns from moving when import stockvar dropdown button is pressed in empty column. For tickets 1053/1064/1066
        if (c>0 && size_t(c)<godleyIcon->table.cols() && selectedCol>0 && size_t(selectedCol)<godleyIcon->table.cols() && c!=selectedCol && !(colLeftMargin[c+1]-x < pulldownHot)) 
          godleyIcon->table.moveCol(selectedCol,c-selectedCol);
      }
    else if (r>0 && selectedCol==0)
      {
        if (r!=selectedRow && !godleyIcon->table.initialConditionRow(selectedRow) && !godleyIcon->table.initialConditionRow(r))  // Cannot move Intitial Conditions row. For ticket 1064.
          godleyIcon->table.moveRow(selectedRow,r-selectedRow);
      } 
    else if ((c!=selectedCol || r!=selectedRow) && c>0 && r>0)
      {
        swap(godleyIcon->table.cell(selectedRow,selectedCol), godleyIcon->table.cell(r,c));
        selectedCol=c;
        selectedRow=r;
      }
    else if (selectIdx!=insertIdx)
      copy();
     
    requestRedraw();
  }

  void GodleyTableWindow::mouseMoveB1(double x, double y)
  {
    x/=zoomFactor;
    y/=zoomFactor;
    motionCol=colX(x), motionRow=rowY(y);
    if (motionCol==selectedCol && motionRow==selectedRow)
      selectIdx=textIdx(x);
  }

  void GodleyTableWindow::mouseMove(double x, double y)
  {
    x/=zoomFactor;
    y/=zoomFactor;
    // clear any existing marks
    for (auto& i: rowWidgets) i.hover(-1);
    for (auto& i: colWidgets) i.hover(-1);
    hoverRow=hoverCol=-1;
    switch (clickType(x,y))
      {
      case rowWidget:
        {
          unsigned r=rowY(y);
          if (r<rowWidgets.size())
            rowWidgets[r].hover(x);
          break;
        }
      case colWidget:
        {
          unsigned c=colX(x);
          if (c<colWidgets.size())
            colWidgets[c].hover(x-colLeftMargin[c]);
          break;
        }
      case background:
        break;
      default:
        hoverRow=rowY(y);
        hoverCol=colX(x);
        break;
      }
    requestRedraw();
  }

  inline constexpr char control(char x) {return x-'`';}
  
  void GodleyTableWindow::keyPress(int keySym, const std::string& utf8)
  {
    
    auto& table=godleyIcon->table;
    if (selectedCol>=0 && selectedRow>=0 && selectedCol<int(table.cols()) &&
        selectedRow<int(table.rows()) && (selectedCol!=0 || selectedRow!=1)) // Cell (1,0) is off-limits. For ticket 1064
          {			  	  
            auto& str=table.cell(selectedRow,selectedCol);
            if (utf8.length() && (keySym<0x7f || (0xffaa <= keySym && keySym <= 0xffbf)))  // Enable numeric keypad key presses. For ticket 1136
              // all printing and control characters have keysym
              // <0x80. But some keys (eg tab, backspace and escape
              // are mapped to control characters
              if (unsigned(utf8[0])>=' ' && utf8[0]!=0x7f)
                {
                  delSelection();
                  if (insertIdx>=str.length()) insertIdx=str.length();
                  str.insert(insertIdx,utf8);
                  selectIdx=insertIdx+=utf8.length();
                }
              else
                {
                  switch (utf8[0]) // process control characters
                    {
                    case control('x'):
                      cut();
                      break;
                    case control('c'):
                      copy();
                      break;
                    case control('v'):
                      paste();
                      break;
                    case control('h'): case 0x7f:
                      handleDelete();
                      break;
                    }
                }
            else
              {
              switch (keySym)
                {
                case 0xff08: // backspace
		          handleBackspace();
		          break;
		        case 0xffff:  // delete
                  handleDelete();
                  break;
                case 0xff1b: // escape
                  if (selectedRow>=0 && size_t(selectedRow)<=table.rows() &&
                      selectedCol>=0 && size_t(selectedCol)<=table.cols())
                    table.cell(selectedRow, selectedCol)=table.savedText;
                  selectedRow=selectedCol=-1;
                  break;
                case 0xff0d: //return
                case 0xff8d: //enter added for ticket 1122                            
                  update();
                  selectedRow=selectedCol=-1;                  
                  break;     
                case 0xff51: //left arrow
                  if (insertIdx>0) insertIdx--;
                  else navigateLeft();
                  break;
                case 0xff53: //right arrow
                  if (insertIdx<str.length()) insertIdx++;
                  else navigateRight();
                  break;
                case 0xff09: // tab
                  navigateRight();
                  break;
                case 0xfe20: // back tab
                  navigateLeft();
                  break;
                case 0xff54: // down
                  navigateDown();
                  break;
                case 0xff52: // up
                  navigateUp();
                  break;
                default:
                  return; // key not handled, just return without resetting selection
                }
                selectIdx=insertIdx;
              }
          }  
    else // nothing selected
      {
        // if one of the navigation keys pressed, move to the first/last etc cell
        switch (keySym)
          {
          case 0xff09: case 0xff53: // tab, right
            selectedRow=0; selectedCol=1; break;
          case 0xfe20: // back tab
            selectedRow=table.rows()-1; selectedCol=table.cols()-1; break;
          case 0xff51: //left arrow
            selectedRow=0; selectedCol=table.cols()-1; break;
          case 0xff54: // down
            selectedRow=2; selectedCol=0; break;           // Start from second row because Initial Conditions cell (1,0) can no longer be selected. For ticket 1064
          case 0xff52: // up
            selectedRow=table.rows()-1; selectedCol=0; break;
          default:
            return; // early return, no need to redraw
          }
      }
    requestRedraw();
  }

  void GodleyTableWindow::delSelection()
  {
    if (insertIdx!=selectIdx)
      {
        auto& str=godleyIcon->table.cell(selectedRow,selectedCol);
        str.erase(min(insertIdx,selectIdx),abs(int(insertIdx)-int(selectIdx))); 
        selectIdx=insertIdx=min(insertIdx,selectIdx);   
      }
  }

    void GodleyTableWindow::handleBackspace()
    {
      auto& table=godleyIcon->table;
      assert(selectedRow>=0 && selectedCol>=0);
      assert(unsigned(selectedRow)<table.rows());
      assert(unsigned(selectedCol)<table.cols());
      auto& str=table.cell(selectedRow,selectedCol); 
      if (insertIdx!=selectIdx)
        delSelection();
      else if (insertIdx>0 && insertIdx<=str.length())
        str.erase(--insertIdx,1);
      selectIdx=insertIdx;
    }

      void GodleyTableWindow::handleDelete()
    {
      auto& table=godleyIcon->table;
      assert(selectedRow>=0 && selectedCol>=0);
      assert(unsigned(selectedRow)<table.rows());
      assert(unsigned(selectedCol)<table.cols());
      auto& str=table.cell(selectedRow,selectedCol); 
      if (insertIdx!=selectIdx)
        delSelection();
      else if (insertIdx>=0 && insertIdx<str.length())
        str.erase(insertIdx,1);
      selectIdx=insertIdx;
    }

  void GodleyTableWindow::cut()
  {
    copy();
    if (selectedCol>=0 && selectedRow>=0 && selectedCol<int(godleyIcon->table.cols()) &&
        selectedRow<int(godleyIcon->table.rows()))
      {	  
         if (selectIdx==insertIdx)
           // delete entire cell
           godleyIcon->table.cell(selectedRow,selectedCol).clear();
         else
           delSelection();
         requestRedraw();   
      }
  }
  
  void GodleyTableWindow::copy()
  {
    if (selectedCol>=0 && selectedRow>=0 && selectedCol<int(godleyIcon->table.cols()) &&
        selectedRow<int(godleyIcon->table.rows()))
      {	  
         auto& str=godleyIcon->table.cell(selectedRow,selectedCol);
         if (selectIdx!=insertIdx)
           cminsky().putClipboard
             (str.substr(min(selectIdx,insertIdx), abs(int(selectIdx)-int(insertIdx))));
         else
           cminsky().putClipboard(str);  
      }
  }

  void GodleyTableWindow::paste()
  {
    if (selectedCol>=0 && selectedRow>=0 && selectedCol<int(godleyIcon->table.cols()) &&
        selectedRow<int(godleyIcon->table.rows()))
      {
	     delSelection();
         auto& str=godleyIcon->table.cell(selectedRow,selectedCol); 
         auto stringToInsert=cminsky().getClipboard();
         // only insert first line
         auto p=stringToInsert.find('\n');
         if (p!=string::npos)
           stringToInsert=stringToInsert.substr(0,p-1);
         str.insert(insertIdx,stringToInsert);
         selectIdx=insertIdx+=stringToInsert.length();
      }
    requestRedraw();
  }

  GodleyTableWindow::ClickType GodleyTableWindow::clickType(double x, double y) const
  {
    int c=colX(x), r=rowY(y);
    if (x<leftTableOffset && r>0)
      return rowWidget;
    if (y<topTableOffset && y>columnButtonsOffset && c>0)
      return colWidget;
  
    if (r==0)
      {
        if (colLeftMargin[c+1]-x < pulldownHot && c!=int(godleyIcon->table.cols())-1) //Disable importStock on Equity column. For ticket 1154
          return importStock;
        return row0;
      }
    if (c==0)
      return col0;

    if (c>0 && c<int(godleyIcon->table.cols()))
      if (r>0 && r<int(godleyIcon->table.rows()))
        return internal;

    return background;
  }

  void GodleyTableWindow::addStockVar(double x)
  {
    int c=colX(x);
    if (c>0)
      godleyIcon->table.insertCol(c+1);
    requestRedraw();
  }
  void GodleyTableWindow::importStockVar(const string& name, double x)
  {
    x/=zoomFactor;
    int c=colX(x);
    if (c>0)
      {
        godleyIcon->table.cell(0,c)=name;
        minsky().importDuplicateColumn(godleyIcon->table, c);
      }
    requestRedraw();
  }

  void GodleyTableWindow::deleteStockVar(double x)
  {
    x/=zoomFactor;
    int c=colX(x);
    if (c>=0)
      godleyIcon->table.deleteCol(c+1);
    requestRedraw();
  }

  void GodleyTableWindow::addFlow(double y)  
  {
    y/=zoomFactor;
    int r=rowY(y);
    if (r>0)                                 
      godleyIcon->table.insertRow(r+1);
    requestRedraw();
  }

  void GodleyTableWindow::deleteFlow(double y)
  {
    y/=zoomFactor;
    int r=rowY(y);
    if (r>1)                                       // Cannot delete flow in Initial Conditions row. For ticket 1064
      godleyIcon->deleteRow(r+1);
    requestRedraw();
  }
  
namespace {
  string constructMessage(GodleyAssetClass::AssetClass& targetAC, GodleyAssetClass::AssetClass& oldAC, string& var)
  {
	  string tmpStr="";
	  tmpStr="This will convert "+var+" from "+classdesc::enumKey<GodleyAssetClass::AssetClass>(oldAC)+" to "+classdesc::enumKey<GodleyAssetClass::AssetClass>(targetAC)+". Are you sure?";
	  return tmpStr;
  }	  
}
  
  string GodleyTableWindow::moveAssetClass(double x, double y)
  {
	x/=zoomFactor;
	y/=zoomFactor;
	unsigned c=colX(x);	
	string tmpStr="";
	if (clickType(x,y)==colWidget) {
	    unsigned visibleCol=c-scrollColStart+1;
        if (c<colWidgets.size() && visibleCol < colLeftMargin.size()) {
		    auto moveVar=godleyIcon->table.cell(0,c);		
		    auto oldAssetClass=godleyIcon->table._assetClass(c);
		    auto targetAssetClassPlus=godleyIcon->table._assetClass(c+1);
		    auto targetAssetClassMinus=godleyIcon->table._assetClass(c-1);
            if (colWidgets[c].button(x-colLeftMargin[visibleCol])==3 && oldAssetClass!=GodleyAssetClass::equity) {
		    	if (targetAssetClassPlus!=oldAssetClass && !moveVar.empty() && targetAssetClassPlus!=GodleyAssetClass::equity && targetAssetClassPlus!=GodleyAssetClass::noAssetClass)
		    	    tmpStr=constructMessage(targetAssetClassPlus,oldAssetClass,moveVar);
		    	else if ((targetAssetClassPlus==GodleyAssetClass::equity || targetAssetClassPlus==GodleyAssetClass::noAssetClass) && !moveVar.empty())
		    	    tmpStr="Cannot convert stock variable to an equity class";    
		    }
		    else if (colWidgets[c].button(x-colLeftMargin[visibleCol])==2 && oldAssetClass==GodleyAssetClass::asset && oldAssetClass!=GodleyAssetClass::equity && targetAssetClassMinus!=GodleyAssetClass::asset) {
		    	if (targetAssetClassPlus!=oldAssetClass && !moveVar.empty() && targetAssetClassPlus!=GodleyAssetClass::equity && targetAssetClassPlus!=GodleyAssetClass::noAssetClass)
		    	    tmpStr=constructMessage(targetAssetClassPlus,oldAssetClass,moveVar);
		    	else if ((targetAssetClassPlus==GodleyAssetClass::equity || targetAssetClassPlus==GodleyAssetClass::noAssetClass) && !moveVar.empty())
		    	    tmpStr="Cannot convert stock variable to an equity class"; 		    	    
		    }
		    else if (colWidgets[c].button(x-colLeftMargin[visibleCol])==2 && oldAssetClass!=GodleyAssetClass::equity) {
		    	if (targetAssetClassMinus!=oldAssetClass && !moveVar.empty())
		    	    tmpStr=constructMessage(targetAssetClassMinus,oldAssetClass,moveVar);
		    }
		}
	}
   return tmpStr;	    		 	
   }
  
  string GodleyTableWindow::swapAssetClass(double x, double y) 
  {  
	x/=zoomFactor;
	y/=zoomFactor;
	int c=colX(x);	
	string tmpStr="";	  
	if (selectedRow==0) {  // clickType triggers pango error which causes this condition to be skipped and thus column gets moved to Equity, which should not be the case   	
	    if (c>0 && selectedCol>0 && c!=selectedCol) {
		    auto swapVar=godleyIcon->table.cell(0,selectedCol);
		    auto oldAssetClass=godleyIcon->table._assetClass(selectedCol);
		    auto targetAssetClass=godleyIcon->table._assetClass(c);
		    if (!swapVar.empty() && !(colLeftMargin[c+1]-x < pulldownHot)) { // ImportVar dropdown button should not trigger this condition. For ticket 1162
		      if (targetAssetClass!=oldAssetClass && targetAssetClass!=GodleyAssetClass::equity && targetAssetClass!=GodleyAssetClass::noAssetClass)
		         tmpStr=constructMessage(targetAssetClass,oldAssetClass,swapVar);
		      else if ((targetAssetClass==GodleyAssetClass::equity || targetAssetClass==GodleyAssetClass::noAssetClass) || oldAssetClass==GodleyAssetClass::noAssetClass)
		          tmpStr="Cannot convert stock variable to an equity class"; 		    
			}
		  }
	  }
	return tmpStr;  	  
  }    

  void GodleyTableWindow::highlightColumn(cairo_t* cairo, unsigned col)
  {
    if (col<scrollColStart) return;
    double x=colLeftMargin[col-scrollColStart+1];
    double width=colLeftMargin[col-scrollColStart+2]-x;
    double tableHeight=(godleyIcon->table.rows()-scrollRowStart+1)*rowHeight;
    cairo_rectangle(cairo,x,topTableOffset,width,tableHeight);
    cairo_set_source_rgba(cairo,1,1,1,0.5);
    cairo_fill(cairo);
  }

  void GodleyTableWindow::highlightRow(cairo_t* cairo, unsigned row)
  {
    if (row<scrollRowStart) return;
    double y=(row-scrollRowStart+1)*rowHeight+topTableOffset;
    cairo_rectangle(cairo,leftTableOffset,y,colLeftMargin.back()-leftTableOffset,rowHeight);
    cairo_set_source_rgba(cairo,1,1,1,0.5);
    cairo_fill(cairo);
  }

  void GodleyTableWindow::highlightCell(cairo_t* cairo, unsigned row, unsigned col)
  {
    if (row<scrollRowStart || col<scrollColStart) return;
    double x=colLeftMargin[col-scrollColStart+1];
    double width=colLeftMargin[col-scrollColStart+2]-x;
    double y=(row-scrollRowStart+1)*rowHeight+topTableOffset;
    cairo_rectangle(cairo,x,y,width,rowHeight);
    cairo_set_source_rgba(cairo,1,1,1,0.5);
    cairo_fill(cairo);
  }

  void GodleyTableWindow::pushHistory()
  {
    while (history.size()>maxHistory) history.pop_front();
    // Perform deep comparison of Godley tables in history to avoid spurious noAssetClass columns from arising during undo. For ticket 1118.
    if (history.empty() || !(history.back()==godleyIcon->table)) {
      history.push_back(godleyIcon->table);
    }
    historyPtr=history.size();
  }
      
  void GodleyTableWindow::undo(int changes)
  { 
    if (historyPtr==history.size())
      pushHistory();
    historyPtr-=changes;
    if (historyPtr > 0 && historyPtr <= history.size())
      {
        auto& d=history[historyPtr-1];
        // Perform deep comparison of Godley tables in history to avoid spurious noAssetClass columns from arising during undo. For ticket 1118.
        if (d.getData().empty()) return; // should not happen
		godleyIcon->table=d; 
        requestRedraw();
      }
  }

  void GodleyTableWindow::adjustWidgets()
  {
    assert(godleyIcon);
    rowWidgets.clear();
    for (size_t i=0; i<godleyIcon->table.rows(); ++i)
      rowWidgets.emplace_back(*godleyIcon, i);
    colWidgets.clear();
    for (size_t i=0; i<godleyIcon->table.cols(); ++i)
      colWidgets.emplace_back(*godleyIcon, i);
    // nb first column/row is actually 1 - 0th element actually
    // just ignored
    if (rowWidgets.size()==2)
      rowWidgets[1].pos=firstAndLast;
    else if (rowWidgets.size()==3)
      {
        rowWidgets[1].pos=first;
        rowWidgets.back().pos=second;       // Position to avoid Initial Conditions row from being moved. For ticket 1064
      }
    else if (rowWidgets.size()>3)
      {
        rowWidgets[1].pos=first;      
        rowWidgets[2].pos=second;          // Position to avoid Initial Conditions row from being moved. For ticket 1064
        rowWidgets.back().pos=last;     
      }  
    if (colWidgets.size()==2)
      colWidgets[1].pos=firstAndLast;
    else if (colWidgets.size()>2)
      {
        colWidgets[1].pos=first;
        colWidgets.back().pos=last;
      }
  }

  void GodleyTableWindow::update()
  {
    if (selectedCol>0 && selectedCol<int(godleyIcon->table.cols()))
      {
        if (selectedRow==0)
          {
            // rename all instances of the stock variable if updated. For ticket #956
            // find stock variable if it exists
            for (auto& sv: godleyIcon->stockVars())
              if (sv->valueId()==godleyIcon->valueId(godleyIcon->table.savedText))
                {
                  auto savedItem=minsky().canvas.item;
                  minsky().canvas.item=sv;
                  minsky().canvas.renameAllInstances(godleyIcon->table.cell(selectedRow,selectedCol));
                  savedItem.swap(minsky().canvas.item);
                }
            minsky().importDuplicateColumn(godleyIcon->table, selectedCol);
          }
        else
          minsky().balanceDuplicateColumns(*godleyIcon,selectedCol);
        // get list of GodleyIcons first, rather than doing recursiveDo, as update munges the items vectors
        auto godleyTables=minsky().model->findItems
          ([](const ItemPtr& i){return dynamic_cast<GodleyIcon*>(i.get());});
        for (auto& i: godleyTables)
          if (auto g=dynamic_cast<GodleyIcon*>(i.get()))
            g->update();
      }
    minsky().canvas.requestRedraw();
  }

  void GodleyTableWindow::checkCell00()
  {
    if (selectedCol==0 && (selectedRow==0 || selectedRow ==1))
      // (0,0) cell not editable
      {
        selectedCol=-1;
        selectedRow=-1;
      }         
  }
  
    void GodleyTableWindow::navigateRight()
    {
      if (selectedCol>=0)
        {
          selectedCol++;
          insertIdx=0;
          if (selectedCol>=int(godleyIcon->table.cols()))
            {
              if (selectedRow>0) selectedCol=0;   // Minor fix: Make sure tabbing and right arrow traverse all editable cells.
              else selectedCol=1;
              navigateDown();
            }
          checkCell00();
        }
    }
  
    void GodleyTableWindow::navigateLeft()
    {
      if (selectedCol>=0)
        {
          selectedCol--;
          insertIdx=0;
          if (selectedCol<0)
            {
              selectedCol=godleyIcon->table.cols()-1;
              navigateUp();
            }
          checkCell00();
        }
    }

    void GodleyTableWindow::navigateUp()
    {
      if (selectedRow>=0)
        selectedRow=(selectedRow-1)%godleyIcon->table.rows();
      checkCell00();
    }
  
    void GodleyTableWindow::navigateDown()
    {
      if (selectedRow>=0)
        selectedRow=(selectedRow+1)%godleyIcon->table.rows();
      checkCell00();
    }

  template <ButtonWidgetEnums::RowCol rowCol>
  void ButtonWidget<rowCol>::drawButton(cairo_t* cairo, const std::string& label, double r, double g, double b, int idx)
  {
    // stash current point for drawing a box
    double x0, y0;
    cairo_get_current_point(cairo,&x0, &y0);
    
    CairoSave cs(cairo);
    ZoomablePango pango(cairo);
    // increase text size a bit for the buttons
    pango.setFontSize(0.8*buttonSpacing*ZoomablePango::zoomFactor);
    pango.setMarkup(label);
    cairo_set_source_rgb(cairo,r,g,b);
    pango.show();
    
    // draw box around button
    cairo_rectangle(cairo, x0, y0+0.2*pango.height(), buttonSpacing, buttonSpacing);
    if (idx==m_mouseOver)
      cairo_set_source_rgb(cairo,0,0,0); // draw in black if mouse over button
    else
      cairo_set_source_rgb(cairo,0.5,0.5,0.5); // draw in grey
    cairo_set_line_width(cairo,1);
    cairo_stroke(cairo);
    cairo_move_to(cairo,x0+buttonSpacing,y0);
  }
  
  template <ButtonWidgetEnums::RowCol rowCol>
  void ButtonWidget<rowCol>::draw(cairo_t* cairo)
  {	    
    CairoSave cs(cairo);
    int idx=0;
    if ((rowCol==row || !cminsky().multipleEquities) || (!cminsky().multipleEquities && godleyIcon.table.singleEquity())) {  // no column widgets on equity column in single equity column mode
      if (rowCol == row || (rowCol == col && pos!=last)) 
        drawButton(cairo,"+",0,1,0,idx++);
      if ((rowCol == row && pos!=first && pos!=firstAndLast) || (rowCol == col && pos!=last)) 	// no delete button for first row containing initial conditions. For ticket 1064
	    drawButton(cairo,"—",1,0,0,idx++);  
      if ((rowCol == row && pos!=first && pos!=second && pos!=firstAndLast) || (rowCol == col && pos!=first && pos!=last))	// no move up button for first row containing initial conditions. For ticket 1064
        drawButton(cairo,rowCol==row? "↑": "←",0,0,0,idx++);
      if ((pos!=first && pos!=last && pos!=firstAndLast) || (rowCol == col && pos!=last))      // no move down button for first row containing initial conditions. For ticket 1064
        drawButton(cairo,rowCol==row? "↓": "→",0,0,0,idx++);
    } else {
      drawButton(cairo,"+",0,1,0,idx++);
      if ((pos!=first && pos!=firstAndLast) || rowCol == col) 	// no delete button for first row containing initial conditions. For ticket 1064
		drawButton(cairo,"—",1,0,0,idx++);
      if (pos!=first && pos!=second && pos!=firstAndLast) 						// no move up button for first row containing initial conditions. For ticket 1064
        drawButton(cairo,rowCol==row? "↑": "←",0,0,0,idx++);
      if ((pos!=first && pos!=last && pos!=firstAndLast) || (rowCol == col && pos!=last))      // no move down button for first row containing initial conditions. For ticket 1064
        drawButton(cairo,rowCol==row? "↓": "→",0,0,0,idx++);		
	}
    
  }  
 
  template class ButtonWidget<ButtonWidgetEnums::row>;
  template class ButtonWidget<ButtonWidgetEnums::col>;
}
