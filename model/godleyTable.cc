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

#include "godleyTable.h"
#include "selection.h"
#include "latexMarkup.h"
#include "minsky.h"
#include <pango.h>
#include <ecolab_epilogue.h>

using namespace std;
using namespace minsky;
using ecolab::Pango;
using namespace ecolab::cairo;

constexpr double GodleyTableWindow::leftTableOffset, GodleyTableWindow::topTableOffset, GodleyTableWindow::pulldownHot;

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

}

GodleyTableWindow::GodleyTableWindow
(const std::shared_ptr<GodleyIcon>& godleyIcon): godleyIcon(godleyIcon)
{
  assert(godleyIcon);
  for (size_t i=0; i<godleyIcon->table.rows(); ++i)
    rowWidgets.emplace_back(*godleyIcon, ButtonWidget::row, i);
  for (size_t i=0; i<godleyIcon->table.cols(); ++i)
    colWidgets.emplace_back(*godleyIcon, ButtonWidget::col, i);
  // nb first column/row is actually 1 - 0th element actually
  // just ignored
  rowWidgets[1].pos=ButtonWidget::first;
  rowWidgets.back().pos=ButtonWidget::last;
  colWidgets[1].pos=ButtonWidget::first;
  colWidgets.back().pos=ButtonWidget::last;
}


void GodleyTableWindow::redraw(int, int, int width, int height)
{
  if (!godleyIcon) return;
  Pango pango(surface->cairo());
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
          cairo_move_to(surface->cairo(), x, 12);
          colWidgets[col].draw(surface->cairo());
        }
      
      if (col>1)
        {
          cairo_move_to(surface->cairo(),x-pulldownHot,topTableOffset);
          pango.setMarkup("▼");
          pango.show();
        }
      
      double y=topTableOffset;
      double colWidth=0;
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
                  // the active cell renders as bare LaTeX code for
                  // editing, all other cells rendered as LaTeX
                  if (int(row)!=selectedRow || int(col)!=selectedCol)
                    {
                      if (row>0 && col>0 && !godleyIcon->table.initialConditionRow(row))
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
                        }
                      else // is flow tag, stock var or initial condition
                        text = latexToPango(text);
                    }
                  text+=value;
                  pango.setMarkup(text+value);
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
  pango.setMarkup("▼");
  pango.show();

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
      pango.setMarkup(godleyIcon->table.rowSum(row));
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
        else if (motionRow>=0 && selectedCol==0 && selectedRow>0) // whole col being moved
          {
            highlightRow(surface->cairo(),selectedRow);
            highlightRow(surface->cairo(),motionRow);
          }
        else if (selectedCol==0 || /* selecting individual cell */
                 (selectedCol>=int(scrollColStart) && selectedCol<int(godleyIcon->table.cols())))
          {
            if (selectedRow!=0 || selectedCol!=0) // can't select flows/stockVars label
              {
                if (selectedCol>=int(scrollColStart)) i=selectedCol-scrollColStart+1;
                double x=colLeftMargin[i];
                cairo_set_source_rgba(surface->cairo(),1,1,1,1);
                cairo_rectangle(surface->cairo(),x,y,colLeftMargin[i+1]-x,rowHeight);
                cairo_fill(surface->cairo());
                pango.setMarkup(godleyIcon->table.cell(selectedRow,selectedCol));
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
  auto p=std::upper_bound(colLeftMargin.begin(), colLeftMargin.end(), x);
  int r=p-colLeftMargin.begin()-1;
  if (r>0) r+=scrollColStart-1;
  return r;
}

int GodleyTableWindow::rowY(double y) const
{
  int c=(y-topTableOffset)/rowHeight;
  if (c>0) c+=scrollRowStart-1;
  return c;
}

int GodleyTableWindow::textIdx(double x) const
{
  cairo::Surface surf(cairo_recording_surface_create(CAIRO_CONTENT_COLOR,NULL));
  Pango pango(surf.cairo());
  auto& str=godleyIcon->table.cell(selectedRow,selectedCol);
  pango.setMarkup(str);
  int j=0;
  if (selectedCol>=int(scrollColStart)) j=selectedCol-scrollColStart+1;
  x-=colLeftMargin[j]+2;
  return x>0 && str.length()>0?pango.posToIdx(x)+1: 0;

}

void GodleyTableWindow::mouseDown(double x, double y)
{
  selectedCol=colX(x);
  selectedRow=rowY(y);
  if (selectedRow>=0 && selectedRow<int(godleyIcon->table.rows()) &&
      selectedCol>=0 && selectedCol<int(godleyIcon->table.cols()))
    selectIdx=insertIdx = textIdx(x);
  else
    selectIdx=insertIdx=0;
  requestRedraw();
}

void GodleyTableWindow::mouseUp(double x, double y)
{
  int c=colX(x), r=rowY(y);
  motionRow=motionCol=-1;
  if (selectedRow==0)
    {
      if (c!=selectedCol)
        godleyIcon->table.moveCol(selectedCol,c-selectedCol);
    }
  else if (selectedCol==0)
    {
      if (r!=selectedRow)
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

void GodleyTableWindow::mouseMove(double x, double y)
{
  motionCol=colX(x), motionRow=rowY(y);
  if (motionCol==selectedCol && motionRow==selectedRow)
    selectIdx=textIdx(x);
}

void GodleyTableWindow::keyPress(int keySym)
{
  if (selectedCol>=0 && selectedRow>=0 && selectedCol<int(godleyIcon->table.cols()) &&
      selectedRow<int(godleyIcon->table.rows()))
    {
      if (controlChar)
        switch (keySym)
          {
          case 'x': case 'X':
            cut();
            break;
          case 'c': case 'C':
            copy();
            break;
          case 'v': case 'V':
            paste();
            break;
          }
      else
        {
          auto& str=godleyIcon->table.cell(selectedRow,selectedCol);
          switch (keySym)
            {
            case 0xff08: case 0xffff:  //backspace/delete
              if (insertIdx!=selectIdx)
                delSelection();
              else if (insertIdx>0 && insertIdx<=str.length())
                str.erase(--insertIdx,1);
              break;
            case 0xff1b: // escape - TODO
              selectedRow=selectedCol=-1;
              break;
            case 0xff0d:
              godleyIcon->update();
              selectedRow=selectedCol=-1;
              break;
            case 0xff51: //left arrow
              if (insertIdx>0) insertIdx--;
              break;
            case 0xff53: //right arrow
              if (insertIdx<str.length()) insertIdx++;
              break;
            case 0xffe3: case 0xffe4:
              controlChar=true;
              return; // no need to redraw + don't reset selection
            default:
              if (keySym>=' ' && keySym<0xff)
                {
                  delSelection();
                  if (insertIdx>=str.length()) insertIdx=str.length();
                  str.insert(str.begin()+insertIdx++,keySym);
                }
              break;
            }
          selectIdx=insertIdx;
        }
      requestRedraw();
    }
}

void GodleyTableWindow::keyRelease(int keySym)
{
  switch (keySym)
    {
    case 0xffe3: case 0xffe4:
      controlChar=false;
      break;
    }
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

void GodleyTableWindow::copy()
{
  auto& str=godleyIcon->table.cell(selectedRow,selectedCol);
  cminsky().putClipboard
    (str.substr(min(selectIdx,insertIdx), abs(int(selectIdx)-int(insertIdx))));
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
  if (r==0)
    {
      if (colLeftMargin[c+1]-x < pulldownHot)
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
  int c=colX(x);
  if (c>0)
    godleyIcon->table.deleteCol(c);
  requestRedraw();
}

void GodleyTableWindow::addFlow(double y)
{
  int r=rowY(y);
  if (r>0)
    godleyIcon->table.insertRow(r+1);
  requestRedraw();
}

void GodleyTableWindow::deleteFlow(double y)
{
  int r=rowY(y);
  if (r>0)
    godleyIcon->deleteRow(r+1);
   requestRedraw();
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
  if (history.empty() || history.back()!=godleyIcon->table.getData())
    history.push_back(godleyIcon->table.getData());
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
      if (d.empty()) return; // should not happen
      godleyIcon->table.resize(d.size(), d[0].size());
      for (size_t r=0; r<godleyIcon->table.rows(); ++r)
        for (size_t c=0; c<godleyIcon->table.cols(); ++c)
          godleyIcon->table.cell(r,c)=d[r][c];
      //      godleyIcon->update();
      requestRedraw();
    }
}

void ButtonWidget::draw(cairo_t* cairo)
{
  CairoSave cs(cairo);
  Pango pango(cairo);
  pango.setMarkup("+");
  cairo_set_source_rgb(cairo,0,1,0);
  pango.show();
  cairo_rel_move_to(cairo,buttonSpacing,0);
  pango.setMarkup("–");
  cairo_set_source_rgb(cairo,1,0,0);
  pango.show();
  cairo_rel_move_to(cairo,buttonSpacing,0);
  if (pos!=first)
    {
      pango.setMarkup(rowCol==row? "↑": "←");
      cairo_set_source_rgb(cairo,0,0,0);
      pango.show();
      cairo_rel_move_to(cairo,buttonSpacing,0);
    }
  if (pos!=last)
    {
      pango.setMarkup(rowCol==row? "↓": "→");
      cairo_set_source_rgb(cairo,0,0,0);
      pango.show();
      cairo_rel_move_to(cairo,buttonSpacing,0);
    }
}

void ButtonWidget::invoke(double x) {}

