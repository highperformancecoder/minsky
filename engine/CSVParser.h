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
#include "variableValue.h"

#include <stddef.h>
#include <string>
#include <set>
#include <fstream>

namespace minsky
{
  class DataSpec
  {
  public:
    char separator=',', quote='"', escape='\\';
    bool mergeDelimiters=false;
    double missingValue=nan("");
    /// size of the header area in rows and columns (alternatively,
    /// start row/col of data area
    size_t nRowAxes=0, nColAxes=0;
    std::string horizontalDimName="?";

    /// rows and columns that are comment lines to be ignored
    std::set<unsigned> commentRows, commentCols;
    
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

    /// figure out the tokenizer function and call givenTFguessRemainder
    void guessRemainder(std::istream&, char separator);
  };

  /// load a variableValue from a stream according to data spec
  void loadValueFromCSVFile(VariableValue&,std::istream&,const DataSpec&);
}

#include "CSVParser.cd"
#endif
