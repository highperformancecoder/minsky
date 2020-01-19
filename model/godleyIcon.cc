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
#include "cairoItems.h"
#include "minsky.h"
#include "selection.h"
#include <flowCoef.h>
#include <evalGodley.h>
#include <arrays.h>
#include <cairo_base.h>
#include <ctype.h>
#include "minsky_epilogue.h"
using namespace ecolab::cairo;
using namespace ecolab;
using namespace std;

namespace minsky
{
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
          // ensure variable type is consistent
          minsky::minsky().convertVarType(vars.back()->valueId(), varType);
          vars.back()->controller=self;
        }
      // remove any previously existing variables
      if (auto g=group.lock())
        for (auto& v: oldVars)
          g->deleteItem(*v);
    }

  double GodleyIcon::schema1ZoomFactor() const
  {
    if (auto g=group.lock())
      return iconScale()*g->zoomFactor();
    else
      return iconScale();
  }

  void GodleyIcon::resize(const LassoBox& b)
  {
    m_iconScale*=min(abs(b.x0-b.x1)/width(), abs(b.y0-b.y1)/height());
    update();
    moveTo(0.5*(b.x0+b.x1), 0.5*(b.y0+b.y1));
    bb.update(*this);
  }

  void GodleyIcon::removeControlledItems() const
  {
    if (auto g=group.lock())
      {
        for (auto& i: m_flowVars)
          g->removeItem(*i);
        for (auto& i: m_stockVars)
          g->removeItem(*i);
      }
  }

  void GodleyIcon::setCell(int row, int col, const string& newVal) 
  {
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
        table.data.erase(table.data.begin()+row-1);
        // if shared column data is deleted, remove it from the other tables too
        for (size_t col=1; col<table.cols(); ++col)
          minsky().balanceDuplicateColumns(*this, col);
        table.markEdited();
      }
  }

  void GodleyIcon::moveCell(int srcRow, int srcCol, int destRow, int destCol)
  {
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

  map<string,double> GodleyIcon::flowSignature(int col) const
  {
    map<string,double> r;
    for (size_t row=1; row<table.rows(); ++row)
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
            VariableValue& v=vi->second;
            v.godleyOverridden=false;
            string::size_type start=table.cell(r,c).find_first_not_of(" ");
            if (start!=string::npos)
              {
                FlowCoef fc(table.cell(r,c).substr(start));
                v.init=fc.str();
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


    // determine height of variables part of icon
    float height=0;
    stockMargin=0;
    flowMargin=0;
    accumulateWidthHeight(m_stockVars, height, stockMargin);
    accumulateWidthHeight(m_flowVars, height, flowMargin);
    iconSize=max(100.0, 1.8*height);

    positionVariables();
    bb.update(*this);
  }

  void GodleyIcon::positionVariables() const
  {
    // position of margin in absolute canvas coordinate
    float zoomFactor=iconScale()*this->zoomFactor();
    float x= this->x() - 0.5*(0.9*iconSize-flowMargin)*zoomFactor;
    float y= this->y() - 0.2*iconSize*zoomFactor;
    for (auto& v: m_flowVars)
      {
        // right justification
        RenderVariable rv(*v);
        v->rotation(0);
        v->bb.update(*v);
        v->moveTo(x-0.5*v->width()*zoomFactor,y);
        y+=2*rv.height()*zoomFactor;
      }
    x= this->x() - 0.5*(0.85*iconSize-flowMargin)*zoomFactor;
    y= this->y() + 0.5*(iconSize-stockMargin)*zoomFactor;

    for (auto& v: m_stockVars)
      {
        // top justification at bottom of icon
        RenderVariable rv(*v);
        v->rotation(90);
        v->bb.update(*v);
        v->moveTo(x,y+0.5*v->height()*zoomFactor);
        x+=2*rv.height()*zoomFactor;
      }
  }

  ItemPtr GodleyIcon::select(float x, float y) const
  {
    for (auto& v: m_flowVars)
      if (RenderVariable(*v).inImage(x,y)) 
        return v;
    for (auto& v: m_stockVars)
      if (RenderVariable(*v).inImage(x,y)) 
        return v;
    return ItemPtr();
  }

  void GodleyIcon::draw(cairo_t* cairo) const
  {
    positionVariables();
    {
      CairoSave cs(cairo);
      cairo_translate(cairo,-0.5*width()+leftMargin(),-0.5*height());
      cairo_scale(cairo, (width()-leftMargin())/svgRenderer.width(), (height()-bottomMargin())/svgRenderer.height());

      {
        CairoSave cs(cairo); //following call mutates transformation matrix
        svgRenderer.render(cairo); 
      }

    }

    if (!table.title.empty())
      {
        CairoSave cs(cairo);
        cairo_move_to(cairo,0.5*leftMargin(),-0.5*bottomMargin()-0.25*height());
        cairo_select_font_face
          (cairo, "sans-serif", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cairo,12);
        cairo_set_source_rgb(cairo,0,0,0);

        cairo_text_extents_t bbox;
        cairo_text_extents(cairo,table.title.c_str(),&bbox);
              
        cairo_rel_move_to(cairo,-0.5*bbox.width,0.5*bbox.height);
        cairo_show_text(cairo,table.title.c_str());
      }
          

    // render the variables
    DrawVars drawVars(cairo,x(),y());
    drawVars(m_flowVars); 
    drawVars(m_stockVars); 

    if (mouseFocus)
      {
        drawPorts(cairo);
        displayTooltip(cairo,tooltip);
        drawResizeHandles(cairo);
      }
      
    cairo_rectangle(cairo,-0.5*width()+leftMargin(),-0.5*height(),
                    width()-leftMargin(),height()-bottomMargin());
    cairo_clip(cairo);
    if (selected) drawSelected(cairo);
  }

  Units GodleyIcon::stockVarUnits(const string& stockName, bool check) const
  {
    unsigned stockCol=1;
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
    return foundFlow? units: cminsky().variableValues[vid].units;
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
    double dx=fabs(x-this->x()), dy=fabs(y-this->y());
    auto z=zoomFactor();
    double w=0.5*Item::width()*z, h=0.5*Item::height()*z;
    // check if (x,y) is within portradius of the 4 corners
    if (fabs(dx-w) < portRadiusMult*z &&
        fabs(dy-h) < portRadiusMult*z &&
        fabs(hypot(dx,dy)-hypot(w,h)) < portRadiusMult*z)
      return ClickType::onResize;
    // Make it possible to pull wires from variables attached to Godley icons. For ticket 940  
    if (auto item=select(x,y))
      return item->clickType(x,y);         
    if (dx < w && dy < h)
      return ClickType::onItem;
    else
      return ClickType::outside;
  }

  
  SVGRenderer GodleyIcon::svgRenderer;
}
