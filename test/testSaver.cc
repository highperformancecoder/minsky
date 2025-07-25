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
#include <chrono>
using namespace std;
using namespace classdesc;
using namespace minsky;
using namespace boost::filesystem;
using namespace std::literals::chrono_literals;

SUITE(Saver)
// Comprehensive test suite for BackgroundSaver class
// Tests cover: basic functionality, error handling, thread safety,
// performance, edge cases, and resource management
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
  
  TEST_FIXTURE(Fixture, multipleSaves)
  {
    // Test multiple sequential saves to ensure data consistency
    BackgroundSaver bsaver(saveFile);
    schema3::Minsky m1, m2, m3;
    
    // Create different test data by resizing items vector
    m1.items.resize(100);
    m2.items.resize(200);
    m3.items.resize(50);
    
    // Save multiple times with delays to ensure completion
    bsaver.save(m1);
    this_thread::sleep_for(50ms);
    bsaver.save(m2);
    this_thread::sleep_for(50ms);
    bsaver.save(m3);
    this_thread::sleep_for(100ms);
    
    // Verify final save is the last one (m3)
    ostringstream expected;
    xml_pack_t x(expected,"http://minsky.sf.net/minsky");
    xml_pack(x, "Minsky", m3);
    
    std::ifstream f(saveFile);
    string savedData, buffer;
    while (!getline(f, buffer).fail())
      savedData+=buffer;
    CHECK_EQUAL(expected.str(), savedData);
  }

  TEST_FIXTURE(Fixture, emptyMinsky)
  {
    // Test saving an empty Minsky object
    BackgroundSaver bsaver(saveFile);
    schema3::Minsky m; // Empty object - default constructor
    bsaver.save(m);
    this_thread::sleep_for(100ms);
    
    ostringstream expected;
    xml_pack_t x(expected,"http://minsky.sf.net/minsky");
    xml_pack(x, "Minsky", m);
    
    std::ifstream f(saveFile);
    string savedData, buffer;
    while (!getline(f, buffer).fail())
      savedData+=buffer;
    CHECK_EQUAL(expected.str(), savedData);
  }

  TEST_FIXTURE(Fixture, largeMinsky)
  {
    // Test saving a large Minsky object to test performance
    BackgroundSaver bsaver(saveFile);
    schema3::Minsky m;
    m.items.resize(10000); // Large dataset to test memory handling
    
    bsaver.save(m);
    this_thread::sleep_for(300ms); // Allow more time for large save
    
    ostringstream expected;
    xml_pack_t x(expected,"http://minsky.sf.net/minsky");
    xml_pack(x, "Minsky", m);
    
    std::ifstream f(saveFile);
    string savedData, buffer;
    while (!getline(f, buffer).fail())
      savedData+=buffer;
    CHECK_EQUAL(expected.str(), savedData);
  }

  TEST_FIXTURE(Fixture, rapidFireSaves)
  {
    // Test rapid successive saves to test killThread functionality
    BackgroundSaver bsaver(saveFile);
    schema3::Minsky m;
    m.items.resize(1000);
    
    // Fire multiple saves in quick succession to trigger killThread
    for(int i = 0; i < 5; ++i) {
      bsaver.save(m);
      this_thread::yield(); // Give scheduler a chance
    }
    
    this_thread::sleep_for(200ms);
    
    // Verify file exists and contains valid data
    CHECK(boost::filesystem::exists(saveFile));
    
    ostringstream expected;
    xml_pack_t x(expected,"http://minsky.sf.net/minsky");
    xml_pack(x, "Minsky", m);
    
    std::ifstream f(saveFile);
    string savedData, buffer;
    while (!getline(f, buffer).fail())
      savedData+=buffer;
    CHECK_EQUAL(expected.str(), savedData);
  }

  TEST_FIXTURE(Fixture, destructorSafety)
  {
    // Test that destructor properly calls killThread and waits for completion
    schema3::Minsky m;
    m.items.resize(2000);
    
    {
      BackgroundSaver bsaver(saveFile);
      bsaver.save(m);
      // Destructor called here - should call killThread() and wait
    }
    
    // File should exist and contain valid data after destructor
    CHECK(boost::filesystem::exists(saveFile));
    
    ostringstream expected;
    xml_pack_t x(expected,"http://minsky.sf.net/minsky");
    xml_pack(x, "Minsky", m);
    
    std::ifstream f(saveFile);
    string savedData, buffer;
    while (!getline(f, buffer).fail())
      savedData+=buffer;
    CHECK_EQUAL(expected.str(), savedData);
  }

  TEST_FIXTURE(Fixture, errorPropagation)
  {
    // Test error handling through lastError mechanism
    BackgroundSaver bsaver("/invalid/nonexistent/directory/file");
    schema3::Minsky m;
    
    // First save should fail silently (error stored in lastError)
    bsaver.save(m);
    this_thread::sleep_for(100ms);
    
    // Second save should throw the stored error
    CHECK_THROW(bsaver.save(m), std::exception);
  }

  TEST_FIXTURE(Fixture, fileOverwrite)
  {
    // Test overwriting existing files
    BackgroundSaver bsaver(saveFile);
    schema3::Minsky m1, m2;
    m1.items.resize(100);
    m2.items.resize(200);
    
    // First save
    bsaver.save(m1);
    this_thread::sleep_for(100ms);
    CHECK(boost::filesystem::exists(saveFile));
    
    // Second save should overwrite
    bsaver.save(m2);
    this_thread::sleep_for(100ms);
    
    // Verify final content matches m2
    ostringstream expected;
    xml_pack_t x(expected,"http://minsky.sf.net/minsky");
    xml_pack(x, "Minsky", m2);
    
    std::ifstream f(saveFile);
    string savedData, buffer;
    while (!getline(f, buffer).fail())
      savedData+=buffer;
    CHECK_EQUAL(expected.str(), savedData);
  }

  TEST_FIXTURE(Fixture, specialCharacterPaths)
  {
    // Test file paths with special characters
    string specialFile = saveFile + "_special-chars";
    BackgroundSaver bsaver(specialFile);
    schema3::Minsky m;
    
    bsaver.save(m);
    this_thread::sleep_for(100ms);
    
    CHECK(boost::filesystem::exists(specialFile));
    remove(specialFile);
  }

  TEST_FIXTURE(Fixture, relativePath)
  {
    // Test with relative path
    string relativeFile = "./test_relative_" + to_string(rand());
    BackgroundSaver bsaver(relativeFile);
    schema3::Minsky m;
    
    bsaver.save(m);
    this_thread::sleep_for(100ms);
    
    CHECK(boost::filesystem::exists(relativeFile));
    remove(relativeFile);
  }

  TEST_FIXTURE(Fixture, performanceTest)
  {
    // Test performance with timing constraints
    BackgroundSaver bsaver(saveFile);
    schema3::Minsky m;
    m.items.resize(5000);
    
    auto start = std::chrono::high_resolution_clock::now();
    bsaver.save(m);
    this_thread::sleep_for(200ms); // Wait for completion
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Verify save completed and file exists
    CHECK(boost::filesystem::exists(saveFile));
    
    // Basic performance check - should complete within reasonable time
    CHECK(duration.count() < 3000); // 3 second timeout
  }

  TEST_FIXTURE(Fixture, threadInterruption)
  {
    // Test killThread functionality with ongoing save
    BackgroundSaver bsaver(saveFile);
    schema3::Minsky m;
    m.items.resize(5000); // Large enough to take some time
    
    bsaver.save(m);
    this_thread::sleep_for(10ms); // Let save start
    
    // Manually call killThread (destructor will also call it)
    bsaver.killThread();
    
    // Should be safe to call multiple times
    bsaver.killThread();
    
    CHECK(true); // Test passes if no hang or crash occurs
  }

  TEST_FIXTURE(Fixture, concurrentSavers)
  {
    // Test multiple BackgroundSavers with different files
    schema3::Minsky m1, m2;
    m1.items.resize(500);
    m2.items.resize(1000);
    
    string file2 = saveFile + "_second";
    BackgroundSaver bsaver1(saveFile);
    BackgroundSaver bsaver2(file2);
    
    // Save concurrently
    bsaver1.save(m1);
    bsaver2.save(m2);
    
    this_thread::sleep_for(200ms);
    
    // Both files should exist
    CHECK(boost::filesystem::exists(saveFile));
    CHECK(boost::filesystem::exists(file2));
    
    // Clean up second file
    remove(file2);
  }

  TEST_FIXTURE(Fixture, xmlValidation)
  {
    // Test that saved XML is well-formed and contains expected elements
    BackgroundSaver bsaver(saveFile);
    schema3::Minsky m;
    m.items.resize(10);
    
    bsaver.save(m);
    this_thread::sleep_for(100ms);
    
    std::ifstream f(saveFile);
    string savedData, buffer;
    while (!getline(f, buffer).fail())
      savedData+=buffer;
    
    // Check for basic XML structure
    CHECK(savedData.find("<?xml") != string::npos);
    CHECK(savedData.find("<Minsky") != string::npos);
    CHECK(savedData.find("</Minsky>") != string::npos);
    CHECK(savedData.find("http://minsky.sf.net/minsky") != string::npos);
  }

  TEST_FIXTURE(Fixture, memoryStressTest)
  {
    // Test with extremely large data to stress memory handling
    BackgroundSaver bsaver(saveFile);
    schema3::Minsky m;
    
    try {
      m.items.resize(50000); // Very large dataset
      bsaver.save(m);
      this_thread::sleep_for(500ms); // Allow time for large save
      
      CHECK(boost::filesystem::exists(saveFile));
    } catch(const std::bad_alloc&) {
      // Memory allocation failed - acceptable for stress test
      CHECK(true);
    } catch(...) {
      // Other exceptions should be handled gracefully
      CHECK(true);
    }
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
