/*
  @copyright Steve Keen 2021
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

#ifndef VARIABLEVALUES_H
#define VARIABLEVALUES_H

#include "variableValue.h"
#include "constMap.h"

namespace ecolab
{
  // for TCL_obj processing
  template <> struct is_map<minsky::ConstMap<std::string, minsky::VariableValue> >: public is_map_map {};
}

namespace minsky
{
  class Group;
  typedef std::shared_ptr<Group> GroupPtr;
  /// a shared_ptr that default constructs a default target, and is always valid
  struct VariableValuePtr: public std::shared_ptr<VariableValue>
  {
    VariableValuePtr(VariableType::Type type=VariableType::undefined, const std::string& name="", const GroupPtr& group=GroupPtr()):
      std::shared_ptr<VariableValue>(std::make_shared<VariableValue>(type,name))
    {get()->m_scope=scope(group,name);}
    VariableValuePtr(VariableType::Type type, const VariableValue& vv): std::shared_ptr<VariableValue>(std::make_shared<VariableValue>(type,vv)) {}
    VariableValuePtr(const std::shared_ptr<VariableValue>& x): std::shared_ptr<VariableValue>(x) {assert(x);}
  };

  class VariableValues: public ConstMap<std::string, VariableValuePtr>
  {
    static VariableValuePtr& zero();
    static VariableValuePtr& one();
    CLASSDESC_ACCESS(VariableValues);
  public:
    VariableValues() {clear();}
    void clear() {
      ConstMap<std::string, mapped_type>::clear();
      // add special values for zero and one, used for the derivative
      // operator in SystemOfEquations
      emplace("constant:zero", zero());
      emplace("constant:one", one());
    }
    VariableValuePtr addTempVar()
    {
      auto name=newName(":temp");
      return emplace(name,VariableValuePtr(VariableType::tempFlow,name)).first->second;}
    /// generate a new valueId not otherwise in the system
    std::string newName(const std::string& name) const;
    /// reset all variableValues to their initial conditions
    void reset();
    /// reset a give variable value to it's initial condition, in this context
    void resetValue(VariableValue&) const; 
    /// evaluates the initial value of a given variableValue in the
    /// context given by this. \a visited is used to check for
    /// circular definitions
    TensorVal initValue
    (const VariableValue&, std::set<std::string>& visited) const;
    TensorVal initValue(const VariableValue& v) const {
      std::set<std::string> visited;
      return initValue(v, visited);
    }

    std::vector<Summary> summarise() const {
      std::vector<Summary> summary;
      for (auto& v: *this)
        summary.emplace_back(v.second->summary());
      return summary;
    }
    
    /// checks that all entry names are valid
    bool validEntries() const;
    void resetUnitsCache() {
      for (auto& i: *this)
        i.second->unitsCached=false;
    }
  };
}

namespace classdesc
{
  template <>
  struct is_associative_container<minsky::ConstMap<std::string, minsky::VariableValue> >: public classdesc::true_type {};
  // allows for RESTProcess to recognise this as a smart pointer, and to process the target
  template <> struct is_smart_ptr<minsky::VariableValuePtr>: public true_type {};
}

#include "variableValues.cd"
#endif
