/*
  @copyright Steve Keen 2025
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

#ifndef DATABSE_INGESTOR_H
#define DATABSE_INGESTOR_H

#include "CSVParser.h"

#include <string>
#include <vector>

namespace minsky
{
  class DatabaseIngestor
  {
    soci::session session;
    template <class T> DatabaseIngestor::load
    (const std::vector<std::string>& filenames, const DataSpecSchema& spec);
  public:
    void connect(const std::string& dbType, const std::string& connection)
    {session.open(dbType,connection);}
    std::string table; // table name to use
    /// import CSV files, using \a spec
    void importFromCSV(const std::vector<std::string>& filenames,
                       const DataSpecSchema& spec);
  };
}

#endif
