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
#include "parVarSheet.h"
#include "latexMarkup.h"
#include "group.h"
#include <pango.h>
#include "minsky_epilogue.h"
#include "minsky.h"
using namespace std;
using ecolab::cairo::Surface;
using ecolab::Pango;
using ecolab::cairo::CairoSave;

namespace minsky
{
	
   void ParVarSheet::markEdited()
   {
     minsky::minsky().markEdited();
   }
   
   void ParVarSheet::insertRow(unsigned row)
   {
     if (row<=data.size())
       {
         data.insert(data.begin()+row, vector<string>(cols()));
         markEdited();
       }
   }
   
   void ParVarSheet::insertCol(unsigned col)
   {
     for (unsigned row=0; row<data.size(); ++row)
       {
         for (size_t i=data[row].size(); i<col; ++i)
           data[row].insert(data[row].end(), "");
         data[row].insert(data[row].begin()+col, "");
       }
     markEdited();
   }   
   	
	
  void ParVarSheet::populateItemVector() {
    itemVector.clear();	
    minsky().canvas.model->recursiveDo(&GroupItems::items,
                                       [&](Items&, Items::iterator i) {                                 
                                         if (variableSelector(*i))		                                 
                                           itemVector.emplace_back(*i);
                                         return false;
                                       });   	
  }
	
  void ParVarSheet::draw(cairo_t* cairo)
  {   
    try
      {	
      		
        if (!itemVector.empty())
          {
            float x0, y0=0.0;//+pango.height();	
            double w,h,w_prev, h_prev,lh; 
            for (auto& it: itemVector)
              {
                auto value=it->variableCast()->vValue();
                auto rank=value->hypercube().rank();
                auto dims=value->hypercube().dims();                
                Pango pango(cairo);      
                x0=0.0;
                float x=x0, y=y0;
                double colWidth=0;
                colLeftMargin.clear();
                pango.setMarkup("9999");
                if (rank==0)
                  { 
                    cairo_move_to(cairo,x,y);
                    pango.setMarkup(str(value->value(0)));
                    pango.show();              
                  }
                else if (rank==1)
                  {
                    string format=value->hypercube().xvectors[0].timeFormat();
                    for (auto& i: value->hypercube().xvectors[0])
                      {
                        cairo_move_to(cairo,x,y);
                        pango.setText(trimWS(str(i,format)));
                        pango.show();
                        y+=rowHeight;
                        colWidth=std::max(colWidth,5+pango.width());
                      }
                    insertCol(size_t(rank));  
                    y=y0;
                    //colLeftMargin.push_back(x);                    
                    x+=colWidth;
                    for (size_t i=0; i<value->size(); ++i)
                      {
                        if (!value->index().empty())
                          y=y0+value->index()[i]*rowHeight;
                        cairo_move_to(cairo,x,y);
                        auto v=value->value(i);
                        insertRow(i+1);
                        if (!std::isnan(v))
                          {
                            pango.setMarkup(str(v));
                            pango.show();
                          }
                        y+=rowHeight;
                      }
                    resize(value->size(),size_t(rank));  
                    h_prev=h;
                    w=0;h=0;      
                    cairo_get_current_point (cairo,&w,&h);   
                    if (h<h_prev) h+=h_prev;                                                                        
                    //cout << " " << w << " " << h << " "<< x0 << " " << y0 << endl;
                    // draw grid
                    {
                      cairo::CairoSave cs(cairo);
                      cairo_set_source_rgba(cairo,0,0,0,0.2);
                      for (y=y0+rowHeight; y<h+rowHeight; y+=2*rowHeight)
                        {
                          cairo_rectangle(cairo,0.0,y,w+colWidth,rowHeight);
                          cairo_fill(cairo);
                        }
                    }
                    cairo::CairoSave cs(cairo);
                    float rectHeight=0;
                    // make sure rectangle has right height
                    if (value->size()%2!=0) rectHeight= y-y0;
                    else rectHeight=y-y0-rowHeight;                    
                    cairo_rectangle(cairo,0.0,y0,w+colWidth,rectHeight);    
                    cairo_stroke(cairo);                          
                    cairo_clip(cairo);
                    
                    colLeftMargin.push_back(x);
                    // indicate cell mouse is hovering over
                    if ((hoverRow>0 || hoverCol>0) &&                                
                        size_t(hoverRow)<rows() &&
                        size_t(hoverCol)<cols())
                      {
                        CairoSave cs(cairo);
                        cairo_rectangle(cairo,
                                        colLeftMargin[hoverCol],hoverRow*rowHeight+topTableOffset,
                                        colLeftMargin[hoverCol+1]-colLeftMargin[hoverCol],rowHeight);                          
                        cairo_set_line_width(cairo,1);
                        cairo_stroke(cairo);
                      }                    
                    
                    y0=h+2.1*rowHeight;                 
			      }
			    else
			      {  
                    for (size_t k=0; k<rank-1; k++)  
                      {   
                        y+=rowHeight; // allow room for header row
                        string format=value->hypercube().xvectors[k].timeFormat();
                        for (auto& i: value->hypercube().xvectors[k])
                          {
                            cairo_move_to(cairo,x,y);
                            pango.setText(trimWS(str(i,format)));
                            pango.show();
                            y+=rowHeight;
                            colWidth=std::max(colWidth,5+pango.width());
                          }
                        y=y0;
                        //colLeftMargin.push_back(x);  
                        x+=colWidth;
                        format=value->hypercube().xvectors[k+1].timeFormat();
                        for (size_t i=0; i<dims[k+1]; ++i)
                          {
                            colWidth=0;
                            y=y0;
                            lh=0;
                            cairo_move_to(cairo,x,y);
                            for (size_t j=0; j<dims[k+0]; ++j)
                              lh+=rowHeight;
                            pango.setText(trimWS(str(value->hypercube().xvectors[k+1][i],format)));
                            pango.show();
                            { // draw vertical grid line
                              cairo::CairoSave cs(cairo);
                              cairo_set_source_rgba(cairo,0,0,0,0.5);
                              cairo_move_to(cairo,x-2.5,y0);
                              cairo_line_to(cairo,x-2.5,y0+lh+1.1*rowHeight);
                              cairo_stroke(cairo);
                            }
                            colWidth=std::max(colWidth, 5+pango.width());
                            for (size_t j=0; j<dims[k+0]; ++j)
                              {
                                y+=rowHeight;
                                if (y>2e09) break;
                                cairo_move_to(cairo,x,y);
                                auto v=value->atHCIndex(j+i*dims[k+0]);
                                if (!std::isnan(v))
                                  {
                                    pango.setText(str(v));
                                    pango.show();
                                  }
                                colWidth=std::max(colWidth, pango.width());
                              }
                            colLeftMargin.push_back(x);    
                            x+=colWidth;
                            if (x>2e09) break;
                          }      
                        h_prev=h;
                        w_prev=w;  
                        w=0;h=0;      
                        cairo_get_current_point (cairo,&w,&h);   
                        if (h<h_prev) h+=h_prev; 
                        if (w<w_prev) w+=w_prev;                                                                         
                        //cout << " " << w << " " << h << " "<< x0 << " " << y0 << endl;
                        // draw grid
                        {
                          cairo::CairoSave cs(cairo);
                          cairo_set_source_rgba(cairo,0,0,0,0.2);
                          for (y=y0+rowHeight; y<h+rowHeight; y+=2*rowHeight)
                            {
                              cairo_rectangle(cairo,x0,y,w+colWidth,rowHeight);
                              cairo_fill(cairo);
                            }
                        }
                        cairo::CairoSave cs(cairo);
                        float rectHeight=0;
                        // make sure rectangle has right height
                        if ((dims[k])%2!=0 || rank%2==0) rectHeight= y-y0;
                        else rectHeight=y-y0-rowHeight;
                        if (rank > 2) rectHeight+=rowHeight;
                        cairo_rectangle(cairo,x0,y0,w+colWidth,rectHeight);    
                        cairo_stroke(cairo);                          
                        cairo_clip(cairo);
                        
                        colLeftMargin.push_back(x);
                        // indicate cell mouse is hovering over
                        if ((hoverRow>0 || hoverCol>0) &&                                
                            size_t(hoverRow)<rows() &&
                            size_t(hoverCol)<cols())
                          {
                            CairoSave cs(cairo);
                            cairo_rectangle(cairo,
                                            colLeftMargin[hoverCol],hoverRow*rowHeight+topTableOffset,
                                            colLeftMargin[hoverCol+1]-colLeftMargin[hoverCol],rowHeight);                          
                            cairo_set_line_width(cairo,1);
                            cairo_stroke(cairo);
                          }				        
                        
                        x+=0.25*colWidth;
				        x0=x;         
				        y=y0;

                  }
               }
               if (rank>0) y0=h+2.1*rowHeight;
               else y0+=2.1*rowHeight;   
               
               // indicate cell mouse is hovering over
               //if ((hoverRow>0 || hoverCol>0) &&                                
               //    size_t(hoverRow)<rows() &&
               //    size_t(hoverCol)<cols())
               //  {
               //    CairoSave cs(cairo);
               //    cairo_rectangle(cairo,
               //                    colLeftMargin[hoverCol],hoverRow*rowHeight+topTableOffset,
               //                    colLeftMargin[hoverCol+1]-colLeftMargin[hoverCol],rowHeight);
               //    cairo_set_line_width(cairo,1);
               //    cairo_stroke(cairo);
               //  }
                     
               // indicate selected cells
               //{
               //  CairoSave cs(cairo);
               //  if (selectedRow==0 || (selectedRow>=int(scrollRowStart) && selectedRow<int(rows())))
               //    {
               //      size_t i=0, j=0;
               //      if (selectedRow>=int(scrollRowStart)) j=selectedRow-scrollRowStart+1;
               //      double y=j*rowHeight+topTableOffset;
	           //
               //      if (motionCol>=0 && selectedRow==0 && selectedCol>0) // whole col being moved
               //        {
               //          highlightColumn(cairo,selectedCol);
               //          highlightColumn(cairo,motionCol);
               //        }
               //      else if (motionRow>=0 && selectedCol==0 && selectedRow>0) // whole Row being moved
               //        {
               //          highlightRow(cairo,selectedRow);
               //          highlightRow(cairo,motionRow);
               //        }
               //      else if (selectedCol==0 || /* selecting individual cell */
               //               (selectedCol>=int(scrollColStart) && selectedCol<int(cols())))   
               //        {
               //          if ((selectedRow>1 || selectedRow <0) || selectedCol!=0) // can't select flows/stockVars or Initial Conditions labels
               //            {
               //              if (selectedCol>=int(scrollColStart)) i=selectedCol-scrollColStart+1;
               //              double x=colLeftMargin[i];
               //              cairo_set_source_rgba(cairo,1,1,1,1);
               //              cairo_rectangle(cairo,x,y,colLeftMargin[i+1]-x,rowHeight);
               //              cairo_fill(cairo);
               //              pango.setMarkup(defang(cell(selectedRow,selectedCol)));
               //              cairo_set_source_rgba(cairo,0,0,0,1);
               //              cairo_move_to(cairo,x,y);
               //              pango.show();
	           //			  
               //              // show insertion cursor
               //              cairo_move_to(cairo,x+Pango::idxToPos(insertIdx),y);
               //              cairo_rel_line_to(cairo,0,rowHeight);
               //              cairo_set_line_width(cairo,1);
               //              cairo_stroke(cairo);
               //              if (motionRow>0 && motionCol>0)
               //                highlightCell(cairo,motionRow,motionCol);
               //              if (selectIdx!=insertIdx)
               //                {
               //                  // indicate some text has been selected
               //                  cairo_rectangle(cairo,x+Pango::idxToPos(insertIdx),y,
               //                                  Pango::idxToPos(selectIdx)-Pango::idxToPos(insertIdx),rowHeight);
               //                  cairo_set_source_rgba(cairo,0.5,0.5,0.5,0.5);
               //                  cairo_fill(cairo);
               //               }
               //            }
               //        }
               //    }
               //}               
		    }
          }
      }
    catch (...) {throw;/* exception most likely invalid variable value */}
  }
  
  int ParVarSheet::colX(double x) const
  { 
	if (x==-1 || itemVector.empty()) return -1;  
    if (x<colLeftMargin[0]) return -1;
    if (x<colLeftMargin[1]) return 0;
    auto p=std::upper_bound(colLeftMargin.begin(), colLeftMargin.end(), x);
    size_t r=p-colLeftMargin.begin()-2+scrollColStart;
    if (r>cols()-1) r=-1; // out of bounds, invalidate. Also exclude A-L-E column. For ticket 1163.
    return r;
  }

  int ParVarSheet::rowY(double y) const
  {
    int c=(y-topTableOffset)/rowHeight;
    if (c>0) c+=scrollRowStart-1;
    if (c<0 || size_t(c)>rows()) c=-1; // out of bounds, invalidate
    return c;
  }

  int ParVarSheet::textIdx(double x) const
  {
    cairo::Surface surf(cairo_recording_surface_create(CAIRO_CONTENT_COLOR,NULL));
    Pango pango(surf.cairo());
    if (selectedRow>=0 && size_t(selectedRow)<rows() &&
        selectedCol>=0 && size_t(selectedCol)<cols() && (selectedRow!=1 || selectedCol!=0)) // No text index needed for a cell that is immutable. For ticket 1064
      {
        auto& str=cell(selectedRow,selectedCol);
        pango.setMarkup(defang(str));
        int j=0;
        if (selectedCol>=int(scrollColStart)) j=selectedCol-scrollColStart+1;
        x-=colLeftMargin[j]+2;
        return x>0 && str.length()>0?pango.posToIdx(x)+1: 0;  
      }
    return 0;
  }

  void ParVarSheet::mouseDown(double x, double y)
  {
     // catch exception, as the intention here is to allow the user to fix a problem
     try {update();}
     catch (...) {}
     selectedCol=colX(x);
     selectedRow=rowY(y);
     if (selectedRow>=0 && selectedRow<int(rows()) &&
         selectedCol>=0 && selectedCol<int(cols()) && (selectedRow!=1 || selectedCol!=0)) // Cannot save text in cell(1,0). For ticket 1064
        {
          selectIdx=insertIdx = textIdx(x);
          savedText=cell(selectedRow, selectedCol);
        }
     else
       selectIdx=insertIdx=0;
  }

  void ParVarSheet::mouseUp(double x, double y)
  {
    int c=colX(x), r=rowY(y);
    motionRow=motionCol=-1;
    // Cannot swap cell(1,0) with another. For ticket 1064. Also cannot move cells outside an existing Godley table to create new rows or columns. For ticket 1066. 
    //if ((selectedCol==0 && selectedRow==1) || (c==0 && r==1) || size_t(selectedRow)>=(godleyIcon->table.rows()) || size_t(r)>=(godleyIcon->table.rows()) || size_t(c)>=(godleyIcon->table.cols()) || size_t(selectedCol)>=(godleyIcon->table.cols()))
    //  return;  
    //else if (selectedRow==0)
    //  {  
	//	// Disallow moving flow labels column and prevent columns from moving when import stockvar dropdown button is pressed in empty column. For tickets 1053/1064/1066
    //    if (c>0 && size_t(c)<godleyIcon->table.cols() && selectedCol>0 && size_t(selectedCol)<godleyIcon->table.cols() && c!=selectedCol && !(colLeftMargin[c+1]-x < pulldownHot)) 
    //      godleyIcon->table.moveCol(selectedCol,c-selectedCol);
    //  }
    //else if (r>0 && selectedCol==0)
    //  {
    //    if (r!=selectedRow && !godleyIcon->table.initialConditionRow(selectedRow) && !godleyIcon->table.initialConditionRow(r))  // Cannot move Intitial Conditions row. For ticket 1064.
    //      godleyIcon->table.moveRow(selectedRow,r-selectedRow);
    //  } 
    //else if ((c!=selectedCol || r!=selectedRow) && c>0 && r>0)
    //  {
    //    swap(godleyIcon->table.cell(selectedRow,selectedCol), godleyIcon->table.cell(r,c));
    //    selectedCol=c;
    //    selectedRow=r;
    //  }
    //else 
    if (selectIdx!=insertIdx)
      copy();
  }

  void ParVarSheet::mouseMoveB1(double x, double y)
  {
    motionCol=colX(x), motionRow=rowY(y);
    if (motionCol==selectedCol && motionRow==selectedRow)
      selectIdx=textIdx(x);
  }

  void ParVarSheet::mouseMove(double x, double y)
  {
    hoverRow=hoverCol=-1;
    switch (clickType(x,y))
      {
      case background:
        break;
      default:
        hoverRow=rowY(y);
        hoverCol=colX(x);
        break;
      }  
  }

  inline constexpr char control(char x) {return x-'`';}
  
  void ParVarSheet::keyPress(int keySym, const std::string& utf8)
  {

    if (selectedCol>=0 && selectedRow>=0 && selectedCol<int(cols()) &&
        selectedRow<int(rows()) && (selectedCol!=0 || selectedRow!=1)) // Cell (1,0) is off-limits. For ticket 1064
          {			  	  
            auto& str=cell(selectedRow,selectedCol);
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
                  if (selectedRow>=0 && size_t(rows()) &&
                      selectedCol>=0 && size_t(cols()))
                    cell(selectedRow, selectedCol)=savedText;
                  selectedRow=selectedCol=-1;
                  break;
                case 0xff0d: //return
                case 0xff8d: //enter added for ticket 1122                            
                  update();
                  selectedRow=selectedCol=-1;                  
                  break;     
                case 0xff51: //left arrow
                  if (insertIdx>0) insertIdx--;
                  else navigateLeft();
                  break;
                case 0xff53: //right arrow
                  if (insertIdx<str.length()) insertIdx++;
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
            selectedRow=rows()-1; selectedCol=cols()-1; break;
          case 0xff51: //left arrow
            selectedRow=0; selectedCol=cols()-1; break;
          case 0xff54: // down
            selectedRow=2; selectedCol=0; break;           // Start from second row because Initial Conditions cell (1,0) can no longer be selected. For ticket 1064
          case 0xff52: // up
            selectedRow=rows()-1; selectedCol=0; break;
          default:
            return; // early return, no need to redraw
          }
      }
  }

  void ParVarSheet::delSelection()
  {
    if (insertIdx!=selectIdx)
      {
        auto& str=cell(selectedRow,selectedCol);
        str.erase(min(insertIdx,selectIdx),abs(int(insertIdx)-int(selectIdx))); 
        selectIdx=insertIdx=min(insertIdx,selectIdx);   
      }
  }

  void ParVarSheet::handleBackspace()
    {
      assert(selectedRow>=0 && selectedCol>=0);
      assert(unsigned(selectedRow)<rows());
      assert(unsigned(selectedCol)<cols());
      auto& str=cell(selectedRow,selectedCol);
      if (insertIdx!=selectIdx)
        delSelection();
      else if (insertIdx>0 && insertIdx<=str.length())
        str.erase(--insertIdx,1);
      selectIdx=insertIdx;
    }

  void ParVarSheet::handleDelete()
    {
      assert(selectedRow>=0 && selectedCol>=0);
      assert(unsigned(selectedRow)<rows());
      assert(unsigned(selectedCol)<cols());
      auto& str=cell(selectedRow,selectedCol); 
      if (insertIdx!=selectIdx)
        delSelection();
      else if (insertIdx>=0 && insertIdx<str.length())
        str.erase(insertIdx,1);
      selectIdx=insertIdx;
    }

  void ParVarSheet::cut()
  {
    copy();
    if (selectedCol>=0 && selectedRow>=0 && selectedCol<int(cols()) &&
        selectedRow<int(rows()))
      {	  
         if (selectIdx==insertIdx)
           // delete entire cell
           cell(selectedRow,selectedCol).clear();
         else
           delSelection();
      }
  }
  
  void ParVarSheet::copy()
  {
    if (selectedCol>=0 && selectedRow>=0 && selectedCol<int(cols()) &&
        selectedRow<int(rows()))
      {	  
         auto& str=cell(selectedRow,selectedCol);
         if (selectIdx!=insertIdx)
           cminsky().putClipboard
             (str.substr(min(selectIdx,insertIdx), abs(int(selectIdx)-int(insertIdx))));
         else
           cminsky().putClipboard(str);  
      }
  }

  void ParVarSheet::paste()
  {
    if (selectedCol>=0 && selectedRow>=0 && selectedCol<int(cols()) &&
        selectedRow<int(rows()))
      {
	     delSelection();
         auto& str=cell(selectedRow,selectedCol); 
         auto stringToInsert=cminsky().getClipboard();
         // only insert first line
         auto p=stringToInsert.find('\n');
         if (p!=string::npos)
           stringToInsert=stringToInsert.substr(0,p-1);
         str.insert(insertIdx,stringToInsert);
         selectIdx=insertIdx+=stringToInsert.length();
      }
  }

  ParVarSheet::ClickType ParVarSheet::clickType(double x, double y) const
  {
    int c=colX(x), r=rowY(y);
  
    if (c>0 && c<int(cols()))
      if (r>0 && r<int(rows()))
        return internal;

    return background;
  }
  
  void ParVarSheet::highlightCell(cairo_t* cairo, unsigned row, unsigned col)
  {
    if (row<scrollRowStart || col<scrollColStart) return;
    double x=colLeftMargin[col-scrollColStart+1];
    double width=colLeftMargin[col-scrollColStart+2]-x;
    double y=(row-scrollRowStart+1)*rowHeight+topTableOffset;
    cairo_rectangle(cairo,x,y,width,rowHeight);
    cairo_set_source_rgba(cairo,1,1,1,0.5);
    cairo_fill(cairo);
  }

  void ParVarSheet::pushHistory(ItemPtr i)
  {
    while (history.size()>maxHistory) history.pop_front();
    // Perform deep comparison of Godley tables in history to avoid spurious noAssetClass columns from arising during undo. For ticket 1118.
    if (history.empty() || !(history.back()==i)) {
      history.push_back(i);
    }
    historyPtr=history.size();
  }
      
  void ParVarSheet::undo(int changes, ItemPtr i)
  { 
    if (historyPtr==history.size())
      pushHistory(i);
    historyPtr-=changes;
    if (historyPtr > 0 && historyPtr <= history.size())
      {
        auto& d=history[historyPtr-1];
        // Perform deep comparison of Godley tables in history to avoid spurious noAssetClass columns from arising during undo. For ticket 1118.
        if (!i->variableCast()->vValue()) return; // should not happen
		i=d; 
      }
  }
  
  void ParVarSheet::update()
  {
    // if the contents of the cell are cleared, set the cell to "0". For #1181
    if (!savedText.empty() && cell(selectedRow,selectedCol).empty())
          cell(selectedRow,selectedCol)="0";

    //requestRedraw();
  }  
  
  
  
  void ParVarSheet::checkCell00()
  {
    if (selectedCol==0 && (selectedRow==0 || selectedRow ==1))
      // (0,0) cell not editable
      {
        selectedCol=-1;
        selectedRow=-1;
      }         
  }
  
    void ParVarSheet::navigateRight()
    {
      if (selectedCol>=0)
        {
          selectedCol++;
          insertIdx=0;
          if (selectedCol>=int(cols()))
            {
              if (selectedRow>0) selectedCol=0;   // Minor fix: Make sure tabbing and right arrow traverse all editable cells.
              else selectedCol=1;
              navigateDown();
            }
          checkCell00();
        }
    }
  
    void ParVarSheet::navigateLeft()
    {
      if (selectedCol>=0)
        {
          selectedCol--;
          insertIdx=0;
          if (selectedCol<0)
            {
              selectedCol=cols()-1;
              navigateUp();
            }
          checkCell00();
        }
    }

    void ParVarSheet::navigateUp()
    {
      if (selectedRow>=0)
        selectedRow=(selectedRow-1)%rows();
      checkCell00();
    }
  
    void ParVarSheet::navigateDown()
    {
      if (selectedRow>=0)
        selectedRow=(selectedRow+1)%rows();
      checkCell00();
    }       

}
