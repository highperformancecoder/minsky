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
#include <pango.h>
#include <ecolab_epilogue.h>

using namespace std;
using namespace minsky;
using ecolab::Pango;

constexpr double GodleyTableWindow::leftTableOffset, GodleyTableWindow::topTableOffset;

void GodleyTableWindow::redraw(int, int, int width, int height)
{
  if (!godleyIcon) return;
  Pango pango(surface->cairo());
  pango.setMarkup(godleyIcon->table.cell(0,0));
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
      colLeftMargin.push_back(x);
      double y=topTableOffset;
      double colWidth=0;
      for (unsigned row=0; row<godleyIcon->table.rows(); ++row)
        {
          if (row>0 && row<scrollRowStart) continue;
          if (row==0 && col==0)
            pango.setMarkup("Flows ↓ / Stock Vars →");
          else if (int(row)==selectedRow && int(col)==selectedCol)
            // the active cell renders as bare LaTeX code for editing
            pango.setMarkup(godleyIcon->table.cell(row,col));
          else
            pango.setMarkup(latexToPango(godleyIcon->table.cell(row,col)));
          colWidth=max(colWidth,pango.width());
          cairo_move_to(surface->cairo(),x+3,y);
          pango.show();
          y+=rowHeight;
        }
      y=topTableOffset;
      colWidth+=5;
      // vertical lines & asset type tag
      if (assetClass!=godleyIcon->table._assetClass(col))
        {
          if (assetClass!=GodleyAssetClass::noAssetClass)
            {
              pango.setMarkup(enumKey<GodleyAssetClass::AssetClass>(assetClass));
              // increase column by enough to fit asset class label
              if (x < pango.width()+lastAssetBoundary+3)
                x=pango.width()+lastAssetBoundary+3;
              cairo_move_to(surface->cairo(),0.5*(x+lastAssetBoundary-pango.width()),0);
              pango.show();
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

      x+=colWidth;
    }
  
  pango.setMarkup(enumKey<GodleyAssetClass::AssetClass>(assetClass));
  // increase column by enough to fit asset class label
  if (x < pango.width()+lastAssetBoundary+3)
    x=pango.width()+lastAssetBoundary+3;
  cairo_move_to(surface->cairo(),0.5*(x+lastAssetBoundary-pango.width()),0);
  pango.show();
  
  // final column vertical line
  colLeftMargin.push_back(x);
  cairo_move_to(surface->cairo(),x,topTableOffset);
  cairo_rel_line_to(surface->cairo(),0,tableHeight);
  cairo_move_to(surface->cairo(),x+3,topTableOffset);
  cairo_rel_line_to(surface->cairo(),0,tableHeight);
  cairo_set_line_width(surface->cairo(),0.5);
  cairo_stroke(surface->cairo());

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
  cairo_save(surface->cairo());
  if (selectedRow==0 || (selectedRow>=scrollRowStart && selectedRow<godleyIcon->table.rows()))
    {
      size_t i=0, j=0;
      if (selectedRow>=scrollRowStart) j=selectedRow-scrollRowStart+1;
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
               (selectedCol>=scrollColStart && selectedCol<godleyIcon->table.cols()))
        {
          if (selectedRow!=0 || selectedCol!=0) // can't select flows/stockVars label
            {
              if (selectedCol>=scrollColStart) i=selectedCol-scrollColStart+1;
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
            }
        }
    }
  cairo_restore(surface->cairo());
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
  return (y-topTableOffset)/rowHeight;
}


void GodleyTableWindow::mouseDown(double x, double y)
{
  selectedCol=colX(x);
  selectedRow=rowY(y);
  if (selectedRow>=0 && selectedRow<godleyIcon->table.rows() &&
      selectedCol>=0 && selectedCol<godleyIcon->table.cols())
    {
      cairo::Surface surf(cairo_recording_surface_create(CAIRO_CONTENT_COLOR,NULL));
      Pango pango(surf.cairo());
      auto& str=godleyIcon->table.cell(selectedRow,selectedCol);
      pango.setMarkup(str);
      int j=0;
      if (selectedCol>=scrollColStart) j=selectedCol-scrollColStart+1;
      x-=colLeftMargin[j]+2;
      insertIdx = x>0 && str.length()>0?pango.posToIdx(x)+1: 0;
    }
  else
    insertIdx=0;
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
  else
    if ((c!=selectedCol || r!=selectedRow) && c>0 && r>0)
      {
        swap(godleyIcon->table.cell(selectedRow,selectedCol), godleyIcon->table.cell(r,c));
        selectedCol=c;
        selectedRow=r;
        requestRedraw();
      }
}

void GodleyTableWindow::mouseMove(double x, double y)
{
  motionCol=colX(x), motionRow=rowY(y);
}

void GodleyTableWindow::keyPress(int keySym)
{
  if (selectedCol>=0 && selectedRow>=0 && selectedCol<godleyIcon->table.cols() &&
      selectedRow<godleyIcon->table.rows())
    {
      auto& str=godleyIcon->table.cell(selectedRow,selectedCol);
      switch (keySym)
        {
        case 0xff08: case 0xffff:  //backspace/delete
          if (insertIdx>0 && insertIdx<=str.length())
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
        default:
          if (keySym>=' ' && keySym<0xff)
            {
              if (insertIdx<0) insertIdx=0;
              if (insertIdx>=str.length()) insertIdx=str.length();
              str.insert(str.begin()+insertIdx++,keySym);
            }
          break;
        }
      requestRedraw();
    }
}

GodleyTableWindow::ClickType GodleyTableWindow::clickType(double x, double y) const
{
  int c=colX(x), r=rowY(y);
  if (r==0)
    return row0;
  if (c==0)
    return col0;

  if (c>0 && c<godleyIcon->table.cols())
    if (r>0 && r<godleyIcon->table.rows())
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

