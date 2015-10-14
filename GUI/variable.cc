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
#include "portManager.h"
#include "variable.h"
#include "variableManager.h"
#include "minsky.h"
#include "ecolab_epilogue.h"

#include <boost/regex.hpp>

using namespace classdesc;

using namespace minsky;
using ecolab::array;

void VariablePorts::addPorts()
{
  m_ports.clear();
  m_ports.resize(numPorts(),-1);
  if (numPorts()>0)
    m_ports[0] = minsky().addPort(Port(x(),y(),false));
  if (numPorts()>1)
    m_ports[1] = minsky().addPort(Port(x(),y(),true));
}

void VariablePorts::delPorts()
{
  for (auto& p: ports())
    if (p>-1) minsky().delPort(p);
  m_ports.clear();
}

void VariablePorts::swapPorts(VariablePorts& v)
{
  int n=min(numPorts(), v.numPorts());
  if (n>0) swap(m_ports[0], v.m_ports[0]);
  if (n>1) swap(m_ports[1], v.m_ports[1]);
}

void VariablePorts::toggleInPort()
{
  if (type()==integral)
    {
      if (m_ports.size()<2)
        m_ports.resize(2,-1);
      if (m_ports[1]==-1)
        m_ports[1] = minsky().addPort(Port(x(),y(),true));
      else 
        {
          minsky().delPort(m_ports[1]);
          m_ports[1]=-1;
        }
    }
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

string VariableBase::valueId() const 
{
  return VariableManager::valueId(m_scope, m_name);
}

string VariableBase::name()  const
{
  if (m_scope==group)
    return m_name;
  return fqName();
}

string VariableBase::fqName()  const
{
  if (m_scope==-1)
    return ":"+m_name;
  else if (cminsky().groupItems.count(m_scope))
    return cminsky().groupItems[m_scope].name().substr(0,5)+"["+str(m_scope)+"]:"+m_name;
  else
    return "["+str(m_scope)+"]:"+m_name;
}


string VariableBase::name(const std::string& name) 
{
  // strip namespace, and extract scope
  boost::regex namespaced_spec(R"((\d*)]?:(([^:])*))"); 
  boost::smatch m;
  if (regex_search(name, m, namespaced_spec))
    {
      m_scope=-1;
      assert(m.size()==4);
      if (m[1].matched && m[1].length()>0)
        {
          int toScope;
          sscanf(m[1].str().c_str(), "%d", &toScope);
          setScope(toScope);
        }
      m_name=m[2];
    }
  else
    {
      m_name=name;
      setScope(group);
    }

  ensureValueExists();
  return this->name();
}

void VariableBase::ensureValueExists() const
{
  string valueId=this->valueId();
  // disallow blank names
  if (valueId.substr(valueId.length()-2)!=":_" && variableManager().values.count(valueId)==0)
    variableManager().values.insert
      (make_pair(valueId,VariableValue(type(), fqName())));
}


void VariableBase::setScope(int s)
{
  if (s>=0 && cminsky().groupItems.count(s)==0)
    return; //invalid scope passed
  m_scope=s;
  ensureValueExists();
}

string VariableBase::init() const
{
  auto value=variableManager().values.find(valueId());
  if (value!=variableManager().values.end())
    return value->second.init;
  else
    return "0";
}

string VariableBase::init(const string& x)
{
  ensureValueExists(); 
  VariableValue& val=minsky().variables.getVariableValue(valueId());
  val.init=x;
  // for constant types, we may as well set the current value. See ticket #433
  if (type()==constant || type()==parameter) 
    val.reset(minsky().variables.values);
  return x;
}

double VariableBase::value() const
{
  return minsky::minsky().variables.getVariableValue(valueId()).value();
}

double VariableBase::value(double x)
{
  if (!m_name.empty())
    minsky().variables.getVariableValue(valueId())=x;
  return x;
}


namespace minsky
{
template <> size_t Variable<VariableBase::undefined>::numPorts() const {return 0;}
template <> size_t Variable<VariableBase::constant>::numPorts() const {return 1;}
template <> size_t Variable<VariableBase::parameter>::numPorts() const {return 1;}
template <> size_t Variable<VariableBase::flow>::numPorts() const {return 2;}
template <> size_t Variable<VariableBase::stock>::numPorts() const {return 1;}
template <> size_t Variable<VariableBase::tempFlow>::numPorts() const {return 2;}
template <> size_t Variable<VariableBase::integral>::numPorts() const 
{
  return inPort()<0? 1: 2;
}
}


void VariableBase::zoom(float xOrigin, float yOrigin,float factor)
{
  if (visible)
    {
      if (group==-1)
        {
          ::zoom(m_x,xOrigin,factor);
          ::zoom(m_y,yOrigin,factor);
        }
      else
        {
          m_x*=factor;
          m_y*=factor;
        }
      zoomFactor*=factor;
    }
}

void VariablePtr::retype(VariableBase::Type type) 
{
  VariablePtr tmp(*this);
  if (tmp && tmp->type()!=type)
    {
      reset(VariableBase::create(type));
      /// gnargh! ports need special handling to avoid clobbering
      /// TODO: fix in refactoring
      auto& thisAsVPorts=dynamic_cast<VariablePorts&>(**this);
      auto& tmpAsVPorts=dynamic_cast<VariablePorts&>(*tmp);
      thisAsVPorts=tmpAsVPorts;
      thisAsVPorts.swapPorts(tmpAsVPorts);
      (*this)->ensureValueExists();
    }
}

void VariableBase::initSliderBounds()
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

void VariableBase::adjustSliderBounds()
{
  if (sliderMax<value()) sliderMax=value();
  if (sliderMin>value()) sliderMin=value();
}

int VarConstant::nextId=0;
