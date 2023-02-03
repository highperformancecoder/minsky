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
#include <boost/filesystem.hpp>
namespace beast=boost::beast;
namespace http=beast::http;
namespace net=boost::asio;
using net::ip::tcp;
using namespace std;
using namespace classdesc;

namespace minsky
{
  namespace
  {
    Minsky* l_minsky=NULL;
  }

  Minsky& minsky()
  {
    static Minsky s_minsky;
    if (l_minsky)
      return *l_minsky;
    return s_minsky;
  }

  LocalMinsky::LocalMinsky(Minsky& minsky) {l_minsky=&minsky;}
  LocalMinsky::~LocalMinsky() {l_minsky=NULL;}

  // GUI callback needed only to solve linkage problems
  void doOneEvent(bool idleTasksOnly) {}
}

tuple<string,string> splitFirstComponent(const boost::string_view& x)
{
  if (x.empty() || x[0]!='/') return {};
  auto i=find(x.begin()+1, x.end(), '/');
  if (i==x.end())
    return std::make_tuple(string(x),string());
  return std::make_tuple(string{x.begin(),i},string{i,x.end()});
}

static cairo_status_t appendDataToBuffer(void *p, const unsigned char* data, unsigned length)
{
  if (!p) return CAIRO_STATUS_WRITE_ERROR;
  auto& buffer=*static_cast<vector<unsigned char>*>(p);
  try
    {
      buffer.insert(buffer.end(), data, data+length);
      return CAIRO_STATUS_SUCCESS;
    }
  catch (...)
    {
      return CAIRO_STATUS_WRITE_ERROR;
    }
}

int main(int argc, const char* argv[])
  try
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

  // try to find icon resources. Firstly if in source tree
  auto executableDir=boost::filesystem::path(argv[0]).parent_path();
  auto groupIcon=executableDir/".."/"gui-tk"/"icons"/"group.svg";
  if (!exists(groupIcon)) // then look in installation directory
    groupIcon=executableDir/".."/"gui-tk"/"icons"/"group.svg";
  if (exists(groupIcon))
    {
      minsky::minsky().setGroupIconResource(groupIcon.string());
      minsky::minsky().setGodleyIconResource((groupIcon.parent_path()/"bank.svg").string());
    }
  
  RESTProcess_t registry;
  RESTProcess(registry,"/minsky",minsky::minsky());

  net::io_context context{1};
  tcp::acceptor acceptor{context, {net::ip::make_address("0.0.0.0"), port}};
  beast::error_code ec;
  beast::flat_buffer buffer;
  
  for (;;)
    try
    {
      tcp::socket socket{context};
      acceptor.accept(socket);
      http::request<http::string_body> req;
      http::read(socket, buffer, req, ec);

      try
        {
          // handle rendering of named components
          auto components=splitFirstComponent(req.target());
          if (get<0>(components)=="/render")
            {
              auto c=registry.find(get<1>(components));
              if (c!=registry.end())
                {
                  ecolab::CairoSurface* cs=c->second->getObject<minsky::Canvas>();
                  // for future, we can try to get other types
                  // if (!cs) c=c->second->getObject<some other type>();
                  if (cs)
                    {
                      // create an in-memory PNG representation of the object
                      vector<unsigned char> buffer;
                      auto tmp=cs->vectorRender
                        (nullptr, [](const char*,double width,double height)
                                  {return cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height);});
                      auto err=cairo_surface_write_to_png_stream(tmp->surface(), appendDataToBuffer, &buffer);
                      if (err!=CAIRO_STATUS_SUCCESS)
                        throw runtime_error(cairo_status_to_string(err));
                      
                      http::response<http::buffer_body> res{http::status::ok, req.version()};
                      res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                      res.set(http::field::content_type, "image/png");
                      
                      res.body().data=buffer.data();
                      res.body().size=buffer.size();
                      res.body().more=false;
                      res.keep_alive(req.keep_alive());
                      http::write(socket, res, ec);
                      continue;
                    }
                }
            }
      
          json_pack_t arguments(json5_parser::mValue{});
          switch (req.method())
            {
            case http::verb::head:
              {
                http::response<http::empty_body> response{http::status::ok, req.version()};
                response.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                response.set(http::field::content_type, "text/html");
                auto body=write(registry.process(string(req.target()),arguments));
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
                res.set(http::field::access_control_allow_origin, "*");
                auto cmd=string(req.target());
                res.body()=write(registry.process(cmd,arguments));
                res.keep_alive(req.keep_alive());
                http::write(socket, res, ec);
                int nargs=arguments.type()==json5_parser::array_type? arguments.get_array().size(): 1;
                cmd.erase(0,1); // remove leading '/'
                replace(cmd.begin(), cmd.end(), '/', '.');
                minsky::minsky().commandHook(cmd,nargs);
              }
              break;
            default:
              throw runtime_error("http method "+string(to_string(req.method()))+" not supported");
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
          cout<<ex.what()<<endl;
        }
      catch (...) {cerr << "Unknown exception thrown"<<endl;}
      // redraw all surfaces that have requested it
      for (auto i: minsky::minsky().nativeWindowsToRedraw)
        i->draw();
      minsky::minsky().nativeWindowsToRedraw.clear();
    }
    catch (const std::exception& ex) {cerr << "Exception thrown in draw(): "<<ex.what()<<endl;}
    catch (...) {cerr << "Unknown exception thrown in draw()"<<endl;}
}
  catch (const std::exception& ex) {cerr << "Exception thrown in main(): "<<ex.what()<<endl; return 1;}
  catch (...) {cerr << "Unknown exception thrown in main()"<<endl; return 1;}

