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
#include "minsky.h"
#include "cairoItems.h"
#include "classdesc_access.h"
#include "flowCoef.h"
#include "userFunction.h"
#include "mdlReader.h"
#include "variableInstanceList.h"

#include "TCL_obj_stl.h"
#include <cairo_base.h>

#include "schema3.h"

//#include <thread>
// std::thread apparently not supported on MXE for now...
#include <boost/thread.hpp>

#include "minskyVersion.h"

#include "fontDisplay.rcd"
#include "minsky.rcd"
#include "minsky.xcd"
#include "signature.h"
#include "signature.rcd"
#include "signature.xcd"
#include "dimension.rcd"
#include "callableFunction.rcd"
#include "tensorInterface.xcd"
#include "tensorVal.xcd"
#include "pannableTab.rcd"
#include "pannableTab.xcd"
#include "polyRESTProcessBase.h"
#include "polyRESTProcessBase.rcd"
#include "polyRESTProcessBase.xcd"
#include "minsky_epilogue.h"

#include <algorithm>
#include <boost/filesystem.hpp>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

#if defined(__linux__)
#include <sys/sysinfo.h>
#endif

using namespace classdesc;
using namespace boost::posix_time;


namespace minsky
{
  namespace
  {
    /// list the possible string values of an enum (for TCL)
    template <class E> vector<string> enumVals()
    {
      vector<string> r;
      for (size_t i=0; i < sizeof(enum_keysData<E>::keysData) / sizeof(EnumKey); ++i)
        r.push_back(enum_keysData<E>::keysData[i].name);
      return r;
    }
}

  bool Minsky::multipleEquities(const bool& m) {
    m_multipleEquities=m;
    canvas.requestRedraw();
    redrawAllGodleyTables();
    return m_multipleEquities;
  }
  
  void Minsky::openLogFile(const string& name)
  {
    outputDataFile.reset(new ofstream(name));
    *outputDataFile<< "#time";
    for (auto& v: variableValues)
      if (logVarList.contains(v.first))
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
          if (logVarList.contains(v.first))
            *outputDataFile<<" "<<v.second->value();
        *outputDataFile<<endl;
      }
  }        
        
  Minsky::~Minsky()
  {
    if (edited() && autoSaver)
      // if we're at this point, then the user has already been asked to save, and chosen no.
      boost::filesystem::remove(autoSaver->fileName);
  }

  void Minsky::clearAllMaps(bool doClearHistory)
  {
    model->clear();
    canvas.openGroupInCanvas(model);
    equations.clear();
    integrals.clear();
    variableValues.clear();
    variablePane.update();
    maxValue.clear();
    PhillipsFlow::maxFlow.clear();
    PhillipsStock::maxStock.clear();
    phillipsDiagram.clear();
    publicationTabs.clear();
    publicationTabs.emplace_back("Publication");
    userFunctions.clear();
    UserFunction::nextId=0;
    
    flowVars.clear();
    stockVars.clear();

    dimensions.clear();
    namedItems.clear();
    flags=reset_needed|fullEqnDisplay_needed;
    fileVersion=minskyVersion;
    if (doClearHistory) clearHistory();
  }


  const std::string Minsky::minskyVersion=MINSKY_VERSION;

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
      clipboard.putClipboard(""); // clear clipboard
    else
      {
        schema3::Minsky m(canvas.selection);
        ostringstream os;
        xml_pack_t packer(os, schemaURL);
        xml_pack(packer, "Minsky", m);
        clipboard.putClipboard(os.str());
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
    
  void Minsky::saveGroupAsFile(const Group& g, const string& fileName)
  {
    const schema3::Minsky m(g);
    Saver(fileName).save(m);
  }

  void Minsky::paste()
    try
      {
        map<string,string> existingParms; 
        // preserve initial conditions.
        for (auto& [valueId,vv]: variableValues)
          existingParms.emplace(valueId,vv->init);

        istringstream is(clipboard.getClipboard());
        xml_unpack_t unpacker(is); 
        schema3::Minsky m(unpacker);
        GroupPtr g(new Group);
        g->self=g;
        m.populateGroup(*g);
        
        // stash values of parameters in the copied group, for ticket 1258
        for (auto& i: g->items) 
          if (auto v=i->variableCast(); v && v->type()==VariableType::parameter)
            existingParms.emplace(v->valueId(),v->init());
        
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
        for (auto& i: copyOfItems) 
          canvas.selection.ensureItemInserted(i);

        // ensure that initial values of pasted parameters are correct. for ticket 1258
        for (auto& p: existingParms)
          if (auto vv=variableValues.find(p.first); vv!=variableValues.end())
            vv->second->init=p.second;
        existingParms.clear();
	
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
    catch (...)
      {
        throw runtime_error("clipboard data invalid");
      }
  
  void Minsky::insertGroupFromFile(const string& file)
  {
    ifstream inf(file);
    if (!inf)
      throw runtime_error(string("failed to open ")+file);
    stripByteOrderingMarker(inf);
    xml_unpack_t saveFile(inf);
    const schema3::Minsky currentSchema(saveFile);

    const GroupPtr g(new Group);
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
      if (existingNames.contains(i->first))
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

  void Minsky::renderEquationsToImage(const string& image)
  {
    ecolab::cairo::TkPhotoSurface surf(Tk_FindPhoto(interp(),image.c_str()));
    cairo_move_to(surf.cairo(),0,0);
    const MathDAG::SystemOfEquations system(*this);
    system.renderEquations(surf, surf.height());
    surf.blit();
  }

  void Minsky::constructEquations()
  {
    if (cycleCheck()) throw error("cyclic network detected");

    const ProgressUpdater pu(progressState,"Construct equations",8);
    garbageCollect();
    equations.clear();
    integrals.clear();
    ++progressState;
    
    // add all user defined functions to the global symbol tables
    userFunctions.clear();
    model->recursiveDo
      (&Group::items,
       [this](const Items&, Items::const_iterator it){
         if (auto f=dynamic_pointer_cast<CallableFunction>(*it))
           userFunctions[valueIdFromScope((*it)->group.lock(), canonicalName(f->name()))]=f;
         return false;
       });
    ++progressState;
    model->recursiveDo
      (&Group::groups,
       [this](const Groups&, Groups::const_iterator it){
         userFunctions[valueIdFromScope((*it)->group.lock(), canonicalName((*it)->name()))]=*it;
         return false;
       });
    ++progressState;

    EvalOpBase::timeUnit=timeUnit;

    MathDAG::SystemOfEquations system(*this);
    ++progressState;
    assert(variableValues.validEntries());
    system.populateEvalOpVector(equations, integrals);
    ++progressState;
    assert(variableValues.validEntries());
    system.updatePortVariableValue(equations);
  }

  void Minsky::dimensionalAnalysis() const
  {
    const_cast<Minsky*>(this)->variableValues.resetUnitsCache();
    // increment varsPassed by one to prevent resettting the cache on each check
    const IncrDecrCounter vpIdc(VariableBase::varsPassed);
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
           for (size_t j=0; j<p->portsSize(); ++j)
             p->ports(j).lock()->checkUnits();
         else if (auto p=dynamic_cast<Sheet*>(i->get()))
           for (size_t j=0; j<p->portsSize(); ++j)
             p->ports(j).lock()->checkUnits();
         return false;
       });
  }

  void Minsky::deleteAllUnits()
  {
    for (auto& i: variableValues)
      i.second->units.clear();
    timeUnit.clear();
  }

  void Minsky::requestReset()
  {
    if (resetDuration<chrono::milliseconds(500))
      {
        try
          {
            reset();
          }
        catch (...)
          {}
        return;
      }
    flags|=reset_needed;
    // schedule reset for some time in the future
    resetAt=std::chrono::system_clock::now()+std::chrono::milliseconds(1500);
  }

  void Minsky::requestRedraw()
  {
    // requestRedraw on all tabs - only the active one will actually do anything
    canvas.requestRedraw();
    equationDisplay.requestRedraw();
    phillipsDiagram.requestRedraw();
    for (auto& pub: publicationTabs)
      pub.requestRedraw();
  }
  
  void Minsky::populateMissingDimensions() {
    // populate from variable value table first, then override by ravels
    for (auto& v: variableValues)
      populateMissingDimensionsFromVariable(*v.second);
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
          message("Incompatible dimension type for dimension "+d->first+". Please adjust the global dimension in the dimensions dialog, which can be found under the Edit menu.");
        
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
            if (varDimensions.contains(ri->handleDescription(i)))
              ri->setHandleSortOrder(ravel::HandleSort::staticForward, i);
        return false;
      });
  }

  void Minsky::renameDimension(const std::string& oldName, const std::string& newName)
  {
    auto i=dimensions.find(oldName);
    if (i!=dimensions.end())
      {
        dimensions[newName]=i->second;
        dimensions.erase(i);
      }

    for (auto& v: variableValues)
      {
        auto hc=v.second->tensorInit.hypercube();
        for (auto& x: hc.xvectors)
          if (x.name==oldName)
            {
              x.name=newName;
            }
        v.second->tensorInit.hypercube(hc);
      }
  }

  
  std::set<string> Minsky::matchingTableColumns(const GodleyIcon& godley, GodleyAssetClass::AssetClass ac)
  {
    std::set<string> r;
    GodleyAssetClass::AssetClass target_ac;
    // matching liability with assets and vice-versa
    switch (ac)
      {
      case GodleyAssetClass::liability:
      case GodleyAssetClass::equity:
        target_ac=GodleyAssetClass::asset;
        break;
      case GodleyAssetClass::asset:
        target_ac=GodleyAssetClass::liability;
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
                   auto scope=minsky::scope(gi->group.lock(),v);
                   if (scope->higher(*godley.group.lock()))
                     v=':'+v; //NOLINT
                   else if (scope!=godley.group.lock())
                     continue; // variable is inaccessible
                   if (r.contains(v) || gi->table._assetClass(i)!=target_ac) 
                     {
                       r.erase(v); // column already duplicated, or in current, nothing to match
                       duplicatedColumns.insert(v);
                     }
                   else if (!duplicatedColumns.contains(v) && gi->table._assetClass(i)==target_ac &&
                            // insert unmatched asset columns from this table only for equity (feature #174)
                            // otherwise matches are between separate tables
                            ((ac!=GodleyAssetClass::equity && gi!=&godley) || (ac==GodleyAssetClass::equity && gi==&godley) ))
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
               for (size_t col=1; col<gi->table.cols(); col++)
                 if ((&gi->table!=&srcTable || int(col)!=srcCol) && trimWS(gi->table.cell(0,col))==colName) // we have a match
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

  void Minsky::balanceColumns(const GodleyIcon& srcGodley, int srcCol, GodleyIcon& destGodley, int destCol) const
  {
    auto& srcTable=srcGodley.table;
    auto& destTable=destGodley.table;
    // reverse lookup tables for mapping flow variable to destination row numbers via row labels
    map<string,string> srcRowLabels;
    map<string, int> destRowLabels;
    set<string> uniqueSrcRowLabels; // check for uniqueness of source row labels
    for (size_t row=1; row!=srcTable.rows(); ++row)
      if (!srcTable.initialConditionRow(row) && !srcTable.cell(row,0).empty() &&
          !srcTable.cell(row,srcCol).empty())
        {
          if (!uniqueSrcRowLabels.insert(srcTable.cell(row,0)).second)
            throw runtime_error("Duplicate source row label: "+srcTable.cell(row,0));
          const FlowCoef fc(srcTable.cell(row,srcCol));
          if (!fc.name.empty())
            srcRowLabels[srcGodley.valueId(fc.name)]=
              trimWS(srcTable.cell(row,0));
        }
      else if (srcTable.initialConditionRow(row))
        // copy directly into destination initial condition,
        for (size_t r=1; r<destTable.rows(); ++r)
          if (destTable.initialConditionRow(r))
            destTable.cell(r,destCol)=srcTable.cell(row,srcCol);
    for (size_t row=1; row!=destTable.rows(); ++row)
      if (!destTable.initialConditionRow(row) && !destTable.cell(row,0).empty())
        destRowLabels[trimWS(destTable.cell(row,0))]=row;


    // compute column signature for both src and destination columns
    map<string,double> srcFlows=srcGodley.flowSignature(srcCol), 
      destFlows=destGodley.flowSignature(destCol);
    // items to add
    for (map<string,double>::iterator i=srcFlows.begin(); i!=srcFlows.end(); ++i)
      if (i->second != destFlows[i->first])
        {
          const int scope=-1;
          if (i->first.find(':')!=string::npos)
            minsky::scope(i->first);
          FlowCoef df;
          if (scope==-1 || !variableValues.count(i->first))
            df.name=uqName(i->first);
          else
            df.name=variableValues[i->first]->name;
          df.coef=i->second-destFlows[i->first];
          if (df.coef==0) continue;
          const string flowEntry=df.str();
          const string rowLabel=srcRowLabels[srcGodley.valueId(i->first)];
          const map<string,int>::iterator dr=destRowLabels.find(rowLabel);
          if (dr!=destRowLabels.end())
            if (FlowCoef(destTable.cell(dr->second, destCol)).coef==0)
              destTable.cell(dr->second, destCol) = flowEntry;
            else
              // add a new blank labelled flow line
              {
                destTable.resize(destTable.rows()+1,destTable.cols());
                destTable.cell(destTable.rows()-1, destCol) = flowEntry;
              }
          else
            // labels don't match, so add a new labelled line
            {
              destTable.resize(destTable.rows()+1,destTable.cols());
              destTable.cell(destTable.rows()-1, 0) = rowLabel;
              destRowLabels[rowLabel] = destTable.rows()-1;
              destTable.cell(destTable.rows()-1, destCol) = flowEntry;
            }
        }
    // items to delete
    set<size_t> rowsToDelete;
    for (map<string,double>::iterator i=destFlows.begin(); i!=destFlows.end(); ++i)
      if (i->second!=0 && srcFlows[i->first]==0)
        for (size_t row=1; row<destTable.rows(); ++row)
          {
            FlowCoef fc(destTable.cell(row, destCol));
            if (!fc.name.empty())
              fc.name=destGodley.valueId(fc.name);
            if (fc.name==destGodley.valueId(i->first))
              {
                destTable.cell(row, destCol).clear();
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
        if (!destTable.singularRow(row, destCol)) continue;
        const FlowCoef fc(destTable.cell(row, destCol));
        unlabelledSigs[fc.name]+=fc.coef;
        rowsToDelete.insert(row);
      }
    // append amalgamated rows
    for (auto& i: unlabelledSigs)
      if (i.second!=0)
        {
          destTable.insertRow(destTable.rows());
          destTable.cell(destTable.rows()-1,destCol)=FlowCoef(i.second,i.first).str();
        }
                   
    for (auto row=rowsToDelete.rbegin(); row!=rowsToDelete.rend(); ++row)
      destTable.deleteRow(*row);
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
           {
             if (&gi->table==&srcTable) // if source table, then check for duplicated asset/equity columns
               {
                 for (size_t col=1; col<gi->table.cols(); col++)
                   if (col==size_t(srcCol)) continue; // skip over source column
                   else if (srcGodley.valueId(trimWS(srcTable.cell(0,col)))==colName)
                     {
                       switch (srcGodley.table._assetClass(srcCol))
                         {
                         case GodleyAssetClass::asset:
                           if (srcTable._assetClass(col)!=GodleyAssetClass::equity)
                             throw error("asset column %s matches a non-liability column",colName.c_str());
                           break;
                         case GodleyAssetClass::equity:
                           if (srcTable._assetClass(col)!=GodleyAssetClass::asset)
                             throw error("equity column %s matches a non-asset column",colName.c_str());
                           break;
                         default:
                           throw error("invalid asset class for duplicate column %s",colName.c_str());
                         }
                       balanceColumns(srcGodley, srcCol, *gi, col);
                     }
               }
             else  // match asset/liability columns on different Godley tables
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
                     balanceColumns(srcGodley, srcCol, *gi, col);
                   }
           }
         return false;
       });  // TODO - this lambda is FAR too long!
  }

  vector<string> Minsky::allGodleyFlowVars() const
  {
    set<string> r;
    model->recursiveDo(&GroupItems::items, [&](const Items&, Items::const_iterator i) {
      if (auto g=dynamic_cast<GodleyIcon*>(i->get()))
        {
          auto flowVars=g->table.getVariables();
          r.insert(flowVars.begin(),flowVars.end());
        }
      return false;
    });
    return {r.begin(),r.end()};
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
      GodleyAssetClass::AssetClass assetClass(size_t col) const
      {return Super::operator*()->table._assetClass(col);}
      bool signConventionReversed(int col) const
      {return Super::operator*()->table.signConventionReversed(col);}
      bool initialConditionRow(int row) const
      {return Super::operator*()->table.initialConditionRow(row);}
      string valueId(const std::string& x) const {
        return valueIdFromScope(Super::operator*()->group.lock(), canonicalName(x));
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

    auto start=chrono::high_resolution_clock::now();
    canvas.itemIndicator=false;
    const BusyCursor busy(*this);
    EvalOpBase::t=t=t0;
    lastT=t0;
    const ProgressUpdater pu(progressState,"Resetting",5);
    constructEquations();
    ++progressState;
    // if no stock variables in system, add a dummy stock variable to
    // make the simulation proceed
    if (stockVars.empty()) stockVars.resize(1,0);

    initGodleys();
    ++progressState;

    if (!stockVars.empty())
      rkreset();
    
    // update flow variable
    evalEquations();
    ++progressState;

    // populate ravel hypercubes first, before reattaching plots.
    model->recursiveDo
      (&Group::items,
       [&](Items& m, Items::iterator i)
       {
         if (auto r=dynamic_cast<Ravel*>(i->get()))
           {
             if (r->ports(1).lock()->numWires()>0)
               if (auto vv=r->ports(1).lock()->getVariableValue())
                 r->populateHypercube(vv->hypercube());
           }
         else if (auto v=(*i)->variableCast())
           { //determine whether a slider should be shown
             if (auto vv=v->vValue())
               vv->sliderVisible = v->enableSlider &&
                 (v->type()==VariableType::parameter || (v->type()==VariableType::flow && !inputWired(v->valueId())));
             v->resetMiniPlot();
           }
         return false;
       });

    // attach the plots
    model->recursiveDo
      (&Group::items,
       [&](Items& m, Items::iterator it)
       {
         if (auto p=(*it)->plotWidgetCast())
           {
             p->disconnectAllVars();// clear any old associations
             p->clearPenAttributes();
             p->autoScale();
             for (size_t i=0; i<p->portsSize(); ++i)
               {
                 auto pp=p->ports(i).lock();
                 if (!pp->wires().empty())
                   if (auto vv=pp->getVariableValue())
                     if (vv->idx()>=0)
                       p->connectVar(vv, i);
               }
             p->clear();
             if (running)
               p->updateIcon(t);
             else
               p->addConstantCurves();
             p->requestRedraw();
           }
         return false;
       });
    ++progressState;

    //    if (running)
    flags &= ~reset_needed; // clear reset flag
    resetAt=std::chrono::time_point<std::chrono::system_clock>::max();
    // else
    //  flags |= reset_needed; // enforce another reset at simulation start
    running=false;

    requestRedraw();
    
    // update maxValues
    PhillipsFlow::maxFlow.clear();
    PhillipsStock::maxStock.clear();
    for (auto& v: variableValues)
      {
        if (v.second->type()==VariableType::stock)
          {
            PhillipsStock::maxStock[v.second->units]+=v.second->value();
          }
      }
    for (auto& i: PhillipsStock::maxStock) i.second=abs(i.second);
    
    resetDuration=chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now()-start);
  }

  vector<double> Minsky::step()
  {
    lastT=t;
    rkstep();

    logVariables();

    model->recursiveDo
      (&Group::items, 
       [&](Items&, Items::iterator i) 
       {(*i)->updateIcon(t); return false;});

    // throttle redraws
    const time_duration maxWait=milliseconds(maxWaitMS);
    if ((microsec_clock::local_time()-(ptime&)lastRedraw) > maxWait)
      {
        requestRedraw();
        lastRedraw=microsec_clock::local_time();
      }

    return {t, deltaT()};
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

  void Minsky::save(const std::string& filename)
  {
    const schema3::Minsky m(*this);
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
    if (autoSaver)
      boost::filesystem::remove(autoSaver->fileName);
  }

  void Minsky::load(const std::string& filename) 
  {
    running=false;
    clearAllMaps(true);

    ifstream inf(filename);
    if (!inf)
      throw runtime_error("failed to open "+filename);
    stripByteOrderingMarker(inf);
    
    {
      const BusyCursor busy(*this);
      xml_unpack_t saveFile(inf);
      const schema3::Minsky currentSchema(saveFile);
      currentSchema.populateMinsky(*this);
      if (currentSchema.schemaVersion<currentSchema.version)
        message("You are converting the model from an older version of Minsky. "
                "Once you save this file, you may not be able to open this file"
                " in older versions of Minsky.");
    }

    const LocalMinsky lm(*this); // populateMinsky resets the local minsky pointer, so restore it here
    flags=fullEqnDisplay_needed;
    
    // try balancing all Godley tables
    try
      {
        model->recursiveDo(&Group::items, 
                           [this](Items&,Items::iterator i) {
                             try
                               {
                                 if (auto g=dynamic_cast<GodleyIcon*>(i->get()))
                                   {
                                     for (unsigned j=1; j<g->table.cols(); ++j)
                                       balanceDuplicateColumns(*g,j);
                                   }
                                 (*i)->adjustBookmark();
                               }
                             catch (...) {}
                             return false;
                           });

        // try resetting the system, but ignore any errors
        reset();
        reset();
        populateMissingDimensions();
      }
    catch (...) {flags|=reset_needed;}
    requestRedraw();
    canvas.recentre();
    canvas.requestRedraw();
    canvas.moveTo(0,0); // force placement of ports
    // sometimes we need to recalculate the bounding boxes
    model->recursiveDo(&Group::items,
                       [](Items&,Items::iterator i) {
                         (*i)->updateBoundingBox();
                         return false;
                       });

    pushHistory();
  }

  void Minsky::exportSchema(const string& filename, int schemaLevel)
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
            if (std::find(stack.begin(), stack.end(), p) != stack.end())
              {
                cminsky().displayErrorItem(p->item());
                return true;
              }
            return false;
          }
        stack.push_back(p);
        const pair<iterator,iterator> range=equal_range(p);
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
    for (auto& w: model->findWires([](const WirePtr&){return true;}))
      net.emplace(w->from().get(), w->to().get());
    for (auto& i: model->findItems([](const ItemPtr&){return true;}))
      if (!dynamic_cast<IntOp*>(i.get()) && !dynamic_cast<GodleyIcon*>(i.get()))
        for (unsigned j=1; j<i->portsSize(); ++j)
          net.emplace(i->ports(j).lock().get(), i->ports(0).lock().get());
    
    for (auto& i: net)
      if (!i.first->input() && !net.portsVisited.contains(i.first))
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
    if (canvas.item)
      {
        auto physX=canvas.item->x();
        auto physY=canvas.item->y();
        if (physX<100 || physX>canvas.frameArgs().childWidth-100 ||
            physY<100 || physY>canvas.frameArgs().childHeight-100)
          {
            canvas.model->moveTo(0.5*canvas.frameArgs().childWidth-physX+canvas.model->x(),
                                 0.5*canvas.frameArgs().childHeight-physY+canvas.model->y());
            minsky().resetScroll();
          }
      }
    //requestRedraw calls back into TCL, so don't call it from the simulation thread. See ticket #973
    if (!RKThreadRunning) canvas.requestRedraw();
  }
  
  bool Minsky::pushHistory()
  {
    // do not pushHistory after undo or redo
    if (undone)
      return undone=false;

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
        return true;
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
            //buf.reseto()>>m;
            //xml_pack_t tb(cout);
            //tb.prettyPrint=true;
            //xml_pack(tb,"Minsky",m); 
            //cout<<"------"<<endl;
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
                  throw;
                }
            return true;
          }
      }
    historyPtr=history.size();
    return false;
  }

  bool Minsky::commandHook(const std::string& command, unsigned nargs)
  {
    static const set<string> constableCommands={ // commands that should not trigger the edit flag
      "minsky.availableOperations",
      "minsky.canvas.select",
      "minsky.canvas.scaleFactor",
      "minsky.canvas.recentre",
      "minsky.canvas.focusFollowsMouse",
      "minsky.canvas.displayDelayedTooltip",
      "minsky.model.moveTo",
      "minsky.canvas.moveTo",
      "minsky.canvas.model.moveTo",
      "minsky.canvas.model.zoom",
      "minsky.canvas.position",
      "minsky.canvas.selectVar",
      /* we record mouse movements, but filter from history */
      "minsky.canvas.mouseDown",
      "minsky.canvas.mouseMove",
      "minsky.canvas.zoom",
      "minsky.canvas.zoomToFit",
      "minsky.clearAll",
      "minsky.doPushHistory",
      "minsky.fontScale",
      "minsky.model.zoom",
      "minsky.newGlobalGroupTCL",
      "minsky.openGroupInCanvas",
      "minsky.openModelInCanvas",
      "minsky.popFlags",
      "minsky.pushFlags",
      "minsky.select",
      "minsky.selectVar",
      "minsky.setGodleyDisplayValue",
      "minsky.setGodleyIconResource",
      "minsky.setGroupIconResource",
      "minsky.setLockIconResource",
      "minsky.setRavelIconResource",
      "minsky.histogramResource.setResource",
      "minsky.setAutoSaveFile",
      "minsky.step",
      "minsky.running",
      "minsky.multipleEquities",
      "minsky.undo",
      "minsky.load",
      "minsky.reverse",
      "minsky.redrawAllGodleyTables"
    };
    if (doPushHistory && constableCommands.contains(command)==0 &&
        command.find("minsky.phillipsDiagram")==string::npos &&
        command.find("minsky.equationDisplay")==string::npos && 
        command.find("minsky.publicationTabs")==string::npos && 
        command.find(".renderFrame")==string::npos && 
        command.find(".requestRedraw")==string::npos && 
        command.find(".backgroundColour")==string::npos && 
        command.find(".get")==string::npos && 
        command.find(".@elem")==string::npos && 
        command.find(".mouseFocus")==string::npos
        )
      {
        auto t=getCommandData(command);
        if (t==generic || (t==is_setterGetter && nargs>0))
          {
            const bool modelChanged=pushHistory();
            if (modelChanged && command.find(".keyPress")==string::npos)
              {
                markEdited();
              }
            return modelChanged;
          }
      }
    return command=="minsky.canvas.requestRedraw" || command=="minsky.canvas.mouseDown" || command=="minsky.canvas.mouseMove" || command.find(".get")!=string::npos;
  }

  
  long Minsky::undo(int changes)
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
        auto stashedValues=std::move(variableValues);
        // preserve bookmarks. For now, we can only preserve model and canvas.model bookmarks
        // count the total number of bookmarks
        unsigned numBookmarks=0;
        model->recursiveDo(&GroupItems::groups, [&](const Groups&,const Groups::const_iterator i) {
          numBookmarks+=(*i)->bookmarks.size();
          return false;
        });
        auto stashedGlobalBookmarks=model->bookmarks;
        auto stashedCanvasBookmarks=canvas.model->bookmarks;
        clearAllMaps(false);
        model->clear();
        m.populateGroup(*model);
        model->setZoom(m.zoomFactor);
        m.phillipsDiagram.populatePhillipsDiagram(phillipsDiagram);
        m.populatePublicationTabs(publicationTabs);
        requestRedraw();
        
        // restore tensorInit data
        for (auto& v: variableValues)
          {
            auto stashedValue=stashedValues.find(v.first);
            if (stashedValue!=stashedValues.end())
              v.second->tensorInit=std::move(stashedValue->second->tensorInit);
          }
        // restore bookmarks
        model->bookmarks=std::move(stashedGlobalBookmarks);
        canvas.model->bookmarks=std::move(stashedCanvasBookmarks);
        unsigned numBookmarksAfterwards=0;
        model->recursiveDo(&GroupItems::groups, [&](const Groups&,const Groups::const_iterator i) {
          numBookmarksAfterwards+=(*i)->bookmarks.size();
          return false;
        });
        if (numBookmarksAfterwards!=numBookmarks)
          message("This undo/redo operation potentially deletes some bookmarks");
        try {reset();}
        catch (...) {}
          
      }
    else
      historyPtr+=changes; // revert
    undone=true; //ensure next pushHistory is ignored 
    return historyPtr;
  }

  void Minsky::convertVarType(const string& name, VariableType::Type type)
  {
    assert(isValueId(name));
    const VariableValues::iterator i=variableValues.find(name);
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
               for (auto& v: g->flowVars())
                 if (v->valueId()==name)
                   {
                     newName=v->name()+"^{Flow}";
                     const VariableValues::iterator iv=variableValues.find(newName);
                     if (iv==variableValues.end()) {g->table.renameFlows(v->name(),newName); v->retype(VariableType::flow);}
                     else throw error("flow variables in Godley tables cannot be converted to a different type");
		   }	
             if (type!=VariableType::stock)
               for (auto& v: g->stockVars())
                 if (v->valueId()==name)
                   {
                     newName=v->name()+"^{Stock}";
                     const VariableValues::iterator iv=variableValues.find(newName);
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
          // remove intVar from its group
          if (auto g=integ->intVar->group.lock())
            g->removeItem(*integ->intVar);
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
                               p->renderToSVG(prefix+"-"+p->title+".svg");
                             else
                               p->renderToSVG(prefix+"-"+str(plotNum++)+".svg");
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
                               p->exportAsCSV((prefix+"-"+p->title+".csv"));
                             else
                               p->exportAsCSV((prefix+"-"+str(plotNum++)+".csv"));
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
    canvas.requestRedraw();
    model->recursiveDo(&GroupItems::items, [](Items&,Items::iterator i) {
      if (auto g=dynamic_cast<GodleyIcon*>(i->get()))
        g->popup.requestRedraw();
      return false;
    });
  }

  void Minsky::importVensim(const string& filename)
  {
    ifstream f(filename);
    readMdl(*model,*this,f);
    canvas.requestRedraw();
  }

  vector<string> Minsky::availableOperations() 
  {return enumVals<OperationType::Type>();}
  vector<string> Minsky::variableTypes()
  {return enumVals<VariableType::Type>();}
  vector<string> Minsky::assetClasses()
  {return enumVals<GodleyTable::AssetClass>();}

  Minsky::AvailableOperationsMapping Minsky::availableOperationsMapping() const
  {
    AvailableOperationsMapping r;
    for (OperationType::Type op{}; op != OperationType::numOps; op=OperationType::Type(int(op)+1))
      {
        if (classifyOp(op)==OperationType::general) continue;
        if (op==OperationType::copy) continue;
        r[classdesc::to_string(classifyOp(op))].push_back(op);
      }
    return r;
  }
  
  void Minsky::autoLayout()
  {
    canvas.model->autoLayout();
    canvas.recentre();
  }
  
  void Minsky::randomLayout()
  {
    canvas.model->randomLayout();
    canvas.recentre();
  }

  void Minsky::listAllInstances()
  {
    if (canvas.item)
      if (auto v=canvas.item->variableCast())
        {
          variableInstanceList=std::make_shared<VariableInstanceList>(*canvas.model, v->valueId());
          return;
        }
    variableInstanceList.reset();
  }

  void Minsky::removeItems(Wire& wire)
  {
    if (wire.from()->wires().size()==1)
      { // only remove higher up the network if this item is the only item feeding from it
        auto& item=wire.from()->item();
        if (!item.variableCast())
          {
            for (size_t i=1; i<item.portsSize(); ++i)
              if (auto p=item.ports(i).lock())
                for (auto w: p->wires())
                  removeItems(*w);
            model->removeItem(item);
          }
        else if (auto p=item.ports(1).lock())
          if (p->wires().empty())
            model->removeItem(item); // remove non-definition variables as well
      }
    model->removeWire(wire);
  }
  
  void Minsky::setDefinition(const std::string& valueId, const std::string& definition)
  {
    auto var=definingVar(valueId);
    if (!var) // find
      var=dynamic_pointer_cast<VariableBase>(model->findAny(&GroupItems::items, [&](const ItemPtr& it) {
        if (auto v=it->variableCast())
          return v->valueId()==valueId;
        return false;
      }));
    if (var)
      if (auto p=var->ports(1).lock())
        {
          auto group=var->group.lock();
          if (!group) group=model;
          UserFunction* udf=p->wires().empty()? nullptr: dynamic_cast<UserFunction*>(&p->wires().front()->from()->item());
          if (!udf)
            {
              // remove previous definition network
              for (auto w: p->wires())
                {
                  assert(w);
                  removeItems(*w);
                }

              udf=new UserFunction(var->name()+"()");
              group->addItem(udf); // ownership passed
              const bool notFlipped=!flipped(var->rotation());
              udf->moveTo(var->x()+(notFlipped? -1:1)*0.6*(var->width()+udf->width()), var->y());
              group->addWire(udf->ports(0), var->ports(1));
            }
          udf->expression=definition;
        }
  }

  
  void Minsky::redrawAllGodleyTables()
  {
    model->recursiveDo(&Group::items, 
                       [&](Items&,Items::iterator i) {
                         if (auto g=dynamic_cast<GodleyIcon*>(i->get()))
                           g->popup.requestRedraw();
                         return false;
                       });
  }

  size_t Minsky::physicalMem() const
  {
#if defined(__linux__)
    struct sysinfo s;
    sysinfo(&s);
    return s.totalram;
#elif defined(WIN32)
    MEMORYSTATUSEX s{sizeof(MEMORYSTATUSEX)};
    GlobalMemoryStatusEx(&s);
    return s.ullTotalPhys;
    //#elif defined(__APPLE__)
    //    int mib[2];
    //    int64_t physical_memory;
    //    size_t length;
    //
    //    // Get the Physical memory size
    //    mib[0] = CTL_HW;
    //    mib[1] = HW_MEMSIZE;
    //    length = sizeof(int64_t);
    //    sysctl(mib, 2, &physical_memory, &length, NULL, 0);
    //    return physical_memory;
#else
    // all else fails, return max value
    return ~0UL;
#endif  
  }
  
  static std::unique_ptr<char[]> _defaultFont;

  string Minsky::defaultFont()
  {return _defaultFont? _defaultFont.get(): "";}
  
  string Minsky::defaultFont(const std::string& x)
  {
    _defaultFont.reset(new char[x.length()+1]);
    strncpy(_defaultFont.get(),x.c_str(),x.length()+1);
    ecolab::Pango::defaultFamily=_defaultFont.get();
    return x;
  }

  double Minsky::fontScale()
  {return ecolab::Pango::scaleFactor;}
  
  double Minsky::fontScale(double s)
  {return ecolab::Pango::scaleFactor=s;}
  
  void Minsky::latex(const string& filename, bool wrapLaTeXLines) 
  {
    if (cycleCheck()) throw error("cyclic network detected");
    ofstream f(filename);

    f<<"\\documentclass{article}\n";
    if (wrapLaTeXLines)
      {
        f<<"\\usepackage{breqn}\n\\begin{document}\n";
        MathDAG::SystemOfEquations(*this).latexWrapped(f);
      }
    else
      {
        f<<"\\begin{document}\n";
        MathDAG::SystemOfEquations(*this).latex(f);
      }
    f<<"\\end{document}\n";
  }

  int Minsky::numOpArgs(OperationType::Type o)
  {
    const OperationPtr op(o);
    return op->numPorts()-1;
  }

  void Minsky::setAutoSaveFile(const std::string& file) {
    if (file.empty())
      autoSaver.reset();
    else
      autoSaver.reset(new BackgroundSaver(file));
  }

  BusyCursor::BusyCursor(Minsky& m): minsky(m)
  {if (!minsky.busyCursorStack++) minsky.setBusyCursor();}

  BusyCursor::~BusyCursor()
  {if (!--minsky.busyCursorStack) minsky.clearBusyCursor();}

  
  void Progress::displayProgress()
  {
    if (*cancel)
      {
        *cancel=false;
        throw std::runtime_error("Cancelled");
      }
    minsky().progress(title, lround(progress));
  }

}

namespace classdesc
{
  // specialise for VariableValues to give it an associative container flavour.
  // defining is_associative_container type attribute doesn't work
  template <> inline
  void RESTProcess(RESTProcess_t& t, const string& d, minsky::VariableValues& a)
  {t.add(d, new RESTProcessAssociativeContainer<minsky::VariableValues>(a));}
}

CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::Minsky);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(classdesc::Signature);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(classdesc::PolyRESTProcessBase);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::CallableFunction);
