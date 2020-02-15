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
//#include "schemaHelper.h"
//#include "str.h"
#include "minsky_epilogue.h"
#include <fstream>
using namespace ecolab;
using namespace classdesc;
using namespace std;

namespace schema0
{
//  // like assignment between containers, but value_types needn't be identical
//  template <class C1, class C2>
//  void asg(C1& c1, const C2& c2)
//  {
//    C1 tmp(c2.begin(), c2.end());
//    tmp.swap(c1);
//  }
//
//  void GroupIcon::updateEdgeVariables(const VariableManager& vm)
//  {
//    inVariables.clear();
//    outVariables.clear();
//    // for each edge port, we need to determine the edgeVariable
//    // associated with it, and whether it is an input or output edge
//    // variable
//    for (size_t p=0; p<m_ports.size(); ++p)
//      {
//        map<int,int>::const_iterator vid=vm.portToVariable.find(m_ports[p]);
//        if (vid!=vm.portToVariable.end())
//          {
//            VariableManager::Variables::const_iterator v=vm.find(vid->second);
//            if (v!=vm.end())
//              {
//                if (v->second.m_inPort==m_ports[p])
//                  inVariables.push_back(v->first);
//                else if (v->second.m_outPort==m_ports[p])
//                  outVariables.push_back(v->first);
//              }
//          }
//      }
//  }
//void Minsky::load(const string& filename)
//{
//  ifstream inf(filename);
//  xml_unpack_t saveFile(inf);
//  saveFile >> *this;
//
//  // if a godley table is present, and no godley icon present, copy
//  // into godleyItems, to support XML migration
//  if (godleyItems.empty() && godley.rows()>2)
//    {
//      godleyItems[0].table=godley;
//      godleyItems[0].x=godleyItems[0].y=10;
//    }
//  
//  map<int, xml_conversions::GodleyIcon> gItems;
//  xml_unpack(saveFile,"root.godleyItems", gItems);
//  
//  for (GodleyItems::iterator g=godleyItems.begin(); g!=godleyItems.end(); ++g)
//    if (g->second.flowVars.empty() && g->second.stockVars.empty())
//      {
//        xml_conversions::GodleyIcon& gicon=gItems[g->first];
//        GodleyIcon& gi=g->second;
//        asg(gi.flowVars, gicon.flowVars);
//        asg(gi.stockVars, gicon.stockVars);
//      }
//  
//  for (size_t i=0; i<groupItems.size(); ++i)
//    groupItems[i].updateEdgeVariables(variables);
//
//  removeIntVarOrphans();
//}

  namespace
  {
    struct IsOrphan
    {
      set<string> ids;
      void insert(const string& name) {ids.insert(name);}
      bool operator()(const VariableManager::value_type& v) {
        return ids.count(v.second.name);
      }
      bool operator()(const VariableManager::VariableValues::value_type& l) {
        return ids.count(l.first);
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
          intNames.count(v->second.name)==0)
        isOrphan.insert(v->second.name);

    for (VariableManager::iterator i=variables.begin();
         i!=variables.end();)
      {
        VariableManager::iterator j=i++;
        if (isOrphan(*j)) variables.erase(j);
      }

    for (VariableManager::VariableValues::iterator i=variables.values.begin();
         i!=variables.values.end();)
      {
        VariableManager::VariableValues::iterator j=i++;
        if (isOrphan(*j)) variables.values.erase(j);
      }

  }


}
