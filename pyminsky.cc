#include "minsky.h"
#include "minsky.pcd"
#include "rungeKutta.pcd"
#include "simulation.pcd"
#include "variableValue.pcd"
#include "minsky_epilogue.h"

namespace minsky
{
  namespace {
    Minsky* s_minsky() {
      static Minsky minsky;
      return &minsky;
    }
    Minsky* l_minsky=s_minsky();
  }
  Minsky& minsky()
  {
    return *l_minsky;
  }

  LocalMinsky::LocalMinsky(Minsky& minsky) {l_minsky=&minsky;}
  LocalMinsky::~LocalMinsky() {l_minsky=s_minsky();}
  // GUI callback needed only to solve linkage problems
  void doOneEvent(bool) {}
}

using namespace minsky;

BOOST_PYTHON_MODULE(pyminsky)
{
  classdesc::python_t p;
  p.defineClass<Minsky>();
  p.addObject("minsky",minsky::minsky());
}
