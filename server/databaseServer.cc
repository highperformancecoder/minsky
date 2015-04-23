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

#include "ecolab.h"
#include "databaseServer.h"
#include "ecolab_epilogue.h"
#include "minskyVersion.h"
#include "version.h"

namespace minsky
{

  namespace currentSchema=schema1;

//  typedef PayloadPtr<schema1::Item> ElementPtr; 
//  typedef PayloadPtr<schema1::Layout> LayoutPtr;

  DatabaseServer databaseServer;
  make_model(databaseServer);

  void DatabaseServer::onMessage(const Client& client, const MsgBase& msg)
  {
    using namespace currentSchema;
    using currentSchema::Minsky;

    cout << "|"+msg.typeName()+"|" <<endl;
    switch (msg.msg)
      {
      case create:
        if (const Msg<Minsky>* m=
            dynamic_cast<const Msg<Minsky>*>(&msg))
          {
            Msg<Minsky> r(*m);
            r.modelId=db.createModel("",// schema1 does not support model names
                           client.username(), m->payload);
            client.send(r);
            return;
          }
        if (const Item* p=dynamic_cast<const Item*>(msg.payloadAsPolyBase()))
          {
            MsgPPtr<Item> r(msg);
            r.setPayload(p->cloneT<Item>());
            db.createElement(r.modelId, *r.payload);
            client.send(r);
            return;
          }
        break;
      case read:
        // TODO check user is able to do this!
        if (const Msg<Minsky>* m=dynamic_cast<const Msg<Minsky>*>(&msg))
          {
            Msg<Minsky> r;
            db.readModel(m->modelId, r.payload);
            client.send(r);
            return;
          }
        if (const Item* p=
            dynamic_cast<const Item*>(msg.payloadAsPolyBase()))
          {
            MsgPPtr<Item> r(msg);
            r.setPayload(db.readElement(msg.modelId, p->id).release());
            client.send(r);
            return;
          }
        if (const Layout* p=dynamic_cast<const Layout*>(msg.payloadAsPolyBase()))
          {
            MsgPPtr<Layout> r(msg);
            r.setPayload(db.readLayout(r.modelId, r.payload->id).release());
            client.send(r);
            return;
          }
        break;
      case listModels:
        {
          Msg<ModelList> r;
          db.listModels(client.username(), r.payload);
          client.send(r);
          return;
        }
      case version:
        {
          Msg<Version> r(msg);
          r.payload.schemaVersion=currentSchema::Minsky::version;
          r.payload.minskyVersion=MINSKY_VERSION;
          r.payload.ecolabVersion=VERSION;
          client.send(r);
          return;
        }
      case commands:
        {
          Msg<vector<string> > r(msg);
          for (EnumKeys<MsgType::Type>::iterator e=
                 enum_keysData<MsgType::Type>::keys.begin(); 
               e!=enum_keysData<MsgType::Type>::keys.end(); ++e)
            r.payload.push_back(e->second);
          client.send(r);
          return;
        }
      case payloads:
        {
          client.send(Msg<vector<string> >(msg, msgFactory.types()));
          return;
        }
      case defaultPayload:
        //msg already has uninitialised fields filled in with default values,
        //and undefined fields removed, so just send it back
        client.send(msg); 
        return;
      default: break;
      }
  }

  void DatabaseServer::load(const string& modelFile)
  {
    ifstream inf(modelFile.c_str());
    xml_unpack_t saveFile(inf);
    schema1::Minsky minsky;
    xml_unpack(saveFile, "Minsky", minsky);

    db.createModel(modelFile, "testUser", minsky);
  }

}

