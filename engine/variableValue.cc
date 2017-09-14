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
        ValueVector::flowVars.resize(ValueVector::flowVars.size()+1,0);
        //      *this=init;
        break;
      case stock:
      case integral:
        m_idx=ValueVector::stockVars.size();
        ValueVector::stockVars.resize(ValueVector::stockVars.size()+1);
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
        assert(size_t(m_idx)<ValueVector::flowVars.size());
        return ValueVector::flowVars[m_idx];
      case stock:
      case integral:
        assert(size_t(m_idx)<ValueVector::stockVars.size());
        return ValueVector::stockVars[m_idx];
      default: break;
      }
    throw error("invalid access of variable value reference");
  }

  double VariableValue::initValue
  (const VariableValues& v, set<string>& visited) const
  {
    FlowCoef fc(init);
    if (trimWS(fc.name).empty())
      return fc.coef;
    else
      {
        if (visited.count(fc.name))
          throw error("circular definition of initial value for %s",
                      fc.name.c_str());
        VariableValues::const_iterator vv=v.end();
        if (isValueId(fc.name)) vv=v.find(valueId(fc.name));
        if (vv==v.end())
          throw error("Unknown variable %s in initialisation of %s",fc.name.c_str(), name.c_str());
        else
          {
            visited.insert(fc.name);
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

  string VariableValue::valueId(shared_ptr<Group> scope, string a_name)
  {
    auto name=stripActive(a_name);
    if (name[0]==':')
      {
        // find maximum enclosing scope that has this same-named variable
        for (auto g=scope; g; g=g->group.lock())
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
      trialName=name+str(i++);
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

}
