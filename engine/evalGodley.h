/*
  @copyright Steve Keen 2013
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
#ifndef EVALGODLEY_H
#define EVALGODLEY_H

#include "schema/assetClass.h"
#include "classdesc_access.h"
#include "variableValue.h"
#include "str.h"
#include "flowCoef.h"
#include "variableManager.h"

#include <ecolab.h>
#include <arrays.h>

namespace minsky
{
  using namespace ecolab;

  class EvalGodley
  {
    /// representation of matrix connecting flow variables to stock variables
    array<int> sidx, fidx;
    array<double> m;

    /// index of stock variables that need to be zeroed at start of eval
    array<int> initIdx;

    CLASSDESC_ACCESS(EvalGodley);
  public:

    /// initialise this to a sequence of Godley tables - adaptors provided below
    template <class GodleyIterator> 
    void initialiseGodleys
    (const GodleyIterator& begin, const GodleyIterator& end, 
     const VariableValues& values);

    /// evaluate Godley tables on sv and current value of fv, storing
    /// result in output variable (of \a fv). \a sv is assume to be of
    /// size \c stockVars and \a fv is assumed to be of size \c
    /// flowVars.
    void eval(double sv[], const double fv[]) const;

    EvalGodley():  compatibility(false) {}
    /// if compatibility is true, then consttrainst between Godley
    /// tables is not applied, and shared columns are merely summed
    bool compatibility;

  };

  /// adaptor suitable for initialiseGodleys
  template <class T>  class GodleyIteratorAdaptor
  {
    T it;
  public:
    GodleyIteratorAdaptor(const T& it): it(it) {}
    void operator++() {++it;}
    bool operator!=(const GodleyIteratorAdaptor& x) const
    {return x.it!=it;}

    int id() const {return it->id();}

    /// Godley table data this points to
    const std::vector<std::vector<std::string> >& data() const;
    const GodleyAssetClass::AssetClass assetClass(size_t col) const;
    bool signConventionReversed(int col) const;
    bool initialConditionRow(int row) const;
    /// returns valueid for variable reference in table
    // TODO: this should be refactored to a more central location
    string valueId(const std::string& x) const {return it->valueId(x);}
  };

  template <class T> GodleyIteratorAdaptor<T> makeGodleyIt(const T& it)
  {return GodleyIteratorAdaptor<T>(it);}

  /// for checking shared columns between tables
  struct SharedColumnCheck: public GodleyAssetClass
  {

    /// asset type of previously seen column of this name
    std::map<string, AssetClass> colAssetType;
    /// indicates a column is shared between Godley tables
    std::set<string> sharedCol;

    /// check whether column \a name has already been seen, and if it
    /// has, whether it is allowed to be shared by business rules
    void checkShared(const string& name, AssetClass ac);

    /// store the sum of flow var contributions for each colum
    /// here. Used for checking that shared column definitions are
    /// equivalent
    ConstMap<string, std::map<string, double> > colDef;

    /// update col defs, give column name, and flow variable entry
    /// @return true if col has been seen before
    bool updateColDefs(const string& col, const FlowCoef& fc);

    /// check shared columns are equivalently defined
    void checkSharedColDefs() const;
  };


  template <class GodleyIterator> void EvalGodley::initialiseGodleys
  (const GodleyIterator& begin, const GodleyIterator& end, 
     const VariableValues& values)
  {
    SharedColumnCheck scCheck;
    sidx.resize(0);
    fidx.resize(0);
    m.resize(0);

    std::set<int> iidx;

    for (GodleyIterator g=begin; g!=end; ++g)
      {
        if (g.data().empty()) continue;
        // check for shared columns
        if (!compatibility)
          for (size_t col=1; col<g.data()[0].size(); ++col)
            scCheck.checkShared(g.valueId(trimWS(g.data()[0][col])), g.assetClass(col));

        for (size_t row=1; row<g.data().size(); ++row)
          if (!g.initialConditionRow(row))
            for (size_t col=1; col<g.data()[row].size(); ++col)
              {
                FlowCoef fvc(g.data()[row][col]);
                fvc.name=g.valueId(fvc.name);
                if (g.signConventionReversed(col)) fvc.coef*=-1;
                string svName(g.valueId(trimWS(g.data()[0][col])));

                if (!fvc.name.empty() && !svName.empty())
                  {
                    const VariableValue& fv=values[g.valueId(fvc.name)];
                    const VariableValue& sv=values[svName];
                    if (fv.idx()>=0 && sv.idx()>=0)
                      {
                        // check for compatible column definitions
                        if (!compatibility && 
                            scCheck.updateColDefs(svName, fvc))
                          continue;
                
                        iidx.insert(sv.idx());
                        sidx<<=sv.idx();
                        fidx<<=fv.idx();
                        m<<=fvc.coef;
                      }
                  }
              }
      }
    
    for (std::set<int>::iterator i=iidx.begin(); i!=iidx.end(); ++i)
      initIdx<<=*i;

    if (!compatibility)
      scCheck.checkSharedColDefs();
  }
                   
}

#include "evalGodley.cd"
#endif
