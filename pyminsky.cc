#include "minsky.h"
#include "minsky.pcd"

BOOST_PYTHON_MODULE(minsky)
{
  python_t p;
  p.addObject("minsky",minsky::minsky());
}
