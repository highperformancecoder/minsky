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
#include "schema0.h"
#include "minsky_epilogue.h"
#include <fstream>
using namespace ecolab;
using namespace classdesc;
using namespace std;

namespace schema0
{
  namespace
  {
    struct IsOrphan
    {
      set<string> ids;
      void insert(const string& name) {ids.insert(name);}
      bool operator()(const VariableManager::value_type& v) const {
        return ids.contains(v.second.name);
      }
      bool operator()(const VariableManager::VariableValues::value_type& l) const {
        return ids.contains(l.first);
      }
    };
  }

  void Minsky::removeIntVarOrphans()
  {
    set<string> intNames;
    for (Operations::const_iterator o=operations.begin(); 
         o!=operations.end(); ++o)
      if (o->second.m_type==minsky::OperationType::integrate)
        {
          if (!o->second.m_description.empty())
            intNames.insert(o->second.m_description);
          else
            if (!o->second.description.empty())
              intNames.insert(o->second.description);
        }

    IsOrphan isOrphan;
    for (VariableManager::const_iterator v=variables.begin();
         v!=variables.end(); ++v)
      // an orphaned variable is an integral variable not attached to an integral and without
      if (v->second.m_type==minsky::VariableType::integral && 
          intNames.contains(v->second.name)==0)
        isOrphan.insert(v->second.name);

    for (VariableManager::iterator i=variables.begin();
         i!=variables.end();)
      {
        const VariableManager::iterator j=i++;
        if (isOrphan(*j)) variables.erase(j);
      }

    for (VariableManager::VariableValues::iterator i=variables.values.begin();
         i!=variables.values.end();)
      {
        const VariableManager::VariableValues::iterator j=i++;
        if (isOrphan(*j)) variables.values.erase(j);
      }

  }


}
