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
#include "minsky.h"
#include <error.h>
#include <ecolab_epilogue.h>

#include <boost/regex.hpp>

using namespace classdesc;
using namespace ecolab;

using namespace minsky;
using ecolab::array;

VariableBase::~VariableBase() {}

void VariableBase::addPorts()
{
  ports.clear();
  if (numPorts()>0)
    ports.emplace_back(new Port(*this,Port::noFlags));
  for (size_t i=1; i<numPorts(); ++i)
    ports.emplace_back
      (new Port(*this, Port::inputPort));
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
  Group* scope=group.lock().get();
  auto name=stripActive(m_name);
  if (name[0]==':')
    {
      // find maximum enclosing scope that has this same-named variable
      for (auto g=group.lock(); g; g=g->group.lock())
        for (auto i: g->items)
          if (auto v=dynamic_cast<VariableBase*>(i.get()))
            {
              auto n=stripActive(v->m_name);
              if (n==name || n==name.substr(1) /* without ':' qualifier*/)
                {
                  scope=g.get();
                  break;
                }
            }
    }
  if (!scope || scope==cminsky().model.get())
    return VariableValue::valueId(-1,name); // retain previous global var id
  else
    return VariableValue::valueId(size_t(scope), name);
}

bool VariableBase::ioVar() const 
{
  if (auto g=group.lock())
    {
      for (auto& i:g->inVariables)
        if (this==i.get()) return true;
      for (auto& i:g->outVariables)
        if (this==i.get()) return true;
    }
  return false;
}

string VariableBase::name()  const
{
  // hide any leading ':' in upper level
  if (m_name[0]==':')
    {
      auto g=group.lock();
      if (!g || g==cminsky().model)
        return m_name.substr(1);
    }
  return m_name;
}
//
//string VariableBase::fqName()  const
//{
//  if (scope.lock()==cminsky.model)
//  if (auto s=scope.lock())
//    if (s!=cminsky().model)
//      return "["+str(s->title.substr(0,10))+"]:"+m_name;
//  return ":"+m_name;
//}


string VariableBase::name(const std::string& name) 
{
//  // strip namespace, and extract scope
//  boost::regex namespaced_spec(R"((\d*)]?:(([^:])*))"); 
//  boost::smatch m;
//  if (regex_search(name, m, namespaced_spec))
//    {
//     assert(m.size()==4);
//      if (m[1].matched && m[1].length()>0)
//        {
//          int toScope;
//          sscanf(m[1].str().c_str(), "%d", &toScope);
//          setScope(toScope);
//        }
//      m_name=m[2];
//    }
//  else
//    {
//      m_name=name;
//      if (auto g=group.lock())
//        m_scope=g->id;
//    }

  m_name=name;
  ensureValueExists();
  return this->name();
}

void VariableBase::ensureValueExists() const
{
  string valueId=this->valueId();
  // disallow blank names
  if (valueId.length()>1 && valueId.substr(valueId.length()-2)!=":_" && 
      minsky().variableValues.count(valueId)==0)
    {
      assert(VariableValue::isValueId(valueId));
      minsky().variableValues.insert
        (make_pair(valueId,VariableValue(type(), name())));
    }
}


//void VariableBase::setScope(int s)
//{
////  if (s>=0 && cminsky().groupItems.count(s)==0)
////    return; //invalid scope passed
//  m_scope=s;
//  ensureValueExists();
//}

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
  ensureValueExists(); 
  if (VariableValue::isValueId(valueId()))
    {
      VariableValue& val=minsky().variableValues[valueId()];
      val.init=x;
      // for constant types, we may as well set the current value. See ticket #433
      if (type()==constant || type()==parameter) 
        val.reset(minsky().variableValues);
    }
  return x;
}

double VariableBase::value() const
{
  return minsky::cminsky().variableValues[valueId()].value();
}

double VariableBase::value(double x)
{
  if (!m_name.empty())
    {
      assert(VariableValue::isValueId(valueId()));
      minsky().variableValues[valueId()]=x;
    }
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
  template <> size_t Variable<VariableBase::integral>::numPorts() const {return 2;}
}


void VariablePtr::retype(VariableBase::Type type) 
{
  VariablePtr tmp(*this);
  if (tmp && tmp->type()!=type)
    {
      reset(VariableBase::create(type));
      for (size_t i=0; i<get()->ports.size() && i< tmp->ports.size(); ++i)
        get()->ports[i]=tmp->ports[i];
      get()->ensureValueExists();
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

void VariablePtr::makeConsistentWithValue()
{
  retype(minsky::cminsky().variableValues[get()->valueId()].type());
}


int VarConstant::nextId=0;
