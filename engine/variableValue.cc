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
#include "variableValue.h"
#include "flowCoef.h"
#include "str.h"
#include "minsky.h"
#include <ecolab_epilogue.h>
#include <error.h>

using namespace ecolab;
using namespace std;
namespace minsky
{
  std::vector<double> ValueVector::stockVars(1);
  std::vector<double> ValueVector::flowVars(1);

  VariableValue& VariableValue::allocValue()
  {
    switch (m_type)
      {
      case undefined:
        m_idx=-1;
        break;
      case flow:
      case tempFlow:
      case constant:
      case parameter:
        m_idx=ValueVector::flowVars.size();
        ValueVector::flowVars.resize
          (ValueVector::flowVars.size()+numElements());
        //      *this=init;
        break;
      case stock:
      case integral:
        m_idx=ValueVector::stockVars.size();
        ValueVector::stockVars.resize(ValueVector::stockVars.size()+numElements());
        //     *this=init;
        break;
      default: break;
      }
    return *this;
  }

  double& VariableValue::valRef()
  {
    if (m_idx==-1)
      allocValue();
    switch (m_type)
      {
      case flow:
      case tempFlow:
      case constant:
      case parameter:
        if (size_t(m_idx)<ValueVector::flowVars.size())
          return ValueVector::flowVars[m_idx];
      case stock:
      case integral:
        if (size_t(m_idx)<ValueVector::stockVars.size())
          return ValueVector::stockVars[m_idx];
      default: break;
      }
    throw error("invalid access of variable value reference: %s",name.c_str());
  }

  double VariableValue::initValue
  (const VariableValues& v, set<string>& visited) const
  {
    FlowCoef fc(init);
    if (trimWS(fc.name).empty())
      return fc.coef;
    else
      {
        // resolve name
        auto valueId=VariableValue::valueId(m_scope.lock(), fc.name);
        if (visited.count(valueId))
          throw error("circular definition of initial value for %s",
                      fc.name.c_str());
        VariableValues::const_iterator vv=v.end();
        vv=v.find(valueId);
        if (vv==v.end())
          throw error("Unknown variable %s in initialisation of %s",fc.name.c_str(), name.c_str());
        else
          {
            visited.insert(valueId);
            return fc.coef*vv->second.initValue(v, visited);
          }
      }
  }

  void VariableValue::reset(const VariableValues& v)
  {
    if (m_idx<0) allocValue(); 
    operator=(initValue(v));
  }


  int VariableValue::scope(const std::string& name) 
  {
    boost::smatch m;
    if (boost::regex_search(name, m, boost::regex(R"((\d*)]?:.*)")))
      if (m.size()>1 && m[1].matched && !m[1].str().empty())
        {
          int r;
          sscanf(m[1].str().c_str(),"%d",&r);
          return r;
        }
      else
        return -1;
    else
      // no scope information is present
      throw error("scope requested for local variable");
  }

  GroupPtr VariableValue::scope(GroupPtr scope, const std::string& a_name)
  {
    auto name=stripActive(a_name);
    if (name[0]==':' && scope)
      {
        // find maximum enclosing scope that has this same-named variable
        for (auto g=scope->group.lock(); g; g=g->group.lock())
          for (auto i: g->items)
            if (auto v=dynamic_cast<VariableBase*>(i.get()))
              {
                auto n=stripActive(v->name());
                if (n==name.substr(1)) // without ':' qualifier
                  {
                    scope=g;
                    goto break_outerloop;
                  }
              }
        scope.reset(); // global var
      break_outerloop: ;
      }
    return scope;
  }
  
  
  string VariableValue::valueIdFromScope(const GroupPtr& scope, const std::string& name)
  {
    if (!scope || !scope->group.lock())
      return VariableValue::valueId(-1,name); // retain previous global var id
    else
      return VariableValue::valueId(size_t(scope.get()), name);
}
  
  std::string VariableValue::uqName(const std::string& name)
  {
    string::size_type p=name.rfind(':');
    if (p==string::npos)
      return name;
    else
    return name.substr(p+1);
  }
 
  string VariableValues::newName(const string& name) const
  {
    int i=1;
    string trialName;
    do
      trialName=name+to_string(i++);
    while (count(VariableValue::valueId(trialName)));
    return trialName;
  }

  void VariableValues::reset()
  {
    // reallocate all variables
    ValueVector::stockVars.clear();
    ValueVector::flowVars.clear();
    for (auto& v: *this)
      v.second.allocValue().reset(*this);
}

  bool VariableValues::validEntries() const
  {
    for (auto& v: *this)
      if (!v.second.isValueId(v.first))
        return false;
    return true;
  }


  EngNotation engExp(double v) 
  {
    EngNotation r;
    r.sciExp=(v!=0)? floor(log(fabs(v))/log(10)): 0;
    r.engExp=r.sciExp>=0? 3*(r.sciExp/3): 3*((r.sciExp+1)/3-1);
    return r;
  }

  string mantissa(double value, const EngNotation& e)
  {
    const char* conv;
    switch (e.sciExp-e.engExp)
      {
      case -3: conv="%7.4f"; break;
      case -2: conv="%6.3f"; break;
      case 0: case -1: conv="%5.2f"; break;
      case 1: conv="%5.1f"; break;
      case 2: conv="%5.0f"; break;
      default: return ""; // shouldn't be here...
      }
    char val[10];
    sprintf(val,conv,value*pow(10,-e.engExp));
    return val;
  }
  
  string expMultiplier(int exp)
  {return exp!=0? "×10<sup>"+std::to_string(exp)+"</sup>": "";}

  void VariableValue::makeXConformant(const VariableValue& a)
  {
    if (xVector.empty())
      xVector=a.xVector;
    else
      {
        set<string> alabels(a.xVector.begin(), a.xVector.end());
        vector<string> newLabels;
        for (auto i: xVector)
          if (alabels.count(i))
            newLabels.push_back(i);
        xVector.swap(newLabels);
      }
  }
  
}
