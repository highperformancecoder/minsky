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

#ifndef DATABASESERVER_H
#define DATABASESERVER_H
#include "TCL_obj_base.h"
#include "database.h"
#include "websocket.h"

namespace minsky
{

  class DatabaseServer: public Websocket, public MsgType
  {
  public:
    Exclude<Database> db;
    void openDb(const std::string& conn) {db.openDB(conn);}
    void onMessage(const Client& client, const MsgBase& msg);
    /// load model given by \a filename into the database
    void load(const string& filename);
  };

}

#include "databaseServer.cd"
#endif
