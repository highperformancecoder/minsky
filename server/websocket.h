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

#ifndef WEBSOCKET_H
#define WEBSOCKET_H
#include "classdesc.h"
#include "message.h"

#include <boost/shared_ptr.hpp>

namespace minsky
{
  using namespace classdesc;
  namespace websocket
  {
    class Impl;
    class ClientImpl;
  }

  class Websocket
  {
    // pimpl pattern, to encapsulate websocket library
    // note boost version required for websocket::server construction
    boost::shared_ptr<websocket::Impl> impl;
  public:
    /// number of threads handling processing of requests
    unsigned workerThreads;
    /// number of threads listening on socket connections
    unsigned listenerThreads;
    /// port on which to listen for requests
    unsigned short port;
    Websocket();
    /// starts the server, using the above parameters
    void start();

    class Client
    {
      // pimpl pattern, to encapsulate websocket library
      shared_ptr<websocket::ClientImpl> impl;
    public:
      Client(websocket::ClientImpl* c);
      Client() {}
      std::string username() const;
      void send(const MsgBase& msg) const;
    };

    /// clients of this class must override this method to provide
    /// message handling. This method needs to be thread-safe
    virtual void onMessage(const Client& client, const MsgBase& msg)=0;

    virtual ~Websocket() {}
  };
}

#ifdef _CLASSDESC
#pragma omit pack minsky::Websocket
#pragma omit unpack minsky::Websocket
#pragma omit pack minsky::Websocket::Client
#pragma omit unpack minsky::Websocket::Client
#endif

namespace classdesc_access
{
  template <> struct access_pack<minsky::Websocket>:
    public classdesc::NullDescriptor<classdesc::pack_t> {};
  template <> struct access_unpack<minsky::Websocket>:
    public classdesc::NullDescriptor<classdesc::unpack_t> {};
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#include "websocket.cd"
#pragma GCC diagnostic pop
#endif
