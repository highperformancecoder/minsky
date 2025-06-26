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

#include "minsky.h"
#include "geometry.h"
#include "valueId.h"
#include "variable.h"
#include "cairoItems.h"
#include "equations.h"
#include "nobble.h"

#include "dimension.rcd"
#include "engNotation.rcd"
#include "hypercube.xcd"
#include "index.xcd"
#include "itemT.rcd"
#include "slider.rcd"
#include "tensorInterface.xcd"
#include "tensorVal.rcd"
#include "tensorVal.xcd"
#include "variable.rcd"

#include <error.h>
#include "minsky_epilogue.h"

#include <algorithm>

#include <boost/locale.hpp>
#include <boost/filesystem.hpp>
using namespace boost::locale::conv;

using namespace classdesc;
using namespace ecolab;

using namespace minsky;
using namespace ecolab::cairo;

void VariableBase::addPorts()
{
#ifndef NDEBUG
  for (auto& i: m_ports)
    assert(i.use_count()==1);
#endif
  m_ports.clear();
  if (numPorts()>0)
    m_ports.emplace_back(make_shared<Port>(*this));
  for (size_t i=1; i<numPorts(); ++i)
    m_ports.emplace_back(make_shared<InputPort>(*this));
}

bool VariableBase::inputWired() const
{
  if (auto p=ports(1).lock())
    return !p->wires().empty();
  return false;
}

std::vector<std::string> VariableBase::accessibleVars() const
{
  if (auto g=group.lock())
    return g->accessibleVars();
  return {};
}


ClickType::Type VariableBase::clickType(float xx, float yy) const
{
  const Rotate r(rotation()+(flipped(rotation())? 180: 0),0,0); // rotate into variable's frame of reference
  const RenderVariable rv(*this);
  const double z=zoomFactor();
  try
    {
      const double hpx=z*rv.handlePos();
      double hpy=-z*rv.height();
      if (rv.height()<0.5*iHeight()) hpy=-z*0.5*iHeight(); 
      const double dx=xx-x(), dy=yy-y();         
      if (type()!=constant && hypot(dx - r.x(hpx,hpy), dy-r.y(hpx,hpy)) < 5)
        return ClickType::inItem;
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
  // scale by GodleyIcon::scaleFactor if part of an Godley icon
  if (auto g=dynamic_cast<GodleyIcon*>(controller.lock().get()))
    return g->scaleFactor() * Item::zoomFactor();
  return Item::zoomFactor();
}

shared_ptr<VariableValue> VariableBase::vValue() const
{
  auto vv=cminsky().variableValues.find(valueId());
  if (vv!=minsky().variableValues.end())
    return vv->second;
  return {};
}

vector<unsigned> VariableBase::dims() const
{
  try
    {
      if (auto v=vValue()) return v->hypercube().dims();
    }
  catch (...) {} // ignore any exceptions caused by evaluating RHS.
  return {};
}
    
vector<string> VariableBase::dimLabels() const
{
  if (auto v=vValue()) return v->hypercube().dimLabels();
  return {};
}    


string VariableBase::valueId() const 
{
  return minsky::valueId(group.lock(), m_name);
}

std::string VariableBase::valueIdInCurrentScope(const std::string& nm) const
{
  return minsky::valueId(group.lock(), nm);
}

bool VariableBase::local() const
{
  return m_name[0]!=':' && group.lock()!=cminsky().canvas.model;
}

string VariableBase::name()  const
{
  if (m_name==":_") return "";
  // hide any leading ':' in upper level
  if (m_name[0]==':')
    {
        return m_name.substr(1);
    }
  return utf_to_utf<char>(m_name);
}

namespace
{
  const char specialLatex[]="#$%&";
  // find and quote certain LaTeX characters
  string quoteLaTeX(const std::string& x)
  {
    string quotedName;
    int next=0;
    for (auto p=x.find_first_of(specialLatex); p!=string::npos;
         p=x.find_first_of(specialLatex,p+1))
      {
        if (p==0||x[p-1]!='\\')
          quotedName+=x.substr(next,p-next)+'\\'+x[p];
        else
          quotedName+=x.substr(next,p-next+1);
        next=p+1;
      }
    return quotedName+x.substr(next);
  }
}

string VariableBase::name(const std::string& name) 
{
  // cowardly refuse to set a blank name
  if (name.empty() || name==":") return name;

  // check if we need to quote certain characters that have meaning in LaTeX 
  for (auto p=name.find_first_of(specialLatex); p!=string::npos;
       p=name.find_first_of(specialLatex,p+1))
    if (p==0||name[p-1]!='\\')
      return this->name(quoteLaTeX(name));
  
  // Ensure value of variable is preserved after rename. For ticket 1106.	
  auto tmpVV=vValue();
  m_name=name;
  m_canonicalName=minsky::canonicalName(name);
  ensureValueExists(tmpVV.get(),name);
  assert(vValue()->valueId()==valueId());
  cachedNameRender.reset();
  bb.update(*this); // adjust bounding box for new name - see ticket #704
  if (auto controllingItem=controller.lock())
    // integrals in particular may have had their size changed with intVar changing name
    controllingItem->updateBoundingBox();
  return this->name();
}

bool VariableBase::ioVar() const
{return dynamic_cast<Group*>(controller.lock().get());}


void VariableBase::ensureValueExists(VariableValue* vv, const std::string&/* nm*/) const
{	
  string valueId=this->valueId();
  // disallow blank names
  if (valueId.length()>1 && valueId.substr(valueId.length()-2)!=":_" && 
      minsky().variableValues.count(valueId)==0)
    {
      assert(isValueId(valueId));
      // Ensure value of variable is preserved after rename. 	      
      if (vv==nullptr)
        minsky().variableValues.emplace(valueId,VariableValuePtr(type(), m_name)).
          first->second->m_scope=minsky::scope(group.lock(),m_name);
      // Ensure variable names are updated correctly everywhere they appear. 
      else
        {
          auto iter=minsky().variableValues.emplace(valueId,VariableValuePtr(type(),*vv)).first->second;
          iter->name=m_name;
          iter->m_scope=minsky::scope(group.lock(),m_name);
        }
          
    }
}

string VariableBase::init() const
{
  auto value=minsky().variableValues.find(valueId());
  if (value!=minsky().variableValues.end()) {   	
    // set initial value of int var to init value of input to second port. for ticket 1137
    if (type()==integral)
      {
        // find attached integral
        auto i=dynamic_cast<IntOp*>(controller.lock().get());
        if (!i && !m_ports[1]->wires().empty())
          i=dynamic_cast<IntOp*>(&(m_ports[1]->wires()[0]->from()->item()));
        if (i && i->portsSize()>2 && !i->ports(2).lock()->wires().empty())
          if (auto lhsVar=i->ports(2).lock()->wires()[0]->from()->item().variableCast())
            if (auto vv=vValue())
              if (auto lhsVv=lhsVar->vValue())
                // Since integral takes initial value from second port, the intVar should have the same intial value.
                if (vv->init()!=lhsVv->init()) vv->init(lhsVv->init());
      }
    return value->second->init();
  }
  return "0";
}

string VariableBase::init(const string& x)
{
  ensureValueExists(nullptr,""); 
  if (isValueId(valueId()))
    {
      VariableValue& val=*minsky().variableValues[valueId()];
      val.init(x);     
      // for constant types, we may as well set the current value. See ticket #433. Also ignore errors (for now), as they will reappear at reset time.
      try
        {
          if (type()==constant || type()==parameter ||
              (type()==flow && !cminsky().definingVar(valueId())))
            {
              minsky().variableValues.resetValue(val);
              cachedNameRender.reset(); // for constants
            }
        }
      catch (...)
        {}
      updateBoundingBox();
    }
  return x;
}

double VariableBase::value() const
{
  auto& vvs=minsky::cminsky().variableValues;
  auto v=vvs.find(valueId());
  if (v==vvs.end() || !v->second) return 0;
  return v->second->value();
}

double VariableBase::value(const double& x)
{
  if (!m_name.empty() && isValueId(valueId()))
    {
      (*minsky().variableValues[valueId()])[0]=x;
      if (type()==constant)
        cachedNameRender.reset();
    }
  return x;
}

static string emptyString;

const std::string& VariableBase::detailedText() const
{
  if (auto vv=vValue())
    return vv->detailedText;
  return emptyString  ;
}

const std::string& VariableBase::detailedText(const std::string& x)
{
    if (auto vv=vValue())
    return vv->detailedText=x;
  return emptyString  ;
}

const std::string& VariableBase::tooltip() const
{
  if (auto vv=vValue())
    return vv->tooltip;
  return emptyString  ;
}

const std::string& VariableBase::tooltip(const std::string& x)
{
    if (auto vv=vValue())
    return vv->tooltip=x;
  return emptyString  ;
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
      if (vv->unitsCached) return vv->units;

      
      const IncrDecrCounter ucIdc(unitsCtr);
      const IncrDecrCounter vpIdc(varsPassed);
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
              else // I/O variable
                units=vv->units;
              if (check && units.str()!=vv->units.str())
                {
                  if (auto i=controller.lock())
                    i->throw_error("Inconsistent units "+units.str()+"≠"+vv->units.str());
                }

              if (check && controller.lock())
                {
                  const FlowCoef fc(init());
                  if (!fc.name.empty())
                    {
                      // extract the valueid corresponding to the initialisation variable
                      auto vid=minsky::valueId(vv->m_scope.lock(), fc.name);
                      // find the first variable matching vid, and check that the units match
                      if (auto initVar=cminsky().model->findAny
                          (&GroupItems::items, [&vid](const ItemPtr& i){
                                                 if (auto v=i->variableCast())
                                                   return v->valueId()==vid;
                                                 return false;}))
                        if (units!=initVar->units(check))
                          throw_error("Inconsistent units in initial conditions");
                    }
                }
            }
        }
      else
        // updates units in the process
        if (m_ports.size()>1 && !m_ports[1]->wires().empty())
          {
            assert(m_ports[1]->wires()[0]->from());
            vv->units=m_ports[1]->wires()[0]->from()->item().units(check);
          }
        else if (auto v=cminsky().definingVar(valueId()))
          vv->units=v->units(check);

      vv->units.normalise();
      vv->unitsCached=true;
      return vv->units;
    }
  return Units();
}

void VariableBase::setUnits(const string& x) const
{
  if (isValueId(valueId()))
    minsky().variableValues[valueId()]->units=Units(x);
}



void VariableBase::exportAsCSV(const std::string& filename, bool tabular) const
{
  auto value=minsky().variableValues.find(valueId());
  if (value!=minsky().variableValues.end())
    value->second->exportAsCSV(filename, name(), tabular);
}

void VariableBase::importFromCSV(const vector<string>& filenames, const DataSpecSchema& spec) const
{
  if (auto v=vValue()) {
    v->csvDialog.spec=spec;
    if (!filenames.empty())
      v->csvDialog.url=filenames[0];
    loadValueFromCSVFile(*v, filenames, v->csvDialog.spec);
    minsky().populateMissingDimensionsFromVariable(*v);
    if (!v->hypercube().dimsAreDistinct())
      throw_error("Axes of imported data should all have distinct names");
  }
}

// TODO - save multifile selections?
void VariableBase::reloadCSV()
{
  if (auto v=vValue()) 
    if (!v->csvDialog.url.empty())
      loadValueFromCSVFile(*v, {v->csvDialog.url}, v->csvDialog.spec);
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
  const VariablePtr tmp(*this);
  if (tmp && tmp->type()!=type)
    {
      reset(VariableBase::create(type));
      static_cast<VariableBase&>(*get()) = *tmp;
      for (size_t i=0; i<get()->portsSize() && i< tmp->portsSize(); ++i)
        for (auto w: tmp->ports(i).lock()->wires())
          {
            if (get()->ports(i).lock()->input())
              w->moveToPorts(w->from(), get()->ports(i).lock());
            else
              w->moveToPorts(get()->ports(i).lock(), w->to());
          }
      get()->ensureValueExists(nullptr,"");
    }
}

bool VariableBase::visible() const
{
  auto g=group.lock();
  //toplevel i/o items always visible
  if ((!g || !g->group.lock()) && g==controller.lock()) return true;
  if (controller.lock()) return false;
  return Item::visible();
}

bool VariableBase::sliderVisible() const
{
  auto vv=vValue();
  return enableSlider() &&
    ((!vv && type()==parameter) ||
     (vv && vv->size()==1 &&
      (type()==parameter ||
       // !inputWired() short circuits the more expensive definingVar operation.
       (type()==flow && !inputWired() && !cminsky().definingVar(valueId())))));
}

void VariableBase::adjustSliderBounds() const 
{
  if (auto vv=vValue())
    vv->adjustSliderBounds();
}

bool VariableBase::onKeyPress(int keySym, const std::string&,int)
{
  switch (keySym)
    {
    case 0xff52: case 0xff53: //Right, Up
      if (auto vv=vValue()) vv->incrSlider(1);
      if (!minsky().running) minsky().requestReset();
      return true;
    case 0xff51: case 0xff54: //Left, Down
      if (auto vv=vValue()) vv->incrSlider(-1);
      if (!minsky().running) minsky().requestReset();
      return true;
    default:
      return false;
    }
}

std::string VariableBase::definition() const
{
  MathDAG::SystemOfEquations system(cminsky());	  
  ostringstream o;
	
  auto varDAG=system.getNodeFromVar(*this);
    
  if (type()!=VariableType::parameter)
    {
      if (varDAG && varDAG->rhs && varDAG->type!=VariableType::constant && varDAG->type!=VariableType::integral)
        o << varDAG->rhs->latex();
      else return system.getDefFromIntVar(*this).str();
    }
          
  return o.str();	  
}

bool VariableBase::miniPlotEnabled(bool enabled)
{
  if (enabled)
    {
      miniPlot=make_shared<ecolab::Plot>();
      miniPlot->plotType=ecolab::Plot::PlotType::bar;
    }
  else
    miniPlot=nullptr;
  return enabled;
}

void VariableBase::resetMiniPlot()
{
  if (miniPlotEnabled())
    miniPlot->clear();
}

bool VariableBase::onMouseMotion(float x, float y) 
{
  if (auto vv=vValue())
    {
      const RenderVariable rv(*this);
      const double rw=fabs(zoomFactor()*(rv.width()<iWidth()? 0.5*iWidth() : rv.width())*cos(rotation()*M_PI/180));
      const double sliderPos=(x-this->x())* (vv->sliderMax-vv->sliderMin)/rw+0.5*(vv->sliderMin+vv->sliderMax);
      const double sliderHatch=sliderPos-fmod(sliderPos,vv->sliderStep);   // matches slider's hatch marks to sliderStep value. for ticket 1258
      vv->sliderSet(sliderHatch);
    }
  // push History to prevent an unnecessary reset when
  // adjusting the slider whilst paused. See ticket #812
  minsky().pushHistory();
  if (minsky().reset_flag())
    minsky().requestReset();
  minsky().evalEquations();
  return true;
}

double VariableBase::sliderMin() const
{
  if (auto vv=vValue())
    return vv->sliderMin;
  return 0;
}


double VariableBase::sliderMin(double x) const
{
  if (auto vv=vValue())
    return vv->sliderMin=x;
  return 0;
}

double VariableBase::sliderMax() const
{
  if (auto vv=vValue())
    return vv->sliderMax;
  return 0;
}

double VariableBase::sliderMax(double x) const
{
  if (auto vv=vValue())
    return vv->sliderMax=x;
  return 0;
}

double VariableBase::sliderStep() const
{
  if (auto vv=vValue())
    return vv->sliderStep;
  return 0;
}

double VariableBase::sliderStep(double x) const
{
  if (auto vv=vValue())
    return vv->sliderStep=x;
  return 0;
}

bool VariableBase::sliderStepRel() const
{
  if (auto vv=vValue())
    return vv->sliderStepRel;
  return false;
}

bool VariableBase::sliderStepRel(bool x) const
 {
  if (auto vv=vValue())
    return vv->sliderStepRel=x;
  return false;
}
 
bool VariableBase::enableSlider() const
{
  if (auto vv=vValue())
    return vv->enableSlider;
  return false;
}
 
bool VariableBase::enableSlider(bool x) const
{
  if (auto vv=vValue())
    return vv->enableSlider=x;
  return false;
}


void VariableBase::draw(cairo_t *cairo) const
{	
  auto [angle,flipped]=rotationAsRadians();
  const float z=zoomFactor();

  // grab a thread local copy of the renderer caches, as MacOSX does
  // rendering on a different thread, and this avoids a race condition
  // when the cache is invalidated
  auto l_cachedNameRender=cachedNameRender;
  if (!l_cachedNameRender || cairo!=cachedNameRender->cairoContext())
    {
      l_cachedNameRender=cachedNameRender=std::make_shared<RenderVariable>(*this,cairo);
      l_cachedNameRender->setFontSize(12.0);
    }
    
  // if rotation is in 1st or 3rd quadrant, rotate as
  // normal, otherwise flip the text so it reads L->R
  const Rotate r(rotation() + (flipped? 180:0),0,0);
  l_cachedNameRender->angle=angle+(flipped? M_PI:0);

  // parameters of icon in userspace (unscaled) coordinates
  const double w=std::max(l_cachedNameRender->width(), 0.5f*iWidth()); 
  const double h=std::max(l_cachedNameRender->height(), 0.5f*iHeight());
  const double hoffs=l_cachedNameRender->top();
  
  unique_ptr<cairo::Path> clipPath;
  {
    const CairoSave cs(cairo);
    cairo_scale(cairo, z,z);
    cairo_move_to(cairo,r.x(-w+1,-h-hoffs+2), r.y(-w+1,-h-hoffs+2));
    {
      const CairoSave cs(cairo);
      if (local())
        cairo_set_source_rgb(cairo,0,0,1);
      l_cachedNameRender->show();
    }

    auto vv=vValue();
    if (miniPlot && vv && vv->size()==1)
      try
        {
          if (cachedTime!=cminsky().t)
            {
              cachedTime=cminsky().t;
              miniPlot->addPt(0,cachedTime,vv->value());
              miniPlot->setMinMax();
            }
          const CairoSave cs(cairo);
          cairo_translate(cairo,-w,-h);
          miniPlot->draw(cairo,2*w,2*h);
        }
      catch (...) {} // ignore errors in obtaining values

    // For feature 47
    try
      {
        if (type()!=constant && !ioVar() && vv && vv->size()==1 && vv->idxInRange())
          {
            auto l_cachedMantissa=cachedMantissa;
            auto l_cachedExponent=cachedExponent;
            if (!l_cachedMantissa || l_cachedMantissa->cairoContext()!=cairo)
              {
                l_cachedMantissa=cachedMantissa=make_shared<Pango>(cairo);
                l_cachedMantissa->setFontSize(6.0);
                l_cachedExponent=cachedExponent=make_shared<Pango>(cairo);
                l_cachedExponent->setFontSize(6.0);
                cachedValue=nan("");
              }
          
            auto val=engExp();    
            if (value()!=cachedValue)
              {
                cachedValue=value();
                if (!isnan(value())) {
                  if (sliderVisible())
                    l_cachedMantissa->setMarkup
                      (mantissa(val,
                                int(1+
                                    (vv->sliderStepRel?
                                     -log10(vv->maxSliderSteps()):
                                     log10(vv->value()/vv->maxSliderSteps())
                                     ))));
                  else
                    l_cachedMantissa->setMarkup(mantissa(val));
                }
                else if (isinf(value())) { // Display non-zero divide by zero as infinity. For ticket 1155
                  if (signbit(value())) l_cachedMantissa->setMarkup("-∞");
                  else l_cachedMantissa->setMarkup("∞");
                }
                else // Display all other NaN cases as ???. For ticket 1155
                  l_cachedMantissa->setMarkup("???");
                l_cachedExponent->setMarkup(expMultiplier(val.engExp));
              }
            l_cachedMantissa->angle=angle+(flipped? M_PI:0);
            
            cairo_move_to(cairo,r.x(w-l_cachedMantissa->width()-2,-h-hoffs+2),
                          r.y(w-l_cachedMantissa->width()-2,-h-hoffs+2));
            l_cachedMantissa->show();

            if (val.engExp!=0 && !isnan(value())) // Avoid large exponential number in variable value display. For ticket 1155
              {
                cairo_move_to(cairo,r.x(w-l_cachedExponent->width()-2,0),r.y(w-l_cachedExponent->width()-2,0));
                l_cachedExponent->show();
              }
          }
      }
    catch (...) {} // ignore errors in obtaining values

    {
      const cairo::CairoSave cs(cairo);
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
        cairo_line_to(cairo,-w+2,0);
      cairo_line_to(cairo,-w,h);
      cairo_line_to(cairo,w,h);
      cairo_line_to(cairo,w+2,0);
      cairo_line_to(cairo,w,-h);
      cairo_close_path(cairo);
      clipPath.reset(new cairo::Path(cairo));
      cairo_stroke(cairo);
      if (sliderVisible())
        {
          // draw slider
          const CairoSave cs(cairo);
          cairo_set_source_rgb(cairo,0,0,0);
          try
            {
              cairo_arc(cairo,(flipped?-1.0:1.0)*l_cachedNameRender->handlePos(), (flipped? h: -h), sliderHandleRadius, 0, 2*M_PI);
            }
          catch (const error&) {} // handlePos() may throw.
          cairo_fill(cairo);
        }
    }// undo rotation

    const double x0=z*w, y0=0, x1=-z*w+2, y1=0;
    const double sa=sin(angle), ca=cos(angle);
    if (!m_ports.empty())
      m_ports[0]->moveTo(x()+(x0*ca-y0*sa), 
                         y()+(y0*ca+x0*sa));
    if (m_ports.size()>1)
      m_ports[1]->moveTo(x()+(x1*ca-y1*sa), 
                         y()+(y1*ca+x1*sa));
  }
    
  auto g=group.lock();
  if (mouseFocus || (ioVar() && g && g->mouseFocus))
    {
      const cairo::CairoSave cs(cairo);
      drawPorts(cairo);
      displayTooltip(cairo,tooltip());
      if (onResizeHandles) drawResizeHandles(cairo);
    }  

  cairo_new_path(cairo);
  clipPath->appendToCurrent(cairo);
  // Rescale size of variable attached to intop. For ticket 94
  cairo_clip(cairo);
  if (selected) drawSelected(cairo);
}

void VariableBase::resize(const LassoBox& b)
{
  const float invZ=1/zoomFactor();
  moveTo(0.5*(b.x0+b.x1), 0.5*(b.y0+b.y1));  
  iWidth(abs(b.x1-b.x0)*invZ);
  iHeight(abs(b.y1-b.y0)*invZ);   
}

void VariablePtr::makeConsistentWithValue()
{
  retype(minsky::cminsky().variableValues[get()->valueId()]->type());
}

int VarConstant::nextId=0;

CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::VariablePtr);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::VariableBase);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::Variable<minsky::VariableType::undefined>);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::Variable<minsky::VariableType::constant>);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::Variable<minsky::VariableType::parameter>);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::Variable<minsky::VariableType::flow>);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::Variable<minsky::VariableType::integral>);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::Variable<minsky::VariableType::stock>);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::Variable<minsky::VariableType::tempFlow>);
