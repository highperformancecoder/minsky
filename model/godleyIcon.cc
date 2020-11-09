/*
  @copyright Steve Keen 2012
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
#include "variable.h"
#include "godleyIcon.h"
#include "godleyTableWindow.h"
#include "cairoItems.h"
#include "minsky.h"
#include "selection.h"
#include <flowCoef.h>
#include <evalGodley.h>
#include <arrays.h>
#include <cairo_base.h>
#include <ctype.h>
#include "minsky_epilogue.h"
#include <boost/locale.hpp>
using namespace boost::locale::conv;
using namespace ecolab::cairo;
using namespace ecolab;
using namespace std;

// width of draggable border 
const float border=10;

namespace minsky
{
  GodleyIcon::CopiableUniquePtr::CopiableUniquePtr() {}
  GodleyIcon::CopiableUniquePtr::~CopiableUniquePtr() {}
  GodleyIcon::CopiableUniquePtr::CopiableUniquePtr(const CopiableUniquePtr&) {}

  namespace
  {
    struct OrderByName
    {
      bool operator()(const VariablePtr& x, const VariablePtr& y) const
      {assert(x&&y); return x->valueId() < y->valueId();} 
    };

    struct DrawVars
    {
      cairo_t* cairo;
      float x, y; // position of this icon
      DrawVars(cairo_t* cairo, float x, float y): 
        cairo(cairo), x(x), y(y) {}
      
      void operator()(const GodleyIcon::Variables& vars)
      {
        for (GodleyIcon::Variables::const_iterator v=vars.begin(); 
             v!=vars.end(); ++v)
          {
            ecolab::cairo::CairoSave cs(cairo);
            const VariableBase& vv=**v;
            // coordinates of variable within the cairo context
            cairo_translate(cairo, vv.x()-x, vv.y()-y);
            vv.draw(cairo);
          }
      }
    };

    // determine the width and maximum height on screen of variables in vars
    void accumulateWidthHeight(const GodleyIcon::Variables& vars,
                               float& height, float& width)
    {
      float h=0;
      for (auto& v: vars)
        { 
          RenderVariable rv(*v);
          h+=2*rv.height();
          if (h>height) height=h;
          float w=2*rv.width();
          if (w>width) width=w;
        }
    }

    inline bool isDotOrDigit(char x)
    {return x=='.' || isdigit(x);}


  }

  bool GodleyIcon::inItem(float xx, float yy) const
  {
    return abs(xx-x())<0.5*width()-border && abs(yy-y())<0.5*height()-border;
  }

  void GodleyIcon::updateVars(GodleyIcon::Variables& vars, 
                              const vector<string>& varNames, 
                              VariableType::Type varType)
  {
    // update the map of variables from the Godley table
    set<VariablePtr, OrderByName> oldVars(vars.begin(), vars.end());
    set<string> alreadyAdded;

    vars.clear();
    shared_ptr<GodleyIcon> self;
    if (auto g=group.lock())
      self=dynamic_pointer_cast<GodleyIcon>(g->findItem(*this));
      
    for (vector<string>::const_iterator nm=varNames.begin(); nm!=varNames.end(); ++nm)
      {
        VariablePtr newVar(varType, *nm);
        auto myGroup=group.lock();
        if (myGroup) myGroup->addItem(newVar); // get scope right
        auto v=oldVars.find(newVar);
          
        if (v==oldVars.end())
          {	
            // allow for the possibility that multiple names map to the same valueId
            if (!alreadyAdded.count(newVar->valueId()))
              {
                // add new variable
                vars.push_back(newVar);
                alreadyAdded.insert(newVar->valueId());
              }
            else
              if (myGroup)
                myGroup->removeItem(*newVar);
          }
        else
          {
            // move existing variable
            vars.push_back(*v);
            alreadyAdded.insert(newVar->valueId());
            oldVars.erase(v);
            assert(*v);
            if (myGroup) myGroup->removeItem(*newVar);
          }
        if (myGroup) myGroup->addItem(vars.back(),true);
        vars.back()->controller=self;
        // ensure variable type is consistent
        minsky::minsky().convertVarType(vars.back()->valueId(), varType);
      }
    // remove any previously existing variables
    if (auto g=group.lock())
      for (auto& v: oldVars)
        g->deleteItem(*v);   
  }

  void GodleyIcon::toggleEditorMode()
  {
    if (editor)
      {
        editor.reset();
        variableDisplay=true;
      }
    else
      if (auto g=group.lock())
        if (auto icon=dynamic_pointer_cast<GodleyIcon>(g->findItem(*this)))
          {
            editor.reset(new GodleyTableEditor(icon));
            editor->disableButtons();
            setEditorDisplayValues();
            variableDisplay=false;
          }
    updateBoundingBox();
  }

  void GodleyIcon::setEditorDisplayValues()
  {
    if (editor)
      {
            editor->displayValues=cminsky().displayValues;
            editor->displayStyle=cminsky().displayStyle;
      }
  }
  
  bool GodleyIcon::buttonDisplay() const {return editor && editor->drawButtons;}
  void GodleyIcon::toggleButtons()
  {
    if (editor)
      {
        if (editor->drawButtons)
          editor->disableButtons();
        else
          editor->enableButtons();
        updateBoundingBox();
      }
  }
  
  void GodleyIcon::scaleIcon(float w, float h)
  {
    update();
    scaleFactor(min(w/(leftMargin()+iWidth()*zoomFactor()),h/(bottomMargin()+iHeight()*zoomFactor())));
  }  

  void GodleyIcon::resize(const LassoBox& b)
  {
    float invZ=1.0/(this->zoomFactor());
    auto bw=abs(b.x0-b.x1), bh=abs(b.y0-b.y1);
    if (bw<=leftMargin() || bh<=bottomMargin()) return;
    moveTo(0.5*(b.x0+b.x1), 0.5*(b.y0+b.y1));
    iWidth((bw-leftMargin())*invZ);
    iHeight((bh-bottomMargin())*invZ);  
    scaleIcon(bw,bh);
    updateBoundingBox();
  }

  void GodleyIcon::removeControlledItems(Group& g) const
  {
    for (auto& i: m_flowVars)
      g.removeItem(*i);
    for (auto& i: m_stockVars)
      g.removeItem(*i);
  }

  void GodleyIcon::setCell(int row, int col, const string& newVal) 
  {
    if (!table.cellInTable(row,col)) return;
    // if this operation is clearing an initial condition cell, set it to 0
    string& c=table.cell(row,col);
    if (newVal.empty() && !c.empty() && table.initialConditionRow(row))
      c="0";
    else
      c=newVal; 
    if (row==0)
      minsky().importDuplicateColumn(table, col);
    else
      minsky().balanceDuplicateColumns(*this, col);
    table.markEdited();
  }

  void GodleyIcon::deleteRow(unsigned row)
  {
    if (row>0 && row<=table.rows())
      {
        table.deleteRow(row-1);
        // if shared column data is deleted, remove it from the other tables too
        for (size_t col=1; col<table.cols(); ++col)
          minsky().balanceDuplicateColumns(*this, col);
        table.markEdited();
      }
  }

  void GodleyIcon::moveCell(int srcRow, int srcCol, int destRow, int destCol)
  {
    if (!table.cellInTable(srcRow, srcCol) || !table.cellInTable(destRow, destCol)) return;
    if (srcCol!=destCol) // if moving between columns, we can delete and
      // add, which ensures any linked columns are
      // correctly updated
      {
        // create a copy here, as setCell may resize the array, changing 
        // what the reference referred to.
        string oldVal=table.cell(srcRow, srcCol);
        setCell(destRow, destCol, oldVal);
        setCell(srcRow, srcCol, "");
      }
    else // within a column, just move the data without affecting any linked column
      {
        table.cell(destRow, destCol)=table.cell(srcRow, srcCol);
        table.cell(srcRow, srcCol)="";
      }
  }

  map<string,double> GodleyIcon::flowSignature(unsigned col) const
  {
    map<string,double> r;
    for (size_t row=1; row<table.rows() && col<table.cols(); ++row)
      if (!table.initialConditionRow(row))
        {
          FlowCoef fc(table.cell(row,col));
          if (!fc.name.empty())
            r[fc.name]+=fc.coef;
        }
    return r;
  }

  void GodleyIcon::update()
  {
    updateVars(m_stockVars, table.getColumnVariables(), VariableType::stock);
    updateVars(m_flowVars, table.getVariables(), VariableType::flow);

    // retrieve initial conditions, if any
    for (size_t r=1; r<table.rows(); ++r)
      if (table.initialConditionRow(r))
        for (size_t c=1; c<table.cols(); ++c)
          {
            string name=trimWS(table.cell(0,c));           
            auto vi=minsky().variableValues.find(VariableValue::valueId(group.lock(),name));
            if (vi==minsky().variableValues.end()) continue;
            VariableValue& v=*vi->second;           
            v.godleyOverridden=false;
            string::size_type start=table.cell(r,c).find_first_not_of(" ");
            if (start!=string::npos)
              {
                FlowCoef fc(table.cell(r,c).substr(start));                                      
                v.init=fc.str();              
                // set initial value of stock var to init value of flow that is defined by a parameter or a constant. for ticket 1137
                if (auto initVar=minsky().definingVar(VariableValue::valueId(group.lock(),fc.str())))
                  if (initVar->inputWired() && initVar->type()==VariableType::flow)
                    if (auto lhsVar=initVar->ports[1]->wires()[0]->from()->item().variableCast()) {
                      FlowCoef fc1(lhsVar->vValue()->init);
                      fc1.coef*=fc.coef;
                      v.init=fc1.str();	
                    }       
                v.godleyOverridden=true;
              }
            else
              { 
                // populate cell with current variable's initial value
                FlowCoef fc(v.init);   
                table.cell(r,c)=fc.str();
                v.godleyOverridden=true;
              }

          }


    if (variableDisplay)
      {
        // determine height of variables part of icon
        float stockH=0, flowH=0;
        stockMargin=0;
        flowMargin=0;
        accumulateWidthHeight(m_stockVars, stockH, stockMargin);
        accumulateWidthHeight(m_flowVars, flowH, flowMargin);
        float iw=this->iWidth(), ih=this->iHeight();
        this->iWidth(max(iw, 1.8f*stockH));
        this->iHeight(max(ih, 1.8f*flowH));
      }
    
    positionVariables();
    updateBB();
  }

  void GodleyIcon::positionVariables() const
  {
    // position of margin in absolute canvas coordinate
    float z=this->zoomFactor()*scaleFactor();
    float x= this->x() - 0.5*iWidth()*z+0.5*leftMargin();
    float y= this->y() - 0.5*bottomMargin()-0.15*iHeight()*z;
    for (auto& v: m_flowVars)
      {
        // right justification if displayed, left otherwisw
        v->rotation(0);
        v->moveTo(x+v->x() - (variableDisplay? v->right(): v->left()), y);
        y+=v->height();
      }
    x= this->x() + 0.55*leftMargin()-0.45*iWidth()*z;
    y= this->y() + 0.5*iHeight()*z-0.5*bottomMargin();

    for (auto& v: m_stockVars)
      {
        // top justification at bottom of icon if displayed, bottom justified otherwise
        v->rotation(90);
        v->moveTo(x, y + v->y() - (variableDisplay? v->top(): v->bottom()));
        x+=v->width();
      }
  }

  ItemPtr GodleyIcon::select(float x, float y) const
  {
	if (variableDisplay)           // Disable selection of stock and flow vars when they are hidden. for tickets 1217 and 1220.
	{   
       for (auto& v: m_flowVars)
         if (v->contains(x,y)) 
           return v;
       for (auto& v: m_stockVars)
         if (v->contains(x,y)) 
           return v; 
     }
    return ItemPtr();
  }

  void GodleyIcon::draw(cairo_t* cairo) const
  {
    float z=zoomFactor()*scaleFactor();
    float w=iWidth()*z, h=iHeight()*z, left=-0.5*(w-leftMargin()), top=-0.5*(bottomMargin()+h);
    positionVariables();
    double titley;
    
    if (editor.get())
      {
        CairoSave cs(cairo);
        cairo_rectangle(cairo, left, top, w, h);
        cairo_stroke_preserve(cairo);
        cairo_rectangle(cairo, left+border, top+border, w-2*border, h-2*border);
        cairo_stroke_preserve(cairo);
        if (onBorder)
          { // shadow the border when mouse is over it
            cairo::CairoSave cs(cairo);
            cairo_set_source_rgba(cairo,0.5,0.5,0.5,0.5);
            cairo_set_fill_rule(cairo,CAIRO_FILL_RULE_EVEN_ODD);
            cairo_fill(cairo);
          }
        cairo_new_path(cairo);
        cairo_rectangle(cairo, left+border, top-border, w-2*border, h-2*border);
        cairo_clip(cairo);
        cairo_translate(cairo,left+border,top+border+12*zoomFactor()/* space for title*/);
        editor->zoomFactor=zoomFactor();
        editor->draw(cairo);
        titley=-0.5*(bottomMargin()+h);
      }
    else
      {
        CairoSave cs(cairo);
        cairo_translate(cairo,left,top);
        cairo_scale(cairo, (w)/svgRenderer.width(), (h)/svgRenderer.height());
        svgRenderer.render(cairo);
        titley=-0.5*bottomMargin()-0.35*(h);
      }
    
    if (!table.title.empty())
      {
        CairoSave cs(cairo);
        Pango pango(cairo);
        pango.setMarkup("<b>"+latexToPango(table.title)+"</b>");
        pango.setFontSize(12*z);
        cairo_move_to(cairo,-0.5*(pango.width()*z-0.5*leftMargin()), titley);
        pango.show();
      }
      
          

    if (variableDisplay)
      {
        // render the variables
        DrawVars drawVars(cairo,x(),y());
        drawVars(m_flowVars); 
        drawVars(m_stockVars); 
      }
    
    if (mouseFocus)
      {
        drawPorts(cairo);
        displayTooltip(cairo,tooltip);
        drawResizeHandles(cairo);
      }
      
    cairo_rectangle(cairo, left,top, w, h);    
    cairo_clip(cairo);
    if (selected)
      {
        drawSelected(cairo);
      }
  }

  Units GodleyIcon::stockVarUnits(const string& stockName, bool check) const
  {
    unsigned stockCol=1;
    //string sName=utf_to_utf<char>(stockName);    
    auto vid=valueId(stockName);
    for (; stockCol<table.cols(); ++stockCol)
      if (valueId(table.cell(0,stockCol))==vid)
        break;

    if (stockCol>=table.cols()) return {};

    bool foundFlow=false;
    Units units;
    for (unsigned row=1; row<table.rows(); ++row)
      {
        if (table.initialConditionRow(row)) continue;
        FlowCoef fc(table.cell(row,stockCol));
        if (fc.coef!=0)
          {
            auto vid=valueId(fc.name);
            // find variable assciated with this flow
            for (auto& v: flowVars())
              if (v->valueId()==vid)
                {
                  auto flowUnits=v->units(check);
                  if (check && foundFlow && units!=flowUnits)
                    throw_error("incompatible units: "+flowUnits.str()+"≠"+units.str()+" on stock "+stockName);
                  foundFlow=true;
                  units=flowUnits;
                }
          }
      }
    if (!cminsky().timeUnit.empty())
      units[cminsky().timeUnit]++;
    return foundFlow? units: cminsky().variableValues[vid]->units;
  }

  void GodleyIcon::insertControlled(Selection& selection)
  {
    for (auto& i: flowVars())
      selection.ensureItemInserted(i);
    for (auto& i: stockVars())
      selection.ensureItemInserted(i);
  }
  
  ClickType::Type GodleyIcon::clickType(float x, float y)
  {
    if (editor) return Item::clickType(x,y);
    double dx=fabs(x-this->x()), dy=fabs(y-this->y());
    auto z=zoomFactor()*scaleFactor();
    double w=0.5*iWidth()*z, h=0.5*iHeight()*z;
    if (onResizeHandle(x,y)) return ClickType::onResize;         
    // Make it possible to pull wires from variables attached to Godley icons. For ticket 940  
    if (auto item=select(x,y))
      return item->clickType(x,y);         
    if (dx < w && dy < h)
      return ClickType::onItem;
    else
      return ClickType::outside;
  }

  float GodleyIcon::toEditorX(float xx) const
  {return xx-x()+0.5f*width()-border;}
  float GodleyIcon::toEditorY(float yy) const
  {return yy-y()+0.5f*height()-border-12*zoomFactor();}
  
  void GodleyIcon::onMouseDown(float x, float y)
  {if (editor) editor->mouseDown(toEditorX(x),toEditorY(y));}
  
  void GodleyIcon::onMouseUp(float x, float y)
  {if (editor) editor->mouseUp(toEditorX(x),toEditorY(y));}
  
  bool GodleyIcon::onMouseMotion(float x, float y)
  {
    if (editor) editor->mouseMoveB1(toEditorX(x),toEditorY(y));
    return false;
  }
  
  bool GodleyIcon::onMouseOver(float x, float y)
  {
    if (editor) editor->mouseMove(toEditorX(x),toEditorY(y));
    return false;
  }
  
  void GodleyIcon::onMouseLeave()
  {
    if (editor)
      {
        editor->mouseMove(-1,-1);
        editor->update();
      }
  }

  bool GodleyIcon::onKeyPress(int keySym, const std::string& utf8, int)
  {
    if (editor) editor->keyPress(keySym, utf8);
    return editor.get();
  }
  
  SVGRenderer GodleyIcon::svgRenderer;
}
