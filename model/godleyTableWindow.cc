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

#include "cairoItems.h"
#include "minsky.h"
#include "godleyTableWindow.h"
#include "selection.h"
#include "latexMarkup.h"
#include <pango.h>

#include "assetClass.rcd"
#include "godleyTableWindow.rcd"
#include "godleyTableWindow.xcd"
#include "minsky_epilogue.h"

#include <boost/locale.hpp>

using namespace std;
using namespace minsky;
using ecolab::Pango;
using namespace ecolab::cairo;
using namespace boost::locale::conv;

#include <cairo/cairo-ps.h>
#include <cairo/cairo-pdf.h>
#include <cairo/cairo-svg.h>

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
    const CairoSave cs(cairo);
    auto& colour=assetColour[assetClass];
    cairo_set_source_rgb(cairo,colour.r,colour.g,colour.b);
    pango.show();
  }

}

namespace minsky
{
  template <>
  void ButtonWidget<ButtonWidgetEnums::row>::invoke(double x)
  {
    const int button=x/buttonSpacing;
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
    const int button=x/buttonSpacing;
    if (!cminsky().multipleEquities() && godleyIcon.table.singleEquity()) {  // no column widgets on equity column in single equity column mode
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

  bool GodleyTableEditor::selectedCellInTable() const
  {
    return m_godleyIcon.table.cellInTable(selectedRow, selectedCol);
  }

  void GodleyTableEditor::draw(cairo_t *cairo)
  {
    const CairoSave cs(cairo);
    cairo_scale(cairo,zoomFactor,zoomFactor);
    Pango pango(cairo);
    pango.setMarkup("Flows ↓ / Stock Vars →");
    rowHeight=pango.height()+2;
    const double tableHeight=(m_godleyIcon.table.rows()-scrollRowStart+1)*rowHeight;
    double x=leftTableOffset;
    double lastAssetBoundary=x;
    auto assetClass=GodleyAssetClass::noAssetClass;
    // only recalculate colmn widths when no cell is selected.
    const bool resizeGrid=selectedCol<0 || selectedRow<0 || motionRow>=0 || motionCol>=0;
    if (resizeGrid)
      colLeftMargin.clear();
  
    for (unsigned col=0; col<m_godleyIcon.table.cols(); ++col)
      {
        // omit stock columns less than scrollColStart
        if (col>0 && col<scrollColStart) continue;
        // vertical lines & asset type tag
        if (assetClass!=m_godleyIcon.table.assetClass(col))
          {
            if (assetClass!=GodleyAssetClass::noAssetClass)
              {
                pango.setMarkup(capitalise(enumKey<GodleyAssetClass::AssetClass>(assetClass)));
                // increase column by enough to fit asset class label
                if (x < pango.width()+lastAssetBoundary+3)
                  x=pango.width()+lastAssetBoundary+3;
                cairo_move_to(cairo,0.5*(x+lastAssetBoundary-pango.width()),0);
                showAsset(pango, cairo, assetClass);
              }
            lastAssetBoundary=x;
          
            assetClass=m_godleyIcon.table.assetClass(col);
            cairo_move_to(cairo,x+3,topTableOffset);
            cairo_rel_line_to(cairo,0,tableHeight);
          }
        cairo_move_to(cairo,x,topTableOffset);
        cairo_rel_line_to(cairo,0,tableHeight);
        cairo_set_line_width(cairo,0.5);
        cairo_stroke(cairo);
      
        if (drawButtons && col>0 && col<colWidgets.size())
          {
            const CairoSave cs(cairo);
            cairo_move_to(cairo, x, columnButtonsOffset);
            colWidgets[col].draw(cairo);   
          }
      
//        if (col>1)
//          {
//            cairo_move_to(cairo,x-pulldownHot,topTableOffset);
//            pango.setMarkup("▼");
//            pango.show();
//          }
      
        double y=topTableOffset;
        double colWidth=minColumnWidth;
        for (unsigned row=0; row<m_godleyIcon.table.rows(); ++row)
          {
            if (row>0 && row<scrollRowStart) continue;

            if (drawButtons && col==0 && row>0 && row<rowWidgets.size())
              {
                const CairoSave cs(cairo);
                cairo_move_to(cairo, 0, y);
                rowWidgets[row].draw(cairo);
              }
            
            const CairoSave cs(cairo);
            if (row!=0 || col!=0)               
              {
                // Make sure non-utf8 chars converted to utf8 as far as possible. for ticket 1166.  
                string text=utf_to_utf<char>(m_godleyIcon.table.cell(row,col));
                if (!text.empty())
                  {
                    string value;
                    FlowCoef fc(text);
                    if (cminsky().displayValues && col!=0)  // Do not add value "= 0.0" to first column. For tickets 1064/1274
                      try
                        {
                          auto vv=cminsky().variableValues
                            [valueId(m_godleyIcon.group.lock(),utf_to_utf<char>(fc.name))];
                          if (vv->idx()>=0)
                            {
                              const double val=fc.coef*vv->value();
                              auto ee=engExp(val);
                              if (ee.engExp==-3) ee.engExp=0;
                              value=" = "+mantissa(val,ee)+expMultiplier(ee.engExp);
                            }
                        }
                      catch (const std::exception& ex)
                        {
                          value=string("= Err: ")+ex.what();
                          // highlight error in red
                          cairo_set_source_rgb(cairo,1,0,0);
                        }
                            
                    // the active cell renders as bare LaTeX code for
                    // editing, all other cells rendered as LaTeX
                    if ((int(row)!=selectedRow || int(col)!=selectedCol) && !m_godleyIcon.table.initialConditionRow(row))
                      {
                        if (row>0 && col>0)    
                          { // handle DR/CR mode and colouring of text
                            if (fc.coef<0)
                              cairo_set_source_rgb(cairo,1,0,0);
                            if (cminsky().displayStyle==GodleyTable::DRCR)
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
                        text+=value;
                      }
                    else
                      //Display values of parameters used as initial conditions in Godley tables. for ticket 1126.  
                      if (m_godleyIcon.table.initialConditionRow(row) && cminsky().displayValues) text=defang(text+=value);
                      else text=defang(text);
                  }
                pango.setMarkup(text);
              }
            // allow extra space for the ▼ in row 0
            colWidth=max(colWidth,pango.width() /* + (row==0? pulldownHot:0)*/);
            cairo_move_to(cairo,x+3,y);
            pango.show();
            y+=rowHeight;
          }
        colWidth+=5;

        if (resizeGrid)
          {
            colLeftMargin.push_back(x);
            x+=colWidth;
          }
        else if (col+1<colLeftMargin.size())
          x=colLeftMargin[col+1];
      }

    // display pulldown for last column
//    cairo_move_to(cairo,x-pulldownHot,topTableOffset);
//    pango.setMarkup("▼");
//    pango.show();

  
    pango.setMarkup
      (capitalise(enumKey<GodleyAssetClass::AssetClass>(assetClass)));
    // increase column by enough to fit asset class label
    if (x < pango.width()+lastAssetBoundary+3)
      x=pango.width()+lastAssetBoundary+3;
    cairo_move_to(cairo,0.5*(x+lastAssetBoundary-pango.width()),0);
    showAsset(pango, cairo, assetClass);
    // final column vertical line
    colLeftMargin.push_back(x);
    cairo_move_to(cairo,x,topTableOffset);
    cairo_rel_line_to(cairo,0,tableHeight);
    cairo_move_to(cairo,x+3,topTableOffset);
    cairo_rel_line_to(cairo,0,tableHeight);
    cairo_set_line_width(cairo,0.5);
    cairo_stroke(cairo);

    cairo_move_to(cairo,x-pulldownHot,topTableOffset);

    // now row sum column
    x+=3;
    double y=topTableOffset;
    cairo_move_to(cairo,x,0);     // display A-L-E above the final column. for ticket 1285
    pango.setMarkup("A-L-E");
    pango.show();
    double colWidth=pango.width();
  
    for (unsigned row=0; row<m_godleyIcon.table.rows(); ++row)    // perform row sum on stock var heading column too. for ticket 1285
      {
        if (row >0 && row<scrollRowStart) continue;                    
        pango.setMarkup(latexToPango(m_godleyIcon.rowSum(row)));
        colWidth=max(colWidth,pango.width());
        cairo_move_to(cairo,x,y);
        pango.show();
        y+=rowHeight;
      }

    x+=colWidth;
    y=topTableOffset;
    for (unsigned row=0; row<=m_godleyIcon.table.rows(); ++row)
      {
        // horizontal lines
        if (row>0 && row<scrollRowStart) continue;
        cairo_move_to(cairo,leftTableOffset,y);
        cairo_line_to(cairo,x,y);
        cairo_set_line_width(cairo,0.5);
        cairo_stroke(cairo);
        y+=rowHeight;
      }

    // final vertical line
    colLeftMargin.push_back(x);
    cairo_move_to(cairo,x,topTableOffset);
    cairo_rel_line_to(cairo,0,tableHeight);
    cairo_set_line_width(cairo,0.5);
    cairo_stroke(cairo);

    // indicate cell mouse is hovering over
    if ((hoverRow>0 || hoverCol>0) &&                                
        size_t(hoverRow)<m_godleyIcon.table.rows() &&
        size_t(hoverCol)<m_godleyIcon.table.cols())
      {
        const CairoSave cs(cairo);
        cairo_rectangle(cairo,
                        colLeftMargin[hoverCol],hoverRow*rowHeight+topTableOffset,
                        colLeftMargin[hoverCol+1]-colLeftMargin[hoverCol],rowHeight);
        cairo_set_line_width(cairo,1);
        cairo_stroke(cairo);
      }
          
    // indicate selected cells
    {
      const CairoSave cs(cairo);
      if (selectedRow==0 || (selectedRow>=int(scrollRowStart) && selectedRow<int(m_godleyIcon.table.rows())))
        {
          size_t i=0, j=0;
          if (selectedRow>=int(scrollRowStart)) j=selectedRow-scrollRowStart+1;

          if (motionCol>=0 && selectedRow==0 && selectedCol>0) // whole col being moved
            {
              highlightColumn(cairo,selectedCol);
              highlightColumn(cairo,motionCol);
            }
          else if (motionRow>=0 && selectedCol==0 && selectedRow>0) // whole Row being moved
            {
              highlightRow(cairo,selectedRow);
              highlightRow(cairo,motionRow);
            }
          else if (selectedCol==0 || /* selecting individual cell */
                   (selectedCol>=int(scrollColStart) && selectedCol<int(m_godleyIcon.table.cols())))   
            {
              if ((selectedRow>1 || selectedRow <0) || selectedCol!=0) // can't select flows/stockVars or Initial Conditions labels
                {
                  if (selectedCol>=int(scrollColStart)) i=selectedCol-scrollColStart+1;
                  const double xx=colLeftMargin[i], yy=j*rowHeight+topTableOffset;
                  {
                    const cairo::CairoSave cs(cairo);
                    cairo_set_source_rgba(cairo,1,1,1,1);
                    cairo_rectangle(cairo,xx,yy,colLeftMargin[i+1]-xx,rowHeight);
                    cairo_fill_preserve(cairo);
                    cairo_set_source_rgba(cairo,1,.55,0,1);
                    cairo_set_line_width(cairo,2);
                    cairo_stroke(cairo);
                  }
                  pango.setMarkup(defang(m_godleyIcon.table.cell(selectedRow,selectedCol)));
                  cairo_move_to(cairo,xx,yy);
                  pango.show();
				  
                  // show insertion cursor
                  cairo_move_to(cairo,xx+pango.idxToPos(insertIdx),yy);
                  cairo_rel_line_to(cairo,0,rowHeight);
                  cairo_set_line_width(cairo,1);
                  cairo_stroke(cairo);
                  if (motionRow>0 && motionCol>0)
                    highlightCell(cairo,motionRow,motionCol);
                  if (selectIdx!=insertIdx)
                    {
                      // indicate some text has been selected
                      cairo_rectangle(cairo,xx+pango.idxToPos(insertIdx),yy,
                                      pango.idxToPos(selectIdx)-pango.idxToPos(insertIdx),rowHeight);
                      cairo_set_source_rgba(cairo,0.5,0.5,0.5,0.5);
                      cairo_fill(cairo);
                   }
                }
            }
        }
    }
  }
  
  double GodleyTableEditor::height() const
  {
    return godleyIcon().table.rows()*rowHeight;
  }

  int GodleyTableEditor::colX(double x) const
  {
    if (colLeftMargin.size()<2 || x<colLeftMargin[0]) return -1;
    if (x<colLeftMargin[1]) return 0;
    auto p=std::upper_bound(colLeftMargin.begin(), colLeftMargin.end(), x);
    size_t r=p-colLeftMargin.begin()-2+scrollColStart;
    if (r>m_godleyIcon.table.cols()-1) r=-1; // out of bounds, invalidate. Also exclude A-L-E column. For ticket 1163.
    return r;
  }

  int GodleyTableEditor::rowY(double y) const
  {
    int c=(y-topTableOffset)/rowHeight;
    if (c>0) c+=scrollRowStart-1;
    if (c<0 || size_t(c)>m_godleyIcon.table.rows()) c=-1; // out of bounds, invalidate
    return c;
  }

  int GodleyTableEditor::textIdx(double x) const
  {
    const cairo::Surface surf(cairo_recording_surface_create(CAIRO_CONTENT_COLOR,NULL));
    Pango pango(surf.cairo());
    if (selectedCellInTable() && (selectedRow!=1 || selectedCol!=0)) // No text index needed for a cell that is immutable. For ticket 1064
      {
		// Make sure non-utf8 chars converted to utf8 as far as possible. for ticket 1166.  
        auto& str=m_godleyIcon.table.cell(selectedRow,selectedCol);
        str=utf_to_utf<char>(str);     
        pango.setMarkup(defang(str));
        int j=0;
        if (selectedCol>=int(scrollColStart)) j=selectedCol-scrollColStart+1;
        x-=colLeftMargin[j]+2;
        x*=zoomFactor;
        if (x>0 && str.length())
          {
            auto p=pango.posToIdx(x);
            if (p<str.length())
              return p+numBytes(str[p]);
          }
      }
    return 0;
  }

  void GodleyTableEditor::mouseDown(double x, double y)
  {
    // catch exception, as the intention here is to allow the user to fix a problem
    try {update();}
    catch (...) {}
    button1=true;
    x/=zoomFactor;
    y/=zoomFactor;
    requestRedrawCanvas();
    switch (clickType(x,y))
      {
      case rowWidget:
        {
          const unsigned r=rowY(y);
          if (r<rowWidgets.size())
            {
              rowWidgets[r].invoke(x);
              adjustWidgets();
              selectedCol=selectedRow=-1;
            }
          return;
        }
      case colWidget:
        {
          const unsigned c=colX(x);
          const unsigned visibleCol=c-scrollColStart+1;
          if (c<colWidgets.size() && visibleCol < colLeftMargin.size())
            {
              colWidgets[c].invoke(x-colLeftMargin[visibleCol]);
              adjustWidgets();
              selectedCol=selectedRow=-1;
            }
          return;
        }
      case background:
        selectIdx=insertIdx=0;
        selectedCol=selectedRow=-1;
        break;
      default:
        if (selectedRow>=0 && selectedCol>=0)
          { // if cell already selected, deselect to allow the chance to redraw
            selectedCol=selectedRow=-1;
            break;
          }
        selectedCol=colX(x);
        selectedRow=rowY(y);
        if (selectedCellInTable() && (selectedRow!=1 || selectedCol!=0)) // Cannot save text in cell(1,0). For ticket 1064
           {
             // Make sure non-utf8 chars converted to utf8 as far as possible. for ticket 1166.
             auto& str=m_godleyIcon.table.cell(selectedRow,selectedCol);
             str=utf_to_utf<char>(str);	 
             m_godleyIcon.table.savedText=str;
             selectIdx=insertIdx = textIdx(x);
           }
        else
          selectIdx=insertIdx=0;
        break;
      }
  }

  void GodleyTableEditor::mouseUp(double x, double y)
  {
    button1=false;
    x/=zoomFactor;
    y/=zoomFactor;
    const int c=colX(x), r=rowY(y);
    motionRow=motionCol=-1;
    // Cannot swap cell(1,0) with another. For ticket 1064. Also cannot move cells outside an existing Godley table to create new rows or columns. For ticket 1066. 
    if ((selectedCol==0 && selectedRow==1) || (c==0 && r==1) || size_t(selectedRow)>=(m_godleyIcon.table.rows()) || size_t(r)>=(m_godleyIcon.table.rows()) || size_t(c)>=(m_godleyIcon.table.cols()) || size_t(selectedCol)>=(m_godleyIcon.table.cols()))
      return;  
    if (selectedRow==0)
      {  
		// Disallow moving flow labels column and prevent columns from moving when import stockvar dropdown button is pressed in empty column. For tickets 1053/1064/1066
        if (c>0 && size_t(c)<m_godleyIcon.table.cols() && selectedCol>0 && size_t(selectedCol)<m_godleyIcon.table.cols() && c!=selectedCol && !(colLeftMargin[c+1]-x < pulldownHot)) 
          m_godleyIcon.table.moveCol(selectedCol,c-selectedCol);
      }
    else if (r>0 && selectedCol==0)
      {
        if (r!=selectedRow && !m_godleyIcon.table.initialConditionRow(selectedRow) && !m_godleyIcon.table.initialConditionRow(r))  // Cannot move Intitial Conditions row. For ticket 1064.
          m_godleyIcon.table.moveRow(selectedRow,r-selectedRow);
      } 
    else if ((c!=selectedCol || r!=selectedRow) && c>0 && r>0)
      {
        swap(m_godleyIcon.table.cell(selectedRow,selectedCol), m_godleyIcon.table.cell(r,c));
        minsky().balanceDuplicateColumns(m_godleyIcon,selectedCol);
        minsky().balanceDuplicateColumns(m_godleyIcon,c);
        selectedCol=-1;
        selectedRow=-1;
      }
    else if (selectIdx!=insertIdx)
      copy();
    requestRedrawCanvas();
  }

  void GodleyTableEditor::mouseMoveB1(double x, double y)
  {
    x/=zoomFactor;
    y/=zoomFactor;
    motionCol=colX(x), motionRow=rowY(y);
    if (motionCol==selectedCol && motionRow==selectedRow)
      selectIdx=textIdx(x);
    requestRedrawCanvas();
  }

  void GodleyTableEditor::mouseMove(double x, double y)
  {
    if (button1)
      {
        mouseMoveB1(x,y);
        return;
      }
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
          const unsigned r=rowY(y);
          if (r<rowWidgets.size())
            rowWidgets[r].hover(x);
          requestRedrawCanvas();
          break;
        }
      case colWidget:
        {
          const unsigned c=colX(x);
          if (c<colWidgets.size())
            colWidgets[c].hover(x-colLeftMargin[c]);
          requestRedrawCanvas();
          break;
        }
      case background:
        break;
      default:
        hoverRow=rowY(y);
        if (hoverRow>0) hoverRow-=scrollRowStart-1;
        hoverCol=colX(x);
        if (hoverCol>0) hoverCol-=scrollColStart-1;
        break;
      }
  }

  inline constexpr char control(char x) {return x-'`';}
  
  void GodleyTableEditor::keyPress(int keySym, const std::string& utf8)
  {
    
    auto& table=m_godleyIcon.table;
    if (selectedCellInTable() && (selectedCol!=0 || selectedRow!=1)) // Cell (1,0) is off-limits. For ticket 1064
          {			  	  
			auto& str=table.cell(selectedRow,selectedCol);
			str=utf_to_utf<char>(str);	 
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
                  if (insertIdx>0) insertIdx=prevIndex(str, insertIdx);
                  else navigateLeft();
                  break;
                case 0xff53: //right arrow
                  if (insertIdx<str.length()) insertIdx+=numBytes(str[insertIdx]);
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
    requestRedrawCanvas();
  }

  void GodleyTableEditor::delSelection()
  {
    if (selectedCellInTable() && insertIdx!=selectIdx)
      {
        auto& str=m_godleyIcon.table.cell(selectedRow,selectedCol);
        str.erase(min(insertIdx,selectIdx),abs(int(insertIdx)-int(selectIdx))); 
        selectIdx=insertIdx=min(insertIdx,selectIdx);   
      }
  }

    void GodleyTableEditor::handleBackspace()
    {
      if (!selectedCellInTable()) return;
      auto& table=m_godleyIcon.table;
      auto& str=table.cell(selectedRow,selectedCol);
      if (insertIdx!=selectIdx)
        delSelection();
      else if (insertIdx>0 && insertIdx<=str.length())
        {
          insertIdx=prevIndex(str, insertIdx);
          str.erase(insertIdx,numBytes(str[insertIdx]));
        }
      selectIdx=insertIdx;
    }

    void GodleyTableEditor::handleDelete()
    {
      if (!selectedCellInTable()) return;
      auto& table=m_godleyIcon.table;
      auto& str=table.cell(selectedRow,selectedCol); 
      if (insertIdx!=selectIdx)
        delSelection();
      else if (insertIdx<str.length())
        str.erase(insertIdx,numBytes(str[insertIdx]));
      selectIdx=insertIdx;
    }

  void GodleyTableEditor::cut()
  {
    if (!selectedCellInTable()) return;
    copy();
    if (selectIdx==insertIdx)
      // delete entire cell
      m_godleyIcon.table.cell(selectedRow,selectedCol).clear();
    else
      delSelection();
    requestRedrawCanvas();
  }
  
  void GodleyTableEditor::copy()
  {
    if (!selectedCellInTable()) return;
    auto& str=m_godleyIcon.table.cell(selectedRow,selectedCol);
    if (selectIdx!=insertIdx)
      cminsky().clipboard.putClipboard
        (str.substr(min(selectIdx,insertIdx), abs(int(selectIdx)-int(insertIdx))));
    else
      cminsky().clipboard.putClipboard(str);  
  }

  void GodleyTableEditor::paste()
  {
    if (!selectedCellInTable()) return;
    delSelection();
    auto& str=m_godleyIcon.table.cell(selectedRow,selectedCol); 
    auto stringToInsert=cminsky().clipboard.getClipboard();
    // only insert first line
    auto p=stringToInsert.find('\n');
    if (p!=string::npos)
      stringToInsert=stringToInsert.substr(0,p-1);
    str.insert(insertIdx,stringToInsert);
    selectIdx=insertIdx+=stringToInsert.length();
    requestRedrawCanvas();
  }

  GodleyTableEditor::ClickType GodleyTableEditor::clickType(double x, double y) const
  {
    const int c=colX(x), r=rowY(y);

    if (x<leftTableOffset && r>0)
      return rowWidget;
    if (y<topTableOffset && y>columnButtonsOffset && c>0)
      return colWidget;

    if (r==0)
      {
        if (colLeftMargin[c+1]-x < pulldownHot)
          return importStock;
        return row0;
      }
    if (c==0)
      return col0;

    if (c>0 && c<int(m_godleyIcon.table.cols()))
      if (r>0 && r<int(m_godleyIcon.table.rows()))
        return internal;

    return background;
  }

  std::set<string> GodleyTableEditor::matchingTableColumns(double x)
  {
    const int col=colXZoomed(x);
    return matchingTableColumnsByCol(col);
  }

  std::set<string> GodleyTableEditor::matchingTableColumnsByCol(int col)
  {
    if (col<0||col>=static_cast<int>(godleyIcon().table.cols())) return {};
    return minsky().matchingTableColumns(godleyIcon(), godleyIcon().table.assetClass(col));
  }

  void GodleyTableEditor::addStockVar(double x)
  {
    const int c=colXZoomed(x);
    addStockVarByCol(c);
  }

  void GodleyTableEditor::addStockVarByCol(int c)
  {
    if (c>0) m_godleyIcon.table.insertCol(c+1);
  }

  void GodleyTableEditor::importStockVar(const string& name, double x)
  {
    const int c=colXZoomed(x);
    importStockVarByCol(name, c);
  }

  void GodleyTableEditor::importStockVarByCol(const string& name, int c)
  {
    if (c>0 && size_t(c)<m_godleyIcon.table.cols())
      {
        m_godleyIcon.table.cell(0,c)=name;
        minsky().importDuplicateColumn(m_godleyIcon.table, c);
        adjustWidgets();
        update(); //TODO I don't know why this is insufficient to update icon on canvas
      }
  }

  void GodleyTableEditor::deleteStockVar(double x)
  {
    const int c=colXZoomed(x);
    deleteStockVarByCol(c);
  }

  void GodleyTableEditor::deleteStockVarByCol(int c)
  {
    if (c>=0)
      m_godleyIcon.table.deleteCol(c+1);
  }

  void GodleyTableEditor::addFlow(double y)  
  {
    const int r=rowYZoomed(y);
    addFlowByRow(r);
  }

  void GodleyTableEditor::addFlowByRow(int r)  
  {
    if (r>0)                                 
      m_godleyIcon.table.insertRow(r+1);
  }

  void GodleyTableEditor::deleteFlow(double y)
  {
    const int r=rowYZoomed(y);
    deleteFlowByRow(r);
  }

  void GodleyTableEditor::deleteFlowByRow(int r)
  {
    if (r>1)                                       // Cannot delete flow in Initial Conditions row. For ticket 1064
      m_godleyIcon.deleteRow(r+1);
  }
  
namespace {
  string constructMessage(GodleyAssetClass::AssetClass& targetAC, GodleyAssetClass::AssetClass& oldAC, string& var)
  {
    string tmpStr="This will convert "+var+" from "+classdesc::enumKey<GodleyAssetClass::AssetClass>(oldAC)+" to "+classdesc::enumKey<GodleyAssetClass::AssetClass>(targetAC)+". Are you sure?";
    return tmpStr;
  }	  
}
  
  string GodleyTableEditor::moveAssetClass(double x, double y)
  {
    x/=zoomFactor;
    y/=zoomFactor;
    const unsigned c=colX(x);
    string tmpStr;
    if (c>=m_godleyIcon.table.cols()) return tmpStr;
    if (clickType(x,y)==colWidget) {
      const unsigned visibleCol=c-scrollColStart+1;
      if (c<colWidgets.size() && visibleCol < colLeftMargin.size())
        {
          auto moveVar=m_godleyIcon.table.cell(0,c);		
          auto oldAssetClass=m_godleyIcon.table.assetClass(c);
          auto targetAssetClassPlus=m_godleyIcon.table.assetClass(c+1);
          auto targetAssetClassMinus=m_godleyIcon.table.assetClass(c-1);
          if (colWidgets[c].button(x-colLeftMargin[visibleCol])==3 && oldAssetClass!=GodleyAssetClass::equity) {
            if (targetAssetClassPlus!=oldAssetClass && !moveVar.empty() && targetAssetClassPlus!=GodleyAssetClass::equity && targetAssetClassPlus!=GodleyAssetClass::noAssetClass)
              tmpStr=constructMessage(targetAssetClassPlus,oldAssetClass,moveVar);
            else if (targetAssetClassPlus==GodleyAssetClass::noAssetClass && !moveVar.empty())
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
  
  string GodleyTableEditor::swapAssetClass(double x, double) 
  {  
    x/=zoomFactor;
    const int c=colX(x);	
    string tmpStr;	  
    if (selectedRow==0 && size_t(selectedCol)<m_godleyIcon.table.cols())
      {
        // clickType triggers pango error which causes this condition to be skipped and thus column gets moved to Equity, which should not be the case   	
        if (c>0 && selectedCol>0 && c!=selectedCol) {
          auto swapVar=m_godleyIcon.table.cell(0,selectedCol);
          auto oldAssetClass=m_godleyIcon.table.assetClass(selectedCol);
          auto targetAssetClass=m_godleyIcon.table.assetClass(c);
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

  void GodleyTableEditor::highlightColumn(cairo_t* cairo, unsigned col)
  {
    if (col<scrollColStart) return;
    const double x=colLeftMargin[col-scrollColStart+1];
    const double width=colLeftMargin[col-scrollColStart+2]-x;
    const double tableHeight=(m_godleyIcon.table.rows()-scrollRowStart+1)*rowHeight;
    cairo_rectangle(cairo,x,topTableOffset,width,tableHeight);
    cairo_set_source_rgba(cairo,1,1,1,0.5);
    cairo_fill(cairo);
  }

  void GodleyTableEditor::highlightRow(cairo_t* cairo, unsigned row)
  {
    if (row<scrollRowStart) return;
    const double y=(row-scrollRowStart+1)*rowHeight+topTableOffset;
    cairo_rectangle(cairo,leftTableOffset,y,colLeftMargin.back()-leftTableOffset,rowHeight);
    cairo_set_source_rgba(cairo,1,1,1,0.5);
    cairo_fill(cairo);
  }

  void GodleyTableEditor::highlightCell(cairo_t* cairo, unsigned row, unsigned col)
  {
    if (row<scrollRowStart || col<scrollColStart) return;
    const double x=colLeftMargin[col-scrollColStart+1];
    const double width=colLeftMargin[col-scrollColStart+2]-x;
    const double y=(row-scrollRowStart+1)*rowHeight+topTableOffset;
    cairo_rectangle(cairo,x,y,width,rowHeight);
    cairo_set_source_rgba(cairo,1,1,1,0.5);
    cairo_fill(cairo);
  }

  void GodleyTableEditor::pushHistory()
  {
    while (history.size()>maxHistory) history.pop_front();
    // Perform deep comparison of Godley tables in history to avoid spurious noAssetClass columns from arising during undo. For ticket 1118.
    if (history.empty() || !(history.back()==m_godleyIcon.table)) {
      history.push_back(m_godleyIcon.table);
    }
    historyPtr=history.size();
  }
      
  void GodleyTableEditor::undo(int changes)
  { 
    if (historyPtr==history.size())
      pushHistory();
    historyPtr-=changes;
    if (historyPtr > 0 && historyPtr <= history.size())
      {
        auto& d=history[historyPtr-1];
        // Perform deep comparison of Godley tables in history to avoid spurious noAssetClass columns from arising during undo. For ticket 1118.
        if (d.getData().empty()) return; // should not happen
		m_godleyIcon.table=d; 
      }
  }

  void GodleyTableEditor::adjustWidgets()
  {
    rowWidgets.clear();
    for (size_t i=0; i<m_godleyIcon.table.rows(); ++i)
      rowWidgets.emplace_back(m_godleyIcon, i);
    colWidgets.clear();
    for (size_t i=0; i<m_godleyIcon.table.cols(); ++i)
      colWidgets.emplace_back(m_godleyIcon, i);
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

  void GodleyTableEditor::update()
  {
    if (selectedCol>0 && selectedCol<int(m_godleyIcon.table.cols()))
      {
        if (selectedRow==0)
          {
            // rename all instances of the stock variable if updated. For ticket #956
            // find stock variable if it exists
            for (const auto& sv: m_godleyIcon.stockVars())
              if (sv->valueId()==m_godleyIcon.valueId(m_godleyIcon.table.savedText))
                {
                  auto savedItem=minsky().canvas.item;
                  minsky().canvas.item=sv;
                  auto newName=utf_to_utf<char>(m_godleyIcon.table.cell(selectedRow,selectedCol));
                  if (!newName.empty())
                    minsky().canvas.renameAllInstances(newName);
                  savedItem.swap(minsky().canvas.item);
                }
            minsky().importDuplicateColumn(m_godleyIcon.table, selectedCol);
          }
        else
          {
            if (m_godleyIcon.table.initialConditionRow(selectedRow))
              {
                // if the contents of the cell are cleared, set the cell to "0". For #1181
                if (!m_godleyIcon.table.savedText.empty() && m_godleyIcon.table.cell(selectedRow,selectedCol).empty())
                  m_godleyIcon.table.cell(selectedRow,selectedCol)="0";
              }
            minsky().balanceDuplicateColumns(m_godleyIcon,selectedCol);
          }
        // get list of GodleyIcons first, rather than doing recursiveDo, as update munges the items vectors
        auto godleyTables=minsky().model->findItems
          ([](const ItemPtr& i){return dynamic_cast<GodleyIcon*>(i.get());});
        for (auto& i: godleyTables)
          if (auto* g=dynamic_cast<GodleyIcon*>(i.get()))
            g->update();
      }
    minsky().canvas.requestRedraw();
    minsky().redrawAllGodleyTables();
  }

  void GodleyTableEditor::checkCell00()
  {
    if (selectedCol==0 && (selectedRow==0 || selectedRow ==1))
      // (0,0) cell not editable
      {
        selectedCol=-1;
        selectedRow=-1;
      }         
  }
  
    void GodleyTableEditor::navigateRight()
    {
      if (selectedCol>=0)
        {
          update();
          selectedCol++;
          insertIdx=0;
          if (selectedCol>=int(m_godleyIcon.table.cols()))
            {
              if (selectedRow>0) selectedCol=0;   // Minor fix: Make sure tabbing and right arrow traverse all editable cells.
              else selectedCol=1;
              navigateDown();
            }
          checkCell00();
        }
    }
  
    void GodleyTableEditor::navigateLeft()
    {
      if (selectedCol>=0)
        {
          update();
          selectedCol--;
          if (selectedCol<0)
            {
              selectedCol=m_godleyIcon.table.cols()-1;
              navigateUp();
            }
          checkCell00();
          insertIdx=godleyIcon().table.cellInTable(selectedRow, selectedCol)?
            godleyIcon().table.cell(selectedRow, selectedCol).length(): 0;
        }
    }

    void GodleyTableEditor::navigateUp()
    {
      update();
      if (selectedRow>=0)
        selectedRow=(selectedRow-1)%m_godleyIcon.table.rows();
      checkCell00();
    }
  
    void GodleyTableEditor::navigateDown()
    {
      update();
      if (selectedRow>=0)
        selectedRow=(selectedRow+1)%m_godleyIcon.table.rows();
      checkCell00();
    }

  template <ButtonWidgetEnums::RowCol rowCol>
  void ButtonWidget<rowCol>::drawButton(cairo_t* cairo, const std::string& label, double r, double g, double b, int idx)
  {
    // stash current point for drawing a box
    double x0, y0;
    cairo_get_current_point(cairo,&x0, &y0);
    
    const CairoSave cs(cairo);
    Pango pango(cairo);
    // increase text size a bit for the buttons
    pango.setFontSize(0.8*buttonSpacing);
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
    const CairoSave cs(cairo);
    int idx=0;
    if (rowCol==row || (!cminsky().multipleEquities() && godleyIcon.table.singleEquity())) {  // no column widgets on equity column in single equity column mode
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
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::GodleyTableWindow);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::GodleyTableEditor);
