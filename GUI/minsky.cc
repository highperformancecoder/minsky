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
#include "classdesc_access.h"
#include "minsky.h"
#include "cairoItems.h"

#include "TCL_obj_stl.h"
#include <gsl/gsl_errno.h>
#include <gsl/gsl_odeiv2.h>

#include <schema/schema0.h>
#include <schema/schema1.h>

using namespace minsky;
using namespace classdesc;

namespace
{
  const char* schemaURL="http://minsky.sf.net/minsky";

  inline bool isFinite(const double y[], size_t n)
  {
    for (size_t i=0; i<n; ++i)
      if (!isfinite(y[i])) return false;
    return true;
  }

  /*
    For using GSL Runge-Kutta routines
  */

  int RKfunction(double t, const double y[], double f[], void *params)
  {
    if (params==NULL) return GSL_EBADFUNC;
    try
      {
        ((Minsky*)params)->evalEquations(f,t,y);
      }
    catch (std::exception& e)
      {
        Tcl_AppendResult(interp(),e.what(),NULL);
        Tcl_AppendResult(interp(),"\n",NULL);
        return GSL_EBADFUNC;
      }
    return GSL_SUCCESS;
  }

  int jacobian(double t, const double y[], double * dfdy, double dfdt[], void * params)
  {
    if (params==NULL) return GSL_EBADFUNC;
    Minsky::Matrix jac(ValueVector::stockVars.size(), dfdy);
    try
      {
        ((Minsky*)params)->jacobian(jac,t,y);
      }
     catch (std::exception& e)
      {
        Tcl_AppendResult(interp(),e.what(),NULL);
        Tcl_AppendResult(interp(),"\n",NULL);
        return GSL_EBADFUNC;
      }   
    return GSL_SUCCESS;
  }
}

namespace minsky
{

  struct RKdata
  {
    gsl_odeiv2_system sys;
    gsl_odeiv2_driver* driver;

    static void errHandler(const char* reason, const char* file, int line, int gsl_errno) {
      throw error("gsl: %s:%d: %s",file,line,reason);
    }

    RKdata(Minsky* minsky) {
      gsl_set_error_handler(errHandler);
      sys.function=RKfunction;
      sys.jacobian=jacobian;
      sys.dimension=ValueVector::stockVars.size();
      sys.params=minsky;
      const gsl_odeiv2_step_type* stepper;
      switch (minsky->order)
        {
        case 1: 
          if (!minsky->implicit)
            throw error("First order explicit solver not available");
          stepper=gsl_odeiv2_step_rk1imp;
          break;
        case 2: 
          stepper=minsky->implicit? gsl_odeiv2_step_rk2imp: gsl_odeiv2_step_rk2;
          break;
        case 4:
          stepper=minsky->implicit? gsl_odeiv2_step_rk4imp: gsl_odeiv2_step_rkf45;
          break;
        default:
          throw error("order %d solver not supported",minsky->order);
        }
      driver = gsl_odeiv2_driver_alloc_y_new
        (&sys, stepper, minsky->stepMax, minsky->epsAbs, 
         minsky->epsRel);
      gsl_odeiv2_driver_set_hmax(driver, minsky->stepMax);
      gsl_odeiv2_driver_set_hmin(driver, minsky->stepMin);
    }
    ~RKdata() {gsl_odeiv2_driver_free(driver);}
  };
}

#include "minskyVersion.h"

#include <ecolab_epilogue.h>


#include <algorithm>
using namespace std;

namespace minsky
{
  void Minsky::openLogFile(const string& name)
  {
    outputDataFile.reset(new ofstream(name));
    *outputDataFile<< "#time";
    for (VariableValues::const_iterator v=variables.values.begin();
         v!=variables.values.end(); ++v)
      *outputDataFile<<" "<<v->first;
    *outputDataFile<<endl;
  }

  /// write current state of all variables to the log file
  void Minsky::logVariables() const
  {
    if (outputDataFile)
      {
        *outputDataFile<<t;
        for (VariableValues::const_iterator v=variables.values.begin();
             v!=variables.values.end(); ++v)
          *outputDataFile<<" "<<v->second.value();
        *outputDataFile<<endl;
      }
  }        
        
      

  void Minsky::clearAllMaps()
  {
    PortManager::clear(); 
    godleyItems.clear();
    // groupItems must be cleared before clearing operations and
    // variables to prevent a destructor cycle from occurring
    groupItems.clear();
    operations.clear();
    variables.clear();
    variables.values.clear();
    plots.clear();
    notes.clear();
    switchItems.clear();
    // these two are needed, because they could be holding onto
    // operations or variables, which might cause port deletions in
    // later models
    equations.clear();
    integrals.clear();
    
    flowVars.clear();
    stockVars.clear();
    evalGodley.initialiseGodleys(makeGodleyIt(godleyItems.begin()),
        makeGodleyIt(godleyItems.end()), variables.values);

#ifdef NDEBUG
    nextId=0;
#else
    resetNextId();
    assert(nextId==0);
#endif

    reset_needed=true;
  }


  const char* Minsky::minskyVersion=MINSKY_VERSION;

  int Minsky::addWire(int from, int to, const array<float>& coords) {
    if (!ports.count(from) || !ports.count(to))
      return -1;
    // wire must go from an output port to an input port
    if (ports[from].input() || !ports[to].input())
      return -1;

    // check we're not wiring an operator to its input
    // TODO: move this into an operationManager class
    for (Operations::value_type& o: operations)
      if (o->selfWire(from, to))
        return -1;

    // check that multiple input wires are only to binary ops.
    if (wiresAttachedToPort(to).size()>=1 && !ports[to].multiWireAllowed())
      return -1;

    // check whether variable manager will allow the connection
    if (!variables.addWire(from, to))
      return -1;

    // check that a wire doesn't already exist connecting these two ports
    for (Wires::const_iterator w=wires.begin(); w!=wires.end(); ++w)
      if (w->from==from && w->to==to)
        return -1;

    Wire w(from, to);
    w.coords(coords);
    if (w.coords().size()<4)
      return -1;

    int id=addWire(w);

    // work out which group to add the wire to (if any)
    groupTest.initGroupList(groupItems);
    array<float> c=w.coords();
    int g=groupTest.groupContainingBoth(c[0],c[1],c[c.size()-2],c[c.size()-1]);
    if (g>-1) groupItems[g].addWire(id);

    markEdited();
    return id;
  }

  void Minsky::deleteWire(int id)
  {
    if (wires.count(id))
      {
        variables.deleteWire(wires[id].to);
        Wire& w=wires[id];
        if (w.group!=-1)
          groupItems[w.group].delWire(id);
        PortManager::deleteWire(id);
        markEdited();
      }
  }


  int Minsky::addOperation(const char* o)
  {
    OperationPtr newOp(static_cast<OperationType::Type>
                       (enumKey<OperationType::Type>(o)));
    if (!newOp) return -1;
    int id=getNewId();
    operations.insert(Operations::value_type(id, newOp));
    markEdited();
    return id;
  }

  int Minsky::copyOperation(int id)
  {
    Operations::iterator source=operations.find(id);
    if (source==operations.end()) return -1;
    int newId=getNewId();
    OperationPtr newOp = (*source)->clone();
    operations.insert(Operations::value_type(newId, newOp));
    markEdited();
    return newId;
  }


  void Minsky::deleteOperation(int opid)
  {
    Operations::iterator op=operations.find(opid);
    if (op!=operations.end())
      {
        equations.clear(); // remove any reference held by evalOps
        if ((*op)->group>=0)
        {
          GroupIcons::iterator g=minsky().groupItems.find((*op)->group);
          if (g!=minsky().groupItems.end())
            g->removeOperation(*op);
        }
        operations.erase(op);
        markEdited();
      }
  }

  int Minsky::createGroup()
  {
    int id=getNewId();
    GroupIcon& g=groupItems[id];
    g.addWires(currentSelection.wires);
    if (currentSelection.group>=0 && groupItems.count(currentSelection.group))
      {
        GroupIcon& oldg=groupItems[currentSelection.group];
        for (int w: currentSelection.wires)
          oldg.delWire(w);
      }

    for (int i: currentSelection.operations)
      addOperationToGroup(id, i);
    for (int i: currentSelection.variables)
      addVariableToGroup(id, i, false /* don't check IO regions */);
    for (int i: currentSelection.groups)
      addGroupToGroup(id, i);
  
    if (g.empty())
      {
        groupItems.erase(id);
        return -1;
      }

    // make width & height slightly smaller than contentBounds
    float x0,y0,x1,y1;
    g.contentBounds(x0,y0,x1,y1);
    g.width=0.95*abs(x1-x0); 
    g.height=0.95*abs(y1-y0);

    g.centreIconOnContents();
    g.computeDisplayZoom();
    g.updatePortLocation();

    markEdited();

    if (currentSelection.group>=0) 
      addGroupToGroup(currentSelection.group, id);

    return id;
  }

  void Minsky::ungroup(int id)
  {
    groupItems[id].ungroup();
    groupItems.erase(id);
    markEdited();
  }

  void Minsky::clearSelection()
  {
    currentSelection.clear();
    for (const VariablePtr& v: variables)
      v->selected=false;
    for (const OperationPtr& o: operations)
      o->selected=false;
    for (GroupIcon& g: groupItems)
      g.selected=false;
    for (GodleyIcon& g: godleyItems)
      g.selected=false;
    for (PlotWidget& p: plots)
      p.selected=false;
  }


  void Minsky::select(float x0, float y0, float x1, float y1)
  {
    LassoBox lasso(x0,y0,x1,y1);
    clearSelection();

    currentSelection.group=-1;

    for (const Wire& w: wires)
      if (w.group==-1 && w.visible && lasso.contains(w))
        {
          assert(w.id()>-1);
          currentSelection.wires.push_back(w.id());
          
        }  

    for (const VariablePtr& v: variables)
      if (v->group==-1 && v->visible && lasso.intersects(*v))
        {
           assert(v.id()>-1);
           currentSelection.variables.push_back(v.id());
           v->selected=true;
        }

    for (const OperationPtr& o: operations)
      if (o->group==-1 && o->visible && lasso.intersects(*o))
        {
          assert(o.id()>-1);
          currentSelection.operations.push_back(o.id());
          o->selected=true;
        }

    for (GroupIcon& g: groupItems)
      if (g.group()==-1 && lasso.intersects(g))
        {
          assert(g.id()>-1);
          currentSelection.groups.push_back(g.id());
          g.selected=true;
        }

    for (GodleyIcon& g: godleyItems)
      if (lasso.intersects(g))
        {
          assert(g.id()>-1);
          currentSelection.godleys.push_back(g.id());
          g.selected=true;
        }

    for (PlotWidget& p: plots)
      if (lasso.intersects(p))
        {
          assert(p.id()>=0);
          currentSelection.plots.push_back(p.id());
          p.selected=true;
        }

    copy();
  }

  void Minsky::deleteGroup(int i)
  {
    GroupIcons::iterator g=groupItems.find(i);
    if (g!=groupItems.end())
      {
        for (int i: g->operations())
          deleteOperation(i);
        for (int i: g->variables())
          deleteVariable(i);
        for (int i: g->groups())
          deleteGroup(i);
        groupItems.erase(g);
      }
  }

  void Minsky::cut()
  {
    copy();
    for (int i: currentSelection.operations)
      deleteOperation(i);
    for (int i: currentSelection.variables)
      deleteVariable(i);
    for (int i: currentSelection.groups)
      deleteGroup(i);
    for (int i: currentSelection.godleys)
      deleteGodleyTable(i);
    for (int i: currentSelection.plots)
      deletePlot(i);
    clearSelection();
  }

  void Minsky::copy() const
  {
    // TODO - maybe use a minsky::Minsky object instead, particularly
    // once real references are deployed
    schema1::Minsky m(*this,currentSelection);
    ostringstream os;
    xml_pack_t packer(os, schemaURL);
    xml_pack(packer, "Minsky", m);
    putClipboard(os.str());
  }

  void Minsky::saveSelectionAsFile(const string& fileName) const
  {
    schema1::Minsky m(*this,currentSelection);
    ofstream os(fileName);
    xml_pack_t packer(os, schemaURL);
    xml_pack(packer, "Minsky", m);
  }

  void Minsky::saveGroupAsFile(int i, const string& fileName) const
  {
    schema1::Minsky m(*this,groupItems[i]);
    ofstream os(fileName);
    xml_pack_t packer(os, schemaURL);
    xml_pack(packer, "Minsky", m);
  }

  int Minsky::paste()
  {
    istringstream is(getClipboard());
    xml_unpack_t unpacker(is);
    schema1::Minsky m;
    xml_unpack(unpacker, "Minsky", m);
    int id=getNewId();
    m.populateGroup(groupItems[id]);
    return id;
  }

  void Minsky::toggleSelected(ItemType itemType, int item)
  {
    //TODO: individually add or remove item from selection
  }


  int Minsky::copyGroup(int id)
  {
    GroupIcons::iterator srcIt=groupItems.find(id);
    if (srcIt==groupItems.end()) return -1; //src not found
    int newId=getNewId();
    GroupIcon& g=groupItems[newId];
    g.copy(*srcIt);
    markEdited();
    return newId;
  }

  int Minsky::insertGroupFromFile(const char* file)
  {
    schema1::Minsky currentSchema;
    ifstream inf(file);
    xml_unpack_t saveFile(inf);
    xml_unpack(saveFile, "Minsky", currentSchema);

    if (currentSchema.version != currentSchema.schemaVersion)
      throw error("Invalid Minsky schema file");

    int newId=getNewId();
    GroupIcon& g=groupItems[newId];
    currentSchema.populateGroup(g);
    // all variables should be set to group scope, as this has come in from a file, so shouldn't reference 
    // outside of itself
    g.rehostGlobalVars(newId);

    return newId;
  }

  array<int> Minsky::unwiredOperations() const
  {
    array<int> ret;
    for (const Operations::value_type& op: operations)
      for (size_t i=0; i<op->numPorts(); ++i)
        if (wiresAttachedToPort(op->ports()[i]).size()==0)
          {
            ret<<=op.id();
            break;
          }
    return ret;
  }

  int Minsky::copyVariable(int id)
  {
    if (variables.count(id)>0)
      {
        VariablePtr v(variables[id]->clone());
        v->visible=true; // a copied variable should always be visible!
        v->group=-1; // toplevel until placed
        v->sliderVisible=false; // sliders should start out invisible
        int id=variables.addVariable(v);
        markEdited();
        return id;
      }
    else
      return -1;
  }



  void Minsky::garbageCollect()
  {
    stockVars.clear();
    flowVars.clear();

    // remove all temporaries
    for (VariableValues::iterator v=variables.values.begin(); 
         v!=variables.values.end();)
      if (v->second.temp())
        variables.values.erase(v++);
      else
        ++v;

    variables.makeConsistent();

    set<int> portsToKeep;

    for (const VariableManager::value_type& v: variables)
      {
        auto pp=v->ports();
        portsToKeep.insert(pp.begin(), pp.end());
      }

    for (const Operations::value_type& o: operations)
      {
        auto pp=o->ports();
        portsToKeep.insert(pp.begin(), pp.end());
      }

    for (auto& o: switchItems)
      {
        auto pp=o->ports();
        portsToKeep.insert(pp.begin(), pp.end());
      }

    for (Plots::iterator pl=plots.begin(); pl!=plots.end(); ++pl)
      {
        auto pp=pl->ports();
        portsToKeep.insert(pp.begin(), pp.end());
      }

    removeUnusedPorts(portsToKeep);
    variables.reset();
  }

  void Minsky::renderEquationsToImage(const char* image)
  {
    ecolab::cairo::TkPhotoSurface surf(Tk_FindPhoto(interp(),image));
    //    surf.clear();
    cairo_move_to(surf.cairo(),0,0);
    MathDAG::SystemOfEquations system(*this);
    system.renderEquations(surf);
    surf.blit();
  }

  void Minsky::constructEquations()
  {
    if (cycleCheck()) throw error("cyclic network detected");
    garbageCollect();
    equations.clear();
    integrals.clear();

    MathDAG::SystemOfEquations system(*this);
    map<int,VariableValue> portValMap;
    system.populateEvalOpVector(equations, integrals, portValMap);

    inputFrom.clear();
    for (Wire w: wires)
      {
        map<int,VariableValue>::iterator i=portValMap.find(w.from);
        if (i!=portValMap.end())
          inputFrom[w.to] = i->second;
      }

    // attach the plots
    for (PlotWidget& p: plots)
      {
        p.yvars.clear(); // clear any old associations
        p.xvars.clear(); 
        p.clearPenAttributes();
        p.autoScale();
        for (size_t i=0; i<p.ports().size(); ++i)
          {
            map<int,VariableValue>::iterator vFrom=inputFrom.find(p.ports()[i]);
            if (vFrom!=inputFrom.end())
              p.connectVar(vFrom->second, i);
          }
      }
    for (EvalOpVector::iterator e=equations.begin(); e!=equations.end(); ++e)
      (*e)->reset();
  }

  std::set<string> Minsky::matchingTableColumns(int currTable, GodleyAssetClass::AssetClass ac)
  {
    std::set<string> r;
    // matching liability with assets and vice-versa
    switch (ac)
      {
      case GodleyAssetClass::liability:
        ac=GodleyAssetClass::asset;
        break;
      case GodleyAssetClass::asset:
        ac=GodleyAssetClass::liability;
        break;
      default:
        return r; // other types do not match anything
      }

    std::set<string> duplicatedColumns;
    for (GodleyItems::iterator gi=godleyItems.begin(); 
         gi!=godleyItems.end(); ++gi)
      {
        vector<string> columns=gi->table.getColumnVariables();
        for (size_t i=0; i<columns.size(); ++i)
          {
//            if (columns[i].find(':')==string::npos) 
//              // local variable, need to qualify
//              columns[i]=gi->table.title.substr(0,5)+"["+str(gi->id())+"]:"+columns[i];
            if (gi->id()==currTable || r.count(columns[i]) || gi->table._assetClass(i+1)!=ac) 
              {
                r.erase(columns[i]); // column already duplicated, or in current, nothing to match
                duplicatedColumns.insert(columns[i]);
              }
            else if (!duplicatedColumns.count(columns[i]))
              r.insert(columns[i]);
          }
      }
    return r;
  }

  void Minsky::importDuplicateColumn(const GodleyTable& srcTable, int srcCol)
  {
    // find any duplicate column, and use it to do balanceDuplicateColumns
    const string& colName=trimWS(srcTable.cell(0,srcCol));
    if (colName.empty()) return; //ignore blank columns

    for (GodleyItems::iterator gi=godleyItems.begin(); gi!=godleyItems.end(); ++gi)
      if (&gi->table!=&srcTable) // skip source table
        for (size_t col=1; col<gi->table.cols(); col++)
          if (trimWS(gi->table.cell(0,col))==colName) // we have a match
            balanceDuplicateColumns(*gi, col);
  }

  void Minsky::balanceDuplicateColumns(const GodleyIcon& srcGodley, int srcCol)
  {
    const GodleyTable& srcTable=srcGodley.table;
    // find if there is a matching column
    const string& colName=srcGodley.valueId(trimWS(srcTable.cell(0,srcCol)));
    if (colName.empty()) return; //ignore blank columns

    bool matchFound=false;
    for (GodleyItems::iterator gi=godleyItems.begin(); gi!=godleyItems.end(); ++gi)
      if (&gi->table!=&srcTable) // skip source table
        for (size_t col=1; col<gi->table.cols(); col++)
          if (gi->valueId(trimWS(gi->table.cell(0,col)))==colName) // we have a match
            {
              if (matchFound)
                throw error("more than one duplicated column detected for %s",colName.c_str());
              matchFound=true;

              GodleyTable& destTable=gi->table;


              // reverse lookup tables for mapping flow variable to destination row numbers via row labels
              map<string,string> srcRowLabels;
              map<string, int> destRowLabels;
              for (size_t row=1; row!=srcTable.rows(); ++row)
                if (!srcTable.initialConditionRow(row) && !srcTable.cell(row,0).empty() &&
                    !srcTable.cell(row,srcCol).empty())
                  {
                    FlowCoef fc(srcTable.cell(row,srcCol));
                    if (!fc.name.empty())
                      srcRowLabels[srcGodley.valueId(fc.name)]=
                        trimWS(srcTable.cell(row,0));
                  }
                else if (srcTable.initialConditionRow(row))
                  // copy directly into destination initial condition,
                  // reversing sign
                  for (size_t r=1; r<destTable.rows(); ++r)
                    if (destTable.initialConditionRow(r))
                      {
                        FlowCoef fc(srcTable.cell(row,srcCol));
                        destTable.cell(r,col)=str(-fc.coef)+fc.name;
                        break;
                      }
              for (size_t row=1; row!=destTable.rows(); ++row)
                if (!destTable.initialConditionRow(row) && !destTable.cell(row,0).empty())
                  destRowLabels[trimWS(destTable.cell(row,0))]=row;


              // compute column signature for both src and destination columns
              map<string,double> srcFlows=srcGodley.flowSignature(srcCol), 
                destFlows=gi->flowSignature(col);
              // items to add
              for (map<string,double>::iterator i=srcFlows.begin(); i!=srcFlows.end(); ++i)
                if (i->second != -destFlows[i->first])
                  {
                    int scope=-1;
                    if (i->first.find(':')!=string::npos)
                      VariableManager::scope(i->first);
                    FlowCoef df;
                    if (scope==-1 || !variables.values.count(i->first))
                      df.name=VariableManager::uqName(i->first);
                    else
                      df.name=variables.values[i->first].name;
                    df.coef=-i->second-destFlows[i->first];
                    string flowEntry=df.str();
                    string rowLabel=srcRowLabels[srcGodley.valueId(i->first)];
                    map<string,int>::iterator dr=destRowLabels.find(rowLabel);
                    if (dr!=destRowLabels.end())
                      if (destTable.cell(dr->second, col).empty())
                        destTable.cell(dr->second, col) = flowEntry;
                      else
                        // add a new blank labelled flow line
                        {
                          destTable.resize(destTable.rows()+1,destTable.cols());
                          destTable.cell(destTable.rows()-1, col) = flowEntry;
                        }
                    else
                      // labels don't match, so add a new labelled line
                      {
                        destTable.resize(destTable.rows()+1,destTable.cols());
                        destTable.cell(destTable.rows()-1, 0) = rowLabel;
                        destRowLabels[rowLabel] = destTable.rows()-1;
                        destTable.cell(destTable.rows()-1, col) = flowEntry;
                      }
                  }
              // items to delete
              for (map<string,double>::iterator i=destFlows.begin(); i!=destFlows.end(); ++i)
                if (i->second!=0 && srcFlows[i->first]==0)
                  for (size_t row=1; row<destTable.rows(); ++row)
                    {
                      FlowCoef fc(destTable.cell(row, col));
                      if (!fc.name.empty())
                        fc.name=gi->valueId(fc.name);
                      if (fc.name==gi->valueId(i->first))
                        destTable.cell(row, col).clear();
                    }
            }   
  }

  void Minsky::reset()
  {
    EvalOpBase::t=t=0;
    constructEquations();
    // if no stock variables in system, add a dummy stock variable to
    // make the simulation proceed
    if (stockVars.empty()) stockVars.resize(1,0);

    evalGodley.initialiseGodleys(makeGodleyIt(godleyItems.begin()), 
                                 makeGodleyIt(godleyItems.end()), variables.values);

    plots.reset();

    if (stockVars.size()>0)
      {
        if (order==1 && !implicit)
          ode.reset(); // do explicit Euler
        else
          ode.reset(new RKdata(this)); // set up GSL ODE routines
      }

    reset_needed=false;
    // update flow variable
    for (size_t i=0; i<equations.size(); ++i)
      equations[i]->eval(&flowVars[0], &stockVars[0]);
  }

  void Minsky::step()
  {
    if (reset_needed)
      reset();


    if (ode)
      {
        gsl_odeiv2_driver_set_nmax(ode->driver, nSteps);
        int err=gsl_odeiv2_driver_apply(ode->driver, &t, numeric_limits<double>::max(), 
                                        &stockVars[0]);
        switch (err)
          {
          case GSL_SUCCESS: case GSL_EMAXITER: break;
          case GSL_FAILURE:
            throw error("unspecified error GSL_FAILURE returned");
          case GSL_EBADFUNC: 
            gsl_odeiv2_driver_reset(ode->driver);
            throw error("Invalid arithmetic operation detected");
          default:
            throw error("gsl error: %s",gsl_strerror(err));
          }
      }
    else // do explicit Euler method
      {
        vector<double> d(stockVars.size());
        for (int i=0; i<nSteps; ++i, t+=stepMax)
          {
            evalEquations(&d[0], t, &stockVars[0]);
            for (size_t j=0; j<d.size(); ++j)
              stockVars[j]+=d[j];
          }
      }

    // update flow variables
    for (size_t i=0; i<equations.size(); ++i)
      equations[i]->eval(&flowVars[0], &stockVars[0]);

    logVariables();

    for (Plots::iterator i=plots.begin(); i!=plots.end(); ++i)
      i->addPlotPt(t);
  }

  string Minsky::diagnoseNonFinite() const
  {
    // firstly check if any variables are not finite
    for (VariableValues::const_iterator v=variables.values.begin();
         v!=variables.values.end(); ++v)
      if (!isfinite(v->second.value()))
        return v->first;

    // now check operator equations
    for (EvalOpVector::const_iterator e=equations.begin(); e!=equations.end(); ++e)
      if (!isfinite(flowVars[(*e)->out]))
        return OperationType::typeName((*e)->type());
    return "";
  }

  void Minsky::evalEquations(double result[], double t, const double vars[])
  {
    EvalOpBase::t=t;
    // firstly evaluate the flow variables. Initialise to flowVars so
    // that no input vars are correctly initialised
    vector<double> flow(flowVars);
    for (size_t i=0; i<equations.size(); ++i)
      equations[i]->eval(&flow[0], vars);

    // then create the result using the Godley table
    for (size_t i=0; i<stockVars.size(); ++i) result[i]=0;
    evalGodley.eval(result, &flow[0]);
    // integrations are kind of a copy
    for (vector<Integral>::iterator i=integrals.begin(); i<integrals.end(); ++i)
      {
        if (i->input.idx()<0)
          {
            if (i->operation)
              displayErrorItem(*i->operation);
            throw error("integral not wired");
          }
        result[i->stock.idx()] = i->input.isFlowVar()? flow[i->input.idx()]: vars[i->input.idx()];
      }
  }

  void Minsky::jacobian(Matrix& jac, double t, const double sv[])
  {
    EvalOpBase::t=t;
    // firstly evaluate the flow variables. Initialise to flowVars so
    // that no input vars are correctly initialised
    vector<double> flow=flowVars;
    for (size_t i=0; i<equations.size(); ++i)
      equations[i]->eval(&flow[0], sv);

    // then determine the derivatives with respect to variable j
    for (size_t j=0; j<stockVars.size(); ++j)
      {
        vector<double> ds(stockVars.size()), df(flowVars.size());
        ds[j]=1;
        for (size_t i=0; i<equations.size(); ++i)
          equations[i]->deriv(&df[0], &ds[0], sv, &flow[0]);
        vector<double> d(stockVars.size());
        //        evalGodley.eval(&d[0], &df[0]);
        for (vector<Integral>::iterator i=integrals.begin(); 
             i!=integrals.end(); ++i)
          {
            assert(i->stock.idx()>=0 && i->input.idx()>=0);
            d[i->stock.idx()] = 
              i->input.isFlowVar()? df[i->input.idx()]: ds[i->input.idx()];
          }
        for (size_t i=0; i<stockVars.size(); i++)
          jac(i,j)=d[i];
      }
  
  }

  void Minsky::save(const std::string& filename)
  {
    ofstream of(filename);
    xml_pack_t saveFile(of, schemaURL);
    saveFile.prettyPrint=true;
    schema1::Minsky m(*this);
    m.relocateCanvas();
    xml_pack(saveFile, "Minsky", m);
    if (!of)
      throw runtime_error("cannot save to "+filename);
    m_edited=false;
  }


  void Minsky::load(const std::string& filename) 
  {
  
    clearAllMaps();

    // current schema
    schema1::Minsky currentSchema;
    ifstream inf(filename);
    if (!inf)
      throw runtime_error("failed to open "+filename);
    xml_unpack_t saveFile(inf);
    xml_unpack(saveFile, "Minsky", currentSchema);
    // fix corruption caused by ticket #329
    currentSchema.removeIntVarOrphans();

    if (currentSchema.version == currentSchema.schemaVersion)
      *this = currentSchema;
    else
      { // fall back to the ill-defined schema '0'
        schema0::Minsky m;
        m.load(filename.c_str());
        *this=m;
      }

    variables.makeConsistent();
    for (GodleyItems::iterator g=godleyItems.begin(); g!=godleyItems.end(); ++g)
      g->update();

    for (GroupIcons::iterator g=groupItems.begin(); g!=groupItems.end(); ++g)
      {
        // ensure group attributes correctly set
        const vector<int>& vars= g->variables();
        for (vector<int>::const_iterator i=vars.begin(); i!=vars.end(); ++i)
          {
            const VariablePtr& v=variables[*i];
            v->group=g->id();
            v->visible=g->displayContents();
          }
        const vector<int>& ops= g->operations();
        for (vector<int>::const_iterator i=ops.begin(); i!=ops.end(); ++i)
          {
            OperationPtr& o=operations[*i];
            o->group=g->id();
            o->visible=g->displayContents();
          }
        const vector<int>& gwires= g->wires();
        for (vector<int>::const_iterator i=gwires.begin(); i!=gwires.end(); ++i)
          {
            Wire& w=wires[*i];
            w.group=g->id();
            w.visible=g->displayContents();
          }
      }

    removeDuplicateWires();

    // try resetting the system, but ignore any errors
    try {reset();}
    catch (...) {}
    reset_needed=true;
    m_edited=false;
  }

  void Minsky::exportSchema(const char* filename, int schemaLevel)
  {
    xsd_generate_t x;
    // currently, there is only 1 schema level, so ignore second arg
    xsd_generate(x,"Minsky",schema1::Minsky());
    ofstream f(filename);
    x.output(f,schemaURL);
  }

  int Minsky::opIdOfEvalOp(const EvalOpBase& e) const
  {
    if (e.state)
      for (Operations::const_iterator j=operations.begin(); 
           j!=operations.end(); ++j)
        if (e.state==*j)
          return j->id();
    return -1;
  }


  array<int> Minsky::opOrder() const
  {
    array<int> r;
    for (size_t i=0; i<equations.size(); ++i)
      r<<opIdOfEvalOp(*equations[i]);
    return r;
  }

  void Minsky::zoom(float xOrigin, float yOrigin, float factor)
  {
    for (Wires::iterator w=wires.begin(); w!=wires.end(); ++w)
      w->zoom(xOrigin, yOrigin, factor);
    for (OperationPtr& o: operations)
      o->zoom(xOrigin, yOrigin, factor);
    for (VariablePtr& v: variables)
      v->zoom(xOrigin, yOrigin, factor);
    for (GodleyIcon& g: godleyItems)
      g.zoom(xOrigin, yOrigin, factor);
    for (GroupIcon& g: groupItems)
      g.zoom(xOrigin, yOrigin, factor);
    for (auto& s: switchItems)
      s->zoom(xOrigin, yOrigin, factor);
    for (PlotWidget& p: plots)
      p.zoom(xOrigin, yOrigin, factor);
    m_zoomFactor*=factor;
  }

  void Minsky::setZoom(float factor)
  {
    for (OperationPtr& o: operations)
      if (o->group==-1)
        o->setZoom(factor);
    for (VariablePtr& v: variables)
      if (v->group==-1)
        v->setZoom(factor);
    for (GroupIcons::iterator g=groupItems.begin(); g!=groupItems.end(); ++g)
      if (g->group()==-1)
        g->setZoom(factor);
    // zoomFactor in a godleyItem also contains relative size
    // attribute of the godley icon within the canvas, so we need to
    // relative zoom this
    for (GodleyItems::iterator g=godleyItems.begin(); g!=godleyItems.end(); ++g)
      g->zoomFactor*=factor/m_zoomFactor;
    for (Plots::iterator p=plots.begin(); p!=plots.end(); ++p)
      p->zoomFactor=factor;
    m_zoomFactor=factor;
  }

  void Minsky::addVariableToGroup(int groupId, int varId, bool checkIOregions)
  {
    GroupIcons::iterator g=groupItems.find(groupId);
    VariableManager::iterator v=variables.find(varId);
    if (g!=groupItems.end() && v!=variables.end())
      {
        if ((*v)->group!=-1)
          {
            GroupIcons::iterator pg=groupItems.find((*v)->group);
            if (pg!=groupItems.end())
              pg->removeVariable(*v);
          }
        g->addVariable(*v, checkIOregions);
        g->addAnyWires((*v)->ports());
      }
  }

  void Minsky::removeVariableFromGroup(int groupId, int varId)
  {
    GroupIcons::iterator g=groupItems.find(groupId);
    VariableManager::iterator v=variables.find(varId);
    if (g!=groupItems.end() && v!=variables.end() && (*v)->group==groupId)
      {
        g->removeVariable(*v);
        g->removeAnyWires((*v)->ports());
        g->addAnyWires((*v)->ports()); //break wires crossing group boundaries
        if (g->parent()!=-1)
          {
            GroupIcons::iterator pg=groupItems.find(g->parent());
            if (pg!=groupItems.end())
              pg->addVariable(*v);
          }
      }
  }

  void Minsky::addOperationToGroup(int groupId, int opId)
  {
    GroupIcons::iterator g=groupItems.find(groupId);
    Operations::iterator o=operations.find(opId);
    if (g!=groupItems.end() && o!=operations.end() && (*o)->group!=groupId)
      {
        if ((*o)->group!=-1)
          {
            GroupIcons::iterator pg=groupItems.find((*o)->group);
            if (pg!=groupItems.end())
              pg->removeOperation(*o);
          }
        g->addOperation(*o);
        g->addAnyWires((*o)->ports());
      }
  }

  void Minsky::removeOperationFromGroup(int groupId, int opId)
  {
    GroupIcons::iterator g=groupItems.find(groupId);
    Operations::iterator o=operations.find(opId);
    if (g!=groupItems.end() && o!=operations.end() && (*o)->group==groupId)
      {
        g->removeOperation(*o);
        g->removeAnyWires((*o)->ports());
        if (g->parent()!=-1)
          {
            GroupIcons::iterator pg=groupItems.find(g->parent());
            if (pg!=groupItems.end())
              pg->addOperation(*o);
          }
      }
  }

  bool Minsky::addGroupToGroup(int destGroup, int groupId)
  {
    if (destGroup==groupId) return false;
    GroupIcons::iterator g=groupItems.find(groupId);
    GroupIcons::iterator dg=groupItems.find(destGroup);
    if (g!=groupItems.end() && dg!=groupItems.end() && 
        g->parent()!=destGroup)
      {
        if (g->parent()!=-1)
          {
            GroupIcons::iterator pg=groupItems.find(g->parent());
            if (pg!=groupItems.end())
              pg->removeGroup(*g);
          }
        return dg->addGroup(*g);
      }
    return false;
  }

  void Minsky::removeGroupFromGroup(int destGroup, int groupId)
  {
    GroupIcons::iterator g=groupItems.find(groupId);
    GroupIcons::iterator dg=groupItems.find(destGroup);
    if (g!=groupItems.end() && dg!=groupItems.end())
      {
        dg->removeGroup(*g);
        if (dg->parent()!=-1)
          {
            GroupIcons::iterator pg=groupItems.find(dg->parent());
            if (pg!=groupItems.end())
              pg->addGroup(*g);
          }
      }
  }

  namespace
  {
    struct Network: public multimap<int,int>
    {
      set<int> portsVisited;
      vector<int> stack;
      // depth-first network walk, return true if cycle detected
      bool followWire(int p)
      {
        if (!portsVisited.insert(p).second)
          { //traverse finished, check for cycle along branch
            if (::find(stack.begin(), stack.end(), p) != stack.end())
              {
                const Port& port=cminsky().ports[p];
                Minsky::displayErrorItem(port.x(), port.y());
                return true;
              }
            else
              return false;
          }
        stack.push_back(p);
        pair<iterator,iterator> range=equal_range(p);
        for (iterator i=range.first; i!=range.second; ++i)
          if (followWire(i->second))
            return true;
        stack.pop_back();
        return false;
      }
    };
  }
    
  bool Minsky::cycleCheck() const
  {
    // construct the network schematic
    Network net;
    for (Wires::const_iterator w=wires.begin(); w!=wires.end(); ++w)
      net.insert(make_pair(w->from, w->to));
    for (const OperationPtr& o: operations)
      for (size_t j=1; j<o->numPorts(); ++j)
        if (o->type()!=OperationType::integrate)
          net.insert(make_pair(o->ports()[j], o->ports()[0]));
    for (const VariablePtr& v: variables)
      if (v->numPorts()>1)
        net.insert(make_pair(v->inPort(), v->outPort()));

    for (Ports::const_iterator p=ports.begin(); p!=ports.end(); ++p)
      if (!p->input() && !net.portsVisited.count(p->id()))
        if (net.followWire(p->id()))
          return true;
    return false;
  }

  bool Minsky::checkEquationOrder() const
  {
    array<bool> fvInit(flowVars.size(), false);
    // firstly, find all flowVars that are constants
    for (VariableValues::const_iterator v=
           variables.values.begin(); v!=variables.values.end(); ++v)
      if (!variables.inputWired(v->first) && v->second.idx()>=0)
        fvInit[v->second.idx()]=true;

    for (EvalOpVector::const_iterator e=equations.begin(); 
         e!=equations.end(); ++e)
      {
        const EvalOpBase& eo=**e;
        if (eo.out < 0|| (eo.numArgs()>0 && eo.in1<0) ||
            (eo.numArgs() > 1 && eo.in2<0))
          {
            cerr << "Incorrectly wired operation "<<opIdOfEvalOp(eo)<<endl;
            return false;
          }
        switch  (eo.numArgs())
          {
          case 0:
            fvInit[eo.out]=true;
            break;
          case 1:
            fvInit[eo.out]=!eo.flow1 || fvInit[eo.in1];
            break;
          case 2:
            // we need to check if an associated binary operator has
            // an unwired input, and if so, treat its input as
            // initialised, since it has already been initialised in
            // getInputFromVar()
            if (auto op=eo.state)
              switch (op->type())
                {
                case OperationType::add: case OperationType::subtract:
                case OperationType::multiply: case OperationType::divide:
                  fvInit[eo.in1] |= 
                    wiresAttachedToPort(op->ports()[1]).size()==0;
                  fvInit[eo.in2] |= 
                    wiresAttachedToPort(op->ports()[3]).size()==0;
                  break;
                default: break;
                }
            
            fvInit[eo.out]=
              (!eo.flow1 ||  fvInit[eo.in1]) && (!eo.flow2 ||  fvInit[eo.in2]);
            break;
          default: break;
          }
        if (!fvInit[eo.out])
          cerr << "Operation "<<opIdOfEvalOp(eo)<<" out of order"<<endl;
      }
    
    return all(fvInit);
  }


  void Minsky::displayErrorItem(float x, float y)
  {
    tclcmd() << "catch {indicateCanvasItemInError"<<x<<y<<"}\n";
    Tcl_ResetResult(interp());
  }

  void Minsky::displayErrorItem(const OpVarBaseAttributes& op) const
  {
    if (op.visible)
      displayErrorItem(op.x(),op.y());
    else if (op.group>-1)
      {
        const GroupIcon* g=&minsky().groupItems[op.group];
        while (!g->visible && g->parent()>-1)
          g=&minsky().groupItems[g->parent()];
        if (g->visible)
          displayErrorItem(g->x(), g->y());
      }
  }
  
    void Minsky::resetNextId()
    {
      nextId=0;
      if (!ports.empty()) nextId=ports.rbegin()->id()+1;
      if (!wires.empty()) nextId=max(nextId, wires.rbegin()->id()+1);
      if (!operations.empty()) nextId=max(nextId, operations.rbegin()->id()+1);
      if (!variables.empty()) nextId=max(nextId, variables.rbegin()->id()+1);
      if (!groupItems.empty()) nextId=max(nextId, groupItems.rbegin()->id()+1);
      if (!godleyItems.empty()) nextId=max(nextId, godleyItems.rbegin()->id()+1);
      if (!notes.empty()) nextId=max(nextId, notes.rbegin()->id()+1);
      if (!plots.empty()) nextId=max(nextId, plots.rbegin()->id()+1);
    }

  bool Minsky::pushHistoryIfDifferent()
  {
    // go via a schema object, as serialising minsky::Minsky has
    // problems due to port management
    schema1::Minsky m(*this);
    pack_t buf;
    buf<<m;
    if (history.empty() || memcmp(buf.data(), history.back().data(), buf.size())!=0)
      {
        // This bit of code outputs an XML representation that can be
        //        used for debugging issues related to unnecessary
        //        history pushes.
        // xml_pack_t tb(cout);
        // xml_pack(tb,"Minsky",m); 
        // cout<<"------"<<endl;
        history.resize(history.size()+1);
        buf.swap(history.back());
        return true;
      }
    return false;
  }

  void Minsky::pushHistory()
  {
    history.resize(historyPtr);
    pushHistoryIfDifferent();
    while (history.size()>maxHistory)
      history.pop_front();
    historyPtr=history.size();
  }
  
  void Minsky::undo(int changes)
  {
    // save current state for later restoration if needed
    if (historyPtr==history.size())
      pushHistoryIfDifferent();
    historyPtr-=changes;
    if (historyPtr > 0 && historyPtr <= history.size())
      {
        schema1::Minsky m;
        history[historyPtr-1].reseto()>>m;
        clearAllMaps();
        *this=m;
      }
    else
      historyPtr+=changes; // revert
  }

}

