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
#ifndef VARIABLEMANAGER_H
#define VARIABLEMANAGER_H

#include "variable.h"
#include "variableValue.h"
#include "intrusiveMap.h"

#include <map>
#include <set>
#include <arrays.h>

#include <boost/regex.hpp>
#include<assert.h>

namespace minsky
{
  using ecolab::array;

  /**
     Variables have certain global constraints. Variables with the same
     name refer to the same numerical value (m_idx are identical), and must
     have only one input across the range of instances of the same name.

     A constant can be a variable with no input
  */
  // public inheritance for debugging, and scripting convenience: should be private
  class VariableManager: public TrackedIntrusiveMap<int,VariablePtr>
  {
    typedef TrackedIntrusiveMap<int,VariablePtr> Super;
  public:
    CLASSDESC_ACCESS(VariableManager);
    friend struct SchemaHelper;
    typedef IntrusiveMap<int,VariablePtr> Variables;
    typedef std::map<int, int> PortMap;
    typedef std::set<std::string> WiredVariables;

  private:
    WiredVariables wiredVariables; /// variables whose input port is wired
    PortMap portToVariable; /// map of ports to variables

    VariableValue undefined;

    void erase(Variables::iterator it);
    // hide insertion
    void insert(int) {}
  public:
    VariableValues values;

    /// remove ability for operator[] to insert non existent values
    /// use addVariable or newVariable instead
    VariablePtr operator[](int i) const {
      if (i<0)
        throw ecolab::error("invalid variable id %d",i);        
      iterator it=find(i);
      if (it!=end())
        return *it;
      else
        throw ecolab::error("invalid variable id %d",i);
    }

    /// returns list of human readable variable names
    std::string valueNames() const;

    /// set of ids of variable that are icons in their own right
    ecolab::array<int> visibleVariables() const;

    // set/get an initial value
    void setInit(const std::string& name, const std::string& val)
    {
      assert(isValueId(name));
      if (values.count(name)>0) values[name].init=val;
    }

    /// add a variable to this manager. if \a id==-1, then use the next available id
    /// @return variable id
    int addVariable(const VariablePtr& var, int id=-1);
    /// returns true if variable has already been added
    bool varExists(const VariablePtr& var) const;
    /// creates a new variable and adds it. If a variable of the same
    /// name already exists, that type is used, otherwise a new
    /// variable of type \a type is created.
    int newVariable(std::string name, VariableType::Type type);
    /// Remove variable i. If \a eraseIfIntegral is true, then disable
    /// check for attached integral, useful when called from IntOp
    /// destructor
    void erase(int i, bool eraseIfIntegral=false);
    void erase(const VariablePtr&);

    bool inputWired(const std::string& name) const {
      assert(isValueId(name));
      return wiredVariables.count(name);
    }

    /// Invariant suitable for assertions: no variables have multiple
    /// wires to their inputs
    bool noMultipleWiredInputs() const;
    /// returns wire connecting to the variable
    int wireToVariable(const std::string& name) const;
    /// returns a list of wires emanating from the variable
    ecolab::array<int> wiresFromVariable(const std::string& name) const;

    /// TCL helper to check if a variable already exists by the same name
    bool exists(std::string name) const {
      if (name.find(':')==std::string::npos) 
        name=":"+name; // make unqualified vars global
     return values.count(name);
    }
    /// remove all instances of variable with valueId \a name
    void removeVariable(std::string name);
    /// returns true if wire can successfully connect to port
    bool addWire(int from, int to);
    /// deletes wire
    void deleteWire(int port);
    /// return ID for variable owning \a port. Returns -1 if no such
    /// variable is registered.
    int getVariableIDFromPort(int port) const;

    /// check that name is a valid valueId (useful for assertions)
    static bool isValueId(const std::string& name)
    {return boost::regex_match(name, boost::regex(R"((constant)?\d*:[^:\s\\{}]*)"));}
    //{return boost::regex_match(name, boost::regex(R"(\d*:.*)"));}

    /// construct a valueId
    static std::string valueId(int scope, std::string name) {
      if (scope<0) return ":"+stripActive(uqName(name));
      else return str(scope)+":"+stripActive(uqName(name));
    }
    static std::string valueId(std::string name) {
      return valueId(scope(name), name);
    }

    /// extract scope from a qualified name
    /// @throw if name is unqualified
    static int scope(const std::string& name);
    /// extract unqualified portion of name
    static std::string uqName(const std::string& name);

    /// return reference to variable owning \a port. Returns a default
    /// constructed VariablePtr if no such variable is registered
    const VariablePtr& getVariableFromPort(int port) const;
    VariableValue& getVariableValue(const std::string& name) {
      assert(isValueId(name));
      VariableValues::iterator v=values.find(name);
      if (v!=values.end()) return v->second;
      return undefined;
    }
    VariableValue& getVariableValueFromPort(int port)  {
      return getVariableValue(getVariableFromPort(port)->valueId());
    }

    const VariableValue& getVariableValue(const std::string& name) const {
      assert(isValueId(name));
      VariableValues::const_iterator v=values.find(name);
      if (v!=values.end()) return v->second;
      return undefined;
    }
    const VariableValue& getVariableValueFromPort(int port) const {
      return getVariableValue(getVariableFromPort(port)->valueId());
    }

    int getIDFromVariable(const VariablePtr& v) const {
      PortMap::const_iterator i=portToVariable.find(v->outPort());
      if (i==portToVariable.end()) return -1;
      else return i->second;
    }

    // return list of stock (or integration) variables
    std::vector<std::string> stockVars() const {
      std::vector<std::string> r;
      for (VariableValues::const_iterator v=values.begin();
           v!=values.end(); ++v)
        if (!v->second.isFlowVar()) r.push_back(v->first);
      return r;
    }

    /// reallocates variables in ValueVector, and set value back to init
    void reset();

    /// scans variable, wire & port definitions to correct any inconsistencies
    /// - useful after a load to correct corrupt xml files
    void makeConsistent();

    /// clears all owned data structures
    void clear();

    /// make variable consistent with value (same type etc)
    void makeVarConsistentWithValue(int id);

    /// Converts variable(s) named by \a name into a variable of type \a type.
    /// @throw if conversion is disallowed
    void convertVarType(const std::string& name, VariableType::Type type);

  };

  /// global variablemanager
  VariableManager& variableManager();
}

#ifdef _CLASSDESC
#pragma omit pack minsky::VariableManager::iterator
#pragma omit unpack minsky::VariableManager::iterator
#pragma omit xml_pack minsky::VariableManager::iterator
#pragma omit xml_unpack minsky::VariableManager::iterator
#endif
#include "variableManager.cd"
#endif

