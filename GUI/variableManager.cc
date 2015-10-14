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
#include "variableManager.h"
#include "portManager.h"
#include "minsky.h"
#include <ecolab_epilogue.h>

#include <set>
#include <boost/regex.hpp>
using namespace std;
using namespace minsky;
using ecolab::array;

VariableManager& minsky::variableManager() {return minsky().variables;}

ecolab::array<int> VariableManager::visibleVariables() const
{
  ecolab::array<int> ret;
  for (Variables::const_iterator i=Variables::begin(); i!=Variables::end(); ++i)
    if ((*i)->visible) ret<<=i->id();
  return ret;
}

int VariableManager::addVariable(const VariablePtr& var, int id)
{
  // do not add the variable if already present and not referring to
  // the same variable
  if (varExists(var))
    {
      if (id==-1) return -1;
      const_iterator v=find(id);
      if (v!=end() && *v!=var) return -1;
    }
  if (id==-1)  id=minsky().getNewId();
  Super::insert(value_type(id,var));
  if (var->lhs()) portToVariable[var->inPort()]=id;
  portToVariable[var->outPort()]=id;
  if (!values.count(var->valueId())/* && !var->valueId().empty()*/)
    values.insert
      (VariableValues::value_type(var->valueId(), VariableValue(var->type(), var->fqName())));
  if(var->type()!=values[var->valueId()].type())
    throw error("variable %s has inconsistent type",var->name().c_str());

  return id;
}

bool VariableManager::varExists(const VariablePtr& var) const
{
  return portToVariable.count(var->outPort());
}

int VariableManager::newVariable(string name, VariableType::Type type)
{
  if (name.find(':')==string::npos)
    name=":"+name; // make unqualified vars global
  VariableValues::iterator v=values.find(VariableManager::valueId(name));
  if (v==values.end())
    return addVariable(VariablePtr(type,name));
  else
    return addVariable(VariablePtr(v->second.type(),name));
}

void VariableManager::erase(Variables::iterator it)
{
  portToVariable.erase((*it)->outPort());
  if ((*it)->lhs()) portToVariable.erase((*it)->inPort());
  Variables::erase(it);
}

void VariableManager::erase(int i, bool eraseIfIntegral)
{
  Variables::iterator it=find(i);
  if (it!=Variables::end()) 
    {
      // see if this is an integral variable, attached to an integral,
      // and do not remove variable if true
      if (!eraseIfIntegral && (*it)->type()==VariableType::integral)
        for (Operations::const_iterator o=minsky().operations.begin();
             o!=minsky().operations.end(); ++o)
          if (const IntOp* itg=dynamic_cast<IntOp*>(o->get()))
            if (itg->intVarID()==i)
              return; 

      // see if any other instance of this variable exists
      iterator j;
      for (j=begin(); j!=end(); ++j)
        if ((*j)->name() == (*it)->name() && 
            (*j)->outPort()!=(*it)->outPort())
          break;
      if (j==end()) // didn't find any others
        values.erase((*it)->valueId());

      if ((*it)->group>=0)
        {
          GroupIcons::iterator g=minsky().groupItems.find((*it)->group);
          if (g!=minsky().groupItems.end())
            g->removeVariable(*it);
        }
      erase(it);
    }
}

void VariableManager::erase(const VariablePtr& v)
{
  PortMap::const_iterator it=portToVariable.find(v->outPort());
  if (it!=portToVariable.end())
    erase(it->second);
}

int VariableManager::wireToVariable(const string& name) const
{
  assert(isValueId(name));
  if (!inputWired(name)) return -1;
  for (const_iterator i=begin(); i!=end(); ++i)
    if ((*i)->inPort()>-1 && (*i)->valueId()==name)
      {
        array<int> wires=cminsky().wiresAttachedToPort((*i)->inPort());
        if (wires.size()>0) 
          return wires[0];
      }
  return -1;
}

bool VariableManager::noMultipleWiredInputs() const
{
  set<string> alreadyWired;
  for (const_iterator v=begin(); v!=end(); ++v)
    if (cminsky().wiresAttachedToPort((*v)->inPort()).size()>0 &&
        !alreadyWired.insert((*v)->valueId()).second)
      return false;
  return true;
}


ecolab::array<int> VariableManager::wiresFromVariable(const string& name) const
{
  assert(isValueId(name));
  ecolab::array<int> wires;
  for (const_iterator i=begin(); i!=end(); ++i)
    if ((*i)->outPort()>-1 && (*i)->valueId()==name)
      wires<<=cminsky().wiresAttachedToPort((*i)->outPort());
  return wires;
}


void VariableManager::removeVariable(string name)
{
  assert(isValueId(name));
  for (Variables::iterator it=Variables::begin(); it!=Variables::end(); )
    if ((*it)->valueId()==name)
      erase(it++);
    else
      ++it;
  values.erase(name);
}     

bool VariableManager::addWire(int from, int to)
{
  PortMap::iterator it=portToVariable.find(to);
  if (it!=portToVariable.end())  
    {
      Variables::iterator v=find(it->second);
      if (v!=Variables::end())
        {
          // cannot have more than one wire to an input, nor self-wire a variable
          if (from==(*v)->outPort())
            return false;
          else
            return wiredVariables.insert((*v)->valueId()).second;
        }
    }
  return true;
}

void VariableManager::deleteWire(int port)
{
  PortMap::iterator it=portToVariable.find(port);
  if (it!=portToVariable.end())
    {
      Variables::iterator v=find(it->second);
      if (v!=Variables::end())
        wiredVariables.erase((*v)->valueId());
    }
}

int VariableManager::scope(const std::string& name) 
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

std::string VariableManager::uqName(const std::string& name)
{
  string::size_type p=name.rfind(':');
  if (p==string::npos)
    return name;
  else
    return name.substr(p+1);
}


int VariableManager::getVariableIDFromPort(int port) const
{
  PortMap::const_iterator it=portToVariable.find(port);
  if (it!=portToVariable.end())
    return it->second;
  else
    return -1;
}

const VariablePtr& VariableManager::getVariableFromPort(int port) const
{
  PortMap::const_iterator it=portToVariable.find(port);
  if (it!=portToVariable.end())
    {
      Variables::const_iterator v=find(it->second);
      if (v!=Variables::end())
        return *v;
    }
  static VariablePtr undef;
  return undef;
}

void VariableManager::reset()
{
  // reallocate all variables
  ValueVector::stockVars.clear();
  ValueVector::flowVars.clear();
  for (VariableValues::iterator v=values.begin(); v!=values.end(); ++v)
    v->second.allocValue().reset(values);
}

void VariableManager::makeConsistent()
{
  // remove variableValues not in variables
  set<string> existingNames;
  existingNames.insert("constant:zero");
  existingNames.insert("constant:one");
  for (iterator i=begin(); i!=end(); ++i)
    existingNames.insert((*i)->valueId());
  for (VariableValues::iterator i=values.begin(); i!=values.end(); )
    if (existingNames.count(i->first))
      ++i;
    else
      values.erase(i++);

  // ensure Godley table variables are the correct types
  for (auto g: minsky().godleyItems) g.update();

  // regenerate portToVariable
  portToVariable.clear();
  for (Variables::iterator i=Variables::begin(); i!=Variables::end(); ++i)
    {
      if ((*i)->inPort()>-1)
        portToVariable[(*i)->inPort()]=i->id();
      if ((*i)->outPort()>-1)
        portToVariable[(*i)->outPort()]=i->id();
    }

  // regenerated wireVariables
  wiredVariables.clear();
  for (PortManager::Wires::iterator w=cminsky().wires.begin();
       w!=minsky().wires.end(); ++w)
    {
      PortMap::iterator v=portToVariable.find(w->to);
      if (v!=portToVariable.end())
        wiredVariables.insert((*this)[v->second]->valueId());
    }

  // anything variable attached to an integral must also be wired.
  // also checks for, and eliminates duplicate entries
  set<string> intVarNames;
  for (OperationPtr o: minsky().operations)
    if (auto i=dynamic_cast<IntOp*>(o.get()))
      if (auto v=i->getIntVar())
        {
          if (!intVarNames.insert(v->valueId()).second)
            {
              i->newName();
              i->setDescription();
            }
          if (v->type()!=VariableType::integral) 
            convertVarType(v->valueId(), VariableType::integral);
          wiredVariables.insert(v->valueId());
        }
}

void VariableManager::clear()
{
  Variables::clear();
  wiredVariables.clear();
  portToVariable.clear();
  values.clear();
}

string VariableManager::valueNames() const
{
  string names;
  // TODO boost regex's used because std::regex not properly implemented in GCC
  boost::regex braces("[{}]"); // used to quote braces
  for (VariableValues::const_iterator i=values.begin(); i!=values.end(); ++i)
    names+=" {"+boost::regex_replace(i->second.name, braces, "\\\\$&")+"}";
  return names;
}

void VariableManager::makeVarConsistentWithValue(int id)
{
  iterator v=find(id);
  if (v!=end())
    v->retype(values[(*v)->valueId()].type());
}

void VariableManager::convertVarType(const string& name, VariableType::Type type)
{
  assert(isValueId(name));
  VariableValues::iterator i=values.find(name);
  if (i==values.end())
    throw error("variable %s doesn't exist",name.c_str());
  if (i->second.type()==type) return; // nothing to do!

  for (auto g: minsky().godleyItems)
    {
      if (type!=VariableType::flow)
        for (auto v: g.flowVars)
          if (v->valueId()==name)
            throw error("flow variables in Godley tables cannot be converted to a different type");
      if (type!=VariableType::stock)
        for (auto v: g.stockVars)
          if (v->valueId()==name)
            throw error("stock variables in Godley tables cannot be converted to a different type");
    }

  if (inputWired(name)) 
    throw error("cannot convert a variable whose input is wired");

  // filter out invalid targets
  switch (type)
    {
    case VariableType::undefined: case VariableType::numVarTypes: 
    case VariableType::tempFlow:
      throw error("convertVarType not supported for type=%s",
                  VariableType::typeName(type).c_str());
    default: break;
    }

  // convert all references
  for (iterator v=begin(); v!=end(); ++v)
    if ((*v)->valueId()==name)
      v->retype(type);
  i->second=VariableValue(type,i->second.name,i->second.init);
}
