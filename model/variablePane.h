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

#ifndef VARIABLEPANE_H
#define VARIABLEPANE_H
#include "cairo.h"
#include "classdesc_access.h"
#include "grid.h"
#include "variable.h"
#include "renderNativeWindow.h"
#include <vector>

namespace minsky
{
  class VariablePaneCell
  {
    VariablePtr var;
    double m_width=0, m_height=0;
    cairo_t* cachedCairo=nullptr;
    CLASSDESC_ACCESS(VariablePaneCell);
  public:
    VariablePaneCell() {}
    VariablePaneCell(const VariableValue& var);
    void reset(cairo_t* cairo) {cachedCairo=cairo;}
    void show();
    double width() const {return m_width+2;}
    double height() const {return m_height+4;}
    void emplace() const;
  };
  
  class VariablePane: public Grid<VariablePaneCell>, public RenderNativeWindow, public VariableType
  {
    unsigned m_numRows=0, m_numCols=0;
    classdesc::Exclude<std::vector<VariablePaneCell>> vars;
    CLASSDESC_ACCESS(VariablePane);
    bool redraw(int, int, int width, int height) override;
  public:
    double offsx=0, offsy=0;
    float moveOffsX, moveOffsY;
    bool shift=false; ///< true if shift pressed
    bool mousePressed=false; ///< true if mouse button pressed
    std::set<Type> selection;
    VariablePane() {selection={parameter, flow, integral, stock};}
    void select(VariableType::Type x) {selection.insert(x);}
    void deselect(VariableType::Type x) {selection.erase(x);}
    VariablePaneCell& cell(unsigned row, unsigned col) override;
    /// update variables from model, given a window of \a height pixels
    void updateWithHeight(unsigned height);
    /// update variables from model, using previous height value
    void update();
    unsigned numRows() const override {return m_numRows;}
    unsigned numCols() const override {return m_numCols;}
    bool evenHeight() const override {return false;}
    void moveCursorTo(double,double) override;
    void mouseDown(float,float) override;
    void mouseUp(float,float) override;
    void mouseMove(float,float) override;
    void zoom(double,double,double) override;
  };
}
#include "variablePane.cd"
#endif
