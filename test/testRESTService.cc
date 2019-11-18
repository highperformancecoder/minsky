#include "minskyRS.h"
#include "minsky_epilogue.h"
#include <UnitTest++/UnitTest++.h>

using namespace minsky;
using namespace classdesc;
using namespace std;

namespace
{
  struct TestFixture: public RESTProcess_t
  {
    Minsky minsky;
    LocalMinsky lm;
    TestFixture(): lm(minsky)
    {
      RESTProcess(*this,"/minsky",minsky);
    }
  };
}

SUITE(RESTService)
{
  TEST_FIXTURE(TestFixture, createLoadSaveModel)
    {
      process("/minsky/canvas/addVariable",{"foo","flow"});
      CHECK_EQUAL(1,minsky.model->items.size());
      process("/minsky/canvas/addOperation","time");
      CHECK_EQUAL(2,minsky.model->items.size());
      process("/minsky/save","test.mky");
      process("/minsky/clearAllMaps",{});
      CHECK_EQUAL(0,minsky.model->items.size());
      process("/minsky/load","test.mky");
      CHECK_EQUAL(2,minsky.model->items.size());
    }
  
  TEST_FIXTURE(TestFixture, containers)
    {
      process("/minsky/load","GoodwinLinear02.mky");
      auto jmodel=process("/minsky/model",{});
      CHECK_EQUAL(json(*minsky.model), write(jmodel));
      auto items=process("/minsky/model/items",{});
      CHECK_EQUAL(minsky.model->items.size(), items.get_array().size());
      CHECK_EQUAL(minsky.model->items.size(), process("/minsky/model/items/@size",{}).get_int());
      auto item0=process("/minsky/model/items/@elem/0",{});
      CHECK_EQUAL(json(minsky.model->items[0]), write(item0));
    }
}
