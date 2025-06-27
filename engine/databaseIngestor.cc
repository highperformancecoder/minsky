#include "databaseIngestor.h"
#include "databaseIngestor.cd"
#include "databaseIngestor.xcd"
#include "databaseIngestor.rcd"
#include "minsky_epilogue.h"

namespace minsky
{
  void DatabaseIngestor::importFromCSV(const std::vector<std::string>& filenames)
  {
    db.loadDatabase(filenames, spec);
  }
}
