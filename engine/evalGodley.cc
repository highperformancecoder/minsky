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

#include "cairoItems.h"
#include "evalGodley.h"
#include "group.h"
#include "selection.h"
#include "lasso.h"
#include "evalGodley.rcd"
#include "minsky_epilogue.h"

using namespace std;

namespace minsky
{
  namespace
  {
    void throwInvalidAssetLiabilityPair(const string& name)
    {
      throw error("shared column %s is not an asset/liability pair",
                  name.c_str());
    }
  }

  void SharedColumnCheck::checkShared(const string& name, AssetClass ac)
  {
    if (name.empty() || name==":_") 
      return; // ignore blank columns
    if (!colAssetType.insert(make_pair(name, ac)).second)
      {
        switch (ac)
          {
          case liability:
            if (colAssetType[name]!=asset)
              throwInvalidAssetLiabilityPair(name);
            break;
          case asset:
            if (colAssetType[name]!=liability)
              throwInvalidAssetLiabilityPair(name);
            break;
          case equity:
            break; // do not check duplicate column
          default:
            throw error("duplicated column %s detected",name.c_str());
          }
        // At this point, a shared pair of columns is OK
        if (!sharedCol.insert(name).second)
          throw error("More than two columns %s detected" ,name.c_str());
      }
  }

  bool SharedColumnCheck::updateColDefs(const string& col, const FlowCoef& fc)
  {
    const bool alreadySeen=sharedCol.contains(col);
    if (alreadySeen)
      colDef[col][fc.name]-=fc.coef;
    else
      colDef[col][fc.name]+=fc.coef;
    return alreadySeen;
  }

  void SharedColumnCheck::checkSharedColDefs() const
  {
    // check that shared column definitions sum to zero
    for (set<string>::iterator i=sharedCol.begin(); i!=sharedCol.end(); ++i)
      {
        const map<string, double>& cdef=colDef[*i];
        for (map<string, double>::const_iterator j=cdef.begin(); 
             j!=cdef.end(); ++j)
          if (abs(j->second)>1e-30)
            throw error("column %s has mismatched flow %s",
                        i->c_str(), j->first.c_str());
      }
  }

  void EvalGodley::eval(double* sv, const double* fv) const
  {
    size_t initIdxSize=initIdx.size();
#ifdef _OPENMP
#pragma omp parallel for if(initIdxSize>20)
#endif
    for (size_t i=0; i<initIdxSize; ++i)
      sv[initIdx[i]]=0;

#ifdef _OPENMP
    size_t sidxSize=sidx.size();
#pragma omp parallel for if(sidxSize>20)
    for (size_t i=0; i<sidxSize; ++i)
      atomic_ref(sv[sidx[i]]) += fv[fidx[i]] * m[i];
#else
    for (size_t i=0; i<sidx.size(); ++i)
      sv[sidx[i]] += fv[fidx[i]] * m[i];
#endif
  }
}
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::EvalGodley);
