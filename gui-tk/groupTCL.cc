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
  Model::~Model() 
  {
    if (model)
      {
        // reset visibility of edge variables
        for (auto& i: model->inVariables)
          i->m_visible=false;
        for (auto& i: model->outVariables)
          i->m_visible=false;
      }
  }
        
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

  namespace
  {
    int groupTCLDeleter(ClientData cd, Tcl_Interp *interp, int argc, const char **argv)
    {
      std::string s(argv[0]); 
      auto delpos=s.length()-strlen(".delete");
      assert( s.rfind(".delete")==delpos);                                 
      ecolab::TCL_obj_deregister(s.substr(0,delpos));
      delete (DeleterBase*)cd;
      return TCL_OK;                                 
    }

  }
  
  template <class M>
  void GroupTCL<M>::newGroupTCL(const std::string& name, int id)
  {
    auto i=items.find(id);
    if (i!=items.end())
      if (auto g=dynamic_pointer_cast<Group>(*i))
        {
          GroupTCL<Model>* newObj=new GroupTCL<Model>;
          newObj->model=g;
          // make edge variables visible
          for (auto& i: g->inVariables)
            i->m_visible=true;
          for (auto& i: g->outVariables)
            i->m_visible=true;
          newObj->buildMaps();
          TCL_obj(minskyTCL_obj(), name, *newObj);
          Tcl_CreateCommand(ecolab::interp(),(name+".delete").c_str(),(Tcl_CmdProc*)groupTCLDeleter, 
                            (ClientData)newObj,NULL); 
        }
  }

  template <class Model>
  void GroupTCL<Model>::newGlobalGroupTCL(const std::string& name)
  {
    TCL_obj(minskyTCL_obj(), name, *this);
    Tcl_CreateCommand(ecolab::interp(),(name+".delete").c_str(),(Tcl_CmdProc*)groupTCLDeleter, 
                            NULL
                      ,NULL);
    clearAllGetterSetters();
  }
  
  template class GroupTCL<Model>;
  template class GroupTCL<Minsky>;
}
