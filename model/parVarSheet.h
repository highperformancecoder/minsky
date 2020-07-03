/*
  @copyright Steve Keen 2019
  @author Russell Standish
  @author Wynand Dednam
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

#ifndef PARVARSHEET_H
#define PARVARSHEET_H
#include <item.h>
#include <variable.h>
#include "classdesc_access.h"

namespace minsky
{
  
  //class ParameterSheet;	
	 
  class ParVarSheet: public ItemT<ParVarSheet>
  {
  public:
    friend struct SchemaHelper; 	   
    typedef std::vector<std::vector<std::string>> Data;  
  private:
    CLASSDESC_ACCESS(ParVarSheet);    
    Data data;  
    void markEdited(); ///< mark model as having changed
    void _resize(unsigned rows, unsigned cols) {                               
      // resize existing
      for (size_t i=0; i<data.size(); ++i) data[i].resize(cols);
      data.resize(rows, std::vector<std::string>(cols));
    }            
  public: 
    ParVarSheet() {}
  
    double xoffs=80;
    double rowHeight=0;
    double colWidth=50;    
    float m_width=600, m_height=800;
    virtual float width() const {return m_width;}
    virtual float height() const {return m_height;}
    Items itemVector;
    
    std::string title;     
    
    size_t rows() const {return data.size();}                                           // useful for editable par tab!!
    size_t cols() const {return data.empty()? 0: data[0].size();}      
       
    void clear() {data.clear(); markEdited();}
    void resize(unsigned rows, unsigned cols){_resize(rows,cols); markEdited();}              
    
    /// insert row at \a row
    void insertRow(unsigned row);  
    
    /// insert col at \a col
    void insertCol(unsigned col);        
    
    std::string& cell(unsigned row, unsigned col) {                                       // useful for editable par tab!!
      if (row>=rows() || col>=cols())
        _resize(row+1, col+1);
      return data[row][col];
    }
    const std::string& cell(unsigned row, unsigned col) const {                          // useful for editable par tab!!
      if (row>=data.size() || col>=data[row].size())
        throw std::out_of_range("Tab sheet index error");
      return data[row][col];
    }
    std::string getCell(unsigned row, unsigned col) const {
      if (row<rows() && col<cols())
        return cell(row,col);
      else
        return "";
    }
    
    std::string savedText;                                                     // useful for editable par tab!!        
    
    /// accessor for schema access
    const Data& getData() const {return data;} 
          
    double topTableOffset=15;

    /// starting row/col number of the scrolling region
    unsigned scrollRowStart=1, scrollColStart=1;    
    /// which cell is active, none initially
    int selectedRow=-1, selectedCol=-1;                                      // useful for editable par tab!!
    int hoverRow=-1, hoverCol=-1;
    /// computed positions of the table columns
    std::vector<double> colLeftMargin;          
    
    unsigned insertIdx=0, selectIdx=0;                                       // useful for editable par tab!!                                

    void populateItemVector();
    virtual bool variableSelector(ItemPtr i) {return false;}
    void draw(cairo_t* cairo);      
    
    /// event handling 
    void mouseDown(double x, double y);                                              // useful for editable par tab!!
    void mouseUp(double x, double y);
    /// mouse motion with button 1 pressed
    void mouseMoveB1(double x, double y);
    void mouseMove(double x, double y);
    void keyPress(int keySym, const std::string& utf8);

    enum ClickType {background, internal};                            // useful for editable par tab!!
    
    void highlightCell(cairo_t* cairo,unsigned row, unsigned col);                             // useful for editable par tab!!

    // support cut/copy/paste operations
    void delSelection();                                                   // useful for editable par tab!!
    void cut();
    void copy();
    void paste();
    
    int textIdx(double x) const;                                                     // useful for editable par tab!!

    size_t maxHistory{100}; ///< maximum no. of history states to save                  
    size_t historyPtr=0;                                                   // useful for editable par tab!!
    // push state onto history if different
    virtual void pushHistory(ItemPtr i);                                                    // useful for editable par tab!!
    /// restore to state \a changes ago 
    virtual void undo(int changes, ItemPtr i);                                                      // useful for editable par tab!!    
    
    /// update canvas godleyIcon, and any related godley icons. Can throw
    void update();     
    
    /// @{ move selected cell right, left, up or down, moving to next
    /// line and wrapping if at end of row or col
    void navigateRight();                                                              // useful for editable par tab!!
    void navigateLeft();
    void navigateUp();
    void navigateDown();
    /// @}
    
    ~ParVarSheet() {}
   
  protected:
    /// column at \a x in unzoomed coordinates
    int colX(double x) const;
    /// row at \a y in unzoomed coordinates
    int rowY(double y) const;
    int motionRow=-1, motionCol=-1; ///< current cell under mouse motion
    // Perform deep comparison of Godley tables in history to avoid spurious noAssetClass columns from arising during undo. For ticket 1118.
    std::deque<ItemPtr> history;
    ClickType clickType(double x, double y) const;                                                              // useful for editable par tab!!
    void checkCell00(); ///<check if cell (0,0) is selected, and deselect if so
    /// handle delete or backspace. Cell assumed selected
    void handleBackspace();                                                                        // useful for editable par tab!!
    void handleDelete();        
    
  };
  
}

#include "parVarSheet.cd"
#endif
