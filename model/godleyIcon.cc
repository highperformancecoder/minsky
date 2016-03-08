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
#include "init.h"
#include <flowCoef.h>
#include <evalGodley.h>
#include <arrays.h>
#include <cairo_base.h>
#include <ctype.h>
#include <ecolab_epilogue.h>
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
      {assert(x&&y); return x->name() < y->name();}
    };

    struct DrawVars
    {
      cairo_t* cairo;
      float x, y; // position of this icon
      float zoomFactor;
      DrawVars(cairo_t* cairo, float x, float y, float zoomFactor): 
        cairo(cairo), x(x), y(y), zoomFactor(zoomFactor) {}
      
      void operator()(const GodleyIcon::Variables& vars)
      {
        for (GodleyIcon::Variables::const_iterator v=vars.begin(); 
             v!=vars.end(); ++v)
          {
            cairo_save(cairo);
            const VariableBase& vv=**v;
            // coordinates of variable within the cairo context
            cairo_translate(cairo, (vv.x()-x), (vv.y()-y));
            vv.draw(cairo);
            cairo_restore(cairo);
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
          float w=2*rv.width()+2;
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

      vars.clear();
      for (vector<string>::const_iterator nm=varNames.begin(); nm!=varNames.end(); ++nm)
        {
          VariablePtr newVar(varType, *nm);
//          if (nm->find(':')==string::npos)
//            newVar->setScope(-1); //unscoped variables are treated as local to containing scope
          newVar->m_visible=false;
          // ensure variable type is consistent
          auto vv=minsky::cminsky().variableValues.find(newVar->valueId());
          if (vv!=minsky::cminsky().variableValues.end() && 
              vv->second.type()!=varType)
            minsky::minsky().convertVarType(newVar->valueId(), varType);
          set<VariablePtr>::const_iterator v=oldVars.find(newVar);
          if (v==oldVars.end())
            {
              // add new variable
              vars.push_back(newVar);
              group.lock()->addItem(newVar);
            }
          else
            {
              // move existing variable
              vars.push_back(*v);
              oldVars.erase(v);
              assert(*v);
            }
        }
      // remove any previously existing variables
      set<string> svName;
      for (auto& v: oldVars) svName.insert(v->uqName());
      minsky::minsky().model->recursiveDo
        (&Group::items,
         [&](Items& m, Items::iterator i) 
         {
           if (auto v=dynamic_pointer_cast<VariableBase>(*i))
             if (svName.count(v->uqName())>0)
               m.erase(i);
           return false;
         });
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
    updateVars(stockVars, table.getColumnVariables(), VariableType::stock);
    updateVars(flowVars, table.getVariables(), VariableType::flow);

    // retrieve initial conditions, if any
    for (size_t r=1; r<table.rows(); ++r)
      if (table.initialConditionRow(r))
        for (size_t c=1; c<table.cols(); ++c)
          {
            string name=trimWS(table.cell(0,c));
            // if local reference, then append namespace
            if (name.find(':')==string::npos)
              name=":"+name;
            auto vi=minsky().variableValues.find(VariableValue::valueId(name));
            if (vi==minsky().variableValues.end()) continue;
            VariableValue& v=vi->second;
            v.godleyOverridden=false;
            string::size_type start=table.cell(r,c).find_first_not_of(" ");
            if (start!=string::npos)
              {
                FlowCoef fc(table.cell(r,c).substr(start));
                if (table.signConventionReversed(c))
                  fc.coef*=-1;
                v.init=fc.str();
              
                v.godleyOverridden=true;
              }
            else
              {
                // populate cell with current variable's initial value
                FlowCoef fc(v.init);
                if (table.signConventionReversed(c))
                  fc.coef*=-1;
                table.cell(r,c)=fc.str();
                v.godleyOverridden=true;
              }
          }

    // determine height of variables part of icon
    float height=0;
    stockMargin=0;
    flowMargin=0;
    accumulateWidthHeight(stockVars, height, stockMargin);
    accumulateWidthHeight(flowVars, height, flowMargin);
    iconSize=max(100.0, 1.8*height);

    // position of margin in absolute canvas coordinate
    float x= this->x() - 0.5*(0.9*iconSize-flowMargin)*zoomFactor;
    float y= this->y() - 0.2/*0.37*/*iconSize*zoomFactor;
    for (Variables::iterator v=flowVars.begin(); v!=flowVars.end(); ++v)
      {
        // right justification
        RenderVariable rv(**v);
        const_cast<VariablePtr&>(*v)->moveTo(x-rv.width()*zoomFactor,y);
        y+=2*RenderVariable(**v).height()*zoomFactor;
      }
    x=this->x() - 0.5*(0.85*iconSize-flowMargin)*zoomFactor;
    y=this->y() + 0.5*(iconSize-stockMargin)*zoomFactor;

    for (Variables::iterator v=stockVars.begin(); v!=stockVars.end(); ++v)
      {
        // top justification at bottom of icon
        RenderVariable rv(**v);
        //OK because we're not changing variable name
        VariableBase& vv=const_cast<VariableBase&>(**v); 
        vv.moveTo(x,y+rv.width()*zoomFactor);
        vv.rotation=90;
        x+=2*rv.height()*zoomFactor;
      }
  }

//  size_t GodleyIcon::numPorts() const
//  {
//    int numPorts=0;
//    for (Variables::const_iterator v=flowVars.begin(); v!=flowVars.end(); ++v)
//      numPorts+=(*v)->numPorts();
//    for (Variables::const_iterator v=stockVars.begin(); v!=stockVars.end(); ++v)
//      numPorts+=(*v)->numPorts();
//    return numPorts;
//  }
//
//  vector<int> GodleyIcon::ports() const
//  {
//    vector<int> ports;
//    for (auto& v: flowVars)
//      ports.insert(ports.end(),v->ports().begin(),v->ports().end());
//    for (auto& v: stockVars)
//      ports.insert(ports.end(),v->ports().begin(),v->ports().end());
//    return ports;
//  }
//
//  void GodleyIcon::moveTo(float x1, float y1)
//  {
//    float dx=x1-x(), dy=y1-y();
//    m_x=x1; m_y=y1;
//    //const_cast OK below because location doesn't affect ordering
//    for (Variables::iterator v=flowVars.begin(); v!=flowVars.end(); ++v)
//      const_cast<VariableBase&>(**v).move(dx, dy); 
//    for (Variables::iterator v=stockVars.begin(); v!=stockVars.end(); ++v)
//      const_cast<VariableBase&>(**v).move(dx, dy);
//  }


//  int GodleyIcon::select(float x, float y) const
//  {
//    for (auto& v: flowVars)
//      if (RenderVariable(*v).inImage(x,y)) 
//        return minsky().model->findItem(*v).id();
//    for (auto& v: stockVars)
//      if (RenderVariable(*v).inImage(x,y)) 
//        return minsky().model->findItem(*v).id();
//    return -1;
//  }

//  void GodleyIcon::zoom(float xOrigin, float yOrigin,float factor) {
//    minsky::zoom(m_x, xOrigin, factor);
//    minsky::zoom(m_y, yOrigin, factor);
//    // RKS - tika changed this to an assignment, but I don't think that is correct
//    zoomFactor*=factor;
//    for (VariablePtr& v: flowVars)
//      v->zoomFactor=zoomFactor;
//    for (VariablePtr& v: stockVars)
//      v->zoomFactor=zoomFactor;
//    update();
//  }

//  void GodleyIcon::updatePortLocation()
//  {
//    double xScale = zoomFactor;   //  zoomFactor should be 1 for Wt
//    double yScale = zoomFactor;
//
//    // determine height of variables part of icon
//
//    float height=0;
//    stockMargin=0;
//    flowMargin=0;
//    accumulateWidthHeight(stockVars, height, stockMargin);
//    accumulateWidthHeight(flowVars, height, flowMargin);
//    iconSize=max(100.0, 1.8*height);
//
//    Variables::iterator v;
//
//    float x= this->x() - 0.5*(0.8*iconSize-flowMargin)*zoomFactor;
//    float y= this->y() - 0.37*iconSize*zoomFactor;
//
//    for (v = flowVars.begin(); v != flowVars.end(); ++v)
//      {
//        VariablePtr var = *v;
//        RenderVariable rv(*var);
//
//        double angle=var->rotation * M_PI / 180.0;
//        double x0=rv.width(), y0=0, x1=-rv.width()+2, y1=0;
//        double sa=sin(angle), ca=cos(angle);
//
//        const_cast<VariablePtr&>(*v)->moveTo(x - rv.width() * zoomFactor, y);
//        y += 2 * rv.height() * zoomFactor;
//
//        minsky().movePortTo(var->outPort(),
//                            var->x()+xScale*(x0*ca-y0*sa),
//                            var->y()+yScale*(y0*ca+x0*sa));
//        minsky().movePortTo(var->inPort(),
//                            var->x()+xScale*(x1*ca-y1*sa),
//                            var->y()+yScale*(y1*ca+x1*sa));
//      }
//
//    x=this->x() - 0.5*(0.85*iconSize-flowMargin)*zoomFactor;
//    y=this->y() + 0.5*(iconSize-stockMargin)*zoomFactor;
//
//    for (v = stockVars.begin(); v != stockVars.end(); ++v)
//      {
//        VariablePtr var = *v;
//        RenderVariable rv(*var);
//
//        // top justification at bottom of icon
//        var->moveTo(x, y + rv.width() * zoomFactor);
//        var->rotation = 90;
//        x += 2 * rv.height() * zoomFactor;
//
//        double angle=var->rotation * M_PI / 180.0;
//        double x0=rv.width(), y0=0, x1=-rv.width()+2, y1=0;
//        double sa=sin(angle), ca=cos(angle);
//        minsky().movePortTo(var->outPort(),
//                            var->x()+xScale*(x0*ca-y0*sa),
//                            var->y()+yScale*(y0*ca+x0*sa));
//        minsky().movePortTo(var->inPort(),
//                            var->x()+xScale*(x1*ca-y1*sa),
//                            var->y()+yScale*(y1*ca+x1*sa));
//      }
//  }

  void GodleyIcon::draw(cairo_t* cairo) const
  {
    cairo_translate(cairo,-0.5*width(),-0.5*height());

    cairo_rectangle(cairo,0,0,width(),height());
    cairo_clip(cairo);

    cairo_translate(cairo, leftMargin(),0);
    cairo_scale(cairo, (width()-leftMargin())/svgRenderer.width(), (height()-bottomMargin())/svgRenderer.height());

    svgRenderer.render(cairo);

    if (!table.title.empty())
      {
        cairo_save(cairo);
        cairo_identity_matrix(cairo);
        cairo_move_to(cairo,0.5*leftMargin(),-0.5*bottomMargin()-0.25*height());
        cairo_select_font_face
          (cairo, "sans-serif", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cairo,12);
        cairo_set_source_rgb(cairo,0,0,0);

        cairo_text_extents_t bbox;
        cairo_text_extents(cairo,table.title.c_str(),&bbox);
              
        cairo_rel_move_to(cairo,-0.5*bbox.width,0.5*bbox.height);
        cairo_show_text(cairo,table.title.c_str());
        cairo_restore(cairo);
      }
          

    // render the variables
    cairo_identity_matrix(cairo);
    DrawVars drawVars(cairo, x(), y(), zoomFactor);
    drawVars(flowVars); 
    drawVars(stockVars); 

    if (mouseFocus)
      drawPorts(cairo);
    if (selected) drawSelected(cairo);
  }

  SVGRenderer GodleyIcon::svgRenderer;

 
}
