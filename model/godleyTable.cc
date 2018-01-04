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

void GodleyTableWindow::redraw(int, int, int width, int height)
{
  if (!godleyIcon) return;
  double leftTableOffset=50, topTableOffset=30;
  Pango pango(surface->cairo());
  pango.setMarkup(godleyIcon->table.cell(0,0));
  double rowHeight=pango.height()+2;
  double tableHeight=(godleyIcon->table.rows()-scrollRowStart+1)*rowHeight;
  double x=leftTableOffset;
  double lastAssetBoundary=x;
  auto assetClass=GodleyAssetClass::noAssetClass;
  
  for (unsigned col=0; col<godleyIcon->table.cols(); ++col)
    {
      // omit stock columns less than scrollColStart
      if (col>0 && col<scrollColStart) continue;
      double y=topTableOffset;
      double colWidth=0;
      for (unsigned row=0; row<godleyIcon->table.rows(); ++row)
        {
          if (row>0 && row<scrollRowStart) continue;
          if (row==0 && col==0)
            pango.setMarkup("Flows ↓ / Stock Vars →");
          else
            pango.setMarkup(latexToPango(godleyIcon->table.cell(row,col)));
          colWidth=max(colWidth,pango.width());
          cairo_move_to(surface->cairo(),x+3,y);
          pango.show();
          y+=rowHeight;
        }
      y=topTableOffset;
      colWidth+=5;
      // vertical lines
      cairo_move_to(surface->cairo(),x,topTableOffset);
      cairo_rel_line_to(surface->cairo(),0,tableHeight);
      if (assetClass!=godleyIcon->table._assetClass(col))
        {
          if (assetClass!=GodleyAssetClass::noAssetClass)
            {
              pango.setMarkup(enumKey<GodleyAssetClass::AssetClass>(assetClass));
              cairo_move_to(surface->cairo(),0.5*(x+lastAssetBoundary-pango.width()),0);
              pango.show();
            }
          lastAssetBoundary=x;
          
          assetClass=godleyIcon->table._assetClass(col);
          cairo_move_to(surface->cairo(),x+3,topTableOffset);
          cairo_rel_line_to(surface->cairo(),0,tableHeight);
        }
      cairo_set_line_width(surface->cairo(),0.5);
      cairo_stroke(surface->cairo());

      x+=colWidth;
    }
  
  pango.setMarkup(enumKey<GodleyAssetClass::AssetClass>(assetClass));
  cairo_move_to(surface->cairo(),0.5*(x+lastAssetBoundary-pango.width()),0);
  pango.show();
  
  // final column vertical line
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
  pango.setMarkup("Row Sum");
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
  cairo_move_to(surface->cairo(),x,topTableOffset);
  cairo_rel_line_to(surface->cairo(),0,tableHeight);
  cairo_set_line_width(surface->cairo(),0.5);
  cairo_stroke(surface->cairo());

}

