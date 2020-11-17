/*
  @copyright Steve Keen 2020
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

#include "autoLayout.h"
#include "selection.h"
#include <map>
#include <vector>
#include <boost/graph/fruchterman_reingold.hpp>
#include <boost/graph/topology.hpp>
#include <boost/graph/directed_graph.hpp>
#include "minsky_epilogue.h"

using namespace std;
using boost::square_topology;
using namespace minsky;

namespace minsky
{
  void randomizeLayout(Group&) {/* TODO */}

  namespace {
    struct GroupGraph
    {
      vector<Item*> vertices;
      vector<pair<Item*,Item*>> edges;
      GroupGraph(const Group& g) {
        for (auto& i: g.items) vertices.push_back(i.get());
        for (auto& i: g.groups) vertices.push_back(i.get());
        for (auto& i: g.wires) edges.emplace_back(&i->from()->item(), &i->to()->item());
      }
    };

    
  }

  void layoutGroup(Group& g)
  {
    using Graph=boost::directed_graph<>;
    Graph gg;
    map<Item*, decltype(gg.add_vertex())> vertexMap;
    for (auto& i: g.items)
      vertexMap.emplace(i.get(), gg.add_vertex()); 
    for (auto& i: g.groups)
      vertexMap.emplace(i.get(), gg.add_vertex()); 

    for (auto& w: g.wires)
      gg.add_edge(vertexMap[&w->from()->item()], vertexMap[&w->to()->item()]);

    using Topology=square_topology<>;
    
    using PosMap=std::map<decltype(gg.add_vertex()), Topology::point_type>;
    PosMap positions;
    boost::associative_property_map<PosMap> pm(positions);

    for (auto& i: vertexMap)
      {
        Topology::point_type p;
        p[0]=i.first->x();
        p[1]=i.first->y();
        positions[i.second]=p;
      }
    
    boost::fruchterman_reingold_force_directed_layout(gg,pm, Topology(1000));
    // move items to result of algorithm
    for (auto& i: g.items)
      {
        auto& p=pm[vertexMap[i.get()]];
        i->moveTo(p[0],p[1]);
      }
    for (auto& i: g.groups)
      {
        auto& p=pm[vertexMap[i.get()]];
        i->moveTo(p[0],p[1]);
      }
    for (auto& i: g.groups)
      layoutGroup(*i);
  }
}

