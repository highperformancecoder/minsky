/*
  @copyright Steve Keen 2013
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

#include "server/database.h"
#include <ecolab_epilogue.h>
#include <UnitTest++/UnitTest++.h>
#include <fstream>
#include <string>
#include <stdlib.h>

using namespace minsky;
using namespace std;

namespace
{
  template <class T>
  void checkVectorElements(Database& db, int modelId, const vector<T>& x)
  {
    for (typename vector<T>::const_iterator i=x.begin(); i!=x.end(); ++i)
      {
        unique_ptr<schema1::Item> item=db.readElement(modelId, i->id);
        T* itemT = dynamic_cast<T*>(item.get());
        CHECK(itemT);
        if (itemT)
          {
            // use JSON serialisation to check that returned values = original
            CHECK_EQUAL(i->json(), itemT->json());

            // now create a copy of an element, then read it back to check
            int newId=db.createElement(modelId, *itemT);
            item=db.readElement(modelId, newId);
            CHECK_EQUAL(newId, item->id);
            CHECK(newId != i->id); // should be a distinct id
            itemT = dynamic_cast<T*>(item.get());
            CHECK(itemT);
            if (itemT)
              {
                // id should have mutated, so for comparison, reset it
                itemT->id=i->id;
                CHECK_EQUAL(i->json(), itemT->json());
              }
          }
      }
  }
}

SUITE(Database)
{
  /*
    to use this test, create a file with two lines in it, containing a .mky file, and a database connection string:

    eg (MySQL)
     testEq.mky
     mysql://db=minsky user=??? password=???

    or PostgreSQL
     testEq.mky
     postgresql://dbname=postgres user=??? password=???

     Then set the MINSKY_TEST_DATABASE_PARAMS environment variable to point to this file
  */

  TEST(loadDatabase)
  {
    // read in some parameters for this text
    char* paramFile=getenv("MINSKY_TEST_DATABASE_PARAMS");
    if (!paramFile) 
      throw runtime_error
        ("MINSKY_TEST_DATABASE_PARAMS environment variable not set");

    ifstream paramF(paramFile);
    string modelFile, dbConnection;
    getline(paramF, modelFile);
    getline(paramF, dbConnection);

//    cout << modelFile << endl;
//    cout << dbConnection << endl;

    Database db(dbConnection);
    ModelList ml;
    db.listModels("",ml);
    for (ModelList::iterator i=ml.begin(); i!=ml.end(); ++i)
      cout << i->modelId << " "<<i->name<<endl;

    ifstream inf(modelFile.c_str());
    xml_unpack_t saveFile(inf);
    schema1::Minsky minsky;
    xml_unpack(saveFile, "Minsky", minsky);

    int model=db.createModel(modelFile, "testUser", minsky);

    checkVectorElements(db, model, minsky.model.ports);
    checkVectorElements(db, model, minsky.model.wires);
    checkVectorElements(db, model, minsky.model.operations);
    checkVectorElements(db, model, minsky.model.variables);
    checkVectorElements(db, model, minsky.model.plots);
    checkVectorElements(db, model, minsky.model.groups);
    checkVectorElements(db, model, minsky.model.godleys);
    
  }
}
