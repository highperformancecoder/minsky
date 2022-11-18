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
#ifndef GODLEYTABLE_H
#define GODLEYTABLE_H

#include <set>
#include <vector>

#include <ecolab.h>

#include "variable.h"
#include "assetClass.h"

namespace minsky
{
  using namespace std;
  using classdesc::shared_ptr;

  class GodleyTable: public GodleyAssetClass
  {
  public:

    friend struct SchemaHelper;
    friend class GodleyIcon;
    typedef std::vector<std::vector<string>> Data;
  private:
    CLASSDESC_ACCESS(GodleyTable);
    /// class of each column (used in DE compliant mode)
    vector<AssetClass> m_assetClass{noAssetClass, asset, liability, equity};
    Data data;

    static void markEdited(); ///< mark model as having changed
    void _resize(unsigned rows, unsigned cols) {
      // resize existing
      for (std::size_t i=0; i<data.size(); ++i) data[i].resize(cols);
      data.resize(rows, vector<string>(cols));
      m_assetClass.resize(cols, noAssetClass);
    }
  public:
    typedef GodleyAssetClass::AssetClass AssetClass;
    
    bool doubleEntryCompliant;

    std::string title;
  
    static const char* initialConditions;
    GodleyTable(): doubleEntryCompliant(true)
    {
      _resize(2,4);
      cell(1,0)=initialConditions;
      
    }
    
    // Perform deep comparison of Godley tables in history to avoid spurious noAssetClass columns from arising during undo. For ticket 1118.
    bool operator==(const GodleyTable& other) const 
    {
		return (data==other.data && m_assetClass==other.m_assetClass &&
		 doubleEntryCompliant==other.doubleEntryCompliant && title==other.title);
    }    

    /// class of each column (used in DE compliant mode)
    const vector<AssetClass>& _assetClass() const {return m_assetClass;}
    AssetClass _assetClass(std::size_t col) const;
    AssetClass _assetClass(std::size_t col, AssetClass cls);
    
    /// Check whether more than one equity column is present
    /// irrespective of single or multiple equity column mode.
    bool singleEquity() const;

    /**
     * Generates a unique name for this table.
     */
    void nameUnique();

    /// The usual mathematical sign convention is reversed in double
    /// entry book keeping conventions if the asset class is a liability
    /// or equity
    bool signConventionReversed(int col) const
    {
      return doubleEntryCompliant && 
        (_assetClass(col)==liability || _assetClass(col)==equity);
    }
    /**
       TCL accessor method 
       @param col - column number
       @param [opt] assetClass (symbolic name).
       @return current asset class value for column \a col
       sets if assetClass present, otherwise gets
    */
    string assetClass(ecolab::TCL_args args); 
  
    /// returns true if \a row is an "Initial Conditions" row
    bool initialConditionRow(unsigned row) const;
    
    /// return true if row is empty apart from a value in column \a col
    bool singularRow(unsigned row, unsigned col);
    
    std::size_t rows() const {return data.size();}
    std::size_t cols() const {return data.empty()? 0: data[0].size();}

    void clear() {data.clear(); m_assetClass.clear(); markEdited();}
    void resize(unsigned rows, unsigned cols){_resize(rows,cols); markEdited();}

    /** @{ In the following, C++ data structure is off by one with
        respect to the TCL table, as the TCL table has an extra row
        and column for the +/- buttons. So deleting the row/column
        before the index has the effect of deleting the row/column at
        the index in the TCL table, and inserting row/column at the
        index in C++ has the effect of inserting before the index in
        TCL
    */
    /// insert row at \a row
    void insertRow(unsigned row);
    /// delete row at \a row
    void deleteRow(unsigned row) {data.erase(data.begin()+row);}
    /// insert col at \a col
    void insertCol(unsigned col);
    /// delete col before \a col
    void deleteCol(unsigned col);
    /** @} */

    /// move row \a row down by \a n places (up if -ve)
    void moveRow(int row, int n);
    void moveCol(int col, int n);

    void dimension(unsigned rows, unsigned cols) {clear(); resize(rows,cols);}

    string& cell(unsigned row, unsigned col) {
      if (row>=rows() || col>=cols())
        _resize(row+1, col+1);
      if (data[row].size()<=col) data[row].resize(cols());
      return data[row][col];
    }
    const string& cell(unsigned row, unsigned col) const {
      if (row>=data.size() || col>=data[row].size())
        throw std::out_of_range("Godley table index error");
      return data[row][col];
    }
    bool cellInTable(int row, int col) const
    {return row>=0 && std::size_t(row)<rows() && col>=0 && std::size_t(col)<cols();}
    string getCell(unsigned row, unsigned col) const {
      if (row<rows() && col<cols())
        return cell(row,col);
      else
        return "";
    }

    /// get the set of column labels, in column order
    std::vector<std::string> getColumnVariables() const;
    /// get the vector of unique variable names from the interior of the
    /// table, in row, then column order
    std::vector<std::string> getVariables() const;
    /// save text in currently highlighted column heading for renaming all variable instances
    /// and to enable user to fix problems
    std::string savedText;

    /// get column data
    std::vector<std::string> getColumn(unsigned col) const;
    
    /// toggle flow signs according to double entry compliant mode
    void setDEmode(bool doubleEntryCompliant);

    /// return the symbolic sum across a row
    string rowSum(int row) const;

    /// accessor for schema access
    const Data& getData() const {return data;}

    void exportToLaTeX(const std::string& filename) const;
    void exportToCSV(const std::string& filename) const;

    /// reorders columns into assets/liabilities and equities. Adds empty columns if an asset class is not present.
    void orderAssetClasses();

    /// rename all instances of a variable
    void rename(const std::string& from, const std::string& to);
    /// rename all instances of a flow variable
    void renameFlows(const std::string& from, const std::string& to);        
    /// rename a stock variable
    void renameStock(const std::string& from, const std::string& to);        
  };

}

#include "godleyTable.cd"
#include "godleyTable.xcd"
#endif
