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

#include "geometry.h"
#include "variable.h"
#include "cairoItems.h"
#include "minsky.h"
//#include "RESTProcess_base.h"
#include <error.h>
#include "minsky_epilogue.h"

#include <boost/regex.hpp>

using namespace classdesc;
using namespace ecolab;

using namespace minsky;
using ecolab::array;
using namespace ecolab::cairo;

namespace minsky
{
 namespace VarAccessors
  {
    NameAccessor::NameAccessor(): ecolab::TCLAccessor<VariableBase,std::string,0>
        ("name",(Getter)&VariableBase::name,(Setter)&VariableBase::name) {}
    InitAccessor::InitAccessor(): ecolab::TCLAccessor<VariableBase,std::string,1>
        ("init",(Getter)&VariableBase::init,(Setter)&VariableBase::init) {}
    ValueAccessor::ValueAccessor(): ecolab::TCLAccessor<VariableBase,double>
      ("value",(Getter)&VariableBase::value,(Setter)&VariableBase::value) {}
    SliderVisibleAccessor::SliderVisibleAccessor(): ecolab::TCLAccessor<VariableBase,bool>
      ("sliderVisible",(Getter)&VariableBase::sliderVisible,(Setter)&VariableBase::sliderVisible) {}
 }
}


VariableBase::~VariableBase() {}

void VariableBase::addPorts()
{
#ifndef NDEBUG
  for (auto& i: ports)
    assert(i.use_count()==1);
#endif
  ports.clear();
  if (numPorts()>0)
    ports.emplace_back(new Port(*this,Port::noFlags));
  for (size_t i=1; i<numPorts(); ++i)
    ports.emplace_back
      (new Port(*this, Port::inputPort));
}

bool VariableBase::inputWired() const
{
  return ports.size()>1 && !ports[1]->wires().empty();
}

ClickType::Type VariableBase::clickType(float xx, float yy)
{
  double fm=std::fmod(rotation(),360);
  bool notflipped=(fm>-90 && fm<90) || fm>270 || fm<-270;
  Rotate r(rotation()+(notflipped? 0: 180),0,0); // rotate into variable's frame of reference
  RenderVariable rv(*this);
  double z=zoomFactor();
  try
    {
      double hpx=z*rv.handlePos();
      double hpy=-z*rv.height();
      if (rv.height()!=iHeight()) hpy=-z*iHeight();
      double dx=xx-x(), dy=yy-y(); 
      if (type()!=constant && hypot(dx - r.x(hpx,hpy), dy-r.y(hpx,hpy)) < 5)
        return ClickType::onSlider;
      double w=z*rv.width(), h=z*rv.height();
      if (rv.width()<iWidth()) w=z*iWidth();
      if (rv.height()<iHeight()) h=z*iHeight();
      if (fabs(fabs(dx)-w) < portRadius*z &&
          fabs(fabs(dy)-h) < portRadius*z &&
          fabs(hypot(dx,dy)-hypot(w,h)) < portRadius*z)
        return ClickType::onResize;
    }
  catch (...) {}
  return Item::clickType(xx,yy);
}

//Factory
VariableBase* VariableBase::create(VariableType::Type type)
{
  switch (type)
    {
    case undefined: return new Variable<undefined>; break;
    case constant: return new VarConstant; break;
    case parameter: return new Variable<parameter>; break;
    case flow: return new Variable<flow>; break;
    case stock: return new Variable<stock>; break;
    case tempFlow: return new Variable<tempFlow>; break;
    case integral: return new Variable<integral>; break;
    default: 
      throw error("unknown variable type %s", typeName(type).c_str());
    }
}

void VariableBase::retype(VariableType::Type type)
{
  if (type==this->type()) return; // nothing to do
  if (auto vv=vValue())
    if (type==vv->type())
      if (auto g=group.lock())
        for (auto& i: g->items)
          if (i.get()==this)
            {
              VariablePtr vp{i};
              vp.retype(type);
              i=vp;
              return;
            }
  minsky().convertVarType(valueId(), type);
}


float VariableBase::zoomFactor() const
{
  if (ioVar())
    if (auto g=group.lock())
      return g->edgeScale();
  // scale by GodleyIcon::iconScale if part of an Godley icon
  if (auto g=dynamic_cast<GodleyIcon*>(controller.lock().get()))
    return g->iconScale() * Item::zoomFactor();
  return Item::zoomFactor();
}

VariableValue* VariableBase::vValue() const
{
  auto vv=minsky().variableValues.find(valueId());
  if (vv!=minsky().variableValues.end())
    return &vv->second;
  else
    return nullptr;
}

string VariableBase::valueId() const 
{
  return VariableValue::valueId(group.lock(), m_name);
}

std::string VariableBase::valueIdInCurrentScope(const std::string& nm) const
{
  return VariableValue::valueId(group.lock(), nm);
}

string VariableBase::name()  const
{
  if (m_name==":_") return "";
  // hide any leading ':' in upper level
  if (m_name[0]==':')
    {
      auto g=group.lock();
      if (!g || g==cminsky().model)
        return m_name.substr(1);
    }
  return m_name;
}

string VariableBase::name(const std::string& name) 
{	
  // cowardly refuse to set a blank name
  if (name.empty() || name==":") return name;
  // Ensure value of variable is preserved after rename. For ticket 1106.	
  auto tmpVV=vValue();
  // ensure integral variables are not global when wired to an integral operation
  m_name=(type()==integral && name[0]==':' &&inputWired())? name.substr(1): name;
  ensureValueExists(tmpVV,name);
  bb.update(*this); // adjust bounding box for new name - see ticket #704
  return this->name();
}

bool VariableBase::ioVar() const
{return dynamic_cast<Group*>(controller.lock().get());}


void VariableBase::ensureValueExists(VariableValue* vv, const std::string& nm) const
{
  string valueId=this->valueId();
  // disallow blank names
  if (valueId.length()>1 && valueId.substr(valueId.length()-2)!=":_" && 
      minsky().variableValues.count(valueId)==0)
    {
      assert(VariableValue::isValueId(valueId));
	  // Ensure value of variable is preserved after rename. For ticket 1106.	      
      if (vv==nullptr) minsky().variableValues.insert
        (make_pair(valueId,VariableValue(type(), name(),"",group.lock())));
      // Ensure variable names are updated correctly everywhere they appear. For tickets 1109/1138.  
      else minsky().variableValues.insert
        (make_pair(valueId,VariableValue(type(),nm,vv->init,group.lock())));
    }
}


string VariableBase::init() const
{
  auto value=minsky().variableValues.find(valueId());
  if (value!=minsky().variableValues.end())
    return value->second.init;
  else
    return "0";
}

string VariableBase::init(const string& x)
{
  ensureValueExists(nullptr,""); 
  if (VariableValue::isValueId(valueId()))
    {
      VariableValue& val=minsky().variableValues[valueId()];
      val.init=x;
      // for constant types, we may as well set the current value. See ticket #433. Also ignore errors (for now), as they will reappear at reset time.
      try
        {
          if (type()==constant || type()==parameter ||
              (type()==flow && !cminsky().definingVar(valueId()))) 
            val.reset(minsky().variableValues);
        }
      catch (...)
        {}
    }
  return x;
}

double VariableBase::value() const
{
  if (VariableValue::isValueId(valueId()))
    return minsky::cminsky().variableValues[valueId()].value();
  else
    return 0;
}

double VariableBase::value(const double& x)
{
  if (!m_name.empty() && VariableValue::isValueId(valueId()))
    minsky().variableValues[valueId()][0]=x;
  return x;
}

int VariableBase::stockVarsPassed=0;
int VariableBase::varsPassed=0;

Units VariableBase::units(bool check) const
{
  if (varsPassed==0) minsky().variableValues.resetUnitsCache(); 
  // we allow possible traversing twice, to allow
  // stock variable to break the cycle
  if (unitsCtr-stockVarsPassed>=1)
    {
      if (check)
        throw_error("Cycle detected on wiring network");
      else
        return {};
    }

  auto it=minsky().variableValues.find(valueId());
  if (it!=minsky().variableValues.end())
    {
      auto& vv=it->second;
      if (vv.unitsCached) return vv.units;

      
      IncrDecrCounter ucIdc(unitsCtr);
      IncrDecrCounter vpIdc(varsPassed);
      // use a unique ptr here to only increment counter inside a stockVar
      unique_ptr<IncrDecrCounter> svp;

      if (isStock()) // we use user defined units
        {
          if (unitsCtr==1)
            {
              svp.reset(new IncrDecrCounter(stockVarsPassed));
              // check that input units match output units
              Units units;
              if (auto i=dynamic_cast<IntOp*>(controller.lock().get()))
                units=i->units(check);
              else if (auto g=dynamic_cast<GodleyIcon*>(controller.lock().get()))
                units=g->stockVarUnits(name(),check);
              if (check && units.str()!=vv.units.str())
                {
                  if (auto i=controller.lock())
                    i->throw_error("inconsistent units "+units.str()+"≠"+vv.units.str());
                }
            }
        }
      else
        // updates units in the process
        if (ports.size()>1 && !ports[1]->wires().empty())
          vv.units=ports[1]->wires()[0]->from()->item().units(check);
        else if (auto v=cminsky().definingVar(valueId()))
          vv.units=v->units(check);

      vv.units.normalise();
      vv.unitsCached=true;
      return vv.units;
    }
  else
    return Units();
}

void VariableBase::setUnits(const string& x)
{
  if (VariableValue::isValueId(valueId()))
    minsky().variableValues[valueId()].units=Units(x);
}



vector<string> VariableBase::accessibleVars() const
{
  set<string> r;
  if (auto g=group.lock())
    {
      // first add local variables
      for (auto& i: g->items)
        if (auto v=i->variableCast())
          r.insert(v->name());
      // now add variables in outer scopes, ensuring they qualified
      for (g=g->group.lock(); g;  g=g->group.lock())
        for (auto& i: g->items)
          if (auto v=i->variableCast())
            {
              auto n=v->name();
              if (n[0]==':')
                r.insert(n);
              else
                r.insert(':'+n);
            }
    }
  return vector<string>(r.begin(),r.end());
}

void VariableBase::exportAsCSV(const std::string& filename) const
{
  auto value=minsky().variableValues.find(valueId());
  if (value!=minsky().variableValues.end())
    value->second.exportAsCSV(filename, name());
}

void VariableBase::insertControlled(Selection& selection)
{
  selection.ensureItemInserted(controller.lock());
}

namespace minsky
{
  template <> size_t Variable<VariableBase::undefined>::numPorts() const {return 0;}
  template <> size_t Variable<VariableBase::constant>::numPorts() const {return 1;}
  template <> size_t Variable<VariableBase::parameter>::numPorts() const {return 1;}
  template <> size_t Variable<VariableBase::flow>::numPorts() const {return 2;}
  template <> size_t Variable<VariableBase::stock>::numPorts() const {return 1;}
  template <> size_t Variable<VariableBase::tempFlow>::numPorts() const {return 2;}
  template <> size_t Variable<VariableBase::integral>::numPorts() const {return 2;}
}


void VariablePtr::retype(VariableBase::Type type) 
{
  VariablePtr tmp(*this);
  if (tmp && tmp->type()!=type)
    {
      reset(VariableBase::create(type));
      static_cast<VariableBase&>(*get()) = *tmp;
      for (size_t i=0; i<get()->ports.size() && i< tmp->ports.size(); ++i)
        for (auto w: tmp->ports[i]->wires())
          {
            assert(!tmp->ports[i]->input());
            w->moveToPorts(get()->ports[i], w->to());
          }
      get()->ensureValueExists(nullptr,"");
    }
}

bool VariableBase::visible() const
{
  auto g=group.lock();
  //toplevel i/o items always visible
  if ((!g || !g->group.lock()) && g==controller.lock()) return true;
  return !controller.lock() && Item::visible();
}


void VariableBase::sliderSet(double x)
{
  if (x<sliderMin) x=sliderMin;
  if (x>sliderMax) x=sliderMax;
  init(to_string(x));
  value(x);
}


void VariableBase::initSliderBounds() const
{
  if (!sliderBoundsSet) 
    {
      if (value()==0)
        {
          sliderMin=-1;
          sliderMax=1;
          sliderStep=0.1;
        }
      else
        {
          sliderMin=-value()*10;
          sliderMax=value()*10;
          sliderStep=abs(0.1*value());
        }
      sliderStepRel=false;
      sliderBoundsSet=true;
    }
}

void VariableBase::adjustSliderBounds() const
{
  if (auto vv=vValue())
  // For feature 47
    if (vv->size()==1)
      {
        if (sliderMax<vv->value()) sliderMax=vv->value();
        if (sliderMin>vv->value()) sliderMin=vv->value();
      }
}

bool VariableBase::handleArrows(int dir,bool reset)
{
  sliderSet(value()+dir*sliderStep);
  if (reset) minsky().reset();
  return true;
}


void VariableBase::draw(cairo_t *cairo) const
{
  double angle=rotation() * M_PI / 180.0;
  double fm=std::fmod(rotation(),360);
  float z=zoomFactor();

  RenderVariable rv(*this,cairo);
  rv.setFontSize(12*z);
  // if rotation is in 1st or 3rd quadrant, rotate as
  // normal, otherwise flip the text so it reads L->R
  bool notflipped=(fm>-90 && fm<90) || fm>270 || fm<-270;
  Rotate r(rotation() + (notflipped? 0: 180),0,0);
  rv.angle=angle+(notflipped? 0: M_PI);

  // parameters of icon in userspace (unscaled) coordinates
  float w, h, hoffs, fontFactor;
  w=rv.width()*z; 
  h=rv.height()*z;
  fontFactor=min(iWidth()/rv.width(),iHeight()/rv.height());
  if (rv.width()<iWidth()) w=iWidth()*z;
  if (rv.height()<iHeight()) h=iHeight()*z;
  if (rv.width()<iWidth() || rv.height()<iHeight()) rv.setFontSize(12*fontFactor*z);
  else fontFactor=1;
  hoffs=rv.top()*z;
  

  cairo_move_to(cairo,r.x(-w+1,-h-hoffs+2), r.y(-w+1,-h-hoffs+2)/*h-2*/);
  rv.show();

  VariableValue vv;
  if (VariableValue::isValueId(valueId()))
    vv=minsky::cminsky().variableValues[valueId()];
  
  // For feature 47
  if (type()!=constant && !ioVar() && (vv.size()==1) )
    try
    {
      auto val=engExp();
  
      Pango pangoVal(cairo);
      if (!isnan(value())) {
		   pangoVal.setFontSize(6*fontFactor*z);
		   pangoVal.setMarkup(mantissa(val));
	   }
      else if (isinf(value())) { // Display non-zero divide by zero as infinity. For ticket 1155
		  pangoVal.setFontSize(8*fontFactor*z);
		  if (signbit(value())) pangoVal.setMarkup("-∞");
          else pangoVal.setMarkup("∞");
	  }
	  else {  // Display all other NaN cases as ???. For ticket 1155
		  pangoVal.setFontSize(6*fontFactor*z);
		  pangoVal.setMarkup("???");
	  }
      pangoVal.angle=angle+(notflipped? 0: M_PI);

      cairo_move_to(cairo,r.x(w-pangoVal.width()-2,-h-hoffs+2),
                    r.y(w-pangoVal.width()-2,-h-hoffs+2));
      pangoVal.show();
      if (val.engExp!=0 && (!isnan(value()))) // Avoid large exponential number in variable value display. For ticket 1155
        {
          pangoVal.setMarkup(expMultiplier(val.engExp));
          cairo_move_to(cairo,r.x(w-pangoVal.width()-2,0),r.y(w-pangoVal.width()-2,0));
          pangoVal.show();
        }
    }
    catch (...) {} // ignore errors in obtaining values

  unique_ptr<cairo::Path> clipPath;
  {
    cairo::CairoSave cs(cairo);
    cairo_rotate(cairo, angle);
    // constants and parameters should be rendered in blue, all others in red
    switch (type())
      {
      case constant: case parameter:
        cairo_set_source_rgb(cairo,0,0,1);
        break;
      default:
        cairo_set_source_rgb(cairo,1,0,0);
        break;
      }
    cairo_move_to(cairo,-w,-h);
    if (lhs())
      cairo_line_to(cairo,-w+2*z,0);
    cairo_line_to(cairo,-w,h);
    cairo_line_to(cairo,w,h);
    cairo_line_to(cairo,w+2*z,0);
    cairo_line_to(cairo,w,-h);
    cairo_close_path(cairo);
    clipPath.reset(new cairo::Path(cairo));
    cairo_stroke(cairo);
    if (type()!=constant && !ioVar())
      {
        // draw slider
        CairoSave cs(cairo);
        cairo_set_source_rgb(cairo,0,0,0);
        try
          {
            cairo_arc(cairo,(notflipped?1:-1)*z*rv.handlePos(), (notflipped? -h: h), sliderHandleRadius, 0, 2*M_PI);
          }
        catch (const error&) {} // handlePos() may throw.
        cairo_fill(cairo);
      }
  }// undo rotation

  double x0=w, y0=0, x1=-w+2, y1=0;
  double sa=sin(angle), ca=cos(angle);
  if (ports.size()>0)
    ports[0]->moveTo(x()+(x0*ca-y0*sa), 
                     y()+(y0*ca+x0*sa));
  if (ports.size()>1)
    ports[1]->moveTo(x()+(x1*ca-y1*sa), 
                     y()+(y1*ca+x1*sa));

  auto g=group.lock();
  if (mouseFocus || (ioVar() && g && g->mouseFocus))
    {
      cairo::CairoSave cs(cairo);
      drawPorts(cairo);
      displayTooltip(cairo,tooltip);
      if (onResizeHandles) drawResizeHandles(cairo);
    }  

  cairo_new_path(cairo);
  clipPath->appendToCurrent(cairo);
  cairo_clip(cairo);
  if (selected) drawSelected(cairo);
}

void VariableBase::resize(const LassoBox& b)
{
  float w=iWidth(), h=iHeight(), invZ=1/zoomFactor();
  iWidth(abs(b.x1-b.x0)*invZ);
  iHeight(abs(b.y1-b.y0)*invZ);
  moveTo(0.5*(b.x0+b.x1), 0.5*(b.y0+b.y1));
  bb.update(*this);	  
}

void VariablePtr::makeConsistentWithValue()
{
	retype(minsky::cminsky().variableValues[get()->valueId()].type());
}

int VarConstant::nextId=0;
