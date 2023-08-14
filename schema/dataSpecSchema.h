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

#ifndef DATASPECSCHEMA_H
#define DATASPECSCHEMA_H
#include <set>
#include <vector>
#include <math.h>

namespace minsky
{
  struct DataSpecSchema
  {
    // these fields are only used for persistence. Need to be handled specially within schema code
    std::size_t dataRowOffset, dataColOffset;
    std::size_t numCols=0; ///< number of columns in CSV. Must be > dataColOffset
    
    char separator=',', quote='"', escape='\\', decSeparator='.';
    bool mergeDelimiters=false;
    bool columnar=false;
    double missingValue=nan("");
    /// number of header rows
    std::size_t headerRow=0;
    std::string horizontalDimName="?";
    civita::Dimension horizontalDimension;
    
    /// what to do with duplicate keys
    enum DuplicateKeyAction {throwException, sum, product, min, max, av};
    DuplicateKeyAction duplicateKeyAction=throwException;
    
    /// rows and columns that are comment lines to be ignored
    std::set<unsigned> dimensionCols, dataCols;
    std::vector<civita::Dimension> dimensions;
    std::vector<std::string> dimensionNames;
  };
}

#include "dataSpecSchema.cd"
#include "dataSpecSchema.xcd"
#endif
