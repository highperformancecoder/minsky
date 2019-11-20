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
      json_pack_t nullj(json_spirit::mValue::null);
      process("/minsky/load","GoodwinLinear02.mky");
      auto jmodel=process("/minsky/model",nullj);
      CHECK_EQUAL(json(*minsky.model), write(jmodel));
      CHECK_EQUAL("classdesc::shared_ptr<::minsky::Group>",process("/minsky/model/@type",nullj).get_str());
      CHECK_EQUAL(R"([{"args":[],"ret":"::minsky::Group"},{"args":["::minsky::Group"],"ret":"::minsky::Group"}])",
                  write(process("/minsky/model/@signature",nullj)));
      
      
      auto modelMethods=process("/minsky/model/@list",nullj);
      CHECK(modelMethods.type()==json_spirit::array_type);
      CHECK(modelMethods.get_array().size()>0);
      CHECK(write(modelMethods).find("/items")!=std::string::npos);
      
      auto items=process("/minsky/model/items",nullj);
      CHECK_EQUAL(minsky.model->items.size(), items.get_array().size());
      CHECK_EQUAL(minsky.model->items.size(), process("/minsky/model/items/@size",{}).get_int());
      CHECK_EQUAL(R"([{"args":[],"ret":"std::vector<classdesc::shared_ptr<::minsky::Item>>"},{"args":["std::vector<classdesc::shared_ptr<::minsky::Item>>"],"ret":"std::vector<classdesc::shared_ptr<::minsky::Item>>"}])",
                  write(process("/minsky/model/items/@signature",nullj)));

      
      auto item0=process("/minsky/model/items/@elem/0",nullj);
      CHECK_EQUAL(json(minsky.model->items[0]), write(item0));
      CHECK_EQUAL("classdesc::shared_ptr<::minsky::Item>",
                  process("/minsky/model/items/@elem/0/@type",nullj).get_str());
      auto item0Methods=process("/minsky/model/items/@elem/0/@list",nullj);
      CHECK(item0Methods.type()==json_spirit::array_type);
      CHECK(item0Methods.get_array().size()>0);
      CHECK(write(item0Methods).find("/classType")!=std::string::npos);
      CHECK_EQUAL(R"({"args":[],"ret":"std::string"})",
                  write(process("/minsky/model/items/@elem/0/classType/@signature",nullj)));
      
      CHECK_EQUAL(minsky.variableValues.size(), process("/minsky/variableValues/@size",nullj).get_int());
      CHECK_EQUAL(minsky.variableValues.size(), process("/minsky/variableValues/",nullj).get_array().size());
      CHECK_EQUAL(json(minsky.variableValues[":K"]), write(process("/minsky/variableValues/@elem/:K/second",nullj)));
      CHECK_EQUAL("K", process("/minsky/variableValues/@elem/:K/second/name",nullj).get_str());
    }
}
