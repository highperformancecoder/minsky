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

#ifndef SCHEMA_HELPER
#define SCHEMA_HELPER

#include "operation.h"
#include "variable.h"
#include "godleyTable.h"
#include "godleyIcon.h"
#include "ravelWrap.h"
#include "variableValue.h"
#include <xml_unpack_base.h>

template <class T>
ecolab::array<T> toArray(const std::vector<T>& v) 
{
  ecolab::array<T> a(v.size());
  for (std::size_t i=0; i<v.size(); ++i) a[i]=v[i];
  return a;
}

template <class T>
std::vector<T> toVector(const ecolab::array<T>& a) 
{
  std::vector<T> v(a.size());
  for (std::size_t i=0; i<v.size(); ++i) v[i]=a[i];
  return v;
}

template <class T>
std::vector<double> toDoubleVector(const ecolab::array<T>& a) 
{
  std::vector<double> v(a.size());
  for (std::size_t i=0; i<v.size(); ++i) v[i]=a[i];
  return v;
}

namespace minsky
{
  /**
     A bridge pattern to allow schemas to update private members of
     various classes, whilst retaining desired
     encapsulation. SchemaHelper is priveleged to allow access to
     private parts of the class to be initialised, but should only be
     used by schema classes.
  */
  struct SchemaHelper
  {
    static void setPrivates
    (minsky::GodleyTable& g, const vector<vector<string> >& data, 
     const vector<GodleyTable::AssetClass>& assetClass)
    {
      g.data=data;
      g.m_assetClass=assetClass;
    }
    static void setPrivates
    (minsky::GodleyIcon& g, const vector<vector<string> >& data, 
     const vector<GodleyTable::AssetClass>& assetClass)
    {
      setPrivates(g.table, data, assetClass);
    }

    static void setVariableDisplay(minsky::GodleyIcon& g, bool variableDisplay)
    {
      g.m_variableDisplay=variableDisplay;
    }
    
    static void setStockAndFlow(minsky::GodleyIcon& g,
                         const minsky::GodleyIcon::Variables& flowVars,
                         const minsky::GodleyIcon::Variables& stockVars)
    {
      g.m_flowVars=flowVars;
      g.m_stockVars=stockVars;
    }

    static void initHandleState(minsky::Ravel& r, const ravel::RavelState& s)
    {r.initState=s;}

  };

  template <class PreviousSchema, class CurrentSchema>
  void loadSchema(CurrentSchema& currentSchema,
                  classdesc::xml_unpack_t& data, const std::string& rootElement)
  {
    xml_unpack(data, rootElement, currentSchema);
    if (currentSchema.schemaVersion < currentSchema.version)
      {
        PreviousSchema prevSchema(data);
        currentSchema=prevSchema;
      }
    else if (currentSchema.schemaVersion > currentSchema.version)
      throw error("Minsky schema version %d not supported",currentSchema.schemaVersion);
  }

  /// decode ascii-encoded representation to binary data
  classdesc::pack_t decode(const classdesc::CDATA&);
  /// encode binary data to ascii-encoded 
  classdesc::CDATA encode(const classdesc::pack_t&);

  
}

#endif
