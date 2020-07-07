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
#include "classdesc_access.h"

#include <stddef.h>
#include <string>
#include <set>
#include <fstream>

namespace minsky
{
  class VariableValue;

  class DataSpec
  {
    size_t m_nRowAxes=0, m_nColAxes=0;
    CLASSDESC_ACCESS(DataSpec);
  public:
    char separator=',', quote='"', escape='\\', decSeparator='.';
    bool mergeDelimiters=false;
    bool columnar=false;
    double missingValue=nan("");
    /// number of header rows
    size_t headerRow=0;
    /// start row of the data area
    size_t nRowAxes() const {return m_nRowAxes;}
    /// start column of the data area
    size_t nColAxes() const {return m_nColAxes;}
    std::string horizontalDimName="?";
    civita::Dimension horizontalDimension;
    
    /// what to do with duplicate keys
    enum DuplicateKeyAction {throwException, sum, product, min, max, av};
    DuplicateKeyAction duplicateKeyAction=throwException;
    
    /// rows and columns that are comment lines to be ignored
    std::set<unsigned> dimensionCols;
    void toggleDimension(size_t c) {
      auto i=dimensionCols.find(c);
      if (i==dimensionCols.end())
        dimensionCols.insert(c);
      else
        dimensionCols.erase(i);
    }

    void setDataArea(size_t row, size_t col);
    
    std::vector<civita::Dimension> dimensions;
    std::vector<std::string> dimensionNames;
    
    /// initial stab at dataspec from examining stream
    void guessFromStream(std::istream& file);

    /// initial stab at dataspec from examining file
    void guessFromFile(const std::string& fileName) {
      std::ifstream is(fileName);
      guessFromStream(is);
    }

    
  private:
    /// try to fill in remainder of spec, given a tokenizer function tf
    /// eg boost::escaped_list_separator<char> tf(escape,separator,quote)
    template <class T>
    void givenTFguessRemainder(std::istream&, const T& tf);

    template <class T>
    void guessDimensionsFromStream(std::istream&, const T& tf);

    void guessDimensionsFromStream(std::istream&);
        
    /// figure out the tokenizer function and call givenTFguessRemainder
    void guessRemainder(std::istream&, char separator);
  };

  /// creates a report CSV file from input, with errors sorted at
  /// begining of file, with a column for error messages
  void reportFromCSVFile(std::istream& input, std::ostream& output, const DataSpec& spec);

  /// load a variableValue from a stream according to data spec
  void loadValueFromCSVFile(VariableValue&,std::istream&,const DataSpec&);
}

#include "CSVParser.cd"
#endif
