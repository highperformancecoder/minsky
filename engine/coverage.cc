/*
  Temporarily in Minsky for development purposes, but will be moved to EcoLab
*/
#include "coverage.h"
#include <ecolab_epilogue.h>

namespace minsky
{
#ifdef TCL_COV
    // TCL coverage support
  TCLTYPE(Coverage);

  string Coverage::key(const string& file, int line) 
  {
      return file+":"+str(line);
  }
  void Coverage::add(const string& file, int line) 
  {
    (*this)[key(file,line)]++;
  }
#endif

}
