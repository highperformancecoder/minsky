/*
  @copyright Steve Keen 2016
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

#include "groupTCL.h"
//#include "minskyTCLObj.h"
#include <ecolab.h>
#include <ecolab_epilogue.h>

namespace minsky
{
  template <class Model>
  void GroupTCL<Model>::checkAddGroup(int id, float x, float y)
  {
    auto i=items.find(id);
    if (i!=items.end())
      {
        if (auto g=Model::model->minimalEnclosingGroup(x,y,x,y))
          {
            if (dynamic_cast<Group*>(i->get())!=g && (*i)->group.lock().get()!=g)
              g->addItem(*i);
          }
        else if ((*i)->group.lock()!=Model::model)
          Model::model->addItem(*i);
      }
  }

  namespace 
  {
    template <class W>
    void adjustWire(const W& w)
    {
      tclcmd cmd;
      cmd |".wiring.canvas coords wire"|w.id();
      for (auto x: w->coords())
        cmd <<x;
      cmd << "\n";
    }
  }

  template <class Model>
  void GroupTCL<Model>::adjustItemWires(Item* it)
  {
    tclcmd cmd;
    for (auto& w: wires)
      if (&w->from()->item == it || &w->to()->item == it)
        adjustWire(w);
      else if (auto g=dynamic_cast<Group*>(it))
        {
          for (auto v: g->inVariables)
            if (&w->to()->item == v.get())
              adjustWire(w);
          for (auto v: g->outVariables)
            if (&w->to()->item == v.get())
              adjustWire(w);
        }
  }

  template <class Model>
  void GroupTCL<Model>::adjustWires(int id) 
  {
    auto it=items.find(id);
    if (it!=items.end())
      {
        if (auto g=dynamic_cast<GodleyIcon*>(it->get()))
          {
            for (auto& v: g->stockVars)
              adjustItemWires(&*v);
            for (auto& v: g->flowVars)
              adjustItemWires(&*v);
          }
        else
          adjustItemWires(it->get());
      }
  }

  template <class Model>
  int GroupTCL<Model>::groupOf(int item)
  {
    auto i=items.find(item);
    if (i!=items.end())
      if (auto g=(*i)->group.lock())
        for (auto& j: items)
          if (j.get()==g.get())
            return j.id();
    return -1;
  }

  template class GroupTCL<Minsky>;
}
