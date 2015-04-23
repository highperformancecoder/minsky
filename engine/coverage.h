/*
  Temporarily in Minsky for development purposes, but will be moved to EcoLab
*/
/*
  Provides support for regression test coverage analysis of TCL code
*/

#include "str.h"
#include <cachedDBM.h>
namespace minsky
{
  using namespace ecolab;
  struct Coverage: public cachedDBM<string,unsigned>
  {
    // copy operators just provide a default (inactive) db file
    Coverage& operator=(const Coverage&) {return *this;}
    Coverage(const Coverage&) {}
    Coverage() {}
    static string key(const string& file, int line);
    // add a code coverage point to the database
    void add(const string& file, int line);
  };
}

#include "coverage.cd"
