#include "databaseIngestor.h"
#include "databaseIngestor.cd"
#include "databaseIngestor.xcd"
#include "databaseIngestor.rcd"
#include "progress.h"
#include "minsky.h"
#include "minsky_epilogue.h"

#include <filesystem>

using namespace std;
namespace minsky
{
  namespace
  {
    ProgressUpdater* ingestorProgress;
    
    void progress(const char* filename, double fraction)
    {
      if (ingestorProgress) ingestorProgress->setProgress(fraction);
    }
  }
  
  void DatabaseIngestor::importFromCSV(const std::vector<std::string>& filenames)
  {
    auto& m=minsky();
    ProgressUpdater pu(m.progressState,"Importing",filenames.size());
    // set the custom progress callback if global Minsky object is derived only
    if (typeid(m)!=typeid(Minsky))
      db.loadDatabaseCallback(progress);
    for (auto& f: filenames)
      {
        filesystem::path p(f);
        ProgressUpdater pu(m.progressState,"Importing: "+p.filename().string(),100);
        ingestorProgress=&pu;
        db.loadDatabase({f}, spec);
        ++m.progressState;
      }
    ingestorProgress=nullptr;
  }
}
