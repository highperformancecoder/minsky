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

#include "databaseIngestor.h"
#include "databaseIngestor.rcd"
#include "minsky_epilogue.h"

using namespace std;

namespace minsky
{
  void DatabaseIngestor::connect(const string& dbType, const string& connection)
  {
  }

  void DatabaseIngestor::createTable
  (const std::vector<std::string>& filenames, const DataSpec& spec)
  {
  }
  
  
  void DatabaseIngestor::importFromCSV
  (const std::vector<std::string>& filenames, const DataSpec& spec)
  {
  }

}

CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::DatabaseIngestor);
