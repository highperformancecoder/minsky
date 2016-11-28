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

#ifndef MESSAGE_H
#define MESSAGE_H
#include "json_pack_base.h"
#include "classdesc_access.h"
#include "factory.h"
#include "polyBase.h"

struct MsgType
{
  enum Type {invalid, create, read, update, del, listModels, 
             version, commands, payloads, defaultPayload};
}; 

namespace minsky
{
  using namespace classdesc;

  struct MsgBase: public MsgType
  {
    Type msg;
    int modelId;
    MsgBase(): msg(invalid), modelId(-1) {}
    virtual string json() const=0;
    virtual void json(const string&)=0;
    virtual string typeName() const=0;
    virtual ~MsgBase() {}
    /// returns payload type cast to a PolyBase, or NULL if payload not
    /// derived from PolyBase. Note - ownership not passed
    virtual PolyBase<string>* payloadAsPolyBase()=0;
    virtual const PolyBase<string>* payloadAsPolyBase() const=0;

  };

  template <class Payload>
  struct Msg: public MsgBase
  {
    string payloadClass;
    Payload payload;

    Msg(): payloadClass(classdesc::typeName<Payload>()) {}
    Msg(const MsgBase& m, const Payload& p=Payload()): 
      MsgBase(m), payloadClass(classdesc::typeName<Payload>()),
      payload(p) {}

    string json() const {
      json_pack_t j;
      json_pack(j,"",*this);
      return write(j);
    }

    void json(const string& s) {
      json_pack_t j;
      json_spirit::read(s, j);
      json_unpack(j,"",*this);
    }
    string typeName() const {return payloadClass;}

    // even though technically, Payload could be a poly type, and can
    // directly infer the payloads type by dynamically casting this
    // object. Use MsgPPtr for polymorphic payloads
    PolyBase<string>* payloadAsPolyBase() {return 0;}
    const PolyBase<string>* payloadAsPolyBase() const {return 0;}

  };

  // specialisation for handling polymorphic types
  // Payload must inherit from PolyJsonBase
  template <class Payload>
  struct PayloadPtr: public classdesc::shared_ptr<Payload> 
  {
    PayloadPtr(Payload* x):  shared_ptr<Payload>(x) {}
    PayloadPtr() {}
  };

  template <class Payload>
  struct MsgPPtr: public MsgBase
  {
    string payloadClass;
    PayloadPtr<Payload> payload;
    void setPayload(Payload* p) {
      payload.reset(p);
      payloadClass=static_cast<PolyBase<string>*>(p)->type();
    }

    MsgPPtr(): 
      payloadClass(classdesc::typeName<Payload>()), payload(new Payload) {}
    MsgPPtr(const MsgBase& mb): 
      MsgBase(mb),
      payloadClass(classdesc::typeName<Payload>()), payload(new Payload) {}

    string json() const {
      json_pack_t j;
      json_pack(j,"",*this);
      payload->json_pack(j,".payload");
      return write(j);
    }

    void json(const string& s) {
      json_pack_t j;
      json_spirit::read(s, j);
      json_unpack(j,"",*this);
      payload->json_unpack(j,".payload");
    }
    string typeName() const {return payloadClass;}

    //assumes this class is only used with PolyBase derived types
    PolyBase<string>* payloadAsPolyBase() {return payload.get();}
    const PolyBase<string>* payloadAsPolyBase() const {return payload.get();}

  };

  class MsgFactory: public minsky::Factory<MsgBase, string>
  {
    string suppressSchema(const string&) const;
  public:
    using Factory<MsgBase, string>::registerType;
    MsgFactory();
    // for enumerate... registration
    template <class T> void registerType() {
      Factory<MsgBase, string>::registerType<MsgPPtr<T> >
        (suppressSchema(typeName<T>()));
    }
  };

  extern MsgFactory msgFactory;

  /// return value of the ListModels call
  struct ModelDescriptor
  {
    int modelId;
    string name;
    bool readOnly;
    ModelDescriptor(int modelId, const string& name, bool readOnly):
      modelId(modelId), name(name), readOnly(readOnly) {}
    ModelDescriptor() {}
  };

  /// return value of version call
  struct Version
  {
    int schemaVersion;
    string minskyVersion, ecolabVersion;
  };


  typedef std::vector<ModelDescriptor> ModelList;

}

#ifdef _CLASSDESC
#pragma omit pack minsky::MsgFactory
#pragma omit unpack minsky::MsgFactory
#pragma omit TCL_obj minsky::MsgFactory
#pragma omit json_pack minsky::MsgFactory
#pragma omit json_unpack minsky::MsgFactory
#pragma omit xml_pack minsky::MsgFactory
#pragma omit xml_unpack minsky::MsgFactory
#pragma omit xsd_generate minsky::MsgFactory

#pragma omit json_pack minsky::PayloadPtr
#pragma omit json_unpack minsky::PayloadPtr
#endif

template <class T>
void json_pack(classdesc::json_pack_t&,const classdesc::string&,minsky::PayloadPtr<T>&) {}
template <class T>
void json_unpack(classdesc::json_unpack_t&,const classdesc::string&,minsky::PayloadPtr<T>&) {}

#include "message.xcd"
#include "message.cd"
#endif
