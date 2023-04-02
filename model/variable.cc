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

#include <boost/regex.hpp>
#include <boost/locale.hpp>
using namespace boost::locale::conv;

using namespace classdesc;
using namespace ecolab;

using namespace minsky;
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
 }
}


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
  return m_ports.size()>1 && !m_ports[1]->wires().empty();
}

std::vector<std::string> VariableBase::accessibleVars() const
{
  if (auto g=group.lock())
    return g->accessibleVars();
  return {};
}


ClickType::Type VariableBase::clickType(float xx, float yy) const
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
      if (rv.height()<0.5*iHeight()) hpy=-z*0.5*iHeight(); 
      double dx=xx-x(), dy=yy-y();         
      if (type()!=constant && hypot(dx - r.x(hpx,hpy), dy-r.y(hpx,hpy)) < 5)
        return ClickType::onSlider;
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
  auto vv=minsky().variableValues.find(valueId());
  if (vv!=minsky().variableValues.end())
    return vv->second;
  return {};
}

vector<unsigned> VariableBase::dims() const
{
  if (auto v=vValue()) return v->hypercube().dims();
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
  // ensure integral variables are not global when wired to an integral operation
  m_name=(type()==integral && name[0]==':' &&inputWired())? name.substr(1): name;
  ensureValueExists(tmpVV.get(),name);
  bb.update(*this); // adjust bounding box for new name - see ticket #704
  if (auto controllingItem=controller.lock())
    // integrals in particular may have had their size changed with intVar changing name
    controllingItem->updateBoundingBox();
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
      assert(isValueId(valueId));
      // Ensure value of variable is preserved after rename. 	      
      if (vv==nullptr)
        minsky().variableValues.emplace(valueId,VariableValuePtr(type(), name(),"")).
          first->second->m_scope=minsky::scope(group.lock(),name());
      // Ensure variable names are updated correctly everywhere they appear. 
      else
        minsky().variableValues.emplace(valueId,VariableValuePtr(type(),*vv)).first->second->name=nm;
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
                if (vv->init!=lhsVv->init) vv->init=lhsVv->init;
      }
    return value->second->init;
  }
  return "0";
}

string VariableBase::init(const string& x)
{
  ensureValueExists(nullptr,""); 
  if (isValueId(valueId()))
    {
      VariableValue& val=*minsky().variableValues[valueId()];
      val.init=x;     
      // for constant types, we may as well set the current value. See ticket #433. Also ignore errors (for now), as they will reappear at reset time.
      try
        {
          if (type()==constant || type()==parameter ||
              (type()==flow && !cminsky().definingVar(valueId()))) 
            minsky().variableValues.resetValue(val);
        }
      catch (...)
        {}
      updateBoundingBox();
    }
  return x;
}

double VariableBase::value() const
{
  if (isValueId(valueId()))
    return minsky::cminsky().variableValues[valueId()]->value();
  return 0;
}

double VariableBase::value(const double& x)
{
  if (!m_name.empty() && isValueId(valueId()))
    (*minsky().variableValues[valueId()])[0]=x;
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
      if (vv->unitsCached) return vv->units;

      
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
              if (check && units.str()!=vv->units.str())
                {
                  if (auto i=controller.lock())
                    i->throw_error("Inconsistent units "+units.str()+"≠"+vv->units.str());
                }

              if (check && controller.lock())
                {
                  FlowCoef fc(init());
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



void VariableBase::exportAsCSV(const std::string& filename) const
{
  auto value=minsky().variableValues.find(valueId());
  if (value!=minsky().variableValues.end())
    value->second->exportAsCSV(filename, name());
}

void VariableBase::importFromCSV(std::string filename, const DataSpecSchema& spec) const
{
  if (auto v=vValue()) {
    if (filename.find("://")!=std::string::npos)
      filename = v->csvDialog.loadWebFile(filename);
    std::ifstream is(filename);
    v->csvDialog.spec=spec;
    v->csvDialog.url=filename;
    loadValueFromCSVFile(*v, is, v->csvDialog.spec);
    minsky().populateMissingDimensionsFromVariable(*v);
  }
}

void VariableBase::destroyFrame()
{
  if (auto vv=vValue())
    vv->csvDialog.destroyFrame();
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
  if (!Item::visible()) return false;
  return visibleWithinGroup();
}

bool VariableBase::visibleWithinGroup() const
{
  // ensure pars, constants and flows with invisible out wires are made invisible. for ticket 1275  
  if ((type()==constant || type()==parameter) && !m_ports[0]->wires().empty())
  {
    return !std::any_of(m_ports[0]->wires().begin(),m_ports[0]->wires().end(), [](Wire* w)
                       {return w->attachedToDefiningVar() && !w->visible();});
  }  
  // ensure flow vars with out wires remain visible. for ticket 1275
    if (attachedToDefiningVar())
    {
      bool visibleOutWires=false;
      for (auto w: m_ports[0]->wires())
        visibleOutWires |= w->visible();
      if (visibleOutWires)
        return true;
    }
  if (auto i=dynamic_cast<IntOp*>(controller.lock().get()))
     if (i->attachedToDefiningVar()) return true;
  return !controller.lock();
}


void VariableBase::sliderSet(double x)
{
  if (x<sliderMin) x=sliderMin;
  if (x>sliderMax) x=sliderMax;
  sliderStep=maxSliderSteps();    
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
  sliderStep=maxSliderSteps();      
}

void VariableBase::adjustSliderBounds() const
{
  if (auto vv=vValue())
  // For feature 47
    if (vv->size()==1 && !isnan(vv->value()))  // make sure sliderBoundsSet is defined. for tickets 1258/1263
      {
        if (sliderMax<vv->value()) sliderMax=vv->value();
        if (sliderMin>vv->value()) sliderMin=vv->value();
        sliderStep=maxSliderSteps(); 
        sliderBoundsSet=true;	                    
      }
}

double VariableBase::maxSliderSteps() const
{
    // ensure there are at most 10000 steps between sliderMin and Max. for ticket 1255. 	
	if ((sliderMax-sliderMin)/sliderStep > 1.0e04) return (sliderMax-sliderMin)/1.0e04;    
	return sliderStep;
}

bool VariableBase::onKeyPress(int keySym, const std::string&,int)
{
  switch (keySym)
    {
    case 0xff52: case 0xff53: //Right, Up
        sliderSet(value()+(sliderStepRel? value(): 1)*sliderStep);
        return true;
    case 0xff51: case 0xff54: //Left, Down
        sliderSet(value()-(sliderStepRel? value(): 1)*sliderStep);
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
        o << varDAG->rhs->matlab();
      else return system.getDefFromIntVar(*this).str();
    }
          
  return o.str();	  
}

void VariableBase::draw(cairo_t *cairo) const
{	
    double angle=rotation() * M_PI / 180.0;
    double fm=std::fmod(rotation(),360);
    float z=zoomFactor();

    RenderVariable rv(*this,cairo);
    // if rotation is in 1st or 3rd quadrant, rotate as
    // normal, otherwise flip the text so it reads L->R
    bool notflipped=(fm>-90 && fm<90) || fm>270 || fm<-270;
    Rotate r(rotation() + (notflipped? 0: 180),0,0);
    rv.angle=angle+(notflipped? 0: M_PI);

    // parameters of icon in userspace (unscaled) coordinates
    float w, h, hoffs, scaleFactor;
    w=rv.width()*z; 
    h=rv.height()*z;
    scaleFactor=max(1.0f,min(0.5f*iWidth()*z/w,0.5f*iHeight()*z/h));
    if (rv.width()<0.5*iWidth()) w=0.5*iWidth()*z;
    if (rv.height()<0.5*iHeight()) h=0.5*iHeight()*z;
    rv.setFontSize(12.0*scaleFactor*z);
    hoffs=rv.top()*z;
  

    cairo_move_to(cairo,r.x(-w+1,-h-hoffs+2), r.y(-w+1,-h-hoffs+2)/*h-2*/);
    {
      CairoSave cs(cairo);
      if (local())
        cairo_set_source_rgb(cairo,0,0,1);
      rv.show();
    }

    auto vv=vValue();
  
    // For feature 47
    if (type()!=constant && !ioVar() && vv && vv->size()==1)
      try
        {
          auto val=engExp();    
          
          Pango pangoVal(cairo);
          if (!isnan(value())) {
            pangoVal.setFontSize(6.0*scaleFactor*z);
            if (sliderBoundsSet && vv->sliderVisible)
              pangoVal.setMarkup
                (mantissa(val,
                          int(1+
                              (sliderStepRel?
                               -log10(maxSliderSteps()):
                               log10(value()/maxSliderSteps())
                               ))));
            else
              pangoVal.setMarkup(mantissa(val));
          }
          else if (isinf(value())) { // Display non-zero divide by zero as infinity. For ticket 1155
            pangoVal.setFontSize(8.0*scaleFactor*z);
            if (signbit(value())) pangoVal.setMarkup("-∞");
            else pangoVal.setMarkup("∞");
          }
          else {  // Display all other NaN cases as ???. For ticket 1155
            pangoVal.setFontSize(6.0*scaleFactor*z);
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
      if (vv && vv->sliderVisible && vv->size()==1)
        {
          // draw slider
          CairoSave cs(cairo);
          cairo_set_source_rgb(cairo,0,0,0);
          try
            {
              cairo_arc(cairo,(notflipped?1.0:-1.0)*z*rv.handlePos(), (notflipped? -h: h), sliderHandleRadius, 0, 2*M_PI);
            }
          catch (const error&) {} // handlePos() may throw.
          cairo_fill(cairo);
        }
    }// undo rotation

    double x0=w, y0=0, x1=-w+2, y1=0;
    double sa=sin(angle), ca=cos(angle);
    if (!m_ports.empty())
      m_ports[0]->moveTo(x()+(x0*ca-y0*sa), 
                       y()+(y0*ca+x0*sa));
    if (m_ports.size()>1)
      m_ports[1]->moveTo(x()+(x1*ca-y1*sa), 
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
    // Rescale size of variable attached to intop. For ticket 94
    cairo_clip(cairo);
    if (selected) drawSelected(cairo);
}

void VariableBase::resize(const LassoBox& b)
{
  float invZ=1/zoomFactor();
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
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::Variable<minsky::VariableType::constant>);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::Variable<minsky::VariableType::parameter>);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::Variable<minsky::VariableType::flow>);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::Variable<minsky::VariableType::integral>);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::Variable<minsky::VariableType::stock>);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::Variable<minsky::VariableType::tempFlow>);
