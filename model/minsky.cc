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

#include "TCL_obj_stl.h"
#include <gsl/gsl_errno.h>
#include <gsl/gsl_odeiv2.h>
#include <cairo_base.h>

//#include <schema/schema0.h>
#include <schema/schema1.h>

#include <cairo/cairo-ps.h>
#include <cairo/cairo-pdf.h>
#include <cairo/cairo-svg.h>

using namespace minsky;
using namespace classdesc;
using namespace boost::posix_time;

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
    for (auto& v: variableValues)
      *outputDataFile<<" "<<v.first;
    *outputDataFile<<endl;
  }

  /// write current state of all variables to the log file
  void Minsky::logVariables() const
  {
    if (outputDataFile)
      {
        *outputDataFile<<t;
        for (auto& v: variableValues)
          *outputDataFile<<" "<<v.second.value();
        *outputDataFile<<endl;
      }
  }        
        
      

  void Minsky::clearAllMaps()
  {
    model->clear();
    equations.clear();
    integrals.clear();
    variableValues.clear();
    
    flowVars.clear();
    stockVars.clear();
//    evalGodley.initialiseGodleys(makeGodleyIt(godleyItems.begin()),
//        makeGodleyIt(godleyItems.end()), variables.values);

    flags=reset_needed;
  }


  const char* Minsky::minskyVersion=MINSKY_VERSION;

  void Minsky::cut()
  {
    copy();
    for (auto& i: canvas.selection.items)
      model->deleteItem(*i);
    for (auto& i: canvas.selection.groups)
      model->removeGroup(*i);
    for (auto& i: canvas.selection.wires)
      model->removeWire(*i);
    garbageCollect();
    canvas.item.reset();
    canvas.itemFocus.reset();
#ifndef NDEBUG
    for (auto& i: canvas.selection.items)
      assert(i.use_count()==1);
    for (auto& i: canvas.selection.groups)
      assert(i.use_count()==1);
    for (auto& i: canvas.selection.wires)
      assert(i.use_count()==1);
#endif
    canvas.selection.clear();
  }

  void Minsky::copy() const
  {
    schema1::Minsky m(canvas.selection);
    ostringstream os;
    xml_pack_t packer(os, schemaURL);
    xml_pack(packer, "Minsky", m);
    putClipboard(os.str());
  }

  void Minsky::saveGroupAsFile(const Group& g, const string& fileName) const
  {
    schema1::Minsky m(g);
    ofstream os(fileName);
    xml_pack_t packer(os, schemaURL);
    xml_pack(packer, "Minsky", m);
  }

  void Minsky::paste()
  {
    istringstream is(getClipboard());
    xml_unpack_t unpacker(is);
    schema1::Minsky m;
    xml_unpack(unpacker, "Minsky", m);
    GroupPtr g(new Group);
    canvas.setItemFocus(model->addGroup(g));
    m.populateGroup(*g);
    g->resizeOnContents();
  }

  void Minsky::toggleSelected(ItemType itemType, int item)
  {
    //TODO: individually add or remove item from selection
  }


  GroupPtr Minsky::insertGroupFromFile(const char* file)
  {
    schema1::Minsky currentSchema;
    ifstream inf(file);
    xml_unpack_t saveFile(inf);
    xml_unpack(saveFile, "Minsky", currentSchema);

    if (currentSchema.version != currentSchema.schemaVersion)
      throw error("Invalid Minsky schema file");

    GroupPtr g(new Group);
    currentSchema.populateGroup(*model->addGroup(g));
    return g;
  }

//  vector<int> Minsky::unwiredOperations() const
//  {
//    return model->findItemIds([&](const ItemPtr& x) {
//        if (auto o=dynamic_cast<OperationBase*>(x.get()))
//          for (auto& p: o->ports)
//            if (p->input() && !p->multiWireAllowed() && p->wires.empty())
//              return true;
//        return false;
//      }
//      );
//  }
//
  void Minsky::makeVariablesConsistent()
  {
    // remove variableValues not in variables
    set<string> existingNames;
    existingNames.insert("constant:zero");
    existingNames.insert("constant:one");
    model->recursiveDo(&Group::items, 
                       [&](Items&,Items::iterator i) {
                         if (auto v=dynamic_cast<VariableBase*>(i->get()))
                           existingNames.insert(v->valueId());
                         // ensure Godley table variables are the correct types
                         if (auto g=dynamic_cast<GodleyIcon*>(i->get()))
                           g->update();
                         return false;
                       }
                       );
    for (auto i=variableValues.begin(); i!=variableValues.end(); )
      if (existingNames.count(i->first))
        ++i;
      else
        variableValues.erase(i++);
    
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
      if (v->second.temp())
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
    assert(variableValues.validEntries());
    system.populateEvalOpVector(equations, integrals);
    assert(variableValues.validEntries());

    // attach the plots
    model->recursiveDo
      (&Group::items,
       [&](Items& m, Items::iterator i)
       {
         if (auto p=dynamic_cast<PlotWidget*>(i->get()))
           {
             p->disconnectAllVars();// clear any old associations
             p->clearPenAttributes();
             p->autoScale();
             for (size_t i=0; i<p->ports.size(); ++i)
               {
                 auto& pp=p->ports[i];
                 if (pp->wires.size()>0 && pp->getVariableValue().idx()>=0)
                   p->connectVar(pp->getVariableValue(), i);
               }
           }
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
    vector<string> columns=godley.table.getColumnVariables();
    // convert to valueIds
    for (auto& i: columns)
      i=VariableValue::valueId(godley.group.lock(), i);
    model->recursiveDo
      (&Group::items,
       [&](Items& m, Items::iterator it)
       {
         if (auto gi=dynamic_cast<GodleyIcon*>(it->get()))
           {
             vector<string> columns=gi->table.getColumnVariables();
             // convert to valueIds
             for (auto& i: columns)
               i=VariableValue::valueId(godley.group.lock(), i);
             for (size_t i=0; i<columns.size(); ++i)
              {
                if (&gi->table==&godley.table || r.count(columns[i]) || gi->table._assetClass(i+1)!=ac) 
                  {
                    r.erase(columns[i]); // column already duplicated, or in current, nothing to match
                    duplicatedColumns.insert(columns[i]);
                  }
                else if (!duplicatedColumns.count(columns[i]))
                  r.insert(columns[i]);
              }
          }
        return false;
      });
    for (size_t i=0; i<columns.size(); ++i)
      {
        if (r.count(columns[i]) || godley.table._assetClass(i+1)!=ac) 
          {
            r.erase(columns[i]); // column already duplicated, or in current, nothing to match
            duplicatedColumns.insert(columns[i]);
          }
        else if (!duplicatedColumns.count(columns[i]))
          r.insert(columns[i]);
      }

    // rewrite valueIds in terms local to the Godley table
    set<string> r1;
    bool toplevel=false;
    if (auto g=godley.group.lock())
      toplevel=!g->group.lock();
    for (auto& i: r)
      {
        auto p=i.find(':');
        if (p==string::npos)
          r1.insert(i);
        else
          {
            auto s=i.substr(p); // strip off scope qualifier
            if (VariableValue::valueId(godley.group.lock(), s)==i)
              r1.insert(toplevel? s.substr(1): s); // variable is accessible from current table
          }
      }
    return r1;
  }

  void Minsky::importDuplicateColumn(const GodleyTable& srcTable, int srcCol)
  {
    // find any duplicate column, and use it to do balanceDuplicateColumns
    const string& colName=trimWS(srcTable.cell(0,srcCol));
    if (colName.empty()) return; //ignore blank columns

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

  void Minsky::balanceDuplicateColumns(const GodleyIcon& srcGodley, int srcCol)
  {
    const GodleyTable& srcTable=srcGodley.table;
    // find if there is a matching column
    const string& colName=srcGodley.valueId(trimWS(srcTable.cell(0,srcCol)));
    if (colName.empty()) return; //ignore blank columns

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
                             destTable.cell(r,col)=to_string(-fc.coef)+fc.name;
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
                           VariableValue::scope(i->first);
                         FlowCoef df;
                         if (scope==-1 || !variableValues.count(i->first))
                           df.name=VariableValue::uqName(i->first);
                         else
                           df.name=variableValues[i->first].name;
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
    EvalOpBase::t=t=0;
    constructEquations();
    // if no stock variables in system, add a dummy stock variable to
    // make the simulation proceed
    if (stockVars.empty()) stockVars.resize(1,0);

    initGodleys();

    model->recursiveDo
      (&Group::items,
       [&](Items& m, Items::iterator i)
       {
         if (auto p=dynamic_cast<PlotWidget*>(i->get()))
           p->clear();
         return false;
       });

    if (stockVars.size()>0)
      {
        if (order==1 && !implicit)
          ode.reset(); // do explicit Euler
        else
          ode.reset(new RKdata(this)); // set up GSL ODE routines
      }

    flags &= ~reset_needed;
    // update flow variable
    for (size_t i=0; i<equations.size(); ++i)
      equations[i]->eval(&flowVars[0], &stockVars[0]);
  }

  void Minsky::step()
  {
    if (reset_flag())
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

    model->recursiveDo
      (&Group::items, 
       [&](Items&, Items::iterator i) 
       {(*i)->updateIcon(t); return false;});

    // throttle redraws
    static const time_duration maxWait=milliseconds(100);
    if ((microsec_clock::local_time()-(ptime&)lastRedraw) > maxWait)
      {
        canvas.requestRedraw();
        lastRedraw=microsec_clock::local_time();
      }

  }

  string Minsky::diagnoseNonFinite() const
  {
    // firstly check if any variables are not finite
    for (VariableValues::const_iterator v=variableValues.begin();
         v!=variableValues.end(); ++v)
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
        evalGodley.eval(&d[0], &df[0]);
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
    flags &= ~is_edited;
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
    if (saveFile.exists("root"))
      {
        // we're dealing with a schema 0 file
        schema0::Minsky m;
        m.load(filename);
        currentSchema=schema1::Minsky(m);
      }
    else
      xml_unpack(saveFile, "Minsky", currentSchema);
    
    // fix corruption caused by ticket #329
    currentSchema.removeIntVarOrphans();

    switch (currentSchema.version)
      {
      default:
        *this = currentSchema;
      }
//
//    variables.makeConsistent();

    

//    for (GodleyItems::iterator g=godleyItems.begin(); g!=godleyItems.end(); ++g)
//      g->update();
//
//    for (GroupIcons::iterator g=groupItems.begin(); g!=groupItems.end(); ++g)
//      {
//        // ensure group attributes correctly set
//        const vector<int>& vars= g->variables();
//        for (vector<int>::const_iterator i=vars.begin(); i!=vars.end(); ++i)
//          {
//            const VariablePtr& v=variables[*i];
//            v->group=g->id();
//            v->visible=g->displayContents();
//          }
//        const vector<int>& ops= g->operations();
//        for (vector<int>::const_iterator i=ops.begin(); i!=ops.end(); ++i)
//          {
//            OperationPtr& o=operations[*i];
//            o->group=g->id();
//            o->visible=g->displayContents();
//          }
//        const vector<int>& gwires= g->wires();
//        for (vector<int>::const_iterator i=gwires.begin(); i!=gwires.end(); ++i)
//          {
//            Wire& w=wires[*i];
//            w.group=g->id();
//            w.visible=g->displayContents();
//          }
//      }
//
//    removeDuplicateWires();
//
    // try resetting the system, but ignore any errors
    try {reset();}
    catch (...) {}
    panopticon.requestRedraw();
    flags=reset_needed;
  }

  void Minsky::exportSchema(const char* filename, int schemaLevel)
  {
    xsd_generate_t x;
    // currently, there is only 1 schema level, so ignore second arg
    xsd_generate(x,"Minsky",schema1::Minsky());
    ofstream f(filename);
    x.output(f,schemaURL);
  }

//  int Minsky::opIdOfEvalOp(const EvalOpBase& e) const
//  {
//    if (e.state)
//      for (Operations::const_iterator j=operations.begin(); 
//           j!=operations.end(); ++j)
//        if (e.state==*j)
//          return j->id();
//    return -1;
//  }


  ecolab::array<int> Minsky::opOrder() const
  {
    ecolab::array<int> r;
//    for (size_t i=0; i<equations.size(); ++i)
//      r<<opIdOfEvalOp(*equations[i]);
    return r;
  }

  vector<string> Minsky::accessibleVars() const
  {
    set<string> r;
    // insert global variables
    for (auto i: variableValues)
      if (i.first[0]==':')
        r.insert(i.first);
    if (canvas.item)
      if (auto g=canvas.item->group.lock())
        {
          // first add local variables
          for (auto& i: g->items)
            if (auto v=dynamic_cast<VariableBase*>(i.get()))
              r.insert(v->name());
          // now add variables in outer scopes, ensuring they qualified
          for (g=g->group.lock(); g;  g=g->group.lock())
            for (auto& i: g->items)
              if (auto v=dynamic_cast<VariableBase*>(i.get()))
                {
                  auto n=v->name();
                  if (!n.empty())
                    if (n[0]==':')
                      r.insert(n);
                    else
                      r.insert(':'+n);
                }
        }
    return vector<string>(r.begin(),r.end());
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
                cminsky().displayErrorItem(p->item);
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
        for (unsigned j=1; j<i->ports.size(); ++j)
          net.emplace(i->ports[j].get(), i->ports[0].get());
    
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
      if (!inputWired(v.first) && v.second.idx()>=0)
        fvInit[v.second.idx()]=true;

    for (auto& e: equations)
      {
        const EvalOpBase& eo=*e;
        if (eo.out < 0|| (eo.numArgs()>0 && eo.in1<0) ||
            (eo.numArgs() > 1 && eo.in2<0))
          {
            //cerr << "Incorrectly wired operation "<<opIdOfEvalOp(eo)<<endl;
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
                  fvInit[eo.in1] |= op->ports[1]->wires.empty();
                  fvInit[eo.in2] |= op->ports[3]->wires.empty();
                  break;
                default: break;
                }
            
            fvInit[eo.out]=
              (!eo.flow1 ||  fvInit[eo.in1]) && (!eo.flow2 ||  fvInit[eo.in2]);
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
    if (op.visible())
      {
        canvas.item=canvas.model->findItem(op);
        canvas.indicateItem();
        canvas.requestRedraw();
      }
    else if (auto g=op.group.lock())
      {
        while (g && !g->visible()) g=g->group.lock();
        if (g && g->visible())
          {
            canvas.item=g;
            canvas.indicateItem();
            canvas.requestRedraw();
          }
      }
  }
  
  bool Minsky::pushHistoryIfDifferent()
  {
    // go via a schema object, as serialising minsky::Minsky has
    // problems due to port management
    schema1::Minsky m(*this);
    pack_t buf;
    buf<<m;
    if (history.empty())
      {
        history.emplace_back();
        buf.swap(history.back());
        return true;
      }
    if (memcmp(buf.data(), history.back().data(), buf.size())!=0)
      {
        // check XML versions differ (slower)
        ostringstream prev, curr;
        xml_pack_t prevXbuf(prev), currXbuf(curr);
        xml_pack(currXbuf,"Minsky",m);
        history.back().reseto()>>m;
        xml_pack(prevXbuf,"Minsky",m);

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
            return true;
          }
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

  void Minsky::convertVarType(const string& name, VariableType::Type type)
  {
    assert(VariableValue::isValueId(name));
    VariableValues::iterator i=variableValues.find(name);
    if (i==variableValues.end())
      throw error("variable %s doesn't exist",name.c_str());
    if (i->second.type()==type) return; // nothing to do!

    model->recursiveDo
      (&GroupItems::items,
       [&](const Items&,Items::const_iterator i)
       {
         if (auto g=dynamic_cast<GodleyIcon*>(i->get()))
           {
             if (type!=VariableType::flow)
               for (auto v: g->flowVars)
                 if (v->valueId()==name)
                   throw error("flow variables in Godley tables cannot be converted to a different type");
             if (type!=VariableType::stock)
               for (auto v: g->stockVars)
                 if (v->valueId()==name)
                   throw error("stock variables in Godley tables cannot be converted to a different type");
           }
         return false;
       });
                       
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
    i->second=VariableValue(type,i->second.name,i->second.init);
  }

  bool Minsky::inputWired(const std::string& name) const
  {
    bool r=false;
    model->recursiveDo
      (&Group::items,
       [&](Items&,Items::const_iterator i) {
        if (auto v=dynamic_cast<VariableBase*>(i->get()))
          if (v->valueId()==name)
            {
              r=v->ports.size()>1 && !v->ports[1]->wires.empty();
              return true;
            }
        return false;
      });
    return r;
  }

  cairo::SurfacePtr Minsky::vectorRender(const char* filename, cairo_surface_t* (*s)(const char *,double,double))
  {
    cairo::SurfacePtr tmp(new cairo::Surface(cairo_recording_surface_create
                                      (CAIRO_CONTENT_COLOR_ALPHA,nullptr)));
    canvas.surface.swap(tmp);
    canvas.redraw();
    double left=canvas.surface->left(), top=canvas.surface->top();
    canvas.surface->surface
      (s(filename, canvas.surface->width(), canvas.surface->height()));
    if (s==cairo_ps_surface_create)
      cairo_ps_surface_set_eps(canvas.surface->surface(),true);
    cairo_surface_set_device_offset(canvas.surface->surface(), -left, -top);
    canvas.redraw();
    canvas.surface.swap(tmp);
    return tmp;
  }
  
  void Minsky::renderCanvasToPS(const char* filename)
  {vectorRender(filename,cairo_ps_surface_create);}

  void Minsky::renderCanvasToPDF(const char* filename)
  {vectorRender(filename,cairo_pdf_surface_create);}

  void Minsky::renderCanvasToSVG(const char* filename)
  {vectorRender(filename,cairo_svg_surface_create);}

  namespace
  {
    cairo_surface_t* pngDummy(const char*,double width,double height)
    {return cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height);}
  }
  
  void Minsky::renderCanvasToPNG(const char* filename)
  {
    auto tmp=vectorRender(filename,pngDummy);
    cairo_surface_write_to_png(tmp->surface(),filename);
  }

  void Minsky::setAllDEmode(bool mode) {
    model->recursiveDo(&GroupItems::items, [mode](Items&,Items::iterator i) {
        if (auto g=dynamic_cast<GodleyIcon*>(i->get()))
          g->table.setDEmode(mode);
        return false;
      });
  }

}

