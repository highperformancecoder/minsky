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
#include "mansouraCairo.h"
#include "pannableTab.rcd"
#include "pannableTab.xcd"
#include "variableValue.h"
#include "variablePane.rcd"
#include "variablePane.xcd"
#include "variableType.rcd"
#include "minsky_epilogue.h"
#include <stdexcept>
using namespace std;
using namespace mansoura;

namespace minsky
{
  VariablePaneCell::VariablePaneCell(const VariableValue& var):
    var(var.type(), ':'+uqName(var.name))
  {
    const auto ink=mansoura::inkExtents
      ([&](const IMansoura& shim)
       {
         shim.moveTo(0,0);
         RenderVariable rv(*this->var, shim);
         rv.draw();
       });
    m_width=ink.width;
    m_height=ink.height;
  }

  void VariablePaneCell::show(const IMansoura& shim)
  {
    if (!var || var->type()==VariableType::undefined) return;
    RenderVariable rv(*var, shim);
    {
      const PreserveContext cs(shim);
      shim.translate(0.5*m_width,0.5*m_height);
      rv.draw();
    }
    shim.resetClip();
  }

  void VariablePaneCell::emplace() const
  {
    if (var && var->type()!=VariableType::undefined)
      minsky().canvas.addVariable(var->rawName(), var->type());
  }

  const VariableBase& VariablePaneCell::variable() const
  {
    if (var) return *var;
    static const Variable<VariableType::undefined> undefined;
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
        if (v.first.empty() || selection.contains(v.second->type())==0) continue; // ignore those filtered out
        vars.emplace_back(*v.second);
      }

    m_numCols=vars.size()/m_numRows+1;
    const unsigned gridSize=m_numRows*m_numCols;
    while (vars.size()<gridSize) vars.emplace_back();
    RenderNativeWindow::requestRedraw();
  }

  bool VariablePane::redraw(const IMansoura& shim, int, int, int, int)
  {
    shim.identityMatrix(); 
    Grid<VariablePaneCell>::draw(shim); 
    return true;
  }    

  void VariablePane::moveCursorTo(const IMansoura& shim, double x, double y)
  {
    shim.identityMatrix();
    shim.translate(x+offsx, y+offsy);
  }
}
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::VariablePaneBase);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::VariablePane);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::VariablePaneCell);
