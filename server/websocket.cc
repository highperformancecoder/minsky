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

#include "websocket.h"
#include <websocketpp/websocketpp.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

using namespace websocketpp;
using namespace boost;
using namespace std;

namespace
{
  // dummy type used for unpacking just the message headers
  struct Dummy {};
}

namespace classdesc
{
  // set dummy type to "", meaning that an unspecified payloadClass gets this object
  template <> string typeName<Dummy>() {return "";}
  template <> void json_pack(json_pack_t&, const string&, Dummy&) {}
  template <> void json_unpack(json_pack_t&, const string&, Dummy&) {}
}

#include "ecolab_epilogue.h"

namespace minsky
{
  namespace websocket
  {
    class ClientImpl: private server::handler::connection_ptr
    {
    public:
      ClientImpl(const server::handler::connection_ptr& con): 
        server::handler::connection_ptr(con) {}
      void send(const MsgBase& msg) const {
        (*this)->send(msg.json());
      }
    };

    struct Request
    {
      Websocket::Client client;
      string msgJson;
      Request(ClientImpl* client, const string& msgJson): 
        client(client), msgJson(msgJson) {}
      Request() {}
    };


    // The coordinator is a simple wrapper around an STL queue.
    // add_request inserts a new request. get_request returns the next
    // available request and blocks (using condition variables) in the
    // case that the queue is empty.
    class RequestCoordinator {
    public:
      void add_request(const Request& r) {
        unique_lock<mutex> lock(m_lock);
        m_requests.push(r);
        lock.unlock();
        m_cond.notify_one();
      }
      
      void get_request(Request& value) {
        boost::unique_lock<boost::mutex> lock(m_lock);
        
        while (m_requests.empty()) {
          m_cond.wait(lock);
        }
        
        value = m_requests.front();
        m_requests.pop();
      }
    private:
      queue<Request>         m_requests;
      mutex                m_lock;
      condition_variable   m_cond;
    };

    class Impl: public server::handler
    {
      Websocket& intf; // interface to handle callbacks
      RequestCoordinator req;
      // shared_ptr, because thread is non-copiable
      vector<classdesc::shared_ptr<thread> > threads;
      void on_message(connection_ptr con, message_ptr msg); 
      void process(const Request& r);
      volatile bool running;
      classdesc::shared_ptr<barrier> terminated;
    public:
      Impl(Websocket& intf): intf(intf), running(false) {}
      void start(unsigned workerThreads);
      void threadRoutine();
      ~Impl();
    };


    void Impl::on_message(connection_ptr con,message_ptr msg)
    {
      Request r(new ClientImpl(con), msg->get_payload());
      if (threads.empty())
        process(r);
      else
        req.add_request(r); //enqueue message for processing on a worker thread
    }

    void Impl::process(const Request& r)
      try
        {      
          Msg<Dummy> header;
          header.json(r.msgJson);
          if (header.payloadClass.empty())
            intf.onMessage(r.client, header);
          else
            {
              cout << header.payloadClass << endl;
              auto_ptr<MsgBase> msg(msgFactory.create(header.payloadClass));
              msg->json(r.msgJson);
              intf.onMessage(r.client, *msg);
            }
        }
      catch (std::exception& ex)
        {
          cerr<<ex.what()<<endl;
        }
      catch (...)
        {
          cerr<<"unexpected exception caught"<<endl;
        }
    

    void Impl::threadRoutine()
    {
      while (running)
        {
          Request r;
          req.get_request(r);
          process(r);
        }
    }


    void Impl::start(unsigned workerThreads)
    {
      running=true;
      for (size_t i=0; i<workerThreads; ++i)
        threads.push_back
          (classdesc::shared_ptr<thread>
           (new thread(bind(&Impl::threadRoutine, this))));
    }
    
    Impl::~Impl()
    {
      running=false;
      // unblock all threads
      for (size_t i=0; i<threads.size(); ++i)
        req.add_request(Request());
      for (size_t i=0; i<threads.size(); ++i)
        threads[i]->join(); // potential block if a thread has died
    }
  }

  // delegate to impl
  Websocket::Websocket(): impl(new websocket::Impl(*this)),
                          workerThreads(2), listenerThreads(2), port(80) {}
  void Websocket::start() 
  {
    impl->start(workerThreads);
    server(impl).listen(port, listenerThreads);
  }

  Websocket::Client::Client(websocket::ClientImpl* c): impl(c) {}
  void Websocket::Client::send(const MsgBase& msg) const {impl->send(msg);}  
  string Websocket::Client::username() const {return ""; /*TODO*/}  
}
