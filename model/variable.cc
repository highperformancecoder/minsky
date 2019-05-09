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
#include <error.h>
#include <ecolab_epilogue.h>

#include <boost/regex.hpp>

using namespace classdesc;
using namespace ecolab;

using namespace minsky;
using ecolab::array;
using namespace ecolab::cairo;

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
  double fm=std::fmod(rotation,360);
  bool notflipped=(fm>-90 && fm<90) || fm>270 || fm<-270;
  float xxx=xx-x(), yyy=yy-y();
  Rotate r(rotation+(notflipped? 0: 180),0,0); // rotate into variable's frame of reference
  RenderVariable rv(*this);
  double z=zoomFactor();
  double hpx=z*rv.handlePos();
  double hpy=-z*rv.height();
  if (type()!=constant && hypot(xx-x() - r.x(hpx,hpy), yy-y()-r.y(hpx,hpy)) < 5)
      return ClickType::onSlider;
  else
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

string VariableBase::_name()  const
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

string VariableBase::_name(const std::string& name) 
{
  // cowardly refuse to set a blank name
  if (name.empty() || name==":") return name;
  // ensure integral variables are not global when wired to an integral operation
  m_name=(type()==integral && name[0]==':' &&inputWired())? name.substr(1): name;
  ensureValueExists();
  bb.update(*this); // adjust bounding box for new name - see ticket #704
  return this->name();
}

bool VariableBase::ioVar() const
{return dynamic_cast<Group*>(controller.lock().get());}


void VariableBase::ensureValueExists() const
{
  string valueId=this->valueId();
  // disallow blank names
  if (valueId.length()>1 && valueId.substr(valueId.length()-2)!=":_" && 
      minsky().variableValues.count(valueId)==0)
    {
      assert(VariableValue::isValueId(valueId));
      minsky().variableValues.insert
        (make_pair(valueId,VariableValue(type(), name(), "", group.lock())));
    }
}


string VariableBase::_init() const
{
  auto value=minsky().variableValues.find(valueId());
  if (value!=minsky().variableValues.end())
    return value->second.init;
  else
    return "0";
}

string VariableBase::_init(const string& x)
{
  ensureValueExists(); 
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

double VariableBase::_value() const
{
  if (VariableValue::isValueId(valueId()))
    return minsky::cminsky().variableValues[valueId()].value();
  else
    return 0;
}

double VariableBase::_value(double x)
{
  if (!m_name.empty() && VariableValue::isValueId(valueId()))
    minsky().variableValues[valueId()]=x;
  return x;
}

int VariableBase::stockVarsPassed=0;
int VariableBase::varsPassed=0;

Units VariableBase::units() const
{
  if (varsPassed==0) minsky().variableValues.resetUnitsCache(); 
  // we allow possible traversing twice, to allow
  // stock variable to break the cycle
  if (unitsCtr-stockVarsPassed>=1)
    throw_error("Cycle detected on wiring network");
  if (isStock() && unitsCtr)
    return {}; // stock var cycles back on itself, normalise to dimensionless

  auto it=minsky().variableValues.find(valueId());
  if (it!=minsky().variableValues.end())
    {
      auto& vv=it->second;
      if (vv.unitsCached) return vv.units;
      
      IncrDecrCounter ucIdc(unitsCtr);
      IncrDecrCounter vpIdc(varsPassed);
      // use a unique ptr here to only increment counter inside a stockVar
      unique_ptr<IncrDecrCounter> svp;
      if (isStock()) svp.reset(new IncrDecrCounter(stockVarsPassed));

      // updates units in the process
      if (ports.size()>1 && !ports[1]->wires().empty())
        vv.units=ports[1]->wires()[0]->from()->item.units();
      else if (auto v=cminsky().definingVar(valueId()))
        vv.units=v->units();
      else if (auto i=dynamic_cast<IntOp*>(controller.lock().get()))
        vv.units=i->units();
      else if (auto g=dynamic_cast<GodleyIcon*>(controller.lock().get()))
        ; //TODO evaluate Godley stock variable
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
  // reset minsky model to propagate units
  try {minsky().reset();} catch (...) {}
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
      get()->ensureValueExists();
    }
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
    if (vv->numElements()==1)
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
  double angle=rotation * M_PI / 180.0;
  double fm=std::fmod(rotation,360);
  float z=zoomFactor();

  RenderVariable rv(*this,cairo);
  rv.setFontSize(12*z);
  // if rotation is in 1st or 3rd quadrant, rotate as
  // normal, otherwise flip the text so it reads L->R
  bool notflipped=(fm>-90 && fm<90) || fm>270 || fm<-270;
  Rotate r(rotation + (notflipped? 0: 180),0,0);
  rv.angle=angle+(notflipped? 0: M_PI);

  // parameters of icon in userspace (unscaled) coordinates
  float w, h, hoffs;
  w=rv.width()*z; 
  h=rv.height()*z;
  hoffs=rv.top()*z;

  cairo_move_to(cairo,r.x(-w+1,-h-hoffs+2), r.y(-w+1,-h-hoffs+2)/*h-2*/);
  rv.show();

  VariableValue vv;
  if (VariableValue::isValueId(valueId()))
    vv=minsky::cminsky().variableValues[valueId()];
  
  if (type()!=constant && !ioVar() && vv.numElements()==1 )
    try
    {
      auto val=engExp();
  
      Pango pangoVal(cairo);
      pangoVal.setFontSize(6*z);
      pangoVal.setMarkup(mantissa(val));
      pangoVal.angle=angle+(notflipped? 0: M_PI);

      cairo_move_to(cairo,r.x(w-pangoVal.width()-2,-h-hoffs+2),
                    r.y(w-pangoVal.width()-2,-h-hoffs+2));
      pangoVal.show();
      if (val.engExp!=0)
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
    }

  cairo_new_path(cairo);
  clipPath->appendToCurrent(cairo);
  cairo_clip(cairo);
  if (selected) drawSelected(cairo);
}

void VariablePtr::makeConsistentWithValue()
{
  retype(minsky::cminsky().variableValues[get()->valueId()].type());
}


int VarConstant::nextId=0;
