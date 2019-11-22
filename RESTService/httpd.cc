/*
  @copyright Steve Keen 2019
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

/// @file An http server implementation of the REST Service

#include "minskyRS.h"
#include "minsky_epilogue.h"

#include <boost/beast.hpp>
#include <boost/asio.hpp>
namespace beast=boost::beast;
namespace http=beast::http;
namespace net=boost::asio;
using net::ip::tcp;
using namespace std;

namespace minsky
{
  Minsky& minsky() {
    static Minsky m;
    return m;
  }
  // GUI callback needed only to solve linkage problems
  void doOneEvent(bool idleTasksOnly) {}
  // not used, but needed for the linker
  LocalMinsky::LocalMinsky(Minsky& m) {}
  LocalMinsky::~LocalMinsky() {}
}


int main(int argc, const char* argv[])
{
  unsigned short port=80;
  if (argc==1)
    cout << "listening on port "<<port<<endl;
  if (argc>1)
    try
      {
        port=stoi(argv[1]);
      }
    catch (...)
      {
        cerr << "usage:"<<argv[0]<<" <port>"<<endl;
        return 1;
      }

  RESTProcess_t registry;
  RESTProcess(registry,"/minsky",minsky::minsky());

  net::io_context context{1};
  tcp::acceptor acceptor{context, {net::ip::make_address("0.0.0.0"), port}};
  beast::error_code ec;
  beast::flat_buffer buffer;
  
  for (;;)
    {
      tcp::socket socket{context};
      acceptor.accept(socket);
      http::request<http::string_body> req;
      http::read(socket, buffer, req, ec);
      json_pack_t arguments(json_spirit::mValue{});
      try
        {
          switch (req.method())
            {
            case http::verb::head:
              {
                http::response<http::empty_body> response{http::status::ok, req.version()};
                response.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                response.set(http::field::content_type, "text/html");
                auto body=write(registry.process(req.target().to_string(),arguments));
                response.content_length(body.size());
                response.keep_alive(req.keep_alive());
                http::write(socket, response, ec);
              }
              break; 
            case http::verb::put:
              read(req.body(), arguments);
              [[fallthrough]];
            case http::verb::get:
              {
                http::response<http::string_body> res{http::status::ok, req.version()};
                res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(http::field::content_type, "application/json");
                res.body()=write(registry.process(req.target().to_string(),arguments));
                res.keep_alive(req.keep_alive());
                http::write(socket, res, ec);
              }
              break;
            default:
              throw runtime_error("http method "+to_string(req.method()).to_string()+" not supported");
            }
        }
      catch (const std::exception& ex)
        {
          http::response<http::string_body> res{http::status::internal_server_error, req.version()};
          res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
          res.set(http::field::content_type, "text/html");
          res.keep_alive(req.keep_alive());
          res.body() = ex.what();
          http::write(socket, res);
        }
    }
}
