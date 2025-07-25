/*
  @copyright Steve Keen 2021
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

#include "saver.h"
#include "schema3.h"
#include "CSVTools.xcd"
#include "dynamicRavelCAPI.xcd"
#include "minsky_epilogue.h"
#undef True
#include <UnitTest++/UnitTest++.h>
#include <boost/filesystem.hpp>
using namespace std;
using namespace classdesc;
using namespace minsky;
using namespace boost::filesystem;
using namespace std::literals::chrono_literals;

SUITE(Saver)
{
  struct Fixture
  {
    string saveFile;
    Fixture()
    {
      do
        saveFile=(temp_directory_path()/to_string(rand())).string();
      while (boost::filesystem::exists(saveFile));
    }
    ~Fixture()
    {
      remove(saveFile);
    }
  };
  
  TEST_FIXTURE(Fixture, backgroundSaver)
  {
    ostringstream os;
    {
      BackgroundSaver bsaver(saveFile);
      schema3::Minsky m;
      bsaver.save(m);
      xml_pack_t x(os,"http://minsky.sf.net/minsky");
      xml_pack(x, "Minsky", m);
      this_thread::sleep_for(100ms);
    }
    std::ifstream f(saveFile);
    string savedData, buffer;
    while (!getline(f, buffer).fail())
      savedData+=buffer;
    CHECK_EQUAL(os.str(), savedData);
  }

  //try to exercise the killthread feature
  TEST_FIXTURE(Fixture, raceSaver)
  {
    ostringstream os;
    {
      BackgroundSaver bsaver(saveFile);
      schema3::Minsky m;
      m.items.resize(10000);
      bsaver.save(m);
      this_thread::yield();
      bsaver.save(m);
      this_thread::yield();
      bsaver.save(m);
      xml_pack_t x(os,"http://minsky.sf.net/minsky");
      xml_pack(x, "Minsky", m);
      this_thread::sleep_for(100ms);
    }
    std::ifstream f(saveFile);
    string savedData, buffer;
    while (!getline(f, buffer).fail())
      savedData+=buffer;
    CHECK_EQUAL(os.str(), savedData);
  }
  
  TEST_FIXTURE(Fixture, badFile)
  {
    ostringstream os;
    BackgroundSaver bsaver("/dummy-dir/"+saveFile);
    schema3::Minsky m;
    bsaver.save(m);
    CHECK_THROW(bsaver.save(m), std::exception);
  }
}
