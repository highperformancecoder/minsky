/*
  @copyright Steve Keen 2018
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
#ifndef CSVPARSER_H
#define CSVPARSER_H

#include "dimension.h"
#include "dataSpecSchema.h"
#include "classdesc_access.h"
#include "str.h"

#include <atomic>
#include <cstddef>
#include <string>
#include <set>
#include <fstream>

namespace minsky
{
  class VariableValue;

  class DataSpec: public DataSpecSchema
  {
    std::size_t m_nRowAxes=0, m_nColAxes=0;
    CLASSDESC_ACCESS(DataSpec);
  public:
    /// start row of the data area
    std::size_t nRowAxes() const {return m_nRowAxes;}
    /// start column of the data area
    std::size_t nColAxes() const {return m_nColAxes;}

    DataSpec()=default;
    DataSpec(const DataSpec& x): DataSpecSchema(x) {}
    DataSpec& operator=(const DataSpec& x) {DataSpecSchema::operator=(x); return *this;}
    
    // handle extra initialisation on conversion
    DataSpecSchema toSchema() {
      dataRowOffset=nRowAxes();
      dataColOffset=nColAxes();
      return *this;
    }

    DataSpec& operator=(const DataSpecSchema& x) {
      DataSpecSchema::operator=(x);
      setDataArea(dataRowOffset, dataColOffset);
      dimensionCols=x.dimensionCols; // revert clobber by setDataArea
      dataCols=x.dataCols; // revert clobber by setDataArea
      return *this;
    }
    
    void toggleDimension(std::size_t c) {
      auto i=dimensionCols.find(c);
      if (i==dimensionCols.end())
        dimensionCols.insert(c);
      else
        dimensionCols.erase(i);
    }

    /// set top left cell of the data area
    void setDataArea(std::size_t row, std::size_t col);
    
    /// initial stab at dataspec from examining stream
    void guessFromStream(std::istream& file);

    /// initial stab at dataspec from examining file
    void guessFromFile(const std::string& fileName) {
      std::ifstream is(fileName);
      stripByteOrderingMarker(is);
      guessFromStream(is);
    }

    /// populates this spec from a "RavelHypercube" entry, \a row is the row being read, used to set the headerRow attribute
    void populateFromRavelMetadata(const std::string& metadata, std::size_t row);

  private:
    /// try to fill in remainder of spec, given a tokenizer function tf
    /// eg boost::escaped_list_separator<char> tf(escape,separator,quote)
    template <class T>
    void givenTFguessRemainder(std::istream& initialInput, std::istream& remainingInput, const T& tf);

    /// figure out the tokenizer function and call givenTFguessRemainder
    void guessRemainder(std::istream& initialInput, std::istream& remainingInput, char separator);

    std::vector<size_t> starts;
    size_t nCols=0;
    size_t row=0;
    size_t firstEmpty=std::numeric_limits<size_t>::max();

    /// process chunk of input, updating guessed spec
    /// @return true if there's no more work to be done
    template <class T>
    bool processChunk(std::istream& input, const T& tf, size_t until);
  };

  /// creates a report CSV file from input, with errors sorted at
  /// begining of file, with a column for error messages
  void reportFromCSVFile(std::istream& input, std::ostream& output, const DataSpec& spec);

  /// load a variableValue from a stream according to data spec
  /// @param fileSize size of file to read (for progress bar)
  void loadValueFromCSVFile(VariableValue&,std::istream&,const DataSpec&,uintmax_t fileSize);

  /// replace doubled quotes with escaped quotes
  void escapeDoubledQuotes(std::string&,const DataSpec&);
}

#include "CSVParser.cd"
#include "CSVParser.xcd"
#endif
