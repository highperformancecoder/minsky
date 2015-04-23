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

#ifndef DATABASE_H
#define DATABASE_H
#include <soci/soci.h>
#include "schema/schema1.h"
#include "message.h"

namespace minsky
{
  class Database
  {
    soci::session db;
  public:
    Database(const std::string& conn): db(conn) {}
    Database() {}
    void openDB(const std::string& conn) {db.open(conn);}
    /// creates element (initialised to \a x), returning item ID
    int createElement(int modelId, schema1::Item& x);
    /// reads element from database, using id as key
    unique_ptr<schema1::Item> readElement(int modelId, int id); 
    /// updates database element with \a x
    void updateElement(int modelId, const schema1::Item& x);
    /// removes database element for \a x
    void deleteElement(int modelId, const schema1::Item& x);

   /// NB no create or delete operations provided for Layouts, as the item
    /// versions perform these 

    /// reads layout element from database, using id as key
    auto_ptr<schema1::Layout> readLayout(int modelId, int id);
    /// updates database layout element with \a x
    void updateLayout(int modelId, const schema1::Layout& x);
 
    int createModel(const string& name, const string& owner, schema1::Minsky);
    void readModel(int modelId, schema1::Minsky&);
    void updateModel(int modelId, const schema1::Minsky&);
    void deleteModel(int modelId);

    /// return a list of models available to \a user
    void listModels(const string& user, ModelList& models);

  }; 
}

#endif
