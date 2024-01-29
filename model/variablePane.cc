/*
  @copyright Steve Keen 2022
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

#include "minsky.h"
#include "variablePane.h"
#include "cairoItems.h"
#include "pannableTab.rcd"
#include "pannableTab.xcd"
#include "variableValue.h"
#include "variablePane.rcd"
#include "variablePane.xcd"
#include "variableType.rcd"
#include "minsky_epilogue.h"
#include <stdexcept>
using namespace std;

namespace minsky
{
  VariablePaneCell::VariablePaneCell(const VariableValue& var):
    var(var.type(), var.name)
  {
    ecolab::cairo::Surface surf
      (cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,NULL));
    cairo_move_to(surf.cairo(),0,0);
    RenderVariable rv(*this->var, surf.cairo());
    rv.draw();
    double l,t;
    cairo_recording_surface_ink_extents(surf.surface(),&l,&t,&m_width,&m_height);
  }

  void VariablePaneCell::show()
  {
    if (!cachedCairo || !var || var->type()==VariableType::undefined) return;
    var->sliderBoundsSet=0; //TODO, this should be the case by default anyway
    RenderVariable rv(*var, cachedCairo);
    ecolab::cairo::CairoSave cs(cachedCairo);
    cairo_translate(cachedCairo,0.5*m_width,0.5*m_height);
    rv.draw();
    cairo_reset_clip(cachedCairo);
  }

  void VariablePaneCell::emplace() const
  {
    if (var && var->type()!=VariableType::undefined)
      minsky().canvas.addVariable(var->rawName(), var->type());
  }

  const VariableBase& VariablePaneCell::variable() const
  {
    if (var) return *var;
    static Variable<VariableType::undefined> undefined;
    return undefined;
  }
  
  VariablePaneCell& VariablePane::cell(unsigned row, unsigned col)
  {
    auto idx=row+m_numRows*col;
    if (idx>=vars.size())
      {
        static VariablePaneCell emptyCell;
        return emptyCell;
      }
    return vars[idx];
  }

  void VariablePane::updateWithHeight(unsigned height)
  {
    const unsigned typicalHeight=38;
    m_numRows=height/typicalHeight;
    update();
  }

  
  void VariablePane::update()
  {
    if (!m_numRows) return;
    vars.clear();
    for (auto& v: cminsky().variableValues)
      {
        if (v.first.empty() || selection.count(v.second->type())==0) continue; // ignore those filtered out
        vars.emplace_back(*v.second);
      }

    m_numCols=vars.size()/m_numRows+1;
    unsigned gridSize=m_numRows*m_numCols;
    while (vars.size()<gridSize) vars.emplace_back();
    if (surface.get()) surface->requestRedraw(); // TODO, plain requestRedraw doesn't work for Tk here...
  }

  bool VariablePane::redraw(int, int, int, int)
  {
    if (surface.get()) {
      cairo_t* cairo=surface->cairo();
      for (auto& i: vars) i.reset(cairo);
      cairo_identity_matrix(cairo); 
      Grid<VariablePaneCell>::draw(); 
    }
    return surface.get();
  }    

  void VariablePane::moveCursorTo(double x, double y)
  {
    if (surface.get())
      {
        cairo_identity_matrix(surface->cairo());
        cairo_translate(surface->cairo(), x+offsx, y+offsy);
      }
  }
}
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::VariablePaneBase);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::VariablePane);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::VariablePaneCell);
