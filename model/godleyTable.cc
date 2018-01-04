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
  double x=leftTableOffset;
  for (unsigned col=0; col<godleyIcon->table.cols(); ++col)
    {
      double colWidth=0;
      double y=topTableOffset;
      for (unsigned row=0; row<godleyIcon->table.rows(); ++row, y+=rowHeight)
        {
          if (row==0 && col==0)
            pango.setMarkup("Flows ↓ / Stock Vars →");
          else
            pango.setMarkup(latexToPango(godleyIcon->table.cell(row,col)));
          colWidth=max(colWidth,pango.width());
          cairo_move_to(surface->cairo(),x+3,y);
          pango.show();
        }
      y=topTableOffset;
      colWidth+=5;
      for (unsigned row=0; row<=godleyIcon->table.rows(); ++row, y+=rowHeight)
        {
          // horizontal lines
          cairo_move_to(surface->cairo(),x,y);
          cairo_rel_line_to(surface->cairo(),colWidth,0);
          cairo_set_line_width(surface->cairo(),0.5);
          cairo_stroke(surface->cairo());
        }
      // vertical lines
      cairo_move_to(surface->cairo(),x,topTableOffset);
      cairo_rel_line_to(surface->cairo(),0,godleyIcon->table.rows()*rowHeight);
      if (col==1)
        {
          cairo_move_to(surface->cairo(),x+3,topTableOffset);
          cairo_rel_line_to(surface->cairo(),0,godleyIcon->table.rows()*rowHeight);
        }
      cairo_set_line_width(surface->cairo(),0.5);
      cairo_stroke(surface->cairo());

      x+=colWidth;
    }
  // final vertical line
  cairo_move_to(surface->cairo(),x,topTableOffset);
  cairo_rel_line_to(surface->cairo(),0,godleyIcon->table.rows()*rowHeight);
  cairo_set_line_width(surface->cairo(),0.5);
  cairo_stroke(surface->cairo());
}

