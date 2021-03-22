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
#include "cairoItems.h"
#include "classdesc_access.h"
#include "minsky.h"
#include "flowCoef.h"
#include "userFunction.h"
#include "mdlReader.h"

#include "TCL_obj_stl.h"
#include <gsl/gsl_errno.h>
#include <gsl/gsl_odeiv2.h>
#include <cairo_base.h>

#include <schema/schema3.h>


//#include <thread>
// std::thread apparently not supported on MXE for now...
#include <boost/thread.hpp>
using namespace std;

using namespace minsky;
using namespace classdesc;
using namespace boost::posix_time;

namespace
{
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
        ((Minsky*)params)->threadErrMsg=e.what();
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
       ((Minsky*)params)->threadErrMsg=e.what();
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

  struct BusyCursor
  {
    Minsky& minsky;
    BusyCursor(Minsky& m): minsky(m) {minsky.setBusyCursor();}
    ~BusyCursor() {minsky.clearBusyCursor();}
  };
}

#include "minskyVersion.h"

#include "minsky_epilogue.h"


#include <algorithm>
using namespace std;

namespace minsky
{
  void EquationDisplay::redraw(int x0, int y0, int width, int height)
  {
    if (surface.get()) {
      m.setBusyCursor();
      MathDAG::SystemOfEquations system(m);
      cairo_rectangle(surface->cairo(),0,0,width,height);
      cairo_clip(surface->cairo());
      cairo_move_to(surface->cairo(),offsx,offsy);
      system.renderEquations(*surface,height);
      if (m.flags & Minsky::fullEqnDisplay_needed)
        {
          ecolab::cairo::Surface surf
            (cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,NULL));
          system.renderEquations(surf,std::numeric_limits<double>::max());
          m_width=surf.width();
          m_height=surf.height();
          m.flags &= ~Minsky::fullEqnDisplay_needed;
        }
      m.clearBusyCursor();
    }
  }

  
  void Minsky::openLogFile(const string& name)
  {
    outputDataFile.reset(new ofstream(name));
    *outputDataFile<< "#time";
    for (auto& v: variableValues)
      if (logVarList.count(v.first))
        *outputDataFile<<" "<<v.second->name;
    *outputDataFile<<endl;
  }

  /// write current state of all variables to the log file
  void Minsky::logVariables() const
  {
    if (outputDataFile)
      {
        *outputDataFile<<t;
        for (auto& v: variableValues)
          if (logVarList.count(v.first))
            *outputDataFile<<" "<<v.second->value();
        *outputDataFile<<endl;
      }
  }        
        
      

  void Minsky::clearAllMaps()
  {
    model->clear();
    equations.clear();
    integrals.clear();
    variableValues.clear();
    UserFunction::nextId=0;
    
    flowVars.clear();
    stockVars.clear();

    dimensions.clear();
    flags=reset_needed|fullEqnDisplay_needed;
    fileVersion=minskyVersion;
  }


  const char* Minsky::minskyVersion=MINSKY_VERSION;

  void Minsky::cut()
  {
    copy();
    for (auto& i: canvas.selection.items)
      {
        if (auto v=i->variableCast())
          if (v->controller.lock())
            continue; // do not delete a variable controlled by another item
        model->deleteItem(*i);
      }
    for (auto& i: canvas.selection.groups)
      model->removeGroup(*i);
    for (auto& i: canvas.selection.wires)
      model->removeWire(*i);
    garbageCollect();
    canvas.item.reset();
    canvas.itemFocus.reset();
#ifndef NDEBUG
    for (auto& i: canvas.selection.items)
      {
        if (auto v=i->variableCast())
          if (v->controller.lock())
            continue; // variable controlled by another item is not being destroyed
        assert(i.use_count()==1);
      }
    for (auto& i: canvas.selection.groups)
      assert(i.use_count()==1);
    for (auto& i: canvas.selection.wires)
      assert(i.use_count()==1);
#endif
    canvas.selection.clear();
    canvas.requestRedraw();
  }

  void Minsky::copy() const
  {
    if (canvas.selection.empty())
      putClipboard(""); // clear clipboard
    else
      {
        schema3::Minsky m(canvas.selection);
        ostringstream os;
        xml_pack_t packer(os, schemaURL);
        xml_pack(packer, "Minsky", m);
        putClipboard(os.str());
      }
  }

  VariablePtr Minsky::definingVar(const string& valueId) const 
  {
    return dynamic_pointer_cast<VariableBase>
      (model->findAny(&Group::items, [&](const ItemPtr& x) {
            auto v=x->variableCast();
            return v && v->valueId()==valueId && v->defined();
          }));
  }
    
  void Minsky::saveGroupAsFile(const Group& g, const string& fileName) const
  {
    schema3::Minsky m(g);
    Saver(fileName).save(m);
  }

  void Minsky::paste()
  {
    istringstream is(getClipboard());
    xml_unpack_t unpacker(is); 
    schema3::Minsky m(unpacker);
    GroupPtr g(new Group);
    g->self=g;
    m.populateGroup(*g);
    // stash values of parameters in copied group, as they are reset for some unknown reason later on. for ticket 1258
    map<string,string> existingParms; 
    for (auto& i: g->items) {
        auto v=i->variableCast(); 
        if (v && v->type()==VariableType::parameter) 
			existingParms.emplace(v->valueId(),v->init());
	}
    // Default pasting no longer occurs as grouped items or as a group within a group. Fix for tickets 1080/1098    
    canvas.selection.clear();
    // The following is only necessary if one pastes into an existing model. For ticket 1258   
    if (!history.empty() || !canvas.model.get()->empty()) {     
      bool alreadyDefinedMessageDisplayed=false;
      
      // convert stock variables that aren't defined to flow variables, and other fix up multiply defined vars
      g->recursiveDo(&GroupItems::items,
                     [&](Items&, Items::iterator i) {
                       if (auto v=(*i)->variableCast())
                         if (v->defined() || v->isStock())
                           {
                             // if defined, check no other defining variable exists
                             auto alreadyDefined = canvas.model->findAny
                               (&GroupItems::items,
                                [&v](const ItemPtr& j)
                                {return j.get()!=v && j->variableCast() &&  j->variableCast()->defined();});
                             if (v->isStock())
                               {
                                 if (v->defined() && alreadyDefined && !alreadyDefinedMessageDisplayed)
                                   {
                                     message("Integral/Stock variable "+v->name()+" already defined.");
                                     alreadyDefinedMessageDisplayed=true;
                                   }
                                 else if (!v->defined() && !alreadyDefined)
                                   {
                                     // need to do this var explicitly, as not currently part of model structure
                                     if (auto vp=VariablePtr(*i))
                                       {
                                         vp.retype(VariableType::flow);
                                         *i=vp;
                                         convertVarType(vp->valueId(), VariableType::flow);
                                       }
                                   }
                               }
                             else if (alreadyDefined)
                               {
                                 // delete defining wire from this
                                 assert(v->portsSize()>1 && !v->ports(1).lock()->wires().empty());
                                 g->removeWire(*v->ports(1).lock()->wires()[0]);
                               }
                           }
                       return false;
                     });
    }                              

    canvas.model->addGroup(g); // needed to ensure wires are correctly handled
    auto copyOfItems=g->items;
    auto copyOfGroups=g->groups;
    
    // ungroup g, putting all its contents on the canvas
    canvas.model->moveContents(*g); 

    // leave newly ungrouped items in selection
    for (auto& i: copyOfItems) {
       canvas.selection.ensureItemInserted(i);
       // ensure that initial values of pasted parameters are correct. for ticket 1258
       if (auto v=i->variableCast())
		 if (v->type()==VariableType::parameter && !existingParms.empty()) 
		 {
		   auto it=existingParms.find(v->valueId());
		   if (it!=existingParms.end()) v->init(it->second);
	   }
	}
	
	if (!existingParms.empty()) existingParms.clear();
	
    // Attach mouse focus only to first visible item in selection. For ticket 1098.      
    for (auto& i: canvas.selection.items)
      if (i->visible())
        {
          canvas.setItemFocus(i);
          break;
        }
                        
    if (!copyOfGroups.empty()) canvas.setItemFocus(copyOfGroups[0]);   

    canvas.model->removeGroup(*g);  
    canvas.requestRedraw();
  }  

  namespace
  {
    /// checks if the input stream has the UTF-8 byte ordering marker,
    /// and removes it if present
    void stripByteOrderingMarker(istream& s)
    {
      char bom[4];
      s.get(bom,4);
      if (strcmp(bom,"\357\273\277")==0) return; //skipped BOM
      s.seekg(0); //rewind input stream
    }
  }

  void Minsky::insertGroupFromFile(const char* file)
  {
    ifstream inf(file);
    if (!inf)
      throw runtime_error(string("failed to open ")+file);
    stripByteOrderingMarker(inf);
    xml_unpack_t saveFile(inf);
    schema3::Minsky currentSchema(saveFile);

    GroupPtr g(new Group);
    currentSchema.populateGroup(*model->addGroup(g));
    g->resizeOnContents();
    canvas.itemFocus=g;
  }

  void Minsky::makeVariablesConsistent()
  {
    // remove variableValues not in variables
    set<string> existingNames;
    existingNames.insert("constant:zero");
    existingNames.insert("constant:one");
    vector<GodleyIcon*> godleysToUpdate;
    model->recursiveDo(&Group::items, 
                       [&](Items&,Items::iterator i) {
                         if (auto v=(*i)->variableCast())
                           existingNames.insert(v->valueId());
                         // ensure Godley table variables are the correct types
                         if (auto g=dynamic_cast<GodleyIcon*>(i->get()))
                           godleysToUpdate.push_back(g);
                         return false;
                       }
                       );

    for (auto g: godleysToUpdate) g->update();
    for (auto i=variableValues.begin(); i!=variableValues.end(); )
      if (existingNames.count(i->first))
        ++i;
      else
        variableValues.erase(i++);
    
  }

  void Minsky::imposeDimensions()
  {
    for (auto& v: variableValues)
      {
        v.second->imposeDimensions(dimensions);
        v.second->tensorInit.imposeDimensions(dimensions);
      }
  }


  void Minsky::garbageCollect()
  {
    makeVariablesConsistent();
    stockVars.clear();
    flowVars.clear();
    equations.clear();
    integrals.clear();

    // remove all temporaries
    for (auto v=variableValues.begin(); v!=variableValues.end();)
      if (v->second->temp())
        variableValues.erase(v++);
      else
        ++v;
    
    variableValues.reset();
  }

  void Minsky::renderEquationsToImage(const char* image)
  {
    ecolab::cairo::TkPhotoSurface surf(Tk_FindPhoto(interp(),image));
    cairo_move_to(surf.cairo(),0,0);
    MathDAG::SystemOfEquations system(*this);
    system.renderEquations(surf, surf.height());
    surf.blit();
  }

  void Minsky::constructEquations()
  {
    if (cycleCheck()) throw error("cyclic network detected");

    garbageCollect();
    equations.clear();
    integrals.clear();

    // add all user defined functions to the global symbol tables
    userFunctions.clear();
    model->recursiveDo
      (&Group::items,
       [this](const Items&, Items::const_iterator it){
         if (auto f=dynamic_pointer_cast<CallableFunction>(*it))
           userFunctions[VariableValue::valueIdFromScope((*it)->group.lock(), f->name())]=f;
         return false;
       });
    model->recursiveDo
      (&Group::groups,
       [this](const Groups&, Groups::const_iterator it){
         userFunctions[VariableValue::valueIdFromScope((*it)->group.lock(), (*it)->name())]=*it;
         return false;
       });

    try
      {
        dimensionalAnalysis();
      }
    catch (const std::exception& ex)
      {
        // do not block reset() on dimensional analysis failure
        message(ex.what());
      }
    
    EvalOpBase::timeUnit=timeUnit;

    MathDAG::SystemOfEquations system(*this);
    assert(variableValues.validEntries());
    system.populateEvalOpVector(equations, integrals);
    assert(variableValues.validEntries());
    system.updatePortVariableValue(equations);
    
    // attach the plots
    model->recursiveDo
      (&Group::items,
       [&](Items& m, Items::iterator i)
       {
         if (auto p=(*i)->plotWidgetCast())
           {
             p->disconnectAllVars();// clear any old associations
             p->clearPenAttributes();
             p->autoScale();
             for (size_t i=0; i<p->portsSize(); ++i)
               {
                 auto pp=p->ports(i).lock();
                 if (pp->wires().size()>0)
                   if (auto vv=pp->getVariableValue())
                     if (vv->idx()>=0)
                       p->connectVar(vv, i);
               }
           }
         
         return false;
       });
  }

  void Minsky::dimensionalAnalysis() const
  {
    const_cast<Minsky*>(this)->variableValues.resetUnitsCache();
    // increment varsPassed by one to prevent resettting the cache on each check
    IncrDecrCounter vpIdc(VariableBase::varsPassed);
    model->recursiveDo
      (&Group::items,
       [&](Items& m, Items::iterator i)
       {
         if (auto v=(*i)->variableCast())
           {
             // check only the defining variables
             if (v->isStock() && (v->inputWired() || v->controller.lock().get()))
               v->checkUnits();
           }
         else if ((*i)->portsSize()>0 && !(*i)->ports(0).lock()->input() &&
                  (*i)->ports(0).lock()->wires().empty())
           (*i)->checkUnits(); // check anything with an unwired output port
         else if (auto p=(*i)->plotWidgetCast())
           for (size_t i=0; i<p->portsSize(); ++i)
             p->ports(i).lock()->checkUnits();
         else if (auto p=dynamic_cast<Sheet*>(i->get()))
           for (size_t i=0; i<p->portsSize(); ++i)
             p->ports(i).lock()->checkUnits();
         return false;
       });
  }

  void Minsky::deleteAllUnits()
  {
    for (auto& i: variableValues)
      i.second->units.clear();
    timeUnit.clear();
  }
  
  void Minsky::populateMissingDimensions() {
    model->recursiveDo
      (&Group::items,[&](Items& m, Items::iterator it)
       {
         if (auto ri=dynamic_cast<Ravel*>(it->get()))
           {
             auto state=ri->getState();
             for (auto& j: state.handleStates)
               dimensions.emplace(j.description,Dimension());
           }
         return false;
       });
  }


  void Minsky::populateMissingDimensionsFromVariable(const VariableValue& v)
  {
    for (auto& xv: v.hypercube().xvectors)
      {
        auto d=dimensions.find(xv.name);
        if (d==dimensions.end())
          dimensions.emplace(xv.name, xv.dimension);
        else if (d->second.type==xv.dimension.type)
          d->second.units=xv.dimension.units;
        else
          message("Incompatible dimension type for dimension "+d->first+". Please adjust the global dimension in the dimensions dialog");
        
      }
    // set all such dimensions on Ravels to forward sort order
    set<string> varDimensions;
    for (auto& xv: v.hypercube().xvectors)
      varDimensions.insert(xv.name);
    model->recursiveDo
      (&Group::items,[&](Items& m, Items::iterator it)
      {
        if (auto ri=dynamic_cast<Ravel*>(it->get()))
          for (size_t i=0; i<ri->numHandles(); ++i)
            if (varDimensions.count(ri->handleDescription(i)))
              ri->setHandleSortOrder(ravel::HandleSort::forward, i);
        return false;
      });
  }
  
  std::set<string> Minsky::matchingTableColumns(const GodleyIcon& godley, GodleyAssetClass::AssetClass ac)
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
    model->recursiveDo
      (&Group::items,
       [&](Items& m, Items::iterator it)
       {
         if (auto gi=dynamic_cast<GodleyIcon*>(it->get()))
           {
             for (size_t i=1; i<gi->table.cols(); ++i)
               if (!gi->table.cell(0,i).empty())
                 {
                   auto v=gi->table.cell(0,i);
                   auto scope=VariableValue::scope(gi->group.lock(),v);
                   if (scope->higher(*godley.group.lock()))
                     v=':'+v;
                   else if (scope!=godley.group.lock())
                     continue; // variable is inaccessible
                   if (gi==&godley || r.count(v) || gi->table._assetClass(i)!=ac) 
                     {
                       r.erase(v); // column already duplicated, or in current, nothing to match
                       duplicatedColumns.insert(v);
                     }
                   else if (!duplicatedColumns.count(v))
                     r.insert(v);
                 }
           }
        return false;
      });
    return r;
  }

  void Minsky::importDuplicateColumn(GodleyTable& srcTable, int srcCol)
  {
    if (srcCol<0 || size_t(srcCol)>=srcTable.cols()) return;
    // find any duplicate column, and use it to do balanceDuplicateColumns
    const string& colName=trimWS(srcTable.cell(0,srcCol));
    if (colName.empty()) return; //ignore blank columns

    try
      {
        model->recursiveDo
          (&Group::items,
           [&](Items& m, Items::iterator i)
           {
             if (auto gi=dynamic_cast<GodleyIcon*>(i->get()))
               if (&gi->table!=&srcTable) // skip source table
                 for (size_t col=1; col<gi->table.cols(); col++)
                   if (trimWS(gi->table.cell(0,col))==colName) // we have a match
                     balanceDuplicateColumns(*gi, col);
             return false;
           });
      }
    catch (...) // in the event of business rules being violated, delete column name
      {
        srcTable.cell(0,srcCol).clear();
        throw;
      }
  }

  void Minsky::balanceDuplicateColumns(const GodleyIcon& srcGodley, int srcCol)
  {
    const GodleyTable& srcTable=srcGodley.table;
    if (srcCol<0 || size_t(srcCol)>=srcTable.cols()) return;
    // find if there is a matching column
    const string& colName=srcGodley.valueId(trimWS(srcTable.cell(0,srcCol)));
    if (colName.empty() || colName==":_") return; //ignore blank columns

    bool matchFound=false;
    model->recursiveDo
      (&Group::items,
       [&](Items& m, Items::iterator i)
       {
         if (auto gi=dynamic_cast<GodleyIcon*>(i->get()))
           if (&gi->table!=&srcTable) // skip source table
             for (size_t col=1; col<gi->table.cols(); col++)
               if (gi->valueId(trimWS(gi->table.cell(0,col)))==colName) // we have a match
                 {
                   // checks asset class rules
                   switch (srcGodley.table._assetClass(srcCol))
                     {
                     case GodleyAssetClass::asset:
                       if (gi->table._assetClass(col)!=GodleyAssetClass::liability)
                         throw error("asset column %s matches a non-liability column",colName.c_str());
                       break;
                     case GodleyAssetClass::liability:
                       if (gi->table._assetClass(col)!=GodleyAssetClass::asset)
                         throw error("liability column %s matches a non-asset column",colName.c_str());
                       break;
                     default:
                       throw error("invalid asset class for duplicate column %s",colName.c_str());
                     }

                   // checks that nom more than two duplicated columns exist
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
                       for (size_t r=1; r<destTable.rows(); ++r)
                         if (destTable.initialConditionRow(r))
                           destTable.cell(r,col)=srcTable.cell(row,srcCol);
                   for (size_t row=1; row!=destTable.rows(); ++row)
                     if (!destTable.initialConditionRow(row) && !destTable.cell(row,0).empty())
                       destRowLabels[trimWS(destTable.cell(row,0))]=row;


                   // compute column signature for both src and destination columns
                   map<string,double> srcFlows=srcGodley.flowSignature(srcCol), 
                     destFlows=gi->flowSignature(col);
                   // items to add
                   for (map<string,double>::iterator i=srcFlows.begin(); i!=srcFlows.end(); ++i)
                     if (i->second != destFlows[i->first])
                       {
                         int scope=-1;
                         if (i->first.find(':')!=string::npos)
                           VariableValue::scope(i->first);
                         FlowCoef df;
                         if (scope==-1 || !variableValues.count(i->first))
                           df.name=VariableValue::uqName(i->first);
                         else
                           df.name=variableValues[i->first]->name;
                         df.coef=i->second-destFlows[i->first];
                         if (df.coef==0) continue;
                         string flowEntry=df.str();
                         string rowLabel=srcRowLabels[srcGodley.valueId(i->first)];
                         map<string,int>::iterator dr=destRowLabels.find(rowLabel);
                         if (dr!=destRowLabels.end())
                           if (FlowCoef(destTable.cell(dr->second, col)).coef==0)
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
                   set<size_t> rowsToDelete;
                   for (map<string,double>::iterator i=destFlows.begin(); i!=destFlows.end(); ++i)
                     if (i->second!=0 && srcFlows[i->first]==0)
                       for (size_t row=1; row<destTable.rows(); ++row)
                         {
                           FlowCoef fc(destTable.cell(row, col));
                           if (!fc.name.empty())
                             fc.name=gi->valueId(fc.name);
                           if (fc.name==gi->valueId(i->first))
                             {
                               destTable.cell(row, col).clear();
                               // if this leaves an empty row, delete entire row
                               for (size_t c=0; c<destTable.cols(); ++c)
                                 if (!destTable.cell(row, c).empty())
                                   goto rowNotEmpty;
                               rowsToDelete.insert(row);
                             rowNotEmpty:;
                             }
                         }
                   // amalgamate unlabelled rows with singular value
                   map<string,double> unlabelledSigs;
                   for (size_t row=1; row<destTable.rows(); ++row)
                     {
                       if (!destTable.singularRow(row, col)) continue;
                       FlowCoef fc(destTable.cell(row, col));
                       unlabelledSigs[fc.name]+=fc.coef;
                       rowsToDelete.insert(row);
                     }
                   // append amalgamated rows
                   for (auto& i: unlabelledSigs)
                     if (i.second!=0)
                       {
                         destTable.insertRow(destTable.rows());
                         destTable.cell(destTable.rows()-1,col)=FlowCoef(i.second,i.first).str();
                       }
                   
                   for (auto row=rowsToDelete.rbegin(); row!=rowsToDelete.rend(); ++row)
                     destTable.deleteRow(*row);
                 }   
         return false;
       });  // TODO - this lambda is FAR too long!
  }

  namespace
  {
    struct GodleyIt: public vector<GodleyIcon*>::iterator
    {
      typedef vector<GodleyIcon*>::iterator Super;
      GodleyIt(const Super& x): Super(x) {}
      GodleyIcon& operator*() {return *Super::operator*();}
      GodleyIcon* operator->() {return Super::operator*();}
      const std::vector<std::vector<std::string> >& data() const {
        return Super::operator*()->table.getData();
      }
      const GodleyAssetClass::AssetClass assetClass(size_t col) const
      {return Super::operator*()->table._assetClass(col);}
      bool signConventionReversed(int col) const
      {return Super::operator*()->table.signConventionReversed(col);}
      bool initialConditionRow(int row) const
      {return Super::operator*()->table.initialConditionRow(row);}
      string valueId(const std::string& x) const {
        Variable<VariableBase::flow> tmp;
        tmp.name(x);
        tmp.group=Super::operator*()->group;
        return tmp.valueId();
      }
    };
  }

  void Minsky::initGodleys()
  {
    auto toGodleyIcon=[](const ItemPtr& i) {return dynamic_cast<GodleyIcon*>(i.get());};
    auto godleyItems=model->findAll<GodleyIcon*>
      (toGodleyIcon, &GroupItems::items, toGodleyIcon);
    evalGodley.initialiseGodleys(GodleyIt(godleyItems.begin()), 
                                 GodleyIt(godleyItems.end()), variableValues);
  }

  void Minsky::reset()
  {
    // do not reset while simulation is running
    if (RKThreadRunning)
      {
        flags |= reset_needed;
        if (RKThreadRunning) return;
      }

    canvas.itemIndicator=false;
    BusyCursor busy(*this);
    EvalOpBase::t=t=t0;
    constructEquations();
    // if no stock variables in system, add a dummy stock variable to
    // make the simulation proceed
    if (stockVars.empty()) stockVars.resize(1,0);

    initGodleys();

    if (stockVars.size()>0)
      {
        if (order==1 && !implicit)
          ode.reset(); // do explicit Euler
        else
          ode.reset(new RKdata(this)); // set up GSL ODE routines
      }

      
    // update flow variable
    evalEquations();
    
    model->recursiveDo
      (&Group::items,
       [&](Items& m, Items::iterator i)
       {
         if (auto p=(*i)->plotWidgetCast())
           {
             p->clear();
             if (running)
               p->updateIcon(t);
             else
               p->addConstantCurves();
             p->requestRedraw();
           }
         else if (auto r=dynamic_cast<Ravel*>(i->get()))
           {
             if (r->ports(1).lock()->numWires()>0)
               if (auto vv=r->ports(1).lock()->getVariableValue())
                 r->populateHypercube(vv->hypercube());
           }
         else if (auto v=(*i)->variableCast())
           { //determine whether a slider should be shown
             if (auto vv=v->vValue())
               vv->sliderVisible = v->type()==VariableType::parameter || (v->type()==VariableType::flow && !inputWired(v->valueId()));
           }
         return false;
       });

    if (running)
      flags &= ~reset_needed; // clear reset flag
    else
      flags |= reset_needed; // enforce another reset at simulation start
    running=false;

    canvas.requestRedraw();
    godleyTab.requestRedraw();
    plotTab.requestRedraw();
    variableTab.requestRedraw();
    parameterTab.requestRedraw();    
  }

  void Minsky::step()
  {
    if (reset_flag())
      reset();
    running=true;
    
    // create a private copy for worker thread use
    vector<double> stockVarsCopy(stockVars);
    RKThreadRunning=true;
    int err=GSL_SUCCESS;
    // run RK algorithm on a separate worker thread so as to no block UI. See ticket #6
    boost::thread rkThread([&]() {
      try
        { 
          double tp=reverse? -t: t;
          if (ode)
            {
              gsl_odeiv2_driver_set_nmax(ode->driver, nSteps);
              // we need to update Minsky's t synchronously to support the t operator
              // potentially means t and stockVars out of sync on GUI, but should still be thread safe
              err=gsl_odeiv2_driver_apply(ode->driver, &tp, numeric_limits<double>::max(), 
                                          &stockVarsCopy[0]);
            }
          else // do explicit Euler method
            {
              vector<double> d(stockVarsCopy.size());
              for (int i=0; i<nSteps; ++i, tp+=stepMax)
                {
                  evalEquations(&d[0], tp, &stockVarsCopy[0]);
                  for (size_t j=0; j<d.size(); ++j)
                    stockVarsCopy[j]+=d[j];
                }
            }
          t=reverse? -tp:tp;
        }
      catch (const std::exception& ex)
        {
          // catch any thrown exception, and report back to GUI thread
          threadErrMsg=ex.what();
        }
      catch (...)
        {
          threadErrMsg="Unknown exception thrown on ODE solver thread";
        }
      RKThreadRunning=false;
    });

    while (RKThreadRunning)
      {
        // while waiting for thread to finish, check and process any UI events
        usleep(1000);
        doOneEvent(false);
      }
    rkThread.join();

    if (!threadErrMsg.empty())
      {
        runtime_error err(threadErrMsg);
        threadErrMsg.clear();
        // rethrow exception so message gets displayed to user
        throw err;
      }
    
    if (reset_flag()) // in case reset() was called during the step evaluation
      {
        reset();
        return;
      }

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

    stockVars.swap(stockVarsCopy);

    // update flow variables
    evalEquations();

    logVariables();

    model->recursiveDo
      (&Group::items, 
       [&](Items&, Items::iterator i) 
       {(*i)->updateIcon(t); return false;});

    // throttle redraws
    time_duration maxWait=milliseconds(maxWaitMS);
    if ((microsec_clock::local_time()-(ptime&)lastRedraw) > maxWait)
      {
        canvas.requestRedraw();
        godleyTab.requestRedraw();
        plotTab.requestRedraw();
        variableTab.requestRedraw();
        parameterTab.requestRedraw();        
        lastRedraw=microsec_clock::local_time();
      }

  }

  string Minsky::diagnoseNonFinite() const
  {
    // firstly check if any variables are not finite
    for (VariableValues::const_iterator v=variableValues.begin();
         v!=variableValues.end(); ++v)
      if (!isfinite(v->second->value()))
        return v->first;

    // now check operator equations
    for (EvalOpVector::const_iterator e=equations.begin(); e!=equations.end(); ++e)
      if (!isfinite(flowVars[(*e)->out]))
        return OperationType::typeName((*e)->type());
    return "";
  }

  void Minsky::evalEquations(double result[], double t, const double vars[])
  {
    EvalOpBase::t=reverse? -t: t;
    double reverseFactor=reverse? -1: 1;
    // firstly evaluate the flow variables. Initialise to flowVars so
    // that no input vars are correctly initialised
    vector<double> flow(flowVars);
    for (size_t i=0; i<equations.size(); ++i)
      equations[i]->eval(&flow[0], flow.size(), vars);

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
        // enable element-wise integration of tensor variables. for feature 147  
	for (size_t j=0; j<i->input.size(); ++j)
	    result[i->stock.idx()+j] = reverseFactor *
	      (i->input.isFlowVar()? flow[i->input.idx()+j] : vars[i->input.idx()+j]);
      } 
  }

  void Minsky::jacobian(Matrix& jac, double t, const double sv[])
  {
    EvalOpBase::t=reverse? -t: t;
    double reverseFactor=reverse? -1: 1;
    // firstly evaluate the flow variables. Initialise to flowVars so
    // that no input vars are correctly initialised
    vector<double> flow=flowVars;
    for (size_t i=0; i<equations.size(); ++i)
      equations[i]->eval(&flow[0], flow.size(), sv);

    // then determine the derivatives with respect to variable j
    for (size_t j=0; j<stockVars.size(); ++j)
      {
        vector<double> ds(stockVars.size()), df(flowVars.size());
        ds[j]=1;
        for (size_t i=0; i<equations.size(); ++i)
          equations[i]->deriv(&df[0], df.size(), &ds[0], sv, &flow[0]);
        vector<double> d(stockVars.size());
        evalGodley.eval(&d[0], &df[0]);
        for (vector<Integral>::iterator i=integrals.begin(); 
             i!=integrals.end(); ++i)
          {
            assert(i->stock.idx()>=0 && i->input.idx()>=0);
            d[i->stock.idx()] = 
              i->input.isFlowVar()? df[i->input.idx()]: ds[i->input.idx()];
          }
        for (size_t i=0; i<stockVars.size(); i++)
          jac(i,j)=reverseFactor*d[i];
      }
  
  }

  void Minsky::save(const std::string& filename)
  {
    schema3::Minsky m(*this);
    Saver saver(filename);
    saver.packer.prettyPrint=true;
    try
      {
        saver.save(m);
      }
    catch (...) {
      // if exception is due to file error, provide a more useful message
      if (!saver.os)
        throw runtime_error("cannot save to "+filename);
      throw;
    }
    flags &= ~is_edited;
    fileVersion=minskyVersion;
  }

  void Minsky::load(const std::string& filename) 
  {
    BusyCursor busy(*this);
    clearAllMaps();

    ifstream inf(filename);
    if (!inf)
      throw runtime_error("failed to open "+filename);
    stripByteOrderingMarker(inf);
    xml_unpack_t saveFile(inf);
    schema3::Minsky currentSchema(saveFile);
    *this=currentSchema;
    if (currentSchema.schemaVersion<currentSchema.version)
      message("You are converting the model from an older version of Minsky. "
              "Once you save this file, you may not be able to open this file"
              " in older versions of Minsky.");
    
    // try balancing all Godley tables
    try
      {
        model->recursiveDo(&Group::items, 
                           [&](Items&,Items::iterator i) {
                             if (auto g=dynamic_cast<GodleyIcon*>(i->get()))
                               for (unsigned i=1; i<g->table.cols(); ++i)
                                 balanceDuplicateColumns(*g,i);
                             return false;
                           });
    
        // try resetting the system, but ignore any errors
        reset();
      }
    catch (...) {}
    panopticon.requestRedraw();
    flags=reset_needed|fullEqnDisplay_needed;
  }

  void Minsky::exportSchema(const char* filename, int schemaLevel)
  {
    xsd_generate_t x;
    switch (schemaLevel)
      {
      case 0:
        xsd_generate(x,"Minsky",schema0::Minsky());
        break;
      case 1:
        xsd_generate(x,"Minsky",schema1::Minsky());
        break;
      case 2:
        xsd_generate(x,"Minsky",schema2::Minsky());
        break;
      case 3:
        xsd_generate(x,"Minsky",schema3::Minsky());
        break;
      }
    ofstream f(filename);
    x.output(f,schemaURL);
  }
  
  namespace
  {
    struct Network: public multimap<const Port*,const Port*>
    {
      set<const Port*> portsVisited;
      vector<const Port*> stack;
      void emplace(Port* x, Port* y) 
      {multimap<const Port*,const Port*>::emplace(x,y);}
      // depth-first network walk, return true if cycle detected
      bool followWire(const Port* p)
      {
        if (!portsVisited.insert(p).second)
          { //traverse finished, check for cycle along branch
            if (::find(stack.begin(), stack.end(), p) != stack.end())
              {
                cminsky().displayErrorItem(p->item());
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
    for (auto& w: model->findWires([](WirePtr){return true;}))
      net.emplace(w->from().get(), w->to().get());
    for (auto& i: model->findItems([](ItemPtr){return true;}))
      if (!dynamic_cast<IntOp*>(i.get()) && !dynamic_cast<GodleyIcon*>(i.get()))
        for (unsigned j=1; j<i->portsSize(); ++j)
          net.emplace(i->ports(j).lock().get(), i->ports(0).lock().get());
    
    for (auto& i: net)
      if (!i.first->input() && !net.portsVisited.count(i.first))
        if (net.followWire(i.first))
          return true;
    return false;
  }

  bool Minsky::checkEquationOrder() const
  {
    ecolab::array<bool> fvInit(flowVars.size(), false);
    // firstly, find all flowVars that are constants
    for (auto& v: variableValues)
      if (!inputWired(v.first) && v.second->idx()>=0)
        fvInit[v.second->idx()]=true;

    for (auto& e: equations)
      if (auto eo=dynamic_cast<const ScalarEvalOp*>(e.get()))
        {
          if (eo->out < 0|| (eo->numArgs()>0 && eo->in1.empty()) ||
            (eo->numArgs() > 1 && eo->in2.empty()))
            {
              //cerr << "Incorrectly wired operation "<<opIdOfEvalOp(eo)<<endl;
              return false;
            }
          switch  (eo->numArgs())
            {
            case 0:
              fvInit[eo->out]=true;
              break;
            case 1:
              fvInit[eo->out]=!eo->flow1 || fvInit[eo->in1[0]];
              break;
            case 2:
              // we need to check if an associated binary operator has
              // an unwired input, and if so, treat its input as
              // initialised, since it has already been initialised in
              // getInputFromVar()
              if (auto op=eo->state)
                switch (op->type())
                  {
                  case OperationType::add: case OperationType::subtract:
                  case OperationType::multiply: case OperationType::divide:
                    fvInit[eo->in1[0]] |= op->ports(1).lock()->wires().empty();
                    fvInit[eo->in2[0][0].idx] |= op->ports(3).lock()->wires().empty();
                    break;
                  default: break;
                  }
            
              fvInit[eo->out]=
                (!eo->flow1 ||  fvInit[eo->in1[0]]) && (!eo->flow2 ||  fvInit[eo->in2[0][0].idx]);
            break;
            default: break;
          }
//        if (!fvInit[eo.out])
//          cerr << "Operation "<<opIdOfEvalOp(eo)<<" out of order"<<endl;
        }
    
    return all(fvInit);
  }


  void Minsky::displayErrorItem(const Item& op) const
  {
    // this method is logically const, but because of the way
    // canvas rendering is done, canvas state needs updating
    auto& canvas=const_cast<Canvas&>(this->canvas);
    canvas.item=nullptr;
    if (op.visible())
      canvas.item=canvas.model->findItem(op);
    else if (auto v=op.variableCast())
      if (auto c=v->controller.lock())
        displayErrorItem(*c);

    if (!canvas.item)
      if (auto g=op.group.lock())
        {
          while (g && !g->visible()) g=g->group.lock();
          if (g && g->visible())
            canvas.item=g;
        }
    
    canvas.itemIndicator=canvas.item.get();
    //requestRedraw calls back into TCL, so don't call it from the simulation thread. See ticket #973
    if (!RKThreadRunning) canvas.requestRedraw();
  }
  
  bool Minsky::pushHistory()
  {
    // go via a schema object, as serialising minsky::Minsky has
    // problems due to port management
    schema3::Minsky m(*this, false /* don't pack tensor data */);
    pack_t buf;
    buf<<m;
    if (history.empty())
      {
        history.emplace_back();
        buf.swap(history.back());
        historyPtr=history.size();
        return false;
      }
    while (history.size()>maxHistory)
      history.pop_front();
    if (memcmp(buf.data(), history.back().data(), buf.size())!=0)
      {
        // check XML versions differ (slower)
        ostringstream prev, curr;
        xml_pack_t prevXbuf(prev), currXbuf(curr);
        xml_pack(currXbuf,"Minsky",m);
        schema3::Minsky previousMinsky;
        history.back().reseto()>>previousMinsky;
        xml_pack(prevXbuf,"Minsky",previousMinsky);

        if (curr.str()!=prev.str())
          {
            // This bit of code outputs an XML representation that can be
            //        used for debugging issues related to unnecessary
            //        history pushes.
            //  buf.reseto()>>m;
            //  xml_pack_t tb(cout);
            //  tb.prettyPrint=true;
            //  xml_pack(tb,"Minsky",m); 
            //  cout<<"------"<<endl;
            history.emplace_back();
            buf.swap(history.back());
            historyPtr=history.size();
            if (autoSaver && doPushHistory)
              try
                {
                  //autoSaver->packer.prettyPrint=true;
                  autoSaver->save(m);
                }
              catch (...)
                {
                  autoSaver.reset();
                  throw std::runtime_error("Unable to autosave to this location");
                }
            return true;
          }
      }
    historyPtr=history.size();
    return false;
  }
  
  void Minsky::undo(int changes)
  {
    // save current state for later restoration if needed
    if (historyPtr==history.size())
      pushHistory();
    historyPtr-=changes;
    if (historyPtr > 0 && historyPtr <= history.size())
      {
        schema3::Minsky m;
        history[historyPtr-1].reseto()>>m;
        // stash tensorInit data for later restoration
        auto stashedValues=move(variableValues);
        clearAllMaps();
        model->clear();
        m.populateGroup(*model);
        // restore tensorInit data
        for (auto& v: variableValues)
          {
            auto stashedValue=stashedValues.find(v.first);
            if (stashedValue!=stashedValues.end())
              v.second->tensorInit=move(stashedValue->second->tensorInit);
          }
        try {reset();}
        catch (...) {}
          
      }
    else
      historyPtr+=changes; // revert
  }

  void Minsky::convertVarType(const string& name, VariableType::Type type)
  {
    assert(VariableValue::isValueId(name));
    VariableValues::iterator i=variableValues.find(name);
    if (i==variableValues.end())
      throw error("variable %s doesn't exist",name.c_str());
    if (i->second->type()==type) return; // nothing to do!

    string newName=name; // useful for checking flows and stocks with same name and renaming them as the case may be. for ticket 1272  
    model->recursiveDo
      (&GroupItems::items,
       [&](const Items&,Items::const_iterator i)
       {
         if (auto g=dynamic_cast<GodleyIcon*>(i->get()))
           {
             if (type!=VariableType::flow)
               for (auto v: g->flowVars())
                 if (v->valueId()==name)
                   {
		       newName=v->name()+"^{Flow}";
                       VariableValues::iterator iv=variableValues.find(newName);
                       if (iv==variableValues.end()) {g->table.renameFlows(v->name(),newName); v->retype(VariableType::flow);}
					   else throw error("flow variables in Godley tables cannot be converted to a different type");
		   }	
             if (type!=VariableType::stock)
               for (auto v: g->stockVars())
                 if (v->valueId()==name)
                   {
		       newName=v->name()+"^{Stock}";
                       VariableValues::iterator iv=variableValues.find(newName);
                       if (iv==variableValues.end()) {g->table.renameStock(v->name(),newName); v->retype(VariableType::stock);}
					   else throw error("stock variables in Godley tables cannot be converted to a different type");
		   }
           }
         return false;
       });   
                         
    if (auto var=definingVar(name))
      // we want to be able to convert stock vars to flow vars when their input is wired. condition is only met when newName has not been changed above. for ticket 1272
      if (name==newName && var->type() != type && (!var->isStock() || var->controller.lock()))
         throw error("cannot convert a variable to a type other than its defined type");

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
    model->recursiveDo(&Group::items,
                       [&](Items&, Items::iterator i) {
                         if (auto v=VariablePtr(*i))
                           if (v->valueId()==name)
                             {
                               v.retype(type);
                               if (*i==canvas.item)
                                 canvas.item=v;
                               *i=v;
                             }
                         return false;
                       });
    i->second=VariableValuePtr(type,i->second->name,i->second->init);
  }

  void Minsky::addIntegral()
  {
    if (auto v=canvas.item->variableCast())
      if (auto g=v->group.lock())
        {
          // nb throws if conversion cannot be performed
          convertVarType(v->valueId(),VariableType::integral);
          auto integ=new IntOp;
          g->addItem(integ);
          integ->moveTo(canvas.item->x(), canvas.item->y());
          integ->intVar=dynamic_pointer_cast<VariableBase>(canvas.item);
          integ->toggleCoupled();
          
          canvas.requestRedraw();
        }
  }

  void Minsky::renderAllPlotsAsSVG(const string& prefix) const
  {
    unsigned plotNum=0;
    model->recursiveDo(&Group::items,
                       [&](Items&, Items::iterator i) {
                         if (auto p=(*i)->plotWidgetCast())
                           {
                             if (!p->title.empty())
                               p->renderToSVG((prefix+"-"+p->title+".svg").c_str());
                             else
                               p->renderToSVG((prefix+"-"+str(plotNum++)+".svg").c_str());
                           }
                         return false;
                       });
  }
  void Minsky::exportAllPlotsAsCSV(const string& prefix) const
  {
    unsigned plotNum=0;
    model->recursiveDo(&Group::items,
                       [&](Items&, Items::iterator i) {
                         if (auto p=(*i)->plotWidgetCast())
                           {
                             if (!p->title.empty())
                               p->exportAsCSV((prefix+"-"+p->title+".csv").c_str());
                             else
                               p->exportAsCSV((prefix+"-"+str(plotNum++)+".csv").c_str());
                           }
                         return false;
                       });
  }

  void Minsky::setAllDEmode(bool mode) {
    model->recursiveDo(&GroupItems::items, [mode](Items&,Items::iterator i) {
        if (auto g=dynamic_cast<GodleyIcon*>(i->get()))
          g->table.setDEmode(mode);
        return false;
      });
  }

  void Minsky::setGodleyDisplayValue(bool displayValues, GodleyTable::DisplayStyle displayStyle)
  {
    this->displayValues=displayValues;
    this->displayStyle=displayStyle;
    model->recursiveDo(&GroupItems::items, [](Items&,Items::iterator i) {
      if (auto g=dynamic_cast<GodleyIcon*>(i->get()))
        g->setEditorDisplayValues();
      return false;
    });
  }

  void Minsky::importVensim(const string& filename)
  {
    ifstream f(filename);
    readMdl(*model,*this,f);
    canvas.requestRedraw();
  }

  void Minsky::setAutoSaveFile(const std::string& file) {
    if (file.empty())
      autoSaver.reset();
    else
      autoSaver.reset(new BackgroundSaver(file));
  }

  
}

