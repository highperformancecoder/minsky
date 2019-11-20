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

/**
   Provide support for the Godley Table spreadsheet window
 */

#ifndef GODLEYTABLEWINDOW_H
#define GODLEYTABLEWINDOW_H
#include "godleyIcon.h"
#include <cairoSurfaceImage.h>
#include <memory>
#include <vector>

namespace minsky
{
  struct ButtonWidgetEnums
  {
    enum RowCol {row, col};
    enum Pos {first, second, middle, last, firstAndLast};
  };
  
  /// supports +/-/←/→/↓/↑ widget
  template <minsky::ButtonWidgetEnums::RowCol rowCol>
  class ButtonWidget: public ButtonWidgetEnums
  {
    CLASSDESC_ACCESS(ButtonWidget);
  protected:
    GodleyIcon& godleyIcon;
    int m_mouseOver=-1;
  public:
    static constexpr double buttonSpacing=15;
    
    Pos pos=middle;
    unsigned idx=0; ///< row or column this widget is located in
    
    void draw(cairo_t*);
    /// draw button \a idx, with label \a label and colour \a r, \a b, \a g
    void drawButton(cairo_t*, const std::string& label,
                    double r, double g, double b, int idx);
    /// invoke action associated with button at x
    void invoke(double x);
    /// indicate which button mouse is hovering over. x<0 means not hovering
    void hover(int button) {m_mouseOver=button;}
    void hover(double x) {m_mouseOver=button(x);}
    /// convert x coordinate into a button
    int button(double x) const {return x/buttonSpacing;}
    int mouseOver() const {return m_mouseOver;}
    
    ButtonWidget(GodleyIcon& godleyIcon, unsigned idx=0):
      godleyIcon(godleyIcon), idx(idx) {}
  };

  class GodleyTableWindow: public ecolab::CairoSurface, public ButtonWidgetEnums
  {
    
    CLASSDESC_ACCESS(GodleyTableWindow);
  public:
    static constexpr double columnButtonsOffset=12;
    /// offset of the table within the window
    static constexpr double leftTableOffset=4*ButtonWidget<col>::buttonSpacing;
    static constexpr double topTableOffset=30;
    static constexpr double pulldownHot=12; ///< space for ▼ in stackVar cells
    /// minimum column width (for eg empty columns)
    static constexpr double minColumnWidth=4*ButtonWidget<col>::buttonSpacing;

    std::shared_ptr<GodleyIcon> godleyIcon;
    /// starting row/col number of the scrolling region
    unsigned scrollRowStart=1, scrollColStart=1;
    /// which cell is active, none initially
    int selectedRow=-1, selectedCol=-1;
    std::string savedText;
    int hoverRow=-1, hoverCol=-1;
    /// computed positions of the table columns
    std::vector<double> colLeftMargin;
    /// computed height of each row
    double rowHeight=0;
    /// location of insertion pointer in selected cell, as well as
    /// other end of selection (if mouse-swiped)
    unsigned insertIdx=0, selectIdx=0;
    bool displayValues=false;
    enum DisplayStyle {DRCR, sign};
    DisplayStyle displayStyle=sign;
    double zoomFactor=1; ///< zoom the display

    GodleyTableWindow(const std::shared_ptr<GodleyIcon>& g): godleyIcon(g)
    {adjustWidgets();}
    
    void redraw(int, int, int width, int height) override;
    void requestRedraw() {if (surface.get()) surface->requestRedraw();}
    /// event handling 
    void mouseDown(double x, double y);
    void mouseUp(double x, double y);
    /// mouse motion with button 1 pressed
    void mouseMoveB1(double x, double y);
    void mouseMove(double x, double y);
    void keyPress(int keySym, const std::string& utf8);

    enum ClickType {background, row0, col0, internal, importStock, rowWidget, colWidget};
    /// returns the clickType in zoomed coordinates
    ClickType clickTypeZoomed(double x, double y) const {return clickType(x/zoomFactor, y/zoomFactor);}

    
    /// add/delete rows/columns at x,y
    void addStockVar(double x);
    void importStockVar(const string& name, double x);
    void deleteStockVar(double x);
    void addFlow(double y);
    void deleteFlow(double y);
    
    int colXZoomed(double x) const {return colX(x/zoomFactor);}
    int rowYZoomed(double y) const {return rowY(y/zoomFactor);}

    void highlightColumn(cairo_t* cairo,unsigned col);
    void highlightRow(cairo_t* cairo,unsigned row);
    void highlightCell(cairo_t* cairo,unsigned row, unsigned col);

    // support cut/copy/paste operations
    void delSelection();
    void cut();
    void copy();
    void paste();
    
    int textIdx(double x) const;

    size_t maxHistory{100}; ///< maximum no. of history states to save
    size_t historyPtr=0;
    // push state onto history if different
    void pushHistory();
    /// restore to state \a changes ago 
    void undo(int changes);
    
    /// ensure button widgets are synced with current table data
    void adjustWidgets();

    /// update canvas godleyIcon, and any related godley icons. Can throw
    void update();

    /// @{ move selected cell right, left, up or down, moving to next
    /// line and wrapping if at end of row or col
    void navigateRight();
    void navigateLeft();
    void navigateUp();
    void navigateDown();
    /// @}
    
  protected:
    std::vector<ButtonWidget<row>> rowWidgets;
    std::vector<ButtonWidget<col>> colWidgets;
    /// column at \a x in unzoomed coordinates
    int colX(double x) const;
    /// row at \a y in unzoomed coordinates
    int rowY(double y) const;
    int motionRow=-1, motionCol=-1; ///< current cell under mouse motion
    std::deque<GodleyTable::Data> history;
    ClickType clickType(double x, double y) const;
    void checkCell00(); ///<check if cell (0,0) is selected, and deselect if so
    /// handle delete or backspace. Cell assumed selected
    void handleDelete();
  };
}

#include "godleyTableWindow.cd"
#endif
